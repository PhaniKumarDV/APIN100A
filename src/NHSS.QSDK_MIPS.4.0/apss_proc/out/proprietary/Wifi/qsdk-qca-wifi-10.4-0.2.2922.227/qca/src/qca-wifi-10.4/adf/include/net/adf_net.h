/*
 * Copyright (c) 2010, Atheros Communications Inc. 
 * All Rights Reserved.
 * 
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * 
 */

/**
 * @defgroup adf_net_public network abstraction API
 */

/**
 * @ingroup adf_net_public
 * @file adf_net.h
 * These APIs abstract the OS networking stack from a driver.
 */

/**
 * @mainpage 
 * @section Introduction
 * The Atheros Driver Framework provides a mechanism to run the Atheros 
 * WLAN driver on a variety of Operating Systems and Platforms. It achieves
 * this by abstracting all OS-specific and platform-specific functionality
 * the driver requires. This ensures the core logic in the driver is OS-
 * and platform-independent.
 * @section Modules
 * The driver framework consists of three main components:
 * @subsection sec1 Network Stack
 * This component abstracts the OS network stack. See @ref adf_net_public for details.
 * @subsection sec2 Network Buffer
 * This component abstracts the OS network buffer. See @ref adf_nbuf_public for details.
 * @subsection sec3 OS services
 * This component abstracts any OS services. See @ref adf_os_public for details.
 */ 

#ifndef _ADF_NET_H
#define _ADF_NET_H

#include <adf_os_types.h>
#include <adf_nbuf.h>
#include "adf_net_types.h"
#include "adf_net_wcmd.h"
#include <adf_net_sw.h>
#include <adf_net_pvt.h>

/*
 * check for a NULL handle
 * */
#define ADF_NET_NULL        __ADF_NET_NULL 

/**
 * @brief this register the driver to the shim, but won't get
 *        any handle until create device is called.
 * 
 * @param[in] drv driver info structure
 * 
 * @return status of operation
 */
static inline a_status_t 
adf_net_register_drv(adf_drv_info_t *drv)
{
    return(__adf_net_register_drv(drv));
}


/**
 * @brief deregister the driver from the shim
 * 
 * @param[in] name driver name passed in adf_drv_info_t
 *
 * @see adf_net_register_drv()
 */
static inline void
adf_net_unregister_drv(a_uint8_t *drv_name)
{
    __adf_net_unregister_drv(drv_name);
}

/**
 * @brief register a wifi device with the kernel
 * 
 * @param[in] hdl       driver handle for this device
 * @param[in] op        per-device switch structure
 * @param[in] info      basic device information
 * @param[in] wifi_cfg  Wifi specific Configuration API(s)
 * 
 * @return opaque device handle
 */
static inline adf_net_handle_t 
adf_net_create_wifidev(adf_drv_handle_t     hdl, adf_dev_sw_t       *op, 
                       adf_net_dev_info_t  *info, void              *wifi_cfg)
{
    return (__adf_net_create_wifidev(hdl, op, info, wifi_cfg));
}

/**
 * @brief register a vap device with the kernel
 * 
 * @param[in] hdl       driver handle for this device
 * @param[in] op        per-device switch structure
 * @param[in] info      basic device information
 * @param[in] vap_cfg   VAP specific Configuration API(s)
 * 
 * @return opaque device handle
 */
static inline adf_net_handle_t 
adf_net_create_vapdev(adf_drv_handle_t      hdl, adf_dev_sw_t       *op, 
                      adf_net_dev_info_t   *info, void              *vap_cfg) 
{
    return (__adf_net_create_vapdev(hdl, op, info, vap_cfg));
}

/**
 * @brief register a ethernet device with the kernel
 * 
 * @param[in] hdl driver handle for this device
 * @param[in] op per-device switch structure
 * @param[in] info basic device information
 * 
 * @return opaque device handle
 */
static inline adf_net_handle_t 
adf_net_create_ethdev(adf_drv_handle_t      hdl, adf_dev_sw_t       *op, 
                      adf_net_dev_info_t   *info)
{
    return (__adf_net_create_ethdev(hdl, op, info));
}
/**
 * @brief unregister a real device with the kernel
 * 
 * @param[in] hdl opaque device handle returned by adf_net_create_vapdev()
 * @see adf_net_create_vapdev()
 */
static inline void
adf_net_delete_dev(adf_net_handle_t hdl)
{
    __adf_net_delete_dev(hdl);
}

/**
 * @brief open the real device
 * 
 * @param[in] hdl opaque device handle
 * 
 * @return status of the operation
 *
 * @see adf_net_create_vapdev()
 */
static inline a_status_t
adf_net_dev_open(adf_net_handle_t hdl)
{
        return (__adf_net_dev_open(hdl));
}


/**
 * @brief close the real device
 * 
 * @param[in] hdl opaque device handle
 * 
 * @see adf_net_dev_open()
 */
static inline void
adf_net_dev_close(adf_net_handle_t hdl)
{
    __adf_net_dev_close(hdl);
}


/**
 * @brief transmit a network buffer using a device
 * 
 * @param[in] hdl opaque device handle
 * @param[in] pkt network buffer to transmit
 * 
 * @return status of the operation
 */
static inline a_status_t 
adf_net_dev_tx(adf_net_handle_t hdl, adf_nbuf_t pkt)
{
       return (__adf_net_dev_tx(hdl,pkt));
}


/**
 * @brief Checks if the interface is running or not
 * 
 * @param[in] hdl opaque device handle
 * 
 * @return true if running, false if not
 */
static inline a_bool_t
adf_net_is_running(adf_net_handle_t hdl)
{
    return (__adf_net_is_running(hdl));
}

/**
 * @brief Checks if the interface is up or not
 * 
 * @param[in] hdl opaque device handle
 * 
 * @return true if up, false if not
 */
static inline a_bool_t
adf_net_is_up(adf_net_handle_t hdl)
{
    return (__adf_net_is_up(hdl));
}
/**
 * @brief check if promiscuous mode is set or not
 * 
 * @param hdl
 * 
 * @return a_bool_t
 */
static inline a_bool_t
adf_net_is_promisc(adf_net_handle_t hdl)
{
    return __adf_net_is_promisc(hdl);
}
/**
 * @brief check if the multicast filter is set to accept all
 *        multicast addresses
 * @param hdl
 * 
 * @return a_bool_t
 */
static inline a_bool_t
adf_net_is_allmulti(adf_net_handle_t  hdl)
{
    return __adf_net_is_allmulti(hdl);
}

/**
 * @brief check whether the carrier is available or not
 * 
 * @param[in] hdl opaque device handle
 * 
 * @return a_bool_t true if available, false if not
 */
static inline a_bool_t 
adf_net_carrier_ok(adf_net_handle_t hdl)
{
    return(__adf_net_carrier_ok(hdl));
}


/**
 * @brief inform the networking stack that the link is down
 * 
 * @param[in] hdl opaque device handle
 */
static inline void 
adf_net_carrier_off(adf_net_handle_t hdl)
{
    __adf_net_carrier_off(hdl);
}


/**
 * @brief inform the networking stack that the link is up
 * 
 * @param[in] hdl opaque device handle
 * 
 * @see adf_net_carrier_off()
 */
static inline void 
adf_net_carrier_on(adf_net_handle_t hdl)
{
    __adf_net_carrier_on(hdl);
}


/*
 * Queue mgmt.
 * driver will use these to keep the native networking stack abreast of its
 * resource (descriptor) situation.
 */

/**
 * @brief inform the networking stack that the device is ready to receive 
 * transmit packets. Typically called during init.
 * 
 * @param[in] hdl opaque device handle
 */
static inline void 
adf_net_start_queue(adf_net_handle_t hdl)
{
    __adf_net_start_queue(hdl);
}

/**
 * @brief inform the networking stack to stop sending transmit packets.
 * Typically called if the driver runs out of resources for the device.
 * 
 * @param[in] hdl opaque device handle
 */
static inline void    
adf_net_stop_queue(adf_net_handle_t hdl)
{
    __adf_net_stop_queue(hdl);
}


/**
 * @brief inform the native stack to resume sending packets
 * to transmit.Typically called when the driver has resources
 * available again for the device. 
 *
 * @note adf_net_wake_queue() is the counterpart of adf_net_stop_queue()
 *
 * @param[in] hdl opaque device handle
 */
static inline void 
adf_net_wake_queue(adf_net_handle_t hdl)
{
    __adf_net_wake_queue(hdl);
}


/**
 * @brief Check the state of the queue
 * 
 * @param[in] hdl opaque device handle
 * 
 * @return true if stopped, false if not
 */
static inline a_bool_t 
adf_net_queue_stopped(adf_net_handle_t hdl)
{
    return(__adf_net_queue_stopped(hdl));
}


/**
 * @brief This indicates a packet to the networking stac (minus
 * the FCS). The driver should just strip the FCS and give the
 * packet as a whole. This is necessary because different native
 * stacks have different expectation of how they want to recv
 * the packet. This fucntion will strip off whatever is
 * required for the OS interface. The routine will also
 * figure out whether its being called in irq context and
 * call the appropriate OS API.
 * 
 * @param[in] hdl opaque device handle
 * @param[in] pkt network buffer to indicate
 * @param[in] len length of buffer
 */
static inline void 
adf_net_indicate_packet(adf_net_handle_t hdl, adf_nbuf_t pkt, a_uint32_t len)
{
    __adf_net_indicate_packet(hdl, pkt, len);
}

/**
 * @brief use this when indicating a vlan tagged packet on RX
 * 
 * @param[in] hdl opaque device handle
 * @param[in] pkt network buffer to indicate
 * @param[in] len length of buffer
 * @param[in] vid vlan id
 * 
 * @return status of operation
 */
static inline a_status_t 
adf_net_indicate_vlanpkt(adf_net_handle_t hdl, adf_nbuf_t pkt, 
                         a_uint32_t len, adf_net_vid_t *vid)
{
    return (__adf_net_indicate_vlanpkt(hdl, pkt, len, vid));
}

/**
 * @brief get interface name
 * 
 * @param[in] hdl opaque device handle
 * 
 * @return name of interface
 */
static inline const a_uint8_t *
adf_net_ifname(adf_net_handle_t  hdl)
{
    return (__adf_net_ifname(hdl));
}

/**
 * @brief send management packets to apps (listener).
 * This is used for wireless applications.
 * 
 * @param[in] hdl opaque device handle
 * @param[in] pkt network buffer to send
 * @param[in] len length of buffer
 */


static inline void
adf_net_fw_mgmt_to_app(adf_net_handle_t hdl, adf_nbuf_t pkt, a_uint32_t len)
{
    __adf_net_fw_mgmt_to_app(hdl, pkt, len);
}
/**
 * @brief send wireless events to listening applications
 * 
 * @param[in] hdl opaque device handle
 * @param[in] what event to send
 * @param[in] data information about event
 * @param[in] data_len length of accompanying information
 */
static inline void
adf_net_send_wireless_event(adf_net_handle_t hdl, 
                            adf_net_wireless_event_t what, 
                            void *data, adf_os_size_t data_len)
{
    __adf_net_send_wireless_event(hdl, what, data, data_len); 
}

/**
 * @brief schedule the poll controller.
 * 
 * @param[in] hdl opaque device handle
 */
static inline void 
adf_net_poll_schedule(adf_net_handle_t hdl)
{
    __adf_net_poll_schedule(hdl);
}


/**
 * @brief per cpu deffered callback (e.g. for RSS)
 * 
 * @param[in] hdl opaque device handle
 * @param[in] cpu_msk
 * @param[in] arg
 */
static inline void 
adf_net_poll_schedule_cpu(adf_net_handle_t hdl, a_uint32_t cpu_msk, void *arg)
{
    __adf_net_poll_schedule_cpu(hdl, cpu_msk, arg);
}

/**
 * @brief Get OS Handle from OS device object.
 *
 * @param[in] osdev OS device object
 * 
 * @return OS handle
 */ 
static inline adf_os_handle_t
adf_net_dev_to_os(adf_os_device_t osdev)
{
    return __adf_net_dev_to_os(osdev);
}
/**
 * @brief Get OS Handle from OS net handle.
 *
 * @param[in] osdev OS net handle
 * 
 * @return OS handle
 */ 
static inline adf_os_handle_t
adf_net_hdl_to_os(adf_net_handle_t hdl)
{
    return __adf_net_hdl_to_os(hdl);
}

/**
 * @brief get the multicast filter list
 * 
 * @param hdl
 * @param mclist
 */
static inline void
adf_net_get_mclist(adf_net_handle_t  hdl, adf_net_devaddr_t *mclist)
{
    __adf_net_get_mclist(hdl, mclist);
}

#define adf_net_indicate_app adf_net_fw_mgmt_to_app
/**
 * @brief check if the multicast is supported or not
 * @param hdl   Opaque netdevice handle
 * 
 * @return true if multi, false if not
 */
static inline a_bool_t
adf_net_is_multi(adf_net_handle_t  hdl)
{
    return (__adf_net_is_multi(hdl));
}


/** 
 * @brief Register WPS PBC callback
 * 
 * @param ctx[in] Context
 * @param cb[in] Callback
 * 
 * @return 
 */
static inline a_status_t
adf_net_register_wpscb(adf_net_handle_t radiodev, void * ctx, adf_net_wps_cb_t cb) 
{
   return  __adf_net_register_wpscb(radiodev, ctx, cb) ;
}


/** 
 * @brief Unregister WPS PBC callback
 * 
 * @param ctx[in]
 * @param cb[in]
 * 
 * @return 
 */
static inline a_status_t
adf_net_unregister_wpscb(adf_net_handle_t radiodev) 
{
   return  __adf_net_unregister_wpscb(radiodev) ;
}

/** 
 * @brief Retrieve the VLAN tag embedded in the packet
 *
 * @param[in]  pkt      The tagged packet
 * @param[out] hdr      The VLAN tag
 * 
 * @return true if the packet has VLAN tag, false if not
 */
static inline a_uint32_t
adf_net_get_vlantag(adf_nbuf_t pkt )
{
        return __adf_net_get_vlantag(pkt );
}

static inline a_uint32_t
adf_net_get_vlanvid( a_uint32_t tag )
{
        return __adf_net_get_vlanvid( tag );
}

/** 
 * @brief Indicate a ACFG_EVENT to the higher layer listeners
 * 
 * @param hdl
 * @param event
 * 
 * @return 
 */
static inline a_status_t
adf_net_indicate_event(adf_net_handle_t hdl, void     *event)
{
    return __adf_net_indicate_event(hdl, event);
}

static inline a_status_t
adf_send_custom_wireless_event(adf_net_handle_t hdl, void     *event)
{
    return __adf_send_custom_wireless_event(hdl, event);
}
/**
 * @brief get new wlan unit
 *
 * @param[in] none
 *
 * @return free wlan unit index
 */
static adf_os_inline int
adf_net_new_wlanunit(void)
{
    return (__adf_net_new_wlanunit());
}

/**
 * @brief alloc/mark  wlan unit passed by user into wlan bit map array
 *
 * @param[in] wlan unit index
 *
 * @return free wlan unit index
 */
static adf_os_inline int
adf_net_alloc_wlanunit(u_int unit)
{
    return (__adf_net_alloc_wlanunit(unit));
}

/**
 * @brief free passed wlan unit
 *
 * @param[in] wlan unit index
 *
 * @return none
 */
static adf_os_inline void
adf_net_delete_wlanunit(u_int unit)
{
    __adf_net_delete_wlanunit( unit );
}

/**
 * @brief extract wlan unit index from the if name
 *
 * @param[in] ifname, wlan unit index
 *
 * @return success/Failure
 */
static adf_os_inline int
adf_net_ifc_name2unit(const char *name, int *unit)
{
    return (__adf_net_ifc_name2unit(name, unit));
}

/**
 * @brief extract the wlan unit index from netdev handle passed and free it in adf bitmap
 *
 * @param[in] nethandle of vap
 *
 * @return none
 */
static adf_os_inline void
adf_net_free_wlanunit(adf_net_handle_t hdl)
{
    __adf_net_free_wlanunit(hdl);
}


/**
 * @brief check if a wlan device exists by the provided interface name 
 *
 * @param[in] ifname
 *
 * @return success/Failure
 */
static adf_os_inline int
adf_net_dev_exist_by_name(const char *name)
{
    return (__adf_net_dev_exist_by_name(name));
}

#endif
