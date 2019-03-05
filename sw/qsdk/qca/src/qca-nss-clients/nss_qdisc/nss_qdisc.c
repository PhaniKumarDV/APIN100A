/*
 **************************************************************************
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include "nss_qdisc.h"

#include "nss_fifo.h"
#include "nss_codel.h"
#include "nss_tbl.h"
#include "nss_prio.h"
#include "nss_bf.h"
#include "nss_wrr.h"
#include "nss_wfq.h"
#include "nss_htb.h"
#include "nss_blackhole.h"
#include "nss_wred.h"

void *nss_qdisc_ctx;			/* Shaping context for nss_qdisc */

#define NSS_QDISC_COMMAND_TIMEOUT (600*HZ) /* We set 1min to be the command */
					   /* timeout value for messages */

/*
 * Defines related to root hash maintenance
 */
#define NSS_QDISC_ROOT_HASH_SIZE 4
#define NSS_QDISC_ROOT_HASH_MASK (NSS_QDISC_ROOT_HASH_SIZE - 1)

/*
 * nss_qdisc_msg_init()
 *      Initialize the qdisc specific message
 */
static void nss_qdisc_msg_init(struct nss_if_msg *nim, uint16_t if_num, uint32_t msg_type, uint32_t len,
				nss_if_msg_callback_t cb, void *app_data)
{
	nss_cmn_msg_init(&nim->cm, if_num, msg_type, len, (void*)cb, app_data);
}

/*
 * nss_qdisc_get_interface_msg()
 *	Returns the correct message that needs to be sent down to the NSS interface.
 */
static inline int nss_qdisc_get_interface_msg(bool is_bridge, uint32_t msg_type)
{
	/*
	 * We re-assign the message based on whether this is for the I shaper
	 * or the B shaper. The is_bridge flag tells if we are on a bridge interface.
	 */
	if (is_bridge) {
		switch(msg_type) {
		case NSS_QDISC_IF_SHAPER_ASSIGN:
			return NSS_IF_BSHAPER_ASSIGN;
		case NSS_QDISC_IF_SHAPER_UNASSIGN:
			return NSS_IF_BSHAPER_UNASSIGN;
		case NSS_QDISC_IF_SHAPER_CONFIG:
			return NSS_IF_BSHAPER_CONFIG;
		default:
			nss_qdisc_info("%s: Unknown message type for a bridge - type %d", __func__, msg_type);
			return -1;
		}
	} else {
		switch(msg_type) {
		case NSS_QDISC_IF_SHAPER_ASSIGN:
			return NSS_IF_ISHAPER_ASSIGN;
		case NSS_QDISC_IF_SHAPER_UNASSIGN:
			return NSS_IF_ISHAPER_UNASSIGN;
		case NSS_QDISC_IF_SHAPER_CONFIG:
			return NSS_IF_ISHAPER_CONFIG;
		default:
			nss_qdisc_info("%s: Unknown message type for an interface - type %d", __func__, msg_type);
			return -1;
		}
	}
}

/*
 * nss_qdisc_attach_bshaper_callback()
 *	Call back funtion for bridge shaper attach to an interface.
 */
static void nss_qdisc_attach_bshaper_callback(void *app_data, struct nss_if_msg *nim)
{
	struct Qdisc *sch = (struct Qdisc *)app_data;
	struct nss_qdisc *nq = qdisc_priv(sch);

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("%s: B-shaper attach FAILED - response: %d\n", __func__,
				nim->cm.error);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: B-shaper attach SUCCESS\n", __func__);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_attach_bridge()
 *	Attaches a given bridge shaper to a given interface (Different from shaper_assign)
 */
static int nss_qdisc_attach_bshaper(struct Qdisc *sch, uint32_t if_num)
{
	struct nss_if_msg nim;
	struct nss_qdisc *nq = (struct nss_qdisc *)qdisc_priv(sch);
	int32_t state, rc;

	nss_qdisc_info("%s: Attaching B-shaper %u to interface %u\n", __func__,
			nq->shaper_id, if_num);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("%s: qdisc %p (type %d) is not ready: State - %d\n",
				__func__, sch, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Populate the message and send it down
	 */
	nss_qdisc_msg_init(&nim, if_num, NSS_IF_BSHAPER_ASSIGN,
				sizeof(struct nss_if_shaper_assign),
				nss_qdisc_attach_bshaper_callback,
				sch);
	/*
	 * Assign the ID of the Bshaper that needs to be assigned to the interface recognized
	 * by if_num.
	 */
	nim.msg.shaper_assign.shaper_id = nq->shaper_id;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("%s: Failed to send bshaper (id: %u) attach for "
				"interface(if_num: %u)\n", __func__, nq->shaper_id, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("%s: bshaper attach command for %x on interface %u timedout!\n",
					__func__, nq->qos_tag, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("%s: Failed to attach B-shaper %u to interface %u - state: %d\n",
				__func__, nq->shaper_id, if_num, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("%s: Attach of B-shaper %u to interface %u is complete\n",
			__func__, nq->shaper_id, if_num);
	return 0;
}

/*
 * nss_qdisc_detach_bshaper_callback()
 *	Call back function for bridge shaper detach
 */
static void nss_qdisc_detach_bshaper_callback(void *app_data, struct nss_if_msg *nim)
{
	struct Qdisc *sch = (struct Qdisc *)app_data;
	struct nss_qdisc *nq = qdisc_priv(sch);

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("%s: B-shaper detach FAILED - response: %d\n",
				__func__, nim->cm.error);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: B-shaper detach SUCCESS\n", __func__);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_detach_bridge()
 *	Detaches a given bridge shaper from a given interface (different from shaper unassign)
 */
static int nss_qdisc_detach_bshaper(struct Qdisc *sch, uint32_t if_num)
{
	struct nss_if_msg nim;
	struct nss_qdisc *nq = (struct nss_qdisc *)qdisc_priv(sch);
	int32_t state, rc;

	nss_qdisc_info("%s: Detaching B-shaper %u from interface %u\n",
			__func__, nq->shaper_id, if_num);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("%s: qdisc %p (type %d) is not ready: %d\n",
				__func__, sch, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create and send shaper unassign message to the NSS interface
	 */
	nss_qdisc_msg_init(&nim, if_num, NSS_IF_BSHAPER_UNASSIGN,
				sizeof(struct nss_if_shaper_unassign),
				nss_qdisc_detach_bshaper_callback,
				sch);
	nim.msg.shaper_unassign.shaper_id = nq->shaper_id;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("%s: Failed to send B-shaper (id: %u) detach "
			"for interface(if_num: %u)\n", __func__, nq->shaper_id, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("%s: bshaper detach command for %x on interface %u timedout!\n",
					__func__, nq->qos_tag, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("%s: Failed to detach B-shaper %u from interface %u - state %d\n",
				__func__, nq->shaper_id, if_num, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("%s: Detach of B-shaper %u to interface %u is complete.",
			__func__, nq->shaper_id, if_num);

	return 0;
}

/*
 * nss_qdisc_refresh_bshaper_assignment()
 *	Performs assign on unassign of bshaper for interfaces on the bridge.
 */
static int nss_qdisc_refresh_bshaper_assignment(struct Qdisc *br_qdisc,
					enum nss_qdisc_bshaper_tasks task)
{
	struct net_device *dev;
	struct net_device *br_dev = qdisc_dev(br_qdisc);
	struct nss_qdisc *nq;
	struct nss_qdisc_bridge_update br_update;
	int i;

	if ((br_qdisc->parent != TC_H_ROOT) && (br_qdisc->parent != TC_H_UNSPEC)) {
		nss_qdisc_error("%s: Qdisc not root qdisc for the bridge interface: "
				"Handle - %x", __func__, br_qdisc->parent);
		return -1;
	}

	nq = qdisc_priv(br_qdisc);

	/*
	 * Initialize the bridge update srtucture.
	 */
	br_update.port_list_count = 0;
	br_update.unassign_count = 0;

	read_lock(&dev_base_lock);
	dev = first_net_device(&init_net);

	while(dev) {
		struct net_bridge_port *br_port;
		int nss_if_num;

		nss_qdisc_info("%s: Scanning device %s", __func__, dev->name);

		rcu_read_lock();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
		br_port = br_port_get_rcu(dev);
#else
		br_port = rcu_dereference(dev->br_port);
#endif

		if (!br_port || !br_port->br) {
			rcu_read_unlock();
			goto nextdev;
		}

		/*
		 * Dont care if this device is not on the
		 * bridge that is of concern.
		 */
		if (br_port->br->dev != br_dev) {
			rcu_read_unlock();
			goto nextdev;
		}

		rcu_read_unlock();

		/*
		 * If the interface is known to NSS then we will have to shape it.
		 * Irrespective of whether it has an interface qdisc or not.
		 */
		nss_if_num = nss_cmn_get_interface_number(nq->nss_shaping_ctx, dev);
		if (nss_if_num < 0) {
			goto nextdev;
		}

		nss_qdisc_info("%s: Will be linking/unlinking %s to/from bridge %s\n", __func__,
						dev->name, br_dev->name);
		br_update.port_list[br_update.port_list_count++] = nss_if_num;
nextdev:
		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);

	nss_qdisc_info("%s: List count %d\n", __func__, br_update.port_list_count);

	if (task == NSS_QDISC_SCAN_AND_ASSIGN_BSHAPER) {
		/*
		 * Loop through the ports and assign them with B-shapers.
		 */
		for (i = 0; i < br_update.port_list_count; i++) {
			if (nss_qdisc_attach_bshaper(br_qdisc, br_update.port_list[i]) >= 0) {
				nss_qdisc_info("%s: Interface %u added to bridge %s\n",
					__func__, br_update.port_list[i], br_dev->name);
				continue;
			}
			nss_qdisc_error("%s: Unable to attach bshaper with shaper-id: %u, "
				"to interface if_num: %d\n", __func__, nq->shaper_id,
				br_update.port_list[i]);
			br_update.unassign_count = i;
			break;
		}
		nss_qdisc_info("%s: Unassign count %d\n", __func__, br_update.unassign_count);
		if (br_update.unassign_count == 0) {
			return 0;
		}

		/*
		 * In case of a failure, unassign the B-shapers that were assigned above
		 */
		for (i = 0; i < br_update.unassign_count; i++) {
			if (nss_qdisc_detach_bshaper(br_qdisc, br_update.port_list[i]) >= 0) {
				continue;
			}
			nss_qdisc_error("%s: Unable to detach bshaper with shaper-id: %u, "
				"from interface if_num: %d\n", __func__, nq->shaper_id,
				br_update.port_list[i]);
		}

		nss_qdisc_info("%s: Failed to link interfaces to bridge\n", __func__);
		return -1;
	} else if (task == NSS_QDISC_SCAN_AND_UNASSIGN_BSHAPER) {
		/*
		 * Loop through the ports and assign them with B-shapers.
		 */
		for (i = 0; i < br_update.port_list_count; i++) {
			if (nss_qdisc_detach_bshaper(br_qdisc, br_update.port_list[i]) >= 0) {
				nss_qdisc_info("%s: Interface %u removed from bridge %s\n",
					__func__, br_update.port_list[i], br_dev->name);
				continue;
			}
			nss_qdisc_error("%s: Unable to detach bshaper with shaper-id: %u, "
				"from interface if_num: %d\n", __func__, nq->shaper_id,
				br_update.port_list[i]);
		}
	}

	return 0;
}

/*
 * nss_qdisc_root_cleanup_final()
 *	Performs final cleanup of a root shaper node after all other
 *	shaper node cleanup is complete.
 */
static void nss_qdisc_root_cleanup_final(struct nss_qdisc *nq)
{
	nss_qdisc_info("%s: Root qdisc %p (type %d) final cleanup\n", __func__,
				nq->qdisc, nq->type);

	/*
	 * If we are a bridge then we have to unregister for bridge bouncing
	 * AND destroy the virtual interface that provides bridge shaping.
	 */
	if (nq->is_bridge) {
		/*
		 * Unregister for bouncing to the NSS for bridge shaping
		 */
		nss_qdisc_info("%s: Unregister for bridge bouncing: %p\n", __func__,
				nq->bounce_context);
		nss_shaper_unregister_shaper_bounce_bridge(nq->nss_interface_number);

		/*
		 * Unregister the virtual interface we use to act as shaper
		 * for bridge shaping.
		 */
		nss_qdisc_info("%s: Release root bridge virtual interface: %p\n",
				__func__, nq->virt_if_ctx);
	}

	/*
	 * If we are a virual interface other than a bridge then we simply
	 * unregister for interface bouncing and not care about deleting the
	 * interface.
	 */
	if (nq->is_virtual && !nq->is_bridge) {
		/*
		 * Unregister for interface bouncing of packets
		 */
		nss_qdisc_info("%s: Unregister for interface bouncing: %p\n",
				__func__, nq->bounce_context);
		nss_shaper_unregister_shaper_bounce_interface(nq->nss_interface_number);
	}

	/*
	 * Finally unregister for shaping
	 */
	nss_qdisc_info("%s: Unregister for shaping\n", __func__);
	nss_shaper_unregister_shaping(nq->nss_shaping_ctx);

	/*
	 * Now set our final state and wake up the caller
	 */
	atomic_set(&nq->state, nq->pending_final_state);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_cleanup_shaper_unassign_callback()
 *	Invoked on the response to a shaper unassign config command issued
 */
static void nss_qdisc_root_cleanup_shaper_unassign_callback(void *app_data,
							struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("%s: Root qdisc %p (type %d) shaper unsassign FAILED\n", __func__, nq->qdisc, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_UNASSIGN_SHAPER_FAIL);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_root_cleanup_final(nq);
}

/*
 * nss_qdisc_root_cleanup_shaper_unassign()
 *	Issue command to unassign the shaper
 */
static void nss_qdisc_root_cleanup_shaper_unassign(struct nss_qdisc *nq)
{
	struct nss_if_msg nim;
	nss_tx_status_t rc;
	int msg_type;

	nss_qdisc_info("%s: Root qdisc %p (type %d): shaper unassign: %d\n",
			__func__, nq->qdisc, nq->type, nq->shaper_id);

	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_UNASSIGN);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type,
			sizeof(struct nss_if_shaper_unassign),
			nss_qdisc_root_cleanup_shaper_unassign_callback,
			nq);
	nim.msg.shaper_unassign.shaper_id = nq->shaper_id;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc == NSS_TX_SUCCESS) {
		/*
		 * Tx successful, simply return.
		 */
		return;
	}

	nss_qdisc_error("%s: Root qdisc %p (type %d): unassign command send failed: "
		"%d, shaper id: %d\n", __func__, nq->qdisc, nq->type, rc, nq->shaper_id);

	atomic_set(&nq->state, NSS_QDISC_STATE_UNASSIGN_SHAPER_SEND_FAIL);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_cleanup_free_node_callback()
 *	Invoked on the response to freeing a shaper node
 */
static void nss_qdisc_root_cleanup_free_node_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("%s: Root qdisc %p (type %d) free FAILED response "
					"type: %d\n", __func__, nq->qdisc, nq->type,
					nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_FAIL);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: Root qdisc %p (type %d) free SUCCESS - response "
			"type: %d\n", __func__, nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);

	nss_qdisc_root_cleanup_shaper_unassign(nq);
}

/*
 * nss_qdisc_root_cleanup_free_node()
 *	Free the shaper node, issue command to do so.
 */
static void nss_qdisc_root_cleanup_free_node(struct nss_qdisc *nq)
{
	struct nss_if_msg nim;
	nss_tx_status_t rc;
	int msg_type;

	nss_qdisc_info("%s: Root qdisc %p (type %d): freeing shaper node\n",
			__func__, nq->qdisc, nq->type);

	/*
	 * Construct and send the shaper configure message down to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type,
				sizeof(struct nss_if_shaper_configure),
				nss_qdisc_root_cleanup_free_node_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_FREE_SHAPER_NODE;
	nim.msg.shaper_configure.config.msg.free_shaper_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc == NSS_TX_SUCCESS) {
		/*
		 * Tx successful, simply return.
		 */
		return;
	}

	nss_qdisc_error("%s: Qdisc %p (type %d): free command send "
		"failed: %d, qos tag: %x\n", __func__, nq->qdisc, nq->type,
		rc, nq->qos_tag);

	atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_SEND_FAIL);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_init_root_assign_callback()
 *	Invoked on the response to assigning shaper node as root
 */
static void nss_qdisc_root_init_root_assign_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("%s: Root assign FAILED for qdisc %p (type %d), "
			"response type: %d\n", __func__, nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		nq->pending_final_state = NSS_QDISC_STATE_ROOT_SET_FAIL;
		nss_qdisc_root_cleanup_free_node(nq);
		return;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): set as root is done. Response - %d"
			, __func__, nq->qdisc, nq->type, nim->msg.shaper_configure.config.response_type);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_init_alloc_node_callback()
 *	Invoked on the response to creating a shaper node as root
 */
static void nss_qdisc_root_init_alloc_node_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	nss_tx_status_t rc;
	int msg_type;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("%s: Qdisc %p (type %d) root alloc node FAILED "
			"response type: %d\n", __func__, nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);

		nq->pending_final_state = NSS_QDISC_STATE_NODE_ALLOC_FAIL;

		/*
		 * No shaper node created, cleanup from unsassigning the shaper
		 */
		nss_qdisc_root_cleanup_shaper_unassign(nq);
		return;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d), shaper node alloc success: %u\n",
				__func__, nq->qdisc, nq->type, nq->shaper_id);

	/*
	 * Create and send shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type,
				sizeof(struct nss_if_shaper_configure),
				nss_qdisc_root_init_root_assign_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_SET_ROOT;
	nim->msg.shaper_configure.config.msg.set_root_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc == NSS_TX_SUCCESS) {
		return;
	}

	nss_qdisc_warning("%s: Root assign send command failed: %d\n",
			__func__, rc);

	nq->pending_final_state = NSS_QDISC_STATE_ROOT_SET_SEND_FAIL;
	nss_qdisc_root_cleanup_free_node(nq);
}

/*
 * nss_qdisc_root_init_shaper_assign_callback()
 *	Invoked on the response to a shaper assign config command issued
 */
static void nss_qdisc_root_init_shaper_assign_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	nss_tx_status_t rc;
	int msg_type;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("%s: Qdisc %x (type %d): shaper assign failed - phys_if response type: %d\n",
			__func__, nq->qos_tag, nq->type, nim->cm.error);
		/*
		 * Unable to assign a shaper, perform cleanup from final stage
		 */
		nq->pending_final_state = NSS_QDISC_STATE_SHAPER_ASSIGN_FAILED;
		nss_qdisc_root_cleanup_final(nq);
		return;
	}

	if (nim->cm.type != NSS_IF_ISHAPER_ASSIGN && nim->cm.type != NSS_IF_BSHAPER_ASSIGN) {
		nss_qdisc_error("%s: Qdisc %x (type %d): shaper assign callback received garbage: %d\n",
			__func__, nq->qos_tag, nq->type, nim->cm.type);
		return;
	}

	nss_qdisc_info("%s: Qdisc %x (type %d): shaper assign callback received sane message: %d\n",
		__func__, nq->qos_tag, nq->type, nim->cm.type);

	/*
	 * Shaper has been allocated and assigned
	 */
	nq->shaper_id = nim->msg.shaper_assign.new_shaper_id;
	nss_qdisc_info("%s: Qdisc %p (type %d), shaper assigned: %u\n",
				__func__, nq->qdisc, nq->type, nq->shaper_id);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_root_init_alloc_node_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_ALLOC_SHAPER_NODE;
	nim->msg.shaper_configure.config.msg.alloc_shaper_node.node_type = nq->type;
	nim->msg.shaper_configure.config.msg.alloc_shaper_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc == NSS_TX_SUCCESS) {
		return;
	}

	/*
	 * Unable to send alloc node command, cleanup from unassigning the shaper
	 */
	nss_qdisc_warning("%s: Qdisc %p (type %d) create command failed: %d\n",
			__func__, nq->qdisc, nq->type, rc);

	nq->pending_final_state = NSS_QDISC_STATE_NODE_ALLOC_SEND_FAIL;
	nss_qdisc_root_cleanup_shaper_unassign(nq);
}


/*
 * nss_qdisc_child_cleanup_final()
 *	Perform final cleanup of a shaper node after all shaper node
 *	cleanup is complete.
 */
static void nss_qdisc_child_cleanup_final(struct nss_qdisc *nq)
{
	nss_qdisc_info("%s: Final cleanup type %d: %p\n", __func__,
			nq->type, nq->qdisc);

	/*
	 * Finally unregister for shaping
	 */
	nss_qdisc_info("%s: Unregister for shaping\n", __func__);
	nss_shaper_unregister_shaping(nq->nss_shaping_ctx);

	/*
	 * Now set our final state
	 */
	atomic_set(&nq->state, nq->pending_final_state);
	wake_up(&nq->wait_queue);
}


/*
 * nss_qdisc_child_cleanup_free_node_callback()
 *	Invoked on the response to freeing a child shaper node
 */
static void nss_qdisc_child_cleanup_free_node_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("%s: Qdisc %p (type %d qos_tag %x): child free FAILED response type: %d\n",
			__func__, nq->qdisc, nq->type, nq->qos_tag, nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_FAIL);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): child shaper node "
			"free complete\n", __func__, nq->qdisc, nq->type);

	/*
	 * Perform final cleanup
	 */
	nss_qdisc_child_cleanup_final(nq);
}

/*
 * nss_qdisc_child_cleanup_free_node()
 *	Free the child shaper node, issue command to do so.
 */
static void nss_qdisc_child_cleanup_free_node(struct nss_qdisc *nq)
{
	struct nss_if_msg nim;
	nss_tx_status_t rc;
	int msg_type;

	nss_qdisc_info("%s: Qdisc %p (type %d qos_tag %x): free shaper node command\n",
			__func__, nq->qdisc, nq->type, nq->qos_tag);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_child_cleanup_free_node_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_FREE_SHAPER_NODE;
	nim.msg.shaper_configure.config.msg.free_shaper_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc == NSS_TX_SUCCESS) {
		return;
	}

	nss_qdisc_error("%s: Qdisc %p (type %d): child free node command send "
			"failed: %d, qos tag: %x\n", __func__, nq->qdisc, nq->type,
			rc, nq->qos_tag);

	atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_SEND_FAIL);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_child_init_alloc_node_callback()
 *	Invoked on the response to creating a child shaper node
 */
static void nss_qdisc_child_init_alloc_node_callback(void *app_data, struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("%s: Qdisc %p (type %d): child alloc node FAILED, response "
			"type: %d\n", __func__, nq->qdisc, nq->type, nim->msg.shaper_configure.config.response_type);
		/*
		 * Cleanup from final stage
		 */
		nq->pending_final_state = NSS_QDISC_STATE_NODE_ALLOC_FAIL_CHILD;
		nss_qdisc_child_cleanup_final(nq);
		return;
	}

	/*
	 * Shaper node has been allocated
	 */
	nss_qdisc_info("%s: Qdisc %p (type %d): shaper node successfully "
			"created as a child node\n",__func__, nq->qdisc, nq->type);

	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_add_to_tail_protected()
 *	Adds to list while holding the qdisc lock.
 */
static inline void nss_qdisc_add_to_tail_protected(struct sk_buff *skb, struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	/*
	 * Since packets can come back from the NSS at any time (in case of bounce),
	 * enqueue's and dequeue's can cause corruption, if not done within locks.
	 */
	spin_lock_bh(&nq->bounce_protection_lock);

	/*
	 * We do not use the qdisc_enqueue_tail() API here in order
	 * to prevent stats from getting updated by the API.
	 */
	__skb_queue_tail(&sch->q, skb);

	spin_unlock_bh(&nq->bounce_protection_lock);
};

/*
 * nss_qdisc_add_to_tail()
 *	Adds to list without holding any locks.
 */
static inline void nss_qdisc_add_to_tail(struct sk_buff *skb, struct Qdisc *sch)
{
	/*
	 * We do not use the qdisc_enqueue_tail() API here in order
	 * to prevent stats from getting updated by the API.
	 */
	__skb_queue_tail(&sch->q, skb);
};

/*
 * nss_qdisc_remove_from_tail_protected()
 *	Removes from list while holding the qdisc lock.
 */
static inline struct sk_buff *nss_qdisc_remove_from_tail_protected(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	struct sk_buff *skb;

	/*
	 * Since packets can come back from the NSS at any time (in case of bounce),
	 * enqueue's and dequeue's can cause corruption, if not done within locks.
	 */
	spin_lock_bh(&nq->bounce_protection_lock);

	/*
	 * We use __skb_dequeue() to ensure that
	 * stats don't get updated twice.
	 */
	skb = __skb_dequeue(&sch->q);

	spin_unlock_bh(&nq->bounce_protection_lock);

	return skb;
};

/*
 * nss_qdisc_remove_to_tail_protected()
 *	Removes from list without holding any locks.
 */
static inline struct sk_buff *nss_qdisc_remove_from_tail(struct Qdisc *sch)
{
	/*
	 * We use __skb_dequeue() to ensure that
	 * stats don't get updated twice.
	 */
	return __skb_dequeue(&sch->q);
};

/*
 * nss_qdisc_bounce_callback()
 *	Enqueues packets bounced back from NSS firmware.
 */
static void nss_qdisc_bounce_callback(void *app_data, struct sk_buff *skb)
{
	struct Qdisc *sch = (struct Qdisc *)app_data;

	/*
	 * Enqueue the packet for transmit and schedule a dequeue
	 * This enqueue has to be protected in order to avoid corruption.
	 */
	nss_qdisc_add_to_tail_protected(skb, sch);
	__netif_schedule(sch);
}

/*
 * nss_qdisc_replace()
 *	Used to replace old qdisc with a new qdisc.
 */
struct Qdisc *nss_qdisc_replace(struct Qdisc *sch, struct Qdisc *new,
					  struct Qdisc **pold)
{
	/*
	 * The qdisc_replace() API is originally introduced in kernel version 4.6,
	 * however this has been back ported to the 4.4. kernal used in QSDK.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	return qdisc_replace(sch, new, pold);
#else
	struct Qdisc *old;

	sch_tree_lock(sch);
	old = *pold;
	*pold = new;
	if (old != NULL) {
		qdisc_tree_decrease_qlen(old, old->q.qlen);
		qdisc_reset(old);
	}
	sch_tree_unlock(sch);

	return old;
#endif
}

/*
 * nss_qdisc_peek()
 *	Called to peek at the head of an nss qdisc
 */
struct sk_buff *nss_qdisc_peek(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	struct sk_buff *skb;

	if (!nq->is_virtual) {
		skb = skb_peek(&sch->q);
	} else {
		spin_lock_bh(&nq->bounce_protection_lock);
		skb = skb_peek(&sch->q);
		spin_unlock_bh(&nq->bounce_protection_lock);
	}

	return skb;
}

/*
 * nss_qdisc_drop()
 *	Called to drop the packet at the head of queue
 */
unsigned int nss_qdisc_drop(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	unsigned int ret;

	if (!nq->is_virtual) {
		ret = __qdisc_queue_drop_head(sch, &sch->q);
	} else {
		spin_lock_bh(&nq->bounce_protection_lock);
		/*
		 * This function is safe to call within locks
		 */
		ret = __qdisc_queue_drop_head(sch, &sch->q);
		spin_unlock_bh(&nq->bounce_protection_lock);
	}

	return ret;
}

/*
 * nss_qdisc_reset()
 *	Called when a qdisc is reset
 */
void nss_qdisc_reset(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	nss_qdisc_info("%s: Qdisc %p (type %d) resetting\n",
			__func__, sch, nq->type);

	/*
	 * Delete all packets pending in the output queue and reset stats
	 */
	if (!nq->is_virtual) {
		qdisc_reset_queue(sch);
	} else {
		spin_lock_bh(&nq->bounce_protection_lock);
		/*
		 * This function is safe to call within locks
		 */
		qdisc_reset_queue(sch);
		spin_unlock_bh(&nq->bounce_protection_lock);
	}

	nss_qdisc_info("%s: Qdisc %p (type %d) reset complete\n",
			__func__, sch, nq->type);
}

/*
 * nss_qdisc_enqueue()
 *	Generic enqueue call for enqueuing packets into NSS for shaping
 */
int nss_qdisc_enqueue(struct sk_buff *skb, struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	nss_tx_status_t status;

	/*
	 * If we are not the root qdisc then we should not be getting packets!!
	 */
	if (unlikely(!nq->is_root)) {
		nss_qdisc_error("%s: Qdisc %p (type %d): unexpected packet "
			"for child qdisc - skb: %p\n", __func__, sch, nq->type, skb);
		nss_qdisc_add_to_tail(skb, sch);
		__netif_schedule(sch);
		return NET_XMIT_SUCCESS;
	}

	/*
	 * Packet enueued in linux for transmit.
	 *
	 * What we do here depends upon whether we are a bridge or not. If not a
	 * bridge then it depends on if we are a physical or virtual interface
	 * The decision we are trying to reach is whether to bounce a packet to
	 * the NSS to be shaped or not.
	 *
	 * is_bridge		is_virtual	Meaning
	 * ---------------------------------------------------------------------------
	 * false		false		Physical interface in NSS
	 *
	 * Action: Simply allow the packet to be dequeued. The packet will be
	 * shaped by the interface shaper in the NSS by the usual transmit path.
	 *
	 *
	 * false		true		Physical interface in Linux.
	 * 					NSS still responsible for shaping
	 *
	 * Action: Bounce the packet to the NSS virtual interface that represents
	 * this Linux physical interface for INTERFACE shaping. When the packet is
	 * returned from being shaped we allow it to be dequeued for transmit.
	 *
	 * true			n/a		Logical Linux interface.
	 *					Root qdisc created a virtual interface
	 *					to represent it in the NSS for shaping
	 *					purposes.
	 *
	 * Action: Bounce the packet to the NSS virtual interface (for BRIDGE shaping)
	 * the bridge root qdisc created for it. When the packet is returned from being
	 * shaped we allow it to be dequeued for transmit.
	 */

	if (!nq->is_virtual) {
		/*
		 * TX to an NSS physical - the shaping will occur as part of normal
		 * transmit path.
		 */
		nss_qdisc_add_to_tail(skb, sch);
		__netif_schedule(sch);
		return NET_XMIT_SUCCESS;
	}

	if (nq->is_bridge) {
		/*
		 * TX to a bridge, this is to be shaped by the b shaper on the virtual interface created
		 * to represent the bridge interface.
		 */
		status = nss_shaper_bounce_bridge_packet(nq->bounce_context, nq->nss_interface_number, skb);
		if (likely(status == NSS_TX_SUCCESS)) {
			return NET_XMIT_SUCCESS;
		}

		nss_qdisc_trace("%s: Qdisc %p (type %d): failed to bounce for bridge %d, skb: %p\n",
					__func__, sch, nq->type, nq->nss_interface_number, skb);
		goto enqueue_drop;
	}

	/*
	 * TX to a physical Linux (NSS virtual).  Bounce packet to NSS for
	 * interface shaping.
	 */
	status = nss_shaper_bounce_interface_packet(nq->bounce_context,
							nq->nss_interface_number, skb);
	if (likely(status == NSS_TX_SUCCESS)) {
		return NET_XMIT_SUCCESS;
	}

	/*
	 * We failed to bounce the packet for shaping on a virtual interface
	 */
	nss_qdisc_trace("%s: Qdisc %p (type %d): failed to bounce for "
		"interface: %d, skb: %p\n", __func__, sch, nq->type,
		nq->nss_interface_number, skb);

enqueue_drop:
	/*
	 * We were unable to transmit the packet for bridge shaping.
	 * We therefore drop it.
	 */
	kfree_skb(skb);

	spin_lock_bh(&nq->lock);
	sch->qstats.drops++;
	spin_unlock_bh(&nq->lock);

	return NET_XMIT_DROP;
}

/*
 * nss_qdisc_dequeue()
 *	Generic dequeue call for dequeuing bounced packets.
 */
inline struct sk_buff *nss_qdisc_dequeue(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	/*
	 * We use the protected dequeue API if the interface involves bounce.
	 * That is, a bridge or a virtual interface. Else, we use the unprotected
	 * API.
	 */
	if (nq->is_virtual) {
		return nss_qdisc_remove_from_tail_protected(sch);
	} else {
		return nss_qdisc_remove_from_tail(sch);
	}
}

/*
 * nss_qdisc_set_default_callback()
 *	The callback function for a shaper node set default
 */
static void nss_qdisc_set_default_callback(void *app_data,
					struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("%s: Qdisc %p (type %d): shaper node set default FAILED, response type: %d\n",
			__func__, nq->qdisc, nq->type, nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): attach complete\n", __func__, nq->qdisc, nq->type);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_node_set_default()
 *	Configuration function that sets shaper node as default for packet enqueue
 */
int nss_qdisc_set_default(struct nss_qdisc *nq)
{
	int32_t state, rc;
	int msg_type;
	struct nss_if_msg nim;

	nss_qdisc_info("%s: Setting qdisc %p (type %d) as default\n", __func__,
			nq->qdisc, nq->type);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): qdisc state not ready: %d\n", __func__,
				nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create the shaper configure message and send it down to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_set_default_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_SET_DEFAULT;
	nim.msg.shaper_configure.config.msg.set_default_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("%s: Failed to send set default message for "
					"qdisc type %d\n", __func__, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("set_default for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("%s: Qdisc %p (type %d): failed to default "
			"State: %d\n", __func__, nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): shaper node default complete\n",
			__func__, nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_node_attach_callback()
 *	The callback function for a shaper node attach message
 */
static void nss_qdisc_node_attach_callback(void *app_data,
					struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("%s: Qdisc %p (type %d) shaper node attach FAILED - response "
			"type: %d\n", __func__, nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: qdisc type %d: %p, attach complete\n", __func__,
			nq->type, nq->qdisc);

	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_node_attach()
 *	Configuration function that helps attach a child shaper node to a parent.
 */
int nss_qdisc_node_attach(struct nss_qdisc *nq, struct nss_qdisc *nq_child,
			struct nss_if_msg *nim, int32_t attach_type)
{
	int32_t state, rc;
	int msg_type;

	nss_qdisc_info("%s: Qdisc %p (type %d) attaching\n",
			__func__, nq->qdisc, nq->type);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): not ready, state: %d\n",
				__func__, nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create the shaper configure message and send it down to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_node_attach_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = attach_type;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("%s: Failed to send configure message for "
					"qdisc type %d\n", __func__, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("attach for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("%s: Qdisc %p (type %d) failed to attach child "
			"node, State: %d\n", __func__, nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Save the parent node (helps in debugging)
	 */
	spin_lock_bh(&nq_child->lock);
	nq_child->parent = nq;
	spin_unlock_bh(&nq_child->lock);

	nss_qdisc_info("%s: Qdisc %p (type %d): shaper node attach complete\n",
			__func__, nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_node_detach_callback()
 *	The callback function for a shaper node detach message
 */
static void nss_qdisc_node_detach_callback(void *app_data,
					struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("%s: Qdisc %p (type %d): shaper node detach FAILED - response "
			"type: %d\n", __func__, nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): detach complete\n",
			__func__, nq->qdisc, nq->type);

	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_node_detach()
 *	Configuration function that helps detach a child shaper node to a parent.
 */
int nss_qdisc_node_detach(struct nss_qdisc *nq, struct nss_qdisc *nq_child,
	struct nss_if_msg *nim, int32_t detach_type)
{
	int32_t state, rc, msg_type;

	nss_qdisc_info("%s: Qdisc %p (type %d) detaching\n",
			__func__, nq->qdisc, nq->type);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): not ready, state: %d\n",
				__func__, nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_node_detach_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = detach_type;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): Failed to send configure "
					"message.", __func__, nq->qdisc, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("detach for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("%s: Qdisc %p (type %d): failed to detach child node, "
				"State: %d\n", __func__, nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	spin_lock_bh(&nq_child->lock);
	nq_child->parent = NULL;
	spin_unlock_bh(&nq_child->lock);

	nss_qdisc_info("%s: Qdisc %p (type %d): shaper node detach complete\n",
			__func__, nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_configure_callback()
 *	The call back function for a shaper node configure message
 */
static void nss_qdisc_configure_callback(void *app_data,
				struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("%s: Qdisc %p (type %d): shaper node configure FAILED "
			"response type: %d\n", __func__, nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): configuration complete\n",
			__func__, nq->qdisc, nq->type);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_configure()
 *	Configuration function that aids in tuning of queuing parameters.
 */
int nss_qdisc_configure(struct nss_qdisc *nq,
	struct nss_if_msg *nim, int32_t config_type)
{
	int32_t state, rc;
	int msg_type;

	nss_qdisc_info("%s: Qdisc %p (type %d) configuring\n", __func__, nq->qdisc, nq->type);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): not ready for configure, "
				"state : %d\n", __func__, nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_configure_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = config_type;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): Failed to send configure "
			"message\n", __func__, nq->qdisc, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("configure for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("%s: Qdisc %p (type %d): failed to configure shaper "
			"node: State: %d\n", __func__, nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): shaper node configure complete\n",
			__func__, nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_destroy()
 *	Destroys a shaper in NSS, and the sequence is based on the position of
 *	this qdisc (child or root) and the interface to which it is attached to.
 */
void nss_qdisc_destroy(struct nss_qdisc *nq)
{
	int32_t state;
	nss_tx_status_t cmd_status;

	nss_qdisc_info("%s: Qdisc %p (type %d) destroy\n",
			__func__, nq->qdisc, nq->type);


	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): destroy not ready, "
				"state: %d\n", __func__, nq->qdisc, nq->type, state);
		return;
	}

	/*
	 * How we begin to tidy up depends on whether we are root or child
	 */
	nq->pending_final_state = NSS_QDISC_STATE_IDLE;
	if (nq->is_root) {

		/*
		 * If this is root on a bridge interface, then unassign
		 * the bshaper from all the attached interfaces.
		 */
		if (nq->is_bridge) {
			nss_qdisc_info("%s: Qdisc %p (type %d): is root on bridge. Need to "
				"unassign bshapers from its interfaces\n", __func__, nq->qdisc, nq->type);
			nss_qdisc_refresh_bshaper_assignment(nq->qdisc, NSS_QDISC_SCAN_AND_UNASSIGN_BSHAPER);
		}

		/*
		 * Begin by freeing the root shaper node
		 */
		nss_qdisc_root_cleanup_free_node(nq);

	} else {
		nss_qdisc_child_cleanup_free_node(nq);
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) == NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("destroy command for %x timedout!\n", nq->qos_tag);
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_IDLE) {
		nss_qdisc_error("%s: clean up for nss qdisc %x failed with "
					"status %d\n", __func__, nq->qos_tag, state);
	}

	if (nq->destroy_virtual_interface) {
		/*
		 * We are using the sync API here since qdisc operations
		 * in Linux are expected to operate synchronously.
		 */
		cmd_status = nss_virt_if_destroy_sync(nq->virt_if_ctx);
		if (cmd_status != NSS_TX_SUCCESS) {
			nss_qdisc_error("%s: Qdisc %p virtual interface %p destroy failed: %d\n", __func__,
						nq->qdisc, nq->virt_if_ctx, cmd_status);
		}
		nq->virt_if_ctx = NULL;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d): destroy complete\n",
			__func__, nq->qdisc, nq->type);
}

/*
 * nss_qdisc_init()
 *	Initializes a shaper in NSS, based on the position of this qdisc (child or root)
 *	and if its a normal interface or a bridge interface.
 */
int nss_qdisc_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type, uint32_t classid)
{
	struct Qdisc *root;
	u32 parent;
	nss_tx_status_t rc;
	struct net_device *dev;
	int32_t state;
	struct nss_if_msg nim;
	int msg_type;
	nss_tx_status_t cmd_status;

	/*
	 * Initialize locks
	 */
	spin_lock_init(&nq->bounce_protection_lock);
	spin_lock_init(&nq->lock);

	/*
	 * Initialize the wait queue
	 */
	init_waitqueue_head(&nq->wait_queue);

	/*
	 * Record our qdisc and type in the private region for handy use
	 */
	nq->qdisc = sch;
	nq->type = type;

	/*
	 * We dont have to destroy a virtual interface unless
	 * we are the ones who created it. So set it to false
	 * by default.
	 */
	nq->destroy_virtual_interface = false;

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * If we are a class, then classid is used as the qos tag.
	 * Else the qdisc handle will be used as the qos tag.
	 */
	if (classid) {
		nq->qos_tag = classid;
		nq->is_class = true;
	} else {
		nq->qos_tag = (uint32_t)sch->handle;
		nq->is_class = false;
	}

	parent = sch->parent;

	/*
	 * If our parent is TC_H_ROOT and we are not a class, then we are the root qdisc.
	 * Note, classes might have its qdisc as root, however we should not set is_root to
	 * true for classes. This is the reason why we check for classid.
	 */
	if ((sch->parent == TC_H_ROOT) && (!nq->is_class)) {
		nss_qdisc_info("%s: Qdisc %p (type %d) is root\n", __func__, nq->qdisc, nq->type);
		nq->is_root = true;
		root = sch;
	} else {
		nss_qdisc_info("%s: Qdisc %p (type %d) not root\n", __func__, nq->qdisc, nq->type);
		nq->is_root = false;
		root = qdisc_root(sch);
	}

	/*
	 * Get the net device as it will tell us if we are on a bridge,
	 * or on a net device that is represented by a virtual NSS interface (e.g. WIFI)
	 */
	dev = qdisc_dev(sch);
	nss_qdisc_info("%s: Qdisc %p (type %d) init dev: %p\n", __func__, nq->qdisc, nq->type, dev);

	/*
	 * Determine if dev is a bridge or not as this determines if we
	 * interract with an I or B shaper.
	 */
	if (dev->priv_flags & IFF_EBRIDGE) {
		nss_qdisc_info("%s: Qdisc %p (type %d) init qdisc: %p, is bridge\n",
			__func__, nq->qdisc, nq->type, nq->qdisc);
		nq->is_bridge = true;
	} else {
		nss_qdisc_info("%s: Qdisc %p (type %d) init qdisc: %p, not bridge\n",
			__func__, nq->qdisc, nq->type, nq->qdisc);
		nq->is_bridge = false;
	}

	nss_qdisc_info("%s: Qdisc %p (type %d) init root: %p, qos tag: %x, "
		"parent: %x rootid: %s owner: %p\n", __func__, nq->qdisc, nq->type, root,
		nq->qos_tag, parent, root->ops->id, root->ops->owner);

	/*
	 * The root must be of an nss type (unless we are of course going to be root).
	 * This is to prevent mixing NSS qdisc with other types of qdisc.
	 */
	if ((parent != TC_H_ROOT) && (root->ops->owner != THIS_MODULE)) {
		nss_qdisc_warning("%s: NSS qdisc %p (type %d) used along with non-NSS qdiscs,"
			" or the interface is currently down", __func__, nq->qdisc, nq->type);
	}

	/*
	 * Register for NSS shaping
	 */
	nq->nss_shaping_ctx = nss_shaper_register_shaping();
	if (!nq->nss_shaping_ctx) {
		nss_qdisc_error("%s: no shaping context returned for type %d\n",
				__func__, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
		goto init_fail;
	}

	/*
	 * If we are not the root qdisc then we have a simple enough job to do
	 */
	if (!nq->is_root) {
		struct nss_if_msg nim_alloc;
		nss_qdisc_info("%s: Qdisc %p (type %d) initializing non-root qdisc\n",
				__func__, nq->qdisc, nq->type);

		/*
		 * The device we are operational on MUST be recognised as an NSS interface.
		 * NOTE: We do NOT support non-NSS known interfaces in this implementation.
		 * NOTE: This will still work where the dev is registered as virtual, in which case
		 * nss_interface_number shall indicate a virtual NSS interface.
		 */
		nq->nss_interface_number = nss_cmn_get_interface_number(nq->nss_shaping_ctx, dev);
		if (nq->nss_interface_number < 0) {
			nss_qdisc_error("%s: Qdisc %p (type %d) net device unknown to "
				"nss driver %s\n", __func__, nq->qdisc, nq->type, dev->name);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}

		/*
		 * Set the virtual flag
		 */
		nq->is_virtual = nss_cmn_interface_is_virtual(nq->nss_shaping_ctx, nq->nss_interface_number);

		/*
		 * Create a shaper node for requested type.
		 * Essentially all we need to do is create the shaper node.
		 */
		nss_qdisc_info("%s: Qdisc %p (type %d) non-root (child) create\n",
				__func__, nq->qdisc, nq->type);

		/*
		 * Create and send the shaper configure message to the interface
		 */
		msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
		nss_qdisc_msg_init(&nim_alloc, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
					nss_qdisc_child_init_alloc_node_callback,
					nq);
		nim_alloc.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_ALLOC_SHAPER_NODE;
		nim_alloc.msg.shaper_configure.config.msg.alloc_shaper_node.node_type = nq->type;
		nim_alloc.msg.shaper_configure.config.msg.alloc_shaper_node.qos_tag = nq->qos_tag;
		rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim_alloc);

		if (rc != NSS_TX_SUCCESS) {
			nss_qdisc_error("%s: Qdisc %p (type %d) create command "
				"failed: %d\n", __func__, nq->qdisc, nq->type, rc);
			nq->pending_final_state = NSS_QDISC_STATE_CHILD_ALLOC_SEND_FAIL;
			nss_qdisc_child_cleanup_final(nq);
			goto init_fail;
		}

		/*
		 * Wait until init operation is complete.
		 */
		if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
					NSS_QDISC_COMMAND_TIMEOUT)) {
			nss_qdisc_error("init for qdisc %x timedout!\n", nq->qos_tag);
			return -1;
		}

		state = atomic_read(&nq->state);
		nss_qdisc_info("%s: Qdisc %p (type %d): initialised with state: %d\n",
					__func__, nq->qdisc, nq->type, state);

		/*
		 * If state is positive, return success
		 */
		if (state > 0) {
			return 0;
		}

		goto init_fail;
	}

	/*
	 * Root qdisc has a lot of work to do. It is responsible for setting up
	 * the shaper and creating the root and default shaper nodes. Also, when
	 * operating on a bridge, a virtual NSS interface is created to represent
	 * bridge shaping. Further, when operating on a bridge, we monitor for
	 * bridge port changes and assign B shapers to the interfaces of the ports.
	 */
	nss_qdisc_info("%s: init qdisc type %d : %p, ROOT\n", __func__, nq->type, nq->qdisc);

	/*
	 * Detect if we are operating on a bridge or interface
	 */
	if (nq->is_bridge) {
		nss_qdisc_info("%s: Qdisc %p (type %d): initializing root qdisc on "
			"bridge\n", __func__, nq->qdisc, nq->type);

		/*
		 * As we are a root qdisc on this bridge then we have to create a
		 * virtual interface to represent this bridge in the NSS. This will
		 * allow us to bounce packets to the NSS for bridge shaping action.
		 */
		nq->virt_if_ctx = nss_virt_if_create_sync(dev);
		if (!nq->virt_if_ctx) {
			nss_qdisc_error("%s: Qdisc %p (type %d): cannot create virtual "
				"interface\n", __func__, nq->qdisc, nq->type);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}
		nss_qdisc_info("%s: Qdisc %p (type %d): virtual interface registered "
			"in NSS: %p\n", __func__, nq->qdisc, nq->type, nq->virt_if_ctx);

		/*
		 * We are the one who have created the virtual interface, so we
		 * must ensure it is destroyed whenever we are done.
		 */
		nq->destroy_virtual_interface = true;

		/*
		 * Get the virtual interface number, and set the related flags
		 */
		nq->nss_interface_number = nss_virt_if_get_interface_num(nq->virt_if_ctx);
		nq->is_virtual = true;
		nss_qdisc_info("%s: Qdisc %p (type %d) virtual interface number: %d\n",
				__func__, nq->qdisc, nq->type, nq->nss_interface_number);

		/*
		 * The root qdisc will get packets enqueued to it, so it must
		 * register for bridge bouncing as it will be responsible for
		 * bouncing packets to the NSS for bridge shaping.
		 */
		nq->bounce_context = nss_shaper_register_shaper_bounce_bridge(nq->nss_interface_number,
							nss_qdisc_bounce_callback, nq->qdisc, THIS_MODULE);
		if (!nq->bounce_context) {
			nss_qdisc_error("%s: Qdisc %p (type %d): is root but cannot register "
					"for bridge bouncing\n", __func__, nq->qdisc, nq->type);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}

	} else {
		nss_qdisc_info("%s: Qdisc %p (type %d): is interface\n", __func__, nq->qdisc, nq->type);

		/*
		 * The device we are operational on MUST be recognised as an NSS interface.
		 * NOTE: We do NOT support non-NSS known interfaces in this basic implementation.
		 * NOTE: This will still work where the dev is registered as virtual, in which case
		 * nss_interface_number shall indicate a virtual NSS interface.
		 */
		nq->nss_interface_number = nss_cmn_get_interface_number(nq->nss_shaping_ctx, dev);
		if (nq->nss_interface_number < 0) {
			nss_qdisc_error("%s: Qdisc %p (type %d): interface unknown to nss driver %s\n",
					__func__, nq->qdisc, nq->type, dev->name);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}

		/*
		 * Is the interface virtual or not?
		 * NOTE: If this interface is virtual then we have to bounce packets to it for shaping
		 */
		nq->is_virtual = nss_cmn_interface_is_virtual(nq->nss_shaping_ctx, nq->nss_interface_number);
		if (!nq->is_virtual) {
			nss_qdisc_info("%s: Qdisc %p (type %d): interface %u is physical\n",
					__func__, nq->qdisc, nq->type, nq->nss_interface_number);
		} else {
			nss_qdisc_info("%s: Qdisc %p (type %d): interface %u is virtual\n",
					__func__, nq->qdisc, nq->type, nq->nss_interface_number);

			/*
			 * Register for interface bounce shaping.
			 */
			nq->bounce_context = nss_shaper_register_shaper_bounce_interface(nq->nss_interface_number,
								nss_qdisc_bounce_callback, nq->qdisc, THIS_MODULE);
			if (!nq->bounce_context) {
				nss_qdisc_error("%s: Qdisc %p (type %d): is root but failed "
				"to register for interface bouncing\n", __func__, nq->qdisc, nq->type);
				nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
				atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
				goto init_fail;
			}
		}
	}

	/*
	 * We need to issue a command to establish a shaper on the interface.
	 */

	/*
	 * Create and send the shaper assign message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_ASSIGN);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_assign),
				nss_qdisc_root_init_shaper_assign_callback,
				nq);
	nim.msg.shaper_assign.shaper_id = 0;	/* Any free shaper will do */
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_error("%s: shaper assign command failed: %d\n", __func__, rc);
		nq->pending_final_state = NSS_QDISC_STATE_ASSIGN_SHAPER_SEND_FAIL;
		nss_qdisc_root_cleanup_final(nq);
		goto init_fail;
	}

	/*
	 * Wait until init operation is complete.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("init for qdisc %x timedout!\n", nq->qos_tag);
		return -1;
	}

	state = atomic_read(&nq->state);
	nss_qdisc_info("%s: Qdisc %p (type %d): is initialised with state: %d\n",
			__func__, nq->qdisc, nq->type, state);

	if (state > 0) {

		/*
		 * Return if this is not a root qdisc on a bridge interface.
		 */
		if (!nq->is_root || !nq->is_bridge) {
			return 0;
		}

		nss_qdisc_info("%s: This is a bridge interface. Linking bridge ...\n",
				__func__);
		/*
		 * This is a root qdisc added to a bridge interface. Now we go ahead
		 * and add this B-shaper to interfaces known to the NSS
		 */
		if (nss_qdisc_refresh_bshaper_assignment(nq->qdisc, NSS_QDISC_SCAN_AND_ASSIGN_BSHAPER) < 0) {
			nss_qdisc_destroy(nq);
			nss_qdisc_error("%s: bridge linking failed\n", __func__);

			/*
			 * We do not go to init_fail since nss_qdisc_destroy()
			 * will take care of deleting the virtual interface.
			 */
			return -1;
		}
		nss_qdisc_info("%s: Bridge linking complete\n", __func__);
		return 0;
	}

init_fail:

	/*
	 * Destroy any virtual interfaces created by us before returning a failure.
	 */
	if (nq->destroy_virtual_interface) {
		/*
		 * We are using the sync API here since qdisc operations
		 * in Linux are expected to operate synchronously.
		 */
		cmd_status = nss_virt_if_destroy_sync(nq->virt_if_ctx);
		if (cmd_status != NSS_TX_SUCCESS) {
			nss_qdisc_error("%s: Qdisc %p virtual interface %p destroy failed: %d\n", __func__,
						nq->qdisc, nq->virt_if_ctx, cmd_status);
		}
		nq->virt_if_ctx = NULL;
	}

	return -1;
}

/*
 * nss_qdisc_basic_stats_callback()
 *	Invoked after getting basic stats
 */
static void nss_qdisc_basic_stats_callback(void *app_data,
				struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	struct Qdisc *qdisc = nq->qdisc;
	struct gnet_stats_basic_packed *bstats;	/* Basic class statistics */
	struct gnet_stats_queue *qstats;	/* Qstats for use by classes */
	atomic_t *refcnt;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("%s: Qdisc %p (type %d): Receive stats FAILED - "
			"response: type: %d\n", __func__, qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_sub(1, &nq->pending_stat_requests);
		wake_up(&nq->wait_queue);
		return;
	}

	/*
	 * Record latest basic stats
	 */
	nq->basic_stats_latest = nim->msg.shaper_configure.config.msg.shaper_node_basic_stats_get;

	/*
	 * Get the right stats pointers based on whether it is a class
	 * or a qdisc.
	 */
	if (nq->is_class) {
		bstats = &nq->bstats;
		qstats = &nq->qstats;
		refcnt = &nq->refcnt;
	} else {
		bstats = &qdisc->bstats;
		qstats = &qdisc->qstats;
		refcnt = &qdisc->refcnt;
		qdisc->q.qlen = nq->basic_stats_latest.qlen_packets;
	}

	/*
	 * Update qdisc->bstats
	 */
	spin_lock_bh(&nq->lock);
	bstats->bytes += (__u64)nq->basic_stats_latest.delta.dequeued_bytes;
	bstats->packets += nq->basic_stats_latest.delta.dequeued_packets;

	/*
	 * Update qdisc->qstats
	 */
	qstats->backlog = nq->basic_stats_latest.qlen_bytes;

	qstats->drops += (nq->basic_stats_latest.delta.enqueued_packets_dropped +
				nq->basic_stats_latest.delta.dequeued_packets_dropped);

	/*
	 * Update qdisc->qstats
	 */
	qstats->qlen = nq->basic_stats_latest.qlen_packets;
	qstats->requeues = 0;
	qstats->overlimits += nq->basic_stats_latest.delta.queue_overrun;
	spin_unlock_bh(&nq->lock);

	/*
	 * All access to nq fields below do not need lock protection. They
	 * do not get manipulated on different thread contexts.
	 */
	if (atomic_read(refcnt) == 0) {
		atomic_sub(1, &nq->pending_stat_requests);
		wake_up(&nq->wait_queue);
		return;
	}

	/*
	 * Requests for stats again, after 1 sec.
	 */
	nq->stats_get_timer.expires += HZ;
	if (nq->stats_get_timer.expires <= jiffies) {
		nss_qdisc_info("losing time %lu, jiffies = %lu\n",
				nq->stats_get_timer.expires, jiffies);
		nq->stats_get_timer.expires = jiffies + HZ;
	}
	add_timer(&nq->stats_get_timer);
}

/*
 * nss_qdisc_get_stats_timer_callback()
 *	Invoked periodically to get updated stats
 */
static void nss_qdisc_get_stats_timer_callback(unsigned long int data)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)data;
	nss_tx_status_t rc;
	struct nss_if_msg nim;
	int msg_type;

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_basic_stats_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_BASIC_STATS_GET;
	nim.msg.shaper_configure.config.msg.shaper_node_basic_stats_get.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	/*
	 * Check if we failed to send the stats request to NSS.
	 */
	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_info("%s: %p: stats fetch request dropped, causing "
				"delay in stats fetch\n", __func__, nq->qdisc);

		/*
		 * Schedule the timer once again for re-trying. Since this is a
		 * re-try we schedule it 100ms from now, instead of a whole second.
		 */
		nq->stats_get_timer.expires = jiffies + HZ/10;
		add_timer(&nq->stats_get_timer);
	}
}

/*
 * nss_qdisc_start_basic_stats_polling()
 *	Call to initiate the stats polling timer
 */
void nss_qdisc_start_basic_stats_polling(struct nss_qdisc *nq)
{
	init_timer(&nq->stats_get_timer);
	nq->stats_get_timer.function = nss_qdisc_get_stats_timer_callback;
	nq->stats_get_timer.data = (unsigned long)nq;
	nq->stats_get_timer.expires = jiffies + HZ;
	atomic_set(&nq->pending_stat_requests, 1);
	add_timer(&nq->stats_get_timer);
}

/*
 * nss_qdisc_stop_basic_stats_polling()
 *	Call to stop polling of basic stats
 */
void nss_qdisc_stop_basic_stats_polling(struct nss_qdisc *nq)
{
	/*
	 * If the timer was active, then delete timer and return.
	 */
	if (del_timer(&nq->stats_get_timer) > 0) {
		/*
		 * The timer was still active (counting down) when it was deleted.
		 * Therefore we are sure that there are no pending stats request
		 * for which we need to wait for. We can therefore return.
		 */
		return;
	}

	/*
	 * The timer has already fired, which means we have a pending stat response.
	 * We will have to wait until we have received the pending response.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->pending_stat_requests) == 0,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("Stats request command for %x timedout!\n", nq->qos_tag);
	}
}

/*
 * nss_qdisc_gnet_stats_copy_basic()
 *  Wrapper around gnet_stats_copy_basic()
 */
int nss_qdisc_gnet_stats_copy_basic(struct gnet_dump *d,
				struct gnet_stats_basic_packed *b)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 18, 0))
	return gnet_stats_copy_basic(d, b);
#else
	return gnet_stats_copy_basic(d, NULL, b);
#endif
}


/*
 * nss_qdisc_gnet_stats_copy_queue()
 *  Wrapper around gnet_stats_copy_queue()
 */
int nss_qdisc_gnet_stats_copy_queue(struct gnet_dump *d,
					struct gnet_stats_queue *q)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 18, 0))
	return gnet_stats_copy_queue(d, q);
#else
	return gnet_stats_copy_queue(d, NULL, q, q->qlen);
#endif
}

/*
 * nss_qdisc_if_event_cb()
 *	Callback function that is registered to listen to events on net_device.
 */
static int nss_qdisc_if_event_cb(struct notifier_block *unused,
					unsigned long event, void *ptr)
{
	struct net_device *dev;
	struct net_device *br;
	struct Qdisc *br_qdisc;
	int if_num, br_num;

	dev = nss_qdisc_get_dev(ptr);
	if (!dev) {
		nss_qdisc_warning("Received event %lu on NULL interface\n", event);
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_BR_JOIN:
		nss_qdisc_info("Reveived NETDEV_BR_JOIN on interface %s\n",
				dev->name);
	case NETDEV_BR_LEAVE:
		nss_qdisc_info("Reveived NETDEV_BR_LEAVE on interface %s\n",
				dev->name);
		br = nss_qdisc_get_dev_master(dev);
		if_num = nss_cmn_get_interface_number(nss_qdisc_ctx, dev);

		if (br == NULL || !(br->priv_flags & IFF_EBRIDGE)) {
			nss_qdisc_error("Sensed bridge activity on interface %s "
				"that is not on any bridge\n", dev->name);
			break;
		}

		br_num = nss_cmn_get_interface_number(nss_qdisc_ctx, br);
		br_qdisc = br->qdisc;
		/*
		 * TODO: Properly ensure that the interface and bridge are
		 * shaped by us.
		 */
		if (if_num < 0 || br_num < 0) {
			nss_qdisc_info("No action taken since if_num is %d for %s "
					"and br_num is %d for bridge %s\n", if_num,
					dev->name, br_num, br->name);
			break;
		}

		/*
		 * Call attach or detach according as per event type.
		 */
		if (event == NETDEV_BR_JOIN) {
			nss_qdisc_info("Instructing interface %s to attach to bridge(%s) "
					"shaping\n", dev->name, br->name);
			nss_qdisc_attach_bshaper(br_qdisc, if_num);
		} else if (event == NETDEV_BR_LEAVE) {
			nss_qdisc_info("Instructing interface %s to detach from bridge(%s) "
					"shaping\n",dev->name, br->name);
			nss_qdisc_detach_bshaper(br_qdisc, if_num);
		}

		break;
	default:
		nss_qdisc_info("Received NETDEV_DEFAULT on interface %s\n", dev->name);
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block nss_qdisc_device_notifier = {
		.notifier_call = nss_qdisc_if_event_cb };

/*
 * TODO: Get the bridge related code out into nss_qdisc_bridge.c
 * Get the stats into nss_qdisc_stats.c
 *
 */

/* ================== Module registration ================= */

static int __init nss_qdisc_module_init(void)
{
	int ret;
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	nss_qdisc_info("Module initializing");
	nss_qdisc_ctx = nss_shaper_register_shaping();

	ret = register_qdisc(&nss_pfifo_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsspfifo registered");

	ret = register_qdisc(&nss_bfifo_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssbfifo registered");

	ret = register_qdisc(&nss_codel_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsscodel registered");

	ret = register_qdisc(&nss_tbl_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsstbl registered");

	ret = register_qdisc(&nss_prio_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssprio registered");

	ret = register_qdisc(&nss_bf_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssbf registered");

	ret = register_qdisc(&nss_wrr_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsswrr registered");

	ret = register_qdisc(&nss_wfq_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsswfq registered");

	ret = register_qdisc(&nss_htb_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsshtb registered");

	ret = register_qdisc(&nss_blackhole_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssblackhole registered");

	ret = register_qdisc(&nss_red_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssred registered");

	ret = register_qdisc(&nss_wred_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsswred registered");

	ret = register_netdevice_notifier(&nss_qdisc_device_notifier);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nss qdisc device notifiers registered");

	return 0;
}

static void __exit nss_qdisc_module_exit(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	unregister_qdisc(&nss_pfifo_qdisc_ops);
	nss_qdisc_info("nsspfifo unregistered");

	unregister_qdisc(&nss_bfifo_qdisc_ops);
	nss_qdisc_info("nssbfifo unregistered");

	unregister_qdisc(&nss_codel_qdisc_ops);
	nss_qdisc_info("nsscodel unregistered");

	unregister_qdisc(&nss_tbl_qdisc_ops);
	nss_qdisc_info("nsstbl unregistered");

	unregister_qdisc(&nss_prio_qdisc_ops);
	nss_qdisc_info("nssprio unregistered");

	unregister_qdisc(&nss_bf_qdisc_ops);
	nss_qdisc_info("nssbf unregistered\n");

	unregister_qdisc(&nss_wrr_qdisc_ops);
	nss_qdisc_info("nsswrr unregistered\n");

	unregister_qdisc(&nss_wfq_qdisc_ops);
	nss_qdisc_info("nsswfq unregistered\n");

	unregister_qdisc(&nss_htb_qdisc_ops);
	nss_qdisc_info("nsshtb unregistered\n");

	unregister_qdisc(&nss_blackhole_qdisc_ops);
	nss_qdisc_info("nssblackhole unregistered\n");

	unregister_qdisc(&nss_red_qdisc_ops);
	nss_qdisc_info("nssred unregistered\n");

	unregister_qdisc(&nss_wred_qdisc_ops);
	nss_qdisc_info("nsswred unregistered\n");

	unregister_netdevice_notifier(&nss_qdisc_device_notifier);
}

module_init(nss_qdisc_module_init)
module_exit(nss_qdisc_module_exit)

MODULE_LICENSE("Dual BSD/GPL");
