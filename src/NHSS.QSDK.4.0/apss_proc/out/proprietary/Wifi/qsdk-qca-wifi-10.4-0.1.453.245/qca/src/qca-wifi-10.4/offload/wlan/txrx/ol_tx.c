/*
 * Copyright (c) 2011-2016 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <qdf_nbuf.h>         /* qdf_nbuf_t, etc. */
#include <ip_prot.h>

#if WLAN_FEATURE_FASTPATH
#include <hif.h>              /* HIF_DEVICE */
#endif
#include <htt.h>              /* HTT_TX_EXT_TID_MGMT */
#if WLAN_FEATURE_FASTPATH
#include <htt_internal.h>     /* */
#include <htt_types.h>        /* htc_endpoint */
#endif
#include <ol_htt_tx_api.h>    /* htt_tx_desc_tid */
#include <cdp_txrx_cmn.h>      /* ol_txrx_vdev_handle */
#include <ol_txrx_api.h>      /* ol_txrx_vdev_handle */

#include <ol_txrx_internal.h> /* TXRX_ASSERT1 */
#include <ol_txrx_types.h>    /* pdev stats */
#include <ol_tx_desc.h>       /* ol_tx_desc */
#include <ol_tx_send.h>       /* ol_tx_send */
#include <ol_txrx.h>          /* TSO */
#include <ol_rawmode_txrx_api.h>  /* Raw Mode specific definitions */
#include <ol_txrx_peer_find.h> /* peerf_hash_find */
#include <linux/icmp.h>

#if ATH_TX_OVERFLOW_IND
#include <ieee80211_acfg.h>   /* acfg event */
#endif
#if MESH_MODE_SUPPORT
#include <if_meta_hdr.h>
#endif

#if QCA_PARTNER_DIRECTLINK_TX
#define QCA_PARTNER_DIRECTLINK_OL_TX 1
#include "ath_carr_pltfrm.h"
#undef QCA_PARTNER_DIRECTLINK_OL_TX
#endif /* QCA_PARTNER_DIRECTLINK_TX */


#if ATH_DEBUG
#include "osif_private.h"
extern void set_rtscts_enable(osif_dev * osdev);
#define ATH_DEBUG_SET_RTSCTS_ENABLE(_osdev) set_rtscts_enable(_osdev)
#else
#define ATH_DEBUG_SET_RTSCTS_ENABLE(_osdev)
#endif

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
#include <osif_nss_wifiol_if.h>
#include <osif_nss_wifiol_vdev_if.h>
#endif

#define MLD_QUERY       0x82
#define MLD_REPORT      0x83
#define MLD_LEAVE       0x84
#define MLDv2_REPORT    0x8f

#if !PEER_FLOW_CONTROL_FORCED_MODE0
#define TX_OVERFLOW_PRINT_LIMIT 100
#endif

static inline void
ol_wakeup_txq(void *arg, struct ieee80211vap *vap, bool is_last_vap)
{
    osif_dev  *osifp = (osif_dev *)vap->iv_ifp;
    struct net_device *netdev = NULL;

    /*
     * Return if for this vap create or delete is in progress.
     */
    if (unlikely(ieee80211_vap_deleted_is_set(vap) || !(osifp))) {
        return;
    }
    netdev = (struct net_device *)(osifp->netdev);
    if (likely(netdev)) {
        if(netif_queue_stopped(netdev)) {
            netif_wake_queue(netdev);
        }
    }
}

#if PEER_FLOW_CONTROL
struct ol_tx_nbuf_classify {
    uint16_t peer_id;
    uint8_t tid;
    uint8_t is_mcast;
    uint8_t is_eap;
    uint8_t is_arp;
    uint8_t is_tcp;
    uint8_t is_dhcp;
    uint8_t is_igmp;
};

void
pflow_ctl_display_pdev_stats(struct ol_txrx_pdev_t *pdev);

u_int32_t
ol_tx_pflow_enqueue_nawds(void *arg, qdf_nbuf_t nbuf, uint8_t tid, uint8_t nodrop, uint8_t hol);

void
ol_dump_tidq(void *arg);

#if QCA_SUPPORT_RAWMODE_PKT_SIMULATION
struct ol_txrx_ast_entry_t *
ol_tx_rawsim_getastentry(struct ol_txrx_vdev_t *vdev, qdf_nbuf_t nbuf)
{
    u_int8_t ftype = 0;
    u_int8_t *hdr_ptr;
    struct ol_txrx_ast_entry_t *ast_entry;
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    u_int8_t is_mcast = 0;
    struct ether_header *eh = NULL;
    struct ol_tx_me_buf_t *mcbuf;

    ftype = qdf_nbuf_get_ftype(nbuf);

    if(ftype != CB_FTYPE_MCAST2UCAST)
        eh = (struct ether_header *) nbuf->data;
    else{
        mcbuf = (struct ol_tx_me_buf_t *) qdf_nbuf_get_fctx(nbuf);
        eh = (struct ether_header *)&mcbuf->buf[0];
    }
    hdr_ptr = eh->ether_dhost;

    is_mcast = IEEE80211_IS_MULTICAST(hdr_ptr);

    switch (vdev->opmode) {

        case wlan_op_mode_ap:

            if (is_mcast) {
                ast_entry = ol_txrx_ast_find_hash_find(pdev, (u_int8_t *) &vdev->mac_addr, 1);
            } else {
                ast_entry = ol_txrx_ast_find_hash_find(pdev, hdr_ptr, 0);
            }

            if (!ast_entry) {
                goto error;
            }

            break;

        default:
            qdf_print("ol_tx_classify - only AP and STA modes supported \n");
            goto error;
    }

error:
    return NULL;
}
#endif

/*
 * Tx Packet Classification
 * Clone of wbuf_classify from direct attach code
 */
    static inline void
ol_tx_classify(struct ol_txrx_vdev_t *vdev, qdf_nbuf_t nbuf, struct ol_tx_nbuf_classify *nbuf_class)
{
    u_int8_t ftype = 0, tos = 0;
    struct ol_tx_me_buf_t *mcbuf;
    u_int8_t *hdr_ptr, *L3datap;
    struct ol_txrx_ast_entry_t *ast_entry;
    struct ol_txrx_peer_t *peer = NULL;
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ieee80211_frame *wh;
    u_int8_t is_mcast = 0;
    struct ether_header *eh = NULL;
    struct ethervlan_header *evh = NULL;
    u_int16_t   ether_type;
    struct llc *llcHdr;
#if ATH_SUPPORT_DSCP_OVERRIDE
    u_int8_t tos_dscp = 0;
#endif
    struct ieee80211_node *ni = NULL;
    osif_dev  *osdev = (osif_dev *)vdev->osif_vdev;
    struct ieee80211vap *vap = osdev->os_if;
    struct ol_ath_softc_net80211 *scn =
        (struct ol_ath_softc_net80211 *)(pdev->ctrl_pdev);
    int tid_q_map;
    qdf_mem_zero(nbuf_class, sizeof(struct ol_tx_nbuf_classify));

    ftype = qdf_nbuf_get_ftype(nbuf);
    if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
        if(ftype != CB_FTYPE_MCAST2UCAST)
            eh = (struct ether_header *) nbuf->data + vap->mhdr_len;
        else{
            mcbuf = (struct ol_tx_me_buf_t *) qdf_nbuf_get_fctx(nbuf);
            eh = (struct ether_header *)&mcbuf->buf[0];
        }
        hdr_ptr = eh->ether_dhost;
        L3datap = hdr_ptr + sizeof(struct ether_header);
    } else {
        wh = (struct ieee80211_frame *) nbuf->data;
        wh = (struct ieee80211_frame *) ((u_int8_t *)wh + vap->mhdr_len);
        /* Always use the RA for peer lookup,
         * because we always have to queue to corresponding
         * STA bss peer queue */
        hdr_ptr = wh->i_addr1;

        /* @todo To Take care of all 802.11 frame types - QoS, 4-addr etc */
        L3datap = hdr_ptr + sizeof(struct ieee80211_frame);
    }

    is_mcast = IEEE80211_IS_MULTICAST(hdr_ptr);

    switch (vdev->opmode) {

        /* For STA mode, peer id = BSS peer id*/
        case wlan_op_mode_sta:
            if (vap != NULL) {
                ni = ieee80211_ref_node(vap->iv_bss);
            }

            if (ni != NULL) {
                peer = (OL_ATH_NODE_NET80211(ni))->an_txrx_handle;
                ieee80211_free_node(ni);
            }

            if (!peer) {
                TXRX_PRINT(TXRX_PRINT_LEVEL_ERR, "ol_tx_classify STA mode, invalid BSS peer \n");
                goto error;
            }

            nbuf_class->peer_id = peer->peer_ids[0];
            break;

            /* Only AP and STA modes are supported */
        case wlan_op_mode_ap:

            if (is_mcast) {
                ast_entry = ol_txrx_ast_find_hash_find(pdev, (u_int8_t *) &vdev->mac_addr, 1);
            } else {
                ast_entry =	ol_txrx_ast_find_hash_find(pdev, hdr_ptr, 0);
            }

            if (!ast_entry) {
                goto error;
            }

            nbuf_class->peer_id = ast_entry->peer_id;
            break;

        default:
            qdf_print("ol_tx_classify - only AP and STA modes supported \n");
            goto error;

    }


    if (OL_CFG_RAW_TX_LIKELINESS(vdev->tx_encap_type == htt_pkt_type_raw)) {
        struct ieee80211_qosframe *qos_wh = (struct ieee80211_qosframe *) nbuf->data;
        tos = qos_wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS ?
            qos_wh->i_qos[0] & IEEE80211_QOS_TID: 0;
        goto out;
    }

    ether_type = eh->ether_type;
    /*
     * Check if packet is dot3 or eth2 type.
     */

    if (IS_LLC_PRESENT(ether_type)) {
        ether_type = (u_int16_t)*(nbuf->data + 2*ETHER_ADDR_LEN + sizeof(*llcHdr) + vap->mhdr_len);

        /* When tx vlan war adds 8 bytes of LLC before vlan tag */
        if (ether_type == __constant_htons(ETHERTYPE_8021Q)) {
            L3datap = hdr_ptr + sizeof(struct ethervlan_header) + sizeof(*llcHdr);
            ether_type = (u_int16_t)*(nbuf->data + 2*ETHER_ADDR_LEN + sizeof(*llcHdr) + sizeof(struct vlan_hdr) + vap->mhdr_len);
        }
        else {
            L3datap = hdr_ptr + sizeof(struct ether_header) + sizeof(*llcHdr);
        }

    }
    else {
        /* When tx vlan war is not used */
        if (ether_type == __constant_htons(ETHERTYPE_8021Q)) {
            evh = (struct ethervlan_header *) eh;
            ether_type = evh->ether_type;
            L3datap = hdr_ptr + sizeof(struct ethervlan_header);
        }
    }


    /*
     * Find priority from IP TOS DSCP field
     */
    if (ether_type == __constant_htons(ETHERTYPE_IP))
    {
        struct iphdr *ip = (struct iphdr *) L3datap;
        /* @todo check this for IPV6 also */
        if (ip->protocol == IP_PROTO_TCP) {
            nbuf_class->is_tcp = 1;
        }
        else if((scn->igmpmld_override) && (ip->protocol == IP_PROTOCOL_IGMP)) {
            nbuf_class->tid = scn->igmpmld_tid;
            nbuf_class->is_igmp = 1;
            return;
        }
        if (ol_txrx_is_dhcp(nbuf, ip)) {
            /* Only for unicast frames - mcast frame check is there above*/
            if (!is_mcast) {
                tos = OSDEP_EAPOL_TID;  /* send it on VO queue */
            }
            nbuf_class->is_dhcp = 1;
        }
        else {
            /*
             * IP frame: exclude ECN bits 0-1 and map DSCP bits 2-7
             * from TOS byte.
             */
            tid_q_map = ol_tx_get_tid_override_queue_mapping(pdev, nbuf);
            if (tid_q_map >= 0) {
                tos = tid_q_map;
            } else {
                tos = (ip->tos & (~0x3)) >> IP_PRI_SHIFT;
            }
#if ATH_SUPPORT_DSCP_OVERRIDE
            tos_dscp = ip->tos;
#endif
        }
    } else if (ether_type == htons(ETHERTYPE_IPV6)) {
        /* TODO
         * use flowlabel
         */
        unsigned long ver_pri_flowlabel;
        unsigned long pri;
        qdf_net_icmpv6hdr_t *mld;
        qdf_net_ipv6hdr_t *ip6h = (qdf_net_ipv6hdr_t *) L3datap;
        u_int8_t *nexthdr = (u_int8_t *)(ip6h + 1);
        ver_pri_flowlabel = *(unsigned long*) L3datap;
        pri = (ntohl(ver_pri_flowlabel) & IPV6_PRIORITY_MASK) >> IPV6_PRIORITY_SHIFT;
        tid_q_map = ol_tx_get_tid_override_queue_mapping(pdev, nbuf);
        if (tid_q_map >= 0) {
            tos = tid_q_map;
        } else {
            tos = (pri & (~INET_ECN_MASK)) >> IP_PRI_SHIFT;
        }
#if ATH_SUPPORT_DSCP_OVERRIDE
        tos_dscp = pri;
#endif
        if ((scn->igmpmld_override) && ((ip6h->ipv6_nexthdr == IPPROTO_ICMPV6) ||
                    ((ip6h->ipv6_nexthdr == IPPROTO_HOPOPTS) &&
                     (*nexthdr == IPPROTO_ICMPV6)))) {
            if (ip6h->ipv6_nexthdr == IPPROTO_ICMPV6) {
                mld = (qdf_net_icmpv6hdr_t *)nexthdr;
            } else {
                mld = (qdf_net_icmpv6hdr_t *)(nexthdr + 8);
            }
            switch(mld->icmp6_type) {
                case MLD_QUERY:
                case MLD_REPORT:
                case MLD_LEAVE:
                case MLDv2_REPORT:
                    nbuf_class->tid = scn->igmpmld_tid;
                    nbuf_class->is_igmp = 1;
                    return;
            }
        }
    } else if (ether_type == __constant_htons(ETHERTYPE_PAE)) {
        /* mark as EAPOL frame */
        if (!is_mcast) {
            tos = OSDEP_EAPOL_TID;  /* send it on VO queue */;
        }
        nbuf_class->is_eap = 1;
    }
    else if (ether_type == __constant_htons(ETHERTYPE_ARP)) {
        /* Only for unicast frames */
        if (!is_mcast) {
            tos = OSDEP_EAPOL_TID;  /* send ucast arp on VO queue */;
        }
        nbuf_class->is_arp = 1;
    }
#ifdef ATH_SUPPORT_WAPI
    else if (ether_type == __constant_htons(ETHERTYPE_WAI)) {
        /* mark as WAI frmae */
        if (!is_mcast) {
            tos = OSDEP_EAPOL_TID; /* send it on VO queue */
        }
    }
#endif

#define TID_MASK 0x7
out:
    /*
     * Assign all MCAST packets to BE
     */
    if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
        if(is_mcast) {
            tos = 0;
        }
    }
#if ATH_SUPPORT_DSCP_OVERRIDE
    if(vap->iv_override_dscp) {
        nbuf_class->tid = vap->iv_dscp_tid_map[(tos_dscp >> IP_DSCP_SHIFT) & IP_DSCP_MASK];
    } else {
#endif
        nbuf_class->tid = (tos & TID_MASK);
#if ATH_SUPPORT_DSCP_OVERRIDE
    }
#endif
    nbuf_class->is_mcast = is_mcast;
    return;
#undef TID_MASK
error:
    nbuf_class->peer_id = HTT_INVALID_PEER;
    nbuf_class->tid = 0;
    return;
}

/*
 * Tx Packet Classify and return peer id
 */
uint16_t ol_tx_classify_get_peer_id(struct ol_txrx_vdev_t *vdev, qdf_nbuf_t nbuf)
{
    struct ol_tx_nbuf_classify nbuf_class;

    ol_tx_classify(vdev, nbuf, &nbuf_class);

    return nbuf_class.peer_id;
}

static inline uint32_t
ol_tx_pflow_ctrl_flush_queue_map(struct ol_txrx_pdev_t *pdev)
{
    ktime_t modified_ktime;
    u_int32_t paddr_lo = 0;
    unsigned long modified_time_ns = pdev->pmap_rotting_timer_interval * 1000000;
    struct ol_ath_softc_net80211 *scn =
        (struct ol_ath_softc_net80211 *)(pdev->ctrl_pdev);

    hrtimer_cancel(&pdev->qmap_ctxt.htimer);

    modified_ktime = ktime_set(0, modified_time_ns);

    pdev->pmap->seq++;
    OS_SYNC_SINGLE(scn->sc_osdev, pdev->htt_pdev->host_q_mem.pool_paddr,
            pdev->htt_pdev->host_q_mem.size, BUS_DMA_TODEVICE, &paddr_lo);

    if(!hrtimer_callback_running(&pdev->qmap_ctxt.htimer)) {
        hrtimer_start(&pdev->qmap_ctxt.htimer, modified_ktime, HRTIMER_MODE_REL);
    }

    return 0;
}

static inline void
ol_tx_pflow_ctrl_queue_map_update(struct ol_txrx_pdev_t *pdev, struct ol_txrx_peer_t *peer, uint32_t peer_id, uint8_t tid)
{
    uint8_t encoded_byte_count = 0;
    uint32_t byte_count = peer->tidq[tid].byte_count;

    if ((byte_count >> 7) < 64) {
        encoded_byte_count |= (byte_count >> 7);
    } else if ((byte_count >> 10) < 64) {
        encoded_byte_count = 0x40;
        encoded_byte_count |= (byte_count >> 10);
    } else if ((byte_count >> 13) < 64) {
        encoded_byte_count = 0x80;
        encoded_byte_count |= (byte_count >> 13);
    } else if ((byte_count >> 16) < 64) {
        encoded_byte_count = 0xc0;
        encoded_byte_count |= (byte_count >> 16);
    } else {
        encoded_byte_count = 0xFF;
    }

    if (byte_count != 0 && ((encoded_byte_count & 0x3f)) == 0) {
        encoded_byte_count |= 1;
    }

    if (byte_count != 0) {
        QUEUE_MAP_CNT_SET(pdev, peer_id, tid, encoded_byte_count);
    } else {
        QUEUE_MAP_CNT_RESET(pdev, peer_id, tid);
    }

    pdev->pmap_qdepth_flush_count++;

    /* Send Queue Depth Map to Target every FLUSH_THRESHOLD packets , here */
    if ((pdev->pmap_qdepth_flush_count & (pdev->pmap_qdepth_flush_interval - 1)) == 0) {
        ol_tx_pflow_ctrl_flush_queue_map(pdev);
        pdev->pmap_qdepth_flush_count = 0;
    }

    /* Compute average every 0x200 pkts  */
#define TIDQ_AVG_SZ_COMPUTE_INTERVAL 0x1FF
    if((peer->tidq[tid].avg_size_compute_cnt & TIDQ_AVG_SZ_COMPUTE_INTERVAL) == 0) {
        if(qdf_nbuf_queue_len(&peer->tidq[tid].nbufq)) {
            peer->tidq[tid].msdu_avg_size =
                          (peer->tidq[tid].byte_count)/(qdf_nbuf_queue_len(&peer->tidq[tid].nbufq));
        }
    }

}

static inline uint32_t
ol_tx_enqueue_pflow_ctrl(ol_txrx_vdev_handle vdev, qdf_nbuf_t nbuf, uint32_t peer_id, uint8_t tid, uint8_t nodrop, uint8_t hol)
{
    struct ol_txrx_peer_t *peer;
    uint32_t len, current_nbufq_len;
    struct ol_txrx_pdev_t *pdev = vdev->pdev;


    if (peer_id == HTT_INVALID_PEER) {
        TXRX_PRINT(TXRX_PRINT_LEVEL_INFO2, "tx_enqueue - peer not found\n");
        PFLOW_CTRL_PDEV_STATS_ADD(pdev, ENQUEUE_FAIL_CNT, 1);
        return OL_TX_PFLOW_CTRL_ENQ_FAIL;
    }

    peer = ol_txrx_peer_find_by_id(pdev, peer_id);

    if (!peer) {
        TXRX_PRINT(TXRX_PRINT_LEVEL_INFO2, "tx_enqueue invalid peer %d \n", peer_id);
        PFLOW_CTRL_PDEV_STATS_ADD(pdev, ENQUEUE_FAIL_CNT, 1);
        return OL_TX_PFLOW_CTRL_ENQ_FAIL;
    }

    current_nbufq_len = qdf_nbuf_queue_len(&peer->tidq[tid].nbufq);
    /*
     * If nodrop flag is set , only check for global queue len limit
     */
    if ((!nodrop && (current_nbufq_len > pdev->pflow_ctl_queue_max_len[peer_id][tid])) ||
            (pdev->pflow_ctl_global_queue_cnt > pdev->pflow_ctl_max_buf_global)) {

        TXRX_PRINT(TXRX_PRINT_LEVEL_INFO2, "tx_enqueue - peer queue full %d %d\n",
                   peer_id, tid);
        TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_discard, 1);
        PFLOW_CTRL_PDEV_STATS_ADD(pdev, PEER_Q_FULL, 1);
        PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_PEER_Q_FULL, 1);
        return OL_TX_PFLOW_CTRL_Q_FULL;
    } else {
        pdev->pflow_ctl_global_queue_cnt++;

        OL_TX_PEER_LOCK(pdev, peer_id);
        if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
#if HOST_SW_TSO_SG_ENABLE
            uint8_t ftype;
            ftype = qdf_nbuf_get_ftype(nbuf);

            if(ftype == CB_FTYPE_TSO_SG) {
                len = ol_tx_tso_sg_get_nbuf_len(nbuf);
            } else
#endif
            {
                len = qdf_nbuf_len(nbuf);
            }
            if (qdf_unlikely(hol)) {
                qdf_nbuf_queue_add(&peer->tidq[OL_TX_PFLOW_CTRL_PRIORITY_TID].nbufq, nbuf);
                PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_HOL_CNT, 1);
            } else {
                qdf_nbuf_queue_add(&peer->tidq[tid].nbufq, nbuf);
            }
        } else {
            len = ol_tx_rawmode_enque_pflow_ctl_helper(pdev, peer, nbuf, peer_id, tid);
        }

        if(pdev->prof_trigger) {
            __net_timestamp(nbuf);
        }

        QDF_NBUF_CB_TX_SUBMIT_TS(nbuf) = OS_GET_TIMESTAMP();

        peer->tidq[tid].byte_count += len;
        peer->tidq[tid].avg_size_compute_cnt++;
        peer->tidq[tid].enq_cnt++;
        ol_tx_pflow_ctrl_queue_map_update(pdev, peer, peer_id, tid);

        OL_TX_PEER_UNLOCK(pdev, peer_id);
        /*
         * Stats need not be under a lock, as these are just for debug/display
         */
        if(pdev->pflow_ctl_global_queue_cnt > PFLOW_CTRL_PDEV_STATS_GET(pdev, QUEUE_DEPTH)) {
            PFLOW_CTRL_PDEV_STATS_SET(pdev, QUEUE_DEPTH, pdev->pflow_ctl_global_queue_cnt);
        }

        /*"high_watermark" is the largest queue length a queue has seen*/
        if(current_nbufq_len > peer->tidq[tid].high_watermark){
            peer->tidq[tid].high_watermark = current_nbufq_len;
        }
        PFLOW_CTRL_PDEV_STATS_ADD(pdev, ENQUEUE_CNT, 1);
        PFLOW_CTRL_PDEV_STATS_ADD(pdev, ENQUEUE_BYTECNT, len);
        PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_ENQUEUE_CNT, 1);
        PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_ENQUEUE_BYTECNT, len);

        /* Remove this after the PFLOW stats are added to iwpriv */
        TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_enqueue, 1);
    }
    return OL_TX_PFLOW_CTRL_ENQ_DONE;
}

u_int32_t
ol_pflow_update_pdev_params(struct ol_txrx_pdev_t *pdev, ol_ath_param_t param, u_int32_t value, void *buff)
{
    switch(param)
    {

        case OL_ATH_PARAM_STATS_FC:
            {
                qdf_print("------- Global Stats -----------------\n");
                pflow_ctl_display_pdev_stats(pdev);
            }
            break;
        case OL_ATH_PARAM_QFLUSHINTERVAL:
            {
                if(!buff)
                    pdev->pmap_qdepth_flush_interval = value;
                else
                    *(int*)buff = pdev->pmap_qdepth_flush_interval;
            }
            break;
         case OL_ATH_PARAM_TOTAL_Q_SIZE:
            {
                if(!buff) {
                    pdev->pflow_ctl_max_buf_global = value;
                }
                else
                    *(int*)buff = pdev->pflow_ctl_max_buf_global;
            }
            break;
         case OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE0:
         case OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE1:
         case OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE2:
         case OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE3:
            {
                if(!buff) {
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
                    osif_nss_tx_queue_cfg(pdev, (param - OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE0), value);
#endif
                }
                else
                    *(int*)buff = pdev->pflow_ctl_max_buf_global;
            }
            break;
         case OL_ATH_PARAM_MIN_THRESHOLD:
            {
                if(!buff)
                {
                    pdev->pflow_ctl_min_threshold = value;
                    if (value > OL_TX_PFLOW_CTRL_MIN_THRESHOLD) {
                        qdf_print("WARNING: setting the threshold value > %d \
                                will impact multiclient performance \n", OL_TX_PFLOW_CTRL_MIN_THRESHOLD);
                    }
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
                    osif_nss_tx_queue_min_threshold_cfg(pdev, value);
#endif
                }
                else
                    *(int*)buff = pdev->pflow_ctl_min_threshold;
            }
            break;
         case OL_ATH_PARAM_MIN_Q_LIMIT:
            {
                if(!buff)
                    pdev->pflow_ctl_min_queue_len = value;
                else
                    *(int*)buff = pdev->pflow_ctl_min_queue_len;
            }
            break;
         case OL_ATH_PARAM_MAX_Q_LIMIT:
            {
                if(!buff)
                    pdev->pflow_ctl_max_queue_len = value;
                else
                    *(int*)buff = pdev->pflow_ctl_max_queue_len;
            }
            break;
         case OL_ATH_PARAM_CONG_CTRL_TIMER_INTV:
            {
                if(!buff) {
                    if(value != 0) {
                        pdev->pflow_cong_ctrl_timer_interval = value;
                        qdf_timer_mod(&pdev->pflow_ctl_cong_timer, pdev->pflow_cong_ctrl_timer_interval);
                    } else {
                        qdf_timer_stop(&pdev->pflow_ctl_cong_timer);
                    }
                }
                else
                    *(int*)buff = pdev->pflow_cong_ctrl_timer_interval;
            }
            break;
         case OL_ATH_PARAM_STATS_TIMER_INTV:
            {
                if(!buff) {
                    if(value != 0) {
                        pdev->pflow_ctl_stats_timer_interval = value;
                        qdf_timer_mod(&pdev->pflow_ctl_stats_timer, pdev->pflow_ctl_stats_timer_interval);
                    } else {
                        qdf_timer_stop(&pdev->pflow_ctl_stats_timer);
                    }
                }
                else
                    *(int*)buff = pdev->pflow_ctl_stats_timer_interval;
            }
            break;
         case OL_ATH_PARAM_ROTTING_TIMER_INTV:
            {
                if(!buff) {
                    if(value != 0) {
                        pdev->pmap_rotting_timer_interval = value;
                    }
                }
                else
                    *(int*)buff = pdev->pmap_rotting_timer_interval;
            }
            break;
         case OL_ATH_PARAM_LATENCY_PROFILE:
            {
                if(!buff)
                    pdev->prof_trigger = value;
                else
                    *(int*)buff = pdev->prof_trigger;
            }
            break;
         case OL_ATH_PARAM_HOSTQ_DUMP:
            {
                if(!buff)
                    pdev->pflow_hostq_dump = value;
                else
                    *(int*)buff = pdev->pflow_hostq_dump;
            }
            break;
         case OL_ATH_PARAM_TIDQ_MAP:
            {
                if(!buff)
                    pdev->pflow_tidq_map = value;
                else
                    *(int*)buff = pdev->pflow_tidq_map;
                if(pdev->pflow_tidq_map)
                    ol_dump_tidq(pdev);
            }
            break;
         case OL_ATH_PARAM_MSDU_TTL:
            {
                if(!buff) {
                    pdev->pflow_msdu_ttl = value/pdev->pflow_cong_ctrl_timer_interval;
                    if(pdev->pflow_msdu_ttl == 0)
                        pdev->pflow_msdu_ttl = 1;
                    qdf_print("%p: check_msdu_ttl_val %d", pdev, pdev->pflow_msdu_ttl);
                } else {
                    *(int*)buff = pdev->pflow_msdu_ttl;
                }
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
                osif_nss_ol_set_msdu_ttl(pdev);
#endif

            }
            break;
         default:
            qdf_print("%d not handled  in %s \n", param, __func__);
            break;
    }

    return 0;
}

#endif /* PEER_FLOW_CONTROL */

u_int32_t
ol_update_txpow_pdev(struct ol_txrx_pdev_t *pdev, uint32_t value, void *buff)
{
    int frame_subtype = 0;
    u_int8_t transmit_power = 0;
    if(!buff) {
        frame_subtype = (value >> 8); /* The first 8 bits of 'value' contains the frame_subtype */
        transmit_power = (value &  0x00FF); /* The last 8 bits of 'value' contains the transmit power */
        pdev->txpow_mgt_frm[(frame_subtype >> IEEE80211_FC0_SUBTYPE_SHIFT)] = transmit_power;
    } else {
        /* 'value' here contains the frame subtype using which we index the txpow_mgt_frm array */
        *(int*)buff = pdev->txpow_mgt_frm[(value >> IEEE80211_FC0_SUBTYPE_SHIFT)];
    }
    return 0;
}

static inline uint32_t
ol_tx_check_enqueue_flow_ctrl(struct ol_txrx_pdev_t *pdev, qdf_nbuf_t nbuf)
{
    if(!qdf_nbuf_is_queue_empty(&pdev->acnbufq)) {
        OL_TX_FLOW_CTRL_LOCK(&pdev->acnbufqlock);
        if(qdf_unlikely(pdev->acqcnt >= pdev->acqcnt_len)) {
            TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_discard, 1);
            OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
            return OL_TX_FLOW_CTRL_Q_FULL;
        } else {
            pdev->acqcnt++;
            qdf_nbuf_queue_add(&pdev->acnbufq, nbuf);

            TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_enqueue, 1);
            if(qdf_unlikely(pdev->acqcnt > (pdev->acqcnt_len - pdev->vdev_count)))
            {
                OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
                return OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD;
            }
            OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
            return OL_TX_FLOW_CTRL_ENQ_DONE;
        }
    }
    return OL_TX_FLOW_CTRL_Q_EMPTY;
}

static inline uint32_t
ol_tx_enqueue_flow_ctrl(struct ol_txrx_pdev_t *pdev, qdf_nbuf_t nbuf)
{
    OL_TX_FLOW_CTRL_LOCK(&pdev->acnbufqlock);
    if (qdf_unlikely(pdev->acqcnt >= pdev->acqcnt_len)) {
        TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_discard, 1);
        OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
        return OL_TX_FLOW_CTRL_Q_FULL;
    } else {
        pdev->acqcnt++;
        qdf_nbuf_queue_add(&pdev->acnbufq, nbuf);
        TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_enqueue, 1);
        if(qdf_unlikely(pdev->acqcnt > (pdev->acqcnt_len - pdev->vdev_count)))
        {
            OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
            return OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD;
        }
    }
    OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
    return OL_TX_FLOW_CTRL_ENQ_DONE;
}

#if HOST_SW_TSO_ENABLE
/* Failure for a particular sement is freeing all further segments of the Jumbo packet
 * Need to tune it further in stress test if only the corresponding segment needs a drop */
static inline void *
ol_tx_tso_prepare_ll(struct ol_tx_desc_t **tx_descriptor,
		ol_txrx_vdev_handle vdev,
		qdf_nbuf_t msdu)
{
    qdf_nbuf_t parent;
    struct ol_tx_tso_desc_t *sw_tso_desc;
    struct ol_tx_desc_t *tx_desc = *tx_descriptor;
    /*
        The TXRX module doesn't accept tx frames unless the target has
        enough descriptors for them.
    */
    if (qdf_unlikely(qdf_atomic_read(&vdev->pdev->target_tx_credit) <= 0)) {
	TXRX_STATS_MSDU_LIST_INCR(
			vdev->pdev, tx.dropped.host_reject, msdu);
#ifdef TSO_DBG
	qdf_print("HOST_TSO::No Tx credits on Target \n");
#endif
	ol_tx_tso_failure_cleanup(vdev->pdev,msdu);
	return msdu;
    }

    /* Extract Parent buffer of this TCP Segment ,
     * and also the SW TSO descriptor prepared during TCP segmentation */
    parent = qdf_nbuf_get_parent(msdu);
    if (qdf_unlikely(!parent)) {
	TXRX_STATS_MSDU_LIST_INCR(
			vdev->pdev, tx.dropped.host_reject, msdu);
	qdf_print("HOST_TSO::No parent buffer \n");
	ol_tx_tso_failure_cleanup(vdev->pdev,msdu);
	return msdu; /* the list of unaccepted MSDUs */
    }

    sw_tso_desc =
	(struct ol_tx_tso_desc_t *)QDF_NBUF_CB_TX_EXTRA_FRAG_VADDR(parent);
    if (qdf_unlikely(!sw_tso_desc)) {
	TXRX_STATS_MSDU_LIST_INCR(
			vdev->pdev, tx.dropped.host_reject, msdu);
	qdf_print("HOST_TSO::No Tx TSO Descriptor \n");
	ol_tx_tso_failure_cleanup(vdev->pdev,msdu);
	return msdu; /* the list of unaccepted MSDUs */
    }

    /* Allocate and prepare the Regular TX Buffer */
    tx_desc = ol_tx_desc_ll(vdev->pdev, vdev, msdu);
    if (qdf_unlikely(!tx_desc)) {
	TXRX_STATS_MSDU_LIST_INCR(
			vdev->pdev, tx.dropped.host_reject, msdu);
#ifdef TSO_DBG
	qdf_print("HOST_TSO::No Tx Descriptors \n");
#endif
	ol_tx_tso_failure_cleanup(vdev->pdev,msdu);
	return msdu; /* the list of unaccepted MSDUs */
    }

    /* Fill TSO information into HW TX descriptor
     *        by extracting from SW TSO Desc. */
    ol_tx_tso_desc_prepare(vdev->pdev, parent,msdu,tx_desc, sw_tso_desc);
    OL_TXRX_PROT_AN_LOG(vdev->pdev->prot_an_tx_sent, msdu);

    /* Return back the allocated memory to caller */
    *tx_descriptor = tx_desc;

    return NULL;
}

static inline
uint32_t ol_tx_prepare_ll_fast(struct ol_txrx_pdev_t *pdev,
        ol_txrx_vdev_handle vdev,
        qdf_nbuf_t nbuf,
        uint32_t pkt_download_len,
        uint32_t ep_id,
       	struct ol_tx_desc_t *tx_desc);

static inline
struct ol_tx_desc_t *ol_tx_fp_tso_prepare_ll(struct ol_txrx_pdev_t *pdev,
                           ol_txrx_vdev_handle vdev,
                           qdf_nbuf_t msdu,
                           uint32_t pkt_download_len,
                           uint32_t ep_id)
{
    struct ol_tx_desc_t *tx_desc = NULL;
    qdf_nbuf_t parent;
    struct ol_tx_tso_desc_t *sw_tso_desc;
    uint32_t status;

    /* Extract Parent buffer of this TCP Segment ,
     * and also the SW TSO descriptor prepared during TCP segmentation */
    parent = qdf_nbuf_get_parent(msdu);
    if (qdf_unlikely(!parent)) {
	TXRX_STATS_MSDU_LIST_INCR(
			vdev->pdev, tx.dropped.host_reject, msdu);
	qdf_print("HOST_TSO::No parent buffer \n");
        return NULL;
    }

    sw_tso_desc =
	(struct ol_tx_tso_desc_t *)QDF_NBUF_CB_TX_EXTRA_FRAG_VADDR(parent);
    if (qdf_unlikely(!sw_tso_desc)) {
	TXRX_STATS_MSDU_LIST_INCR(
			vdev->pdev, tx.dropped.host_reject, msdu);
	qdf_print("HOST_TSO::No Tx TSO Descriptor \n");
        return NULL;
    }

    OL_TX_PDEV_LOCK(&pdev->tx_lock);

    /* Allocate and prepare the Regular TX Buffer */
    tx_desc = ol_tx_desc_alloc(pdev);
    if (qdf_unlikely(!tx_desc)) {
        TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
        OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
	TXRX_STATS_MSDU_LIST_INCR(
			vdev->pdev, tx.dropped.host_reject, msdu);
	qdf_print("HOST_TSO::No Tx Descriptors \n");
        return NULL;
    }
    status = ol_tx_prepare_ll_fast(pdev,vdev, msdu,pkt_download_len,ep_id,tx_desc);
    if (status == OL_TXRX_FAILURE) {
        ol_tx_desc_free(pdev, tx_desc);
        OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
        return NULL;
    }

    OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
    /* Fill TSO information into HW TX descriptor
     * by extracting from SW TSO Desc. */
    ol_tx_tso_desc_prepare(vdev->pdev, parent,msdu,tx_desc, sw_tso_desc);
    OL_TXRX_PROT_AN_LOG(vdev->pdev->prot_an_tx_sent, msdu);

    tx_desc->pkt_type = ol_tx_frm_tso;
    /* Return back the allocated memory to caller */
    return tx_desc;
}
#endif /* HOST_SW_TSO_ENABLE */

#if WLAN_FEATURE_FASTPATH
#include <htc_api.h>         /* Layering violation, but required for fast path */
//#include <ce_api.h>
#endif  /* WLAN_FEATURE_FASTPATH */

#define ol_tx_prepare_ll(tx_desc, vdev, msdu) \
    do {                                                                      \
        /*
         * The TXRX module doesn't accept tx frames unless the target has
         * enough descriptors for them.
         */                                                                   \
        if (qdf_atomic_read(&vdev->pdev->target_tx_credit) <= 0) {         \
            TXRX_STATS_MSDU_LIST_INCR(                                        \
                vdev->pdev, tx.dropped.host_reject, msdu);                    \
            return msdu;                                                      \
        }                                                                     \
                                                                              \
        tx_desc = ol_tx_desc_ll(vdev->pdev, vdev, msdu);                      \
        if (! tx_desc) {                                                      \
            TXRX_STATS_MSDU_LIST_INCR(                                        \
                vdev->pdev, tx.dropped.host_reject, msdu);                    \
            return msdu; /* the list of unaccepted MSDUs */                   \
        }                                                                     \
        OL_TXRX_PROT_AN_LOG(vdev->pdev->prot_an_tx_sent, msdu);               \
    } while (0)

qdf_nbuf_t
ol_tx_ll(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
    qdf_nbuf_t msdu = msdu_list;
    /*
     * The msdu_list variable could be used instead of the msdu var,
     * but just to clarify which operations are done on a single MSDU
     * vs. a list of MSDUs, use a distinct variable for single MSDUs
     * within the list.
     */

    int ret = 0;
    osif_dev *osdev = (osif_dev *)(vdev->osif_vdev);
    struct ieee80211vap *vap = osdev->os_if;
#if !ATH_MCAST_HOST_INSPECT
    struct ether_header *eh = (struct ether_header *)qdf_nbuf_data(msdu);
#endif

#if HOST_SW_TSO_ENABLE
    TXRX_STATS_MSDU_INCR(vdev->pdev, tx.tso.non_tso_pkts, msdu);
#endif /* HOST_SW_TSO_ENABLE */

    if (OL_CFG_RAW_TX_LIKELINESS(vdev->tx_encap_type == htt_pkt_type_raw)) {
        OL_TX_LL_WRAPPER(vdev, msdu, vap->iv_ic->ic_qdf_dev);
        goto out;
    }

#if !ATH_MCAST_HOST_INSPECT
    if(IEEE80211_IS_IPV4_MULTICAST((eh)->ether_dhost) ||
            IEEE80211_IS_IPV6_MULTICAST((eh)->ether_dhost)) {
        /*
         * if the convert function returns some value larger
         * than 0, it means that one or more frames have been
         * transmitted and we are safe to return from here.
         */
        if (ol_tx_mcast_enhance_process(vap, msdu) > 0 ) {
          goto out;
        }
        /* Else continue with normal path. This happens when
         * mcast frame is recieved but enhance is not enabled
         * OR if it a broadcast frame send it as-is
         */
    }
#endif /*!ATH_MCAST_HOST_INSPECT*/

#if (HOST_SW_TSO_ENABLE || HOST_SW_TSO_SG_ENABLE || HOST_SW_SG_ENABLE)
    /* Look for TSO Marked packets from Network Stack */
    if (qdf_unlikely(qdf_nbuf_is_tso(msdu))) {
        qdf_nbuf_reset_ctxt(msdu);
#if HOST_SW_TSO_ENABLE
	ret = ol_tx_tso_process_skb(vdev,skb);
#elif HOST_SW_TSO_SG_ENABLE  /* HOST_SW_TSO_ENABLE */
        ret = ol_tx_tso_sg_process_skb(vdev, msdu);
#endif /* HOST_SW_TSO_SG_ENABLE */
    } else if (qdf_unlikely(qdf_nbuf_is_nonlinear(msdu))) {
        qdf_nbuf_reset_ctxt(msdu);
#if HOST_SW_SG_ENABLE
        ret = ol_tx_sg_process_skb(vdev,msdu);
#endif /* HOST_SW_SG_ENABLE */
    } else
#endif	/* HOST_SW_TSO_ENABLE || HOST_SW_TSO_SG_ENABLE || HOST_SW_SG_ENABLE */
    {
        msdu = qdf_nbuf_unshare(msdu);
        if (msdu == NULL) {
	  goto out;
	}

        /* Initializing CB_FTYPE with 0 */
        qdf_nbuf_set_fctx_type(msdu, 0, 0);
        ret = OL_TX_LL_WRAPPER(vdev, msdu, vap->iv_ic->ic_qdf_dev);
    }

out:
      return NULL;
}

#if WLAN_FEATURE_FASTPATH
static inline
uint32_t ol_tx_prepare_ll_fast(struct ol_txrx_pdev_t *pdev,
                           ol_txrx_vdev_handle vdev,
                           qdf_nbuf_t nbuf,
                           uint32_t pkt_download_len,
                           uint32_t ep_id,
                           struct ol_tx_desc_t *tx_desc)
{
    uint32_t *htt_tx_desc;
    void *htc_hdr_vaddr;
    uint32_t htc_hdr_paddr_lo; /* LSB of physical address */
    int num_frags, i;
    uint8_t ftype;
    uint16_t len = 0, dlen = pdev->me_buf.size;
    u_int8_t tidno = QDF_NBUF_TX_EXT_TID_INVALID;
#if MESH_MODE_SUPPORT
    u_int8_t power = 0;
    u_int8_t ratecode = 0;
    u_int8_t retries = 0;
    u_int8_t kix_valid_flags = 0;
    u_int8_t noqos = 0;
    u_int32_t pkt_subtype = 0;
    struct meta_hdr_s *mhdr;
#endif

    tx_desc->netbuf = nbuf;
    tx_desc->tx_encap_type = vdev->tx_encap_type;
#if MESH_MODE_SUPPORT
    tx_desc->extnd_desc = ext_desc_none;
#endif

    htt_tx_desc = tx_desc->htt_tx_desc;

    /* Make sure frags num is set to 0 */
    /*
     * Do this here rather than in hardstart, so
     * that we can hopefully take only one cache-miss while
     * accessing skb->cb.
     */
   // qdf_nbuf_frags_num_init(nbuf);
#if MESH_MODE_SUPPORT
    if (vdev->mesh_vdev) {
        tx_desc->extnd_desc = ext_desc_fp;
        /* metahdr extraction */
        mhdr = (struct meta_hdr_s *)qdf_nbuf_data(nbuf);
        power = mhdr->power;
        ratecode = mhdr->rates[0];
        retries = mhdr->max_tries[0];
        if (!(mhdr->flags & METAHDR_FLAG_AUTO_RATE)) {
            kix_valid_flags = 0x27;
        }
        if (mhdr->flags & METAHDR_FLAG_NOENCRYPT) {
            pkt_subtype = 0b100;
        }
        if (mhdr->flags & METAHDR_FLAG_NOQOS) {
            noqos = 1;
        }
        kix_valid_flags |= ((mhdr->keyix & 0x3) << 6);
        qdf_nbuf_pull_head(nbuf, sizeof(struct meta_hdr_s));
    }
#endif

    ftype = qdf_nbuf_get_ftype(nbuf);
    if( ftype == CB_FTYPE_MCAST2UCAST ) {
        struct ol_tx_me_buf_t *buf;
        uint32_t buf_paddr = 0;

        len = qdf_nbuf_len(nbuf);
        dlen = len < dlen ? len : dlen;

        tx_desc->pkt_type = ol_tx_frm_meucast;

        buf = (struct ol_tx_me_buf_t*) qdf_nbuf_get_fctx(nbuf);

#if !MCAST_ENHANCE_NO_CONSISTENT_MEM
        if (buf->nonpool_based_alloc) {
            if (QDF_STATUS_E_FAILURE == qdf_mem_map_nbytes_single(pdev->osdev, &buf->buf[0], QDF_DMA_TO_DEVICE, dlen,
                        &buf_paddr)) {
                return OL_TXRX_FAILURE;
            }
        } else {
            buf_paddr = HTT_ME_BUF_PADDR(pdev, &buf->buf[0]);
        }
#else
            if (QDF_STATUS_E_FAILURE == qdf_mem_map_nbytes_single(pdev->osdev, &buf->buf[0], QDF_DMA_TO_DEVICE, dlen,
                        &buf_paddr)) {
                return OL_TXRX_FAILURE;
            }
#endif /*MCAST_ENHANCE_NO_CONSISTENT_MEM*/
        num_frags = qdf_nbuf_get_num_frags(nbuf);
        htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc, num_frags);
        htt_tx_desc_frag(
                pdev->htt_pdev, tx_desc->htt_frag_desc, 0, buf_paddr, dlen);
        if( len > dlen ) {
            htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc, num_frags + 1);
            for (i = 1; i < num_frags + 1; i++) {
                int frag_len;
                u_int32_t frag_paddr;

                frag_len = qdf_nbuf_get_frag_len(nbuf, i);
                frag_paddr = qdf_nbuf_get_frag_paddr(nbuf, i);

                if(i==1) {
                    frag_paddr = frag_paddr + dlen;
                    frag_len = len - dlen;
                }

                htt_tx_desc_frag(
                        pdev->htt_pdev, tx_desc->htt_frag_desc, i, frag_paddr, frag_len);
                //printk("%d: %x %x %x %x\n", i, buf_paddr, dlen, frag_paddr, frag_len);
            }
        }

	/*Push it onto nbuf frag so that CE picks this to tx for FW*/
	/* qdf_nbuf_frag_push_head(
	   nbuf,
	   dlen,
	   &buf->buf[0], buf_paddr); */ /* phy addr LSB*/

	/*
	 * Disable push head as it populates extra frag which
	 * will fail mcas2ucast testcase.
	 * Store only in SKB CB Common Paddr so that frag1 will
	 * get mc buf physical address
	 */
	QDF_NBUF_CB_PADDR(nbuf) =  buf_paddr;
    }

#if HOST_SW_SG_ENABLE
    else if (ftype == CB_FTYPE_SG) {
        struct ol_tx_sg_desc_t *sw_sg_desc;

        tx_desc->pkt_type = ol_tx_frm_sg; /* this can be prefilled? */
        sw_sg_desc = (struct ol_tx_sg_desc_t *) qdf_nbuf_get_fctx(nbuf);
        len = sw_sg_desc->total_len;
        num_frags = sw_sg_desc->frag_cnt;

        /* Following function initialises the htt_frag desc */
        htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc, num_frags);
        for (i = 0; i < num_frags; i++) {

            htt_tx_desc_frag(
                    pdev->htt_pdev, tx_desc->htt_frag_desc, i,
		    sw_sg_desc->frag_paddr_lo[i],
		    sw_sg_desc->frag_len[i]);
        }
        ol_tx_sg_desc_free(pdev, sw_sg_desc);

    }
#endif /* HOST_SW_SG_ENABLE */
#if HOST_SW_TSO_SG_ENABLE
    else if (ftype == CB_FTYPE_TSO_SG) {
        struct ol_tx_tso_desc_t *sw_tso_sg_desc;

        tx_desc->pkt_type = ol_tx_frm_tso; /* this can be prefilled? */
        sw_tso_sg_desc = (struct ol_tx_tso_desc_t *) qdf_nbuf_get_fctx(nbuf);
        len = sw_tso_sg_desc->data_len + sw_tso_sg_desc->l2_l3_l4_hdr_size;
        num_frags = sw_tso_sg_desc->segment_count;

        /* Following function initialises the htt_frag desc */
        htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc, num_frags);
        for (i = 0; i < num_frags; i++) {

            htt_tx_desc_frag(
                    pdev->htt_pdev, tx_desc->htt_frag_desc, i,
		    sw_tso_sg_desc->seg_paddr_lo[i],
		    sw_tso_sg_desc->seg_len[i]);
        }
	/* perpare the HTT tx descriptor with TSO info of the MSDU */
        ol_tx_tso_sg_desc_prepare(vdev->pdev, tx_desc, sw_tso_sg_desc);

    }
#endif /* HOST_SW_TSO_SG_ENABLE */
    else {
        tx_desc->pkt_type = ol_tx_frm_std; /* this can be prefilled? */
        /* Following function initialises the htt_frag desc */
        len = qdf_nbuf_len(nbuf);
        num_frags = qdf_nbuf_get_num_frags(nbuf);
        htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc, num_frags);
        for (i = 0; i < num_frags; i++) {
            int frag_len;
            u_int32_t frag_paddr;

            frag_len = qdf_nbuf_get_frag_len(nbuf, i);
            frag_paddr = qdf_nbuf_get_frag_paddr(nbuf, i);
            htt_tx_desc_frag(
                    pdev->htt_pdev, tx_desc->htt_frag_desc, i, frag_paddr, frag_len);
        }
    }
#if MESH_MODE_SUPPORT
    if (vdev->mesh_vdev) {
        if (noqos) {
            tidno = HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST;
        } else {
            tidno = wbuf_get_priority(nbuf);
        }
    }
#endif

    /* Following function initialises plain htt tx desc fields of the packet(exculding htt_frag desc) */
    htt_tx_desc_init(pdev->htt_pdev, htt_tx_desc, tx_desc->id,
         len, vdev->vdev_id, htt_pkt_type_ethernet, qdf_nbuf_get_tx_cksum(nbuf),
         tidno);

#if MESH_MODE_SUPPORT
    /* Initialize rate info */
    if (vdev->mesh_vdev) {
        htt_tx_desc_rateinfo_init(pdev->htt_pdev, htt_tx_desc,pkt_subtype, power, ratecode, retries, kix_valid_flags);
    }
#endif

#if TCP_CHECKSUM_OFFLOAD
    if(qdf_nbuf_get_tx_cksum(nbuf)==HTT_TX_L4_CKSUM_OFFLOAD) {
        uint32_t *cksum_enable;
        cksum_enable = (uint32_t *) (((uint32_t *)tx_desc->htt_frag_desc) + 3);
        *cksum_enable |= HTT_ENDIAN_BYTE_IDX_SWAP(0x1f0000);
    }
#endif /* TCP_CHECKSUM_OFFLOAD */

    /* Check to make sure, data download length is correct */

    /* TODO : Can we remove this check and always download a fixed length ? */
    if (qdf_unlikely(len < pkt_download_len)) {
        pkt_download_len = len;
    }

    /*
     *  Do we want to turn on word_stream bit-map here ? For linux, non-TSO this is
     *  not required.
     *  We still have to mark the swap bit correctly, when posting to the ring
     */

    /* After this point num_extra_frags = 1 */
    /* Can this be passed as parameters to CE to program?
     * Why should we store this at all ?
     */
    /* Virtual address of the HTT/HTC header, added by driver */
    htc_hdr_vaddr = (char *)htt_tx_desc - HTC_HEADER_LEN;

    /* TODO: Precompute and store paddr in ol_tx_desc_t */
    htc_hdr_paddr_lo = (u_int32_t)
		      HTT_TX_DESC_PADDR(pdev->htt_pdev, htc_hdr_vaddr);

    qdf_nbuf_frag_push_head(
		              nbuf,
                      HTC_HEADER_LEN + HTT_TX_DESC_LEN,
                      htc_hdr_vaddr, htc_hdr_paddr_lo); /*phy addr LSB*/

    /* Fill the HTC header information */
    /*
     * Passing 0 as the seq_no field, we can probably get away
     * with it for the time being, since this is not checked in f/w
     */
    /* TODO : Prefill this, look at multi-fragment case */
    HTC_TX_DESC_FILL(htc_hdr_vaddr, pkt_download_len, ep_id, 0);
    return OL_TXRX_SUCCESS;
}
/*
 * ol_tx_ll_fast(): Fast path OL layer send function
 * Function:
 * 1) Get OL Tx desc
 * 2) Fill out HTT + HTC headers
 * 3) Fill out SG list
 * 4) Store meta-data (implicit, because we use pre-allocated pool)
 * 5) Call CE send function
 * Returns:
 *  No. of nbufs that could not be sent.
 */

#if ATH_SUPPORT_HYFI_ENHANCEMENTS

extern void ol_notify_if_low_on_buffers(void *scn, uint32_t free_buff);
#define ATH_HYFI_NOTIFY_LOW_ON_BUFFER( _ctrldev, _freebuff) \
    ol_notify_if_low_on_buffers(_ctrldev, (pdev->tx_desc.pool_size - pdev->stats.pub.tx.desc_in_use));
#else

#define ATH_HYFI_NOTIFY_LOW_ON_BUFFER( _scn, _freebuff)

#endif

uint32_t
ol_tx_ll_fast(ol_txrx_vdev_handle vdev,
              qdf_nbuf_t *nbuf_arr,
#if PEER_FLOW_CONTROL
              uint32_t num_msdus,
              uint16_t peer_id,
              uint8_t tid)
#else
              uint32_t num_msdus)
#endif

{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ol_tx_desc_t *tx_desc = NULL;
    qdf_nbuf_t nbuf;
    uint32_t pkt_download_len, num_sent = 0;
    uint32_t ep_id, status;
    int i = 0;
    uint8_t ftype = CB_FTYPE_INVALID;

    ATH_DEBUG_SET_RTSCTS_ENABLE((osif_dev *)vdev->osif_vdev);
    ASSERT(num_msdus);
    pkt_download_len = ((struct htt_pdev_t *)(pdev->htt_pdev))->download_len;

    /* This can be statically inited once, during allocation */
    /* Call this HTC_HTTEPID_GET */
    ep_id = HTT_EPID_GET(pdev->htt_pdev);


    for (i = 0; i < num_msdus; i++) {
        nbuf = nbuf_arr[i];
#if HOST_SW_TSO_ENABLE
        if(qdf_unlikely(qdf_nbuf_is_tso(nbuf) == A_TRUE)) {
            if( !(tx_desc = ol_tx_fp_tso_prepare_ll(pdev,
                            vdev, nbuf,pkt_download_len,ep_id) )) {
                break;
            }
        } else
#endif /* HOST_SW_TSO_ENABLE */
         {
             qdf_nbuf_num_frags_init(nbuf);

             ftype = qdf_nbuf_get_ftype(nbuf);
#if !PEER_FLOW_CONTROL
             /*
              * check if flow ctrl queue is empty or not, if not empty
              * it indicates that the tx_descriptors are exhausted,
              * hence enqueue the pkt to flow ctrl queue and return.
              * if flow ctrl queue is empty go ahead and allocate a
              * tx_descriptor.
              */
             /* Queuing is enabled only for mcast2ucast in non std path.
              * This will be removed once tso also enables flow control */
             if( (ftype == CB_FTYPE_MCAST2UCAST)) {
                 qdf_nbuf_set_vdev_ctx(nbuf, (void *)vdev);
                 status = ol_tx_check_enqueue_flow_ctrl(pdev, nbuf);
                 if (qdf_likely(status == OL_TX_FLOW_CTRL_ENQ_DONE) ||
                     (status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD)) {
                     if(qdf_unlikely(status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD)) {
                         /* request OS not to forward more packets untill requested */
                         netif_stop_queue(((osif_dev *)vdev->osif_vdev)->netdev);
                     }
                     return OL_TXRX_SUCCESS;
                 } else if (status == OL_TX_FLOW_CTRL_Q_FULL) {
                     goto out_free;
             }
         }
#endif
             tx_desc = ol_tx_desc_alloc(pdev);

             if (qdf_unlikely(!tx_desc)) {
#if !PEER_FLOW_CONTROL
                 if( (ftype == CB_FTYPE_MCAST2UCAST)) {
                     /*
                      * enqueue the pkt to flow ctrl queue as tx_descriptor
                      * allocation failed
                      */
                     qdf_nbuf_set_vdev_ctx(nbuf, (void *)vdev);
                     status = ol_tx_enqueue_flow_ctrl(pdev, nbuf);
                     if (qdf_likely(status == OL_TX_FLOW_CTRL_ENQ_DONE) ||
                         (status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD)) {
                         if(qdf_unlikely(status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD)) {
                             /* request OS not to forward more packets untill requested */
                             netif_stop_queue(((osif_dev *)vdev->osif_vdev)->netdev);
                         }
                         return OL_TXRX_SUCCESS;
                     }
                     else if (status == OL_TX_FLOW_CTRL_Q_FULL)
                     {
                         TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
                         goto out_free;
                     }
                 } else
#endif
                 {
                     /* No descriptors, drop the frame */
                     TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
                     TXRX_PRINT(TXRX_PRINT_LEVEL_INFO1,
                             "%s desc allocation failed \n", __func__);
                     goto out_free;
                 }
             }

             TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_avoid, 1);

             status = ol_tx_prepare_ll_fast(pdev, vdev, nbuf, pkt_download_len, ep_id, tx_desc);
             if (status == OL_TXRX_FAILURE) {
                 ol_tx_desc_free(pdev, tx_desc);
                 tx_desc = NULL;
                 break;
             }
#if PEER_FLOW_CONTROL
             if((peer_id != HTT_INVALID_PEER)){
                 htt_tx_desc_set_peer_id((u_int32_t *)(tx_desc->htt_tx_desc), peer_id);
             }

             if(tid != HTT_TX_EXT_TID_INVALID) {
                 htt_tx_desc_tid(pdev->htt_pdev, tx_desc->htt_tx_desc, tid);
             }
#endif
         }
    }

    /*
     * If we could get descriptor for i packets, just send them one shot
     * to the CE ring
     * Assumption: if there is enough descriptors i should be equal to num_msdus
     */
    if (i) {
        num_sent = hif_send_fast(pdev->htt_pdev->osc, *nbuf_arr, ep_id, pkt_download_len);
    }

    ASSERT((num_msdus - num_sent) == 0);

    /* Assume num_msdus == 1 */
    if ((num_sent == 0) && tx_desc) {
        ol_tx_desc_free(pdev, tx_desc);
    }

out:
    ATH_HYFI_NOTIFY_LOW_ON_BUFFER(pdev->ctrl_pdev, (pdev->tx_desc.pool_size - pdev->stats.pub.tx.desc_in_use));

    /*
     * If there was only a partial send,
     * send the part of the nbuf_arr that could not be
     * sent back to the caller.
     */
    return (num_msdus - num_sent);
out_free:
#if HOST_SW_TSO_SG_ENABLE
    if (ftype == CB_FTYPE_TSO_SG )
        ol_tx_tso_sg_desc_free(pdev, (struct ol_tx_tso_desc_t *)qdf_nbuf_get_fctx(nbuf));
#endif
#if HOST_SW_SG_ENABLE
    if (ftype == CB_FTYPE_SG)
        ol_tx_sg_desc_free(pdev, (struct ol_tx_sg_desc_t *)qdf_nbuf_get_fctx(nbuf));
#endif
    goto out;
}

qdf_nbuf_t ol_tx_reinject_fast(
        struct ol_txrx_vdev_t *vdev,
        qdf_nbuf_t msdu, uint32_t peer_id)
{
    struct ol_tx_desc_t *tx_desc = NULL;
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    uint32_t pkt_download_len =
        ((struct htt_pdev_t *)(pdev->htt_pdev))->download_len;
    uint32_t ep_id = HTT_EPID_GET(pdev->htt_pdev);
    uint32_t status;

#if QCA_PARTNER_DIRECTLINK_TX
    if (CE_is_directlink(pdev->ce_tx_hdl)) {
        ol_tx_partner(msdu, OSIF_TO_NETDEV(vdev->osif_vdev), peer_id);
    } else
#endif /* QCA_PARTNER_DIRECTLINK_TX */

{
    OL_TX_PDEV_LOCK(&pdev->tx_lock);

    tx_desc = ol_tx_desc_alloc(pdev);

    if (qdf_unlikely(!tx_desc)) {
        TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
        OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
        return msdu;
    }

#if MESH_MODE_SUPPORT
    if (!vdev->mesh_vdev) {
#endif
        status = ol_tx_prepare_ll_fast(pdev, vdev, msdu,
                pkt_download_len, ep_id, tx_desc);
#if MESH_MODE_SUPPORT
    } else {
        status = OL_TXRX_FAILURE;
    }
#endif

    if (status == OL_TXRX_FAILURE) {
        ol_tx_desc_free(pdev, tx_desc);
        OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
        return msdu;
    }

    if (qdf_likely(tx_desc)) {
        /* Additional plumbing for re-injection */
        HTT_TX_DESC_POSTPONED_SET(*((u_int32_t *)(tx_desc->htt_tx_desc)), TRUE);
        htt_tx_desc_set_peer_id((u_int32_t *)(tx_desc->htt_tx_desc), peer_id);

        if ((0 == hif_send_fast(pdev->htt_pdev->osc, msdu, ep_id, pkt_download_len))) {
            /* The packet could not be sent */
            /* Free the descriptor, return the packet to the caller */
            ol_tx_desc_free(pdev, tx_desc);
            OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
            return msdu;
        }
    }
    OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
}

    return NULL;
}
#ifdef QCA_PARTNER_PLATFORM
void
ol_tx_stats_inc_pkt_cnt(ol_txrx_vdev_handle vdev)
#else
inline void
ol_tx_stats_inc_pkt_cnt(ol_txrx_vdev_handle vdev)
#endif
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;

    TXRX_STATS_ADD(pdev, pub.tx.from_stack.pkts, 1);
}

void
ol_tx_stats_inc_map_error(ol_txrx_vdev_handle vdev,
                             uint32_t num_map_error)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;

    TXRX_STATS_ADD(pdev, pub.tx.dma_map_error, 1);
}

void
ol_tx_stats_inc_ring_error(ol_txrx_pdev_handle pdev,
                             uint32_t num_ring_error)
{
    TXRX_STATS_ADD(pdev, pub.tx.ce_ring_full, 1);
}

void ol_dump_txdesc(struct ol_tx_desc_t *tx_desc)
{
    qdf_print("\nhtt_tx_desc bad for tx_desc:%p, id (%d), htt_tx_status (%d), allocated (%d)\n",tx_desc, tx_desc->id, tx_desc->status, tx_desc->allocated);
}

#if ATH_TX_OVERFLOW_IND
static void ol_vap_send_overflow_event(ol_txrx_vdev_handle vdev)
{
    osif_dev * osdev = (osif_dev *)(vdev->osif_vdev);
    struct ieee80211vap *vap = osdev->os_if;
    uint64_t tick_current = qdf_system_ticks();
#if !PEER_FLOW_CONTROL_FORCED_MODE0
    static uint32_t txoverflow_limit_print = 0;
#endif

    if(qdf_system_time_after(tick_current,vdev->tx_overflow_start+HZ)){
        /*Send TX overflow event every 1s*/
        IEEE80211_DELIVER_EVENT_TX_OVERFLOW(vap);
        vdev->tx_overflow_start = tick_current;
#if !PEER_FLOW_CONTROL_FORCED_MODE0
        if(!(txoverflow_limit_print % TX_OVERFLOW_PRINT_LIMIT))
        {
            IEEE80211_DPRINTF(vap,IEEE80211_MSG_ANY, "TX OVERFLOW!\n");
        }
        txoverflow_limit_print++;
#endif
    }
}
#endif

#if QCA_OL_TX_CACHEDHDR
u_int32_t
ol_tx_ll_cachedhdr_prep(ol_txrx_vdev_handle vdev,
        qdf_nbuf_t netbuf, uint32_t peerid, uint8_t tid, struct ol_tx_desc_t *tx_desc )
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ol_txrx_peer_t *peer;
    u_int8_t *data;
    u_int32_t *netdata;
    uint32_t netlen;
    qdf_nbuf_t currnbuf = NULL, nextnbuf = NULL;
    u_int8_t num_frags = 1;

    u_int32_t *hdr_cache = vdev->hdrcache;
    u_int16_t msdu_len =  qdf_nbuf_len(netbuf);
    u_int16_t total_len;
    u_int32_t *htt_tx_desc;
    u_int32_t frag_paddr;
#if MESH_MODE_SUPPORT
    u_int32_t power = 0;
    u_int32_t ratecode = 0;
    u_int32_t retries = 0;
    u_int32_t kix_valid_flags = 0;
    u_int32_t pkt_subtype = 0;
    u_int32_t noqos = 0;
    struct meta_hdr_s *mhdr;
#endif

    tx_desc->netbuf = netbuf;
    tx_desc->tx_encap_type = vdev->tx_encap_type;
    tx_desc->pkt_type = ol_tx_frm_std;
#if MESH_MODE_SUPPORT
    tx_desc->extnd_desc = ext_desc_none;
#endif

    htt_tx_desc = (u_int32_t *) tx_desc->htt_tx_desc;

    total_len = msdu_len;

#if MESH_MODE_SUPPORT
    if (vdev->mesh_vdev) {
        tx_desc->extnd_desc = ext_desc_chdr;
        /* metahdr extraction */
        mhdr = (struct meta_hdr_s *)qdf_nbuf_data(netbuf);
        power = mhdr->power;
        ratecode = mhdr->rates[0];
        retries = mhdr->max_tries[0];
        if (!(mhdr->flags & METAHDR_FLAG_AUTO_RATE)) {
            kix_valid_flags = 0x27;
        } else {
            /*use auto rate*/
            kix_valid_flags = 0x20;
        }
        if (mhdr->flags & METAHDR_FLAG_NOENCRYPT) {
            /* This is a binary value */
            pkt_subtype = 0b100;
        }
        if (mhdr->flags & METAHDR_FLAG_NOQOS) {
            noqos = 1;
        }
        kix_valid_flags |= ((mhdr->keyix & 0x3) << 6);
        qdf_nbuf_pull_head(netbuf, sizeof(struct meta_hdr_s));
        msdu_len =  qdf_nbuf_len(netbuf);
        total_len = msdu_len;
    }
#endif

    if (OL_CFG_RAW_TX_LIKELINESS(vdev->tx_encap_type == htt_pkt_type_raw)) {
        if (qdf_unlikely(OL_TX_PREPARE_RAW_DESC_CHDHDR(pdev,
                        vdev, netbuf, tx_desc,
                        &total_len, &num_frags) != 0)) {
            return OL_TXRX_FAILURE;
        }
    }

    htt_ffcache_update_vdev_id(hdr_cache, vdev->vdev_id);
#if PEER_FLOW_CONTROL
    /*
     * WIFI IP Ver > 2 only :  Send MSDU AVG size in DWORD 2
     */
    if (HTT_WIFI_IP(pdev->htt_pdev,2,0)) {
        peer = ol_txrx_peer_find_by_id(pdev, peerid);
        if (peer && (tid < OL_TX_PFLOW_CTRL_HOST_MAX_TIDS)) {
            htt_ffcache_update_msduavglen(pdev->htt_pdev, hdr_cache, peer->tidq[tid].msdu_avg_size);
        }
    } else {
        htt_ffcache_update_descphy_addr(hdr_cache, htt_tx_desc);
    }
#else
    htt_ffcache_update_descphy_addr(hdr_cache, htt_tx_desc);
#endif // PEER_FLOW_CONTROL

    htt_ffcache_update_msdu_len_and_id(hdr_cache, total_len, tx_desc->id);
#if ATH_TX_PRI_OVERRIDE
    /* Overwrite TID with customized tid value in netbuf */
    tid = wbuf_get_priority(netbuf);
#endif

#if MESH_MODE_SUPPORT
    if (vdev->mesh_vdev) {
        htt_ffcache_update_extvalid(hdr_cache, 1);
        htt_ffcache_update_power(hdr_cache, power);
        htt_ffcache_update_ratecode(hdr_cache, ratecode);
        htt_ffcache_update_rateretries(hdr_cache, retries);
        htt_ffcache_update_validflags(hdr_cache, kix_valid_flags);
        htt_ffcache_update_pktsubtype(hdr_cache, pkt_subtype);
        if (noqos) {
            tid = HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST;
        } else {
            tid = wbuf_get_priority(netbuf);
        }
    } else {
        htt_ffcache_update_extvalid(hdr_cache, 0);
    }
#endif
    htt_ffcache_update_tid(hdr_cache, tid);

    data = skb_push(netbuf, vdev->htc_htt_hdr_size);//qdf_nbuf_push_head(netbuf, vdev->htc_htt_hdr_size);
    qdf_mem_copy(data, hdr_cache, vdev->htc_htt_hdr_size);

    qdf_nbuf_peek_header(netbuf, (uint8_t **)&netdata, &netlen);

    /* Additional plumbing for re-injection */
    if(peerid != HTT_INVALID_PEER){
        htt_ffcache_update_desc_postponed_set(netdata);
        htt_ffcache_update_desc_set_peer_id(netdata, peerid);
    }

    if(QDF_STATUS_E_FAILURE == qdf_nbuf_map_nbytes_single(vdev->osdev, netbuf, QDF_DMA_TO_DEVICE, netlen)){
	    qdf_print("Map error \n");
        printk("Map Error\n");
        if (OL_CFG_RAW_TX_LIKELINESS(vdev->tx_encap_type == htt_pkt_type_raw)) {
            /* Free all non master nbufs in the chain, if any */
            currnbuf = qdf_nbuf_next(netbuf);
            while (currnbuf) {
                nextnbuf = qdf_nbuf_next(currnbuf);
                qdf_nbuf_unmap_single(
                    vdev->osdev, currnbuf, QDF_DMA_TO_DEVICE);
                qdf_nbuf_free(currnbuf);
                currnbuf = nextnbuf;
            }
        }
	    qdf_nbuf_free(netbuf);
	    return OL_TXRX_FAILURE;
    }

    frag_paddr = qdf_nbuf_get_frag_paddr(netbuf, 0);
#if 0
    /* initialize the fragmentation descriptor */
    num_frags = qdf_nbuf_get_num_frags(netbuf);
    htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc, num_frags);
    for (i = 0; i < num_frags; i++) {
        int frag_len;
        u_int32_t frag_paddr;

        frag_len = qdf_nbuf_get_frag_len(netbuf, i);
        frag_paddr = qdf_nbuf_get_frag_paddr(netbuf, i);
        htt_tx_desc_frag(
            pdev->htt_pdev, tx_desc->htt_frag_desc, i, frag_paddr, frag_len);
    }
#else
    htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc, num_frags);
    htt_tx_desc_frag(
            pdev->htt_pdev, tx_desc->htt_frag_desc, 0, frag_paddr + vdev->htc_htt_hdr_size, msdu_len);

#if TCP_CHECKSUM_OFFLOAD
    if( qdf_unlikely(qdf_nbuf_get_tx_cksum(netbuf)==HTT_TX_L4_CKSUM_OFFLOAD) ) {
        uint32_t *cksum_enable;
        cksum_enable = (uint32_t *) (((uint32_t *)tx_desc->htt_frag_desc) + 3);
        *cksum_enable |= HTT_ENDIAN_BYTE_IDX_SWAP(0x1f0000);
    }
#endif /* TCP_CHECKSUM_OFFLOAD */

#endif

    return OL_TXRX_SUCCESS;
}

#if PEER_FLOW_CONTROL
int
ol_tx_ll_pflow_ctrl(ol_txrx_vdev_handle vdev, qdf_nbuf_t netbuf)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    uint32_t status;
    uint8_t nodrop = 0, hol = 0;
    uint32_t headroom;
    uint8_t ftype;

    struct ol_tx_nbuf_classify nbuf_class;
    int tid_q_map;

    ATH_DEBUG_SET_RTSCTS_ENABLE((osif_dev *)vdev->osif_vdev);

    qdf_nbuf_num_frags_init(netbuf);
    ftype = qdf_nbuf_get_ftype(netbuf);
    if ( ( ftype != CB_FTYPE_MCAST2UCAST)
#if HOST_SW_TSO_SG_ENABLE
         && ( ftype != CB_FTYPE_TSO_SG )
#endif
#if HOST_SW_SG_ENABLE
         && ( ftype != CB_FTYPE_SG)
#endif
    ) {
        if (qdf_unlikely((headroom = qdf_nbuf_headroom(netbuf))< HTC_HTT_TRANSFER_HDRSIZE)){
            netbuf = qdf_nbuf_realloc_headroom(netbuf, HTC_HTT_TRANSFER_HDRSIZE);
            if (netbuf == NULL ){
                qdf_print("Realloc failed \n");
                return OL_TXRX_FAILURE;
            }
        }
    }

    /*
     * HTT PUSH Mode :
     * We start enqueuing packets to peer/tidq only after number of used descriptors is > min_threshold.
     * For pure UL traffic, or ping traffic, host queueing and dequeuing from fetches will add latency.
     * Instead, we enqueue these packets directly to Target.
     */
#if !PEER_FLOW_CONTROL_FORCED_MODE0
    if((pdev->pflow_ctrl_mode == HTT_TX_MODE_PUSH_NO_CLASSIFY) ||
            ((pdev->pflow_ctl_desc_count < pdev->pflow_ctl_min_threshold) &&
             (pdev->pflow_ctl_global_queue_cnt == 0))) {
#else
        {
#endif
            uint8_t tid = HTT_TX_EXT_TID_INVALID;
            ftype = qdf_nbuf_get_ftype(netbuf);

            if ( ( ftype == CB_FTYPE_MCAST2UCAST)
#if HOST_SW_TSO_SG_ENABLE
                    || ( ftype == CB_FTYPE_TSO_SG )
#endif
#if HOST_SW_SG_ENABLE
                    || ( ftype == CB_FTYPE_SG)
#endif
            ) {
                tid_q_map = ol_tx_get_tid_override_queue_mapping(pdev, netbuf);
                if (tid_q_map >= 0) {
                    tid = tid_q_map;
                }
                status = ol_tx_ll_fast(vdev, &netbuf, 1, HTT_INVALID_PEER, tid);
            } else {
                tid_q_map = ol_tx_get_tid_override_queue_mapping(pdev, netbuf);
                if (tid_q_map >= 0) {
                    tid = tid_q_map;
                }
                status = ol_tx_ll_cachedhdr(vdev, netbuf, HTT_INVALID_PEER, tid);
            }

            if (status != OL_TXRX_SUCCESS) {
                if (ftype == CB_FTYPE_MCAST2UCAST)
                    ol_tx_me_free_buf(pdev, (struct ol_tx_me_buf_t *) qdf_nbuf_get_fctx(netbuf));
                qdf_nbuf_free(netbuf);
                TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_discard, 1);
                return OL_TXRX_FAILURE;
            }
            PFLOW_CTRL_PDEV_STATS_ADD(pdev, QUEUE_BYPASS_CNT, 1);
            return OL_TXRX_SUCCESS;
    }// HTT_TX_MODE_PUSH_NO_CLASSIFY Check

    /*
     * Classify - get the peer_id, TID and other flags
     */
    ol_tx_classify(vdev, netbuf, &nbuf_class);

    if (nbuf_class.is_tcp) {
        nodrop = 1;
    }

    if (nbuf_class.is_arp || nbuf_class.is_dhcp || nbuf_class.is_igmp) {
        if (nbuf_class.is_mcast) {
            hol = 1;
        }
        nodrop = 1;
        nbuf_class.tid = OSDEP_EAPOL_TID;
    }

    /* CPU cycle reduction : Per Peer loop avoid to detect
     * NAWDS enabled or not.
     * NAWDS is actually per peer enabling.
     * BSS Peer and there could be Wireless STA which
     * are Non-Aware of NAWDS. This is required
     * to differentiate only between Normal/NAWDS path
     * TODO: Find a way to get a peer without looping
     */
#define OL_NAWDS_ENABLED(_vdev) (_vdev->nawds_enabled & 0x1)

    if (qdf_unlikely(OL_NAWDS_ENABLED(vdev)) && nbuf_class.is_mcast) {
        status = ol_tx_pflow_enqueue_nawds(vdev, netbuf, 0, nodrop, hol);
        qdf_nbuf_free(netbuf);
        return OL_TXRX_SUCCESS;
    }

    /* Set the vdev ctx before enqueue */
    qdf_nbuf_set_vdev_ctx(netbuf, (void *)vdev);
    status = ol_tx_enqueue_pflow_ctrl(vdev, netbuf, nbuf_class.peer_id, nbuf_class.tid , nodrop, hol);

    if (qdf_likely(status == OL_TX_PFLOW_CTRL_DESC_AVAIL)) {
        return OL_TXRX_SUCCESS;
    } else if (qdf_likely(status == OL_TX_PFLOW_CTRL_ENQ_DONE)) {
        return OL_TXRX_SUCCESS;
    } else {
        ol_tx_free_buf_generic(pdev, netbuf);
        TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_discard, 1);
        return OL_TXRX_FAILURE;
    }

    return OL_TXRX_SUCCESS;
}

uint32_t
ol_tx_ll_pflow_ctrl_list(ol_txrx_vdev_handle vdev,
        qdf_nbuf_t *nbuf_arr,
        uint32_t num_msdus)
{
    /* struct ol_txrx_pdev_t *pdev = vdev->pdev; */
    qdf_nbuf_t nbuf;
    int i = 0;

    ATH_DEBUG_SET_RTSCTS_ENABLE((osif_dev *)vdev->osif_vdev);
    ASSERT(num_msdus);

    for (i = 0; i < num_msdus; i++) {
        nbuf = nbuf_arr[i];
        ol_tx_ll_pflow_ctrl(vdev, nbuf);
    }

    return 0;
}

static inline uint32_t
ol_tx_pflow_dequeue_priority_tid(struct ol_txrx_pdev_t *pdev, struct ol_txrx_peer_t *peer,
                                                                      uint32_t peer_id, uint8_t tid)
{
    struct ol_tx_desc_t *tx_desc = NULL;
    uint32_t status    = OL_TX_PFLOW_CTRL_DEQ_DONE;
    qdf_nbuf_t netbuf  = NULL;
    qdf_nbuf_t tnetbuf = NULL;
    qdf_nbuf_t hnetbuf = NULL;
    ol_txrx_vdev_handle vdev;

    while(!qdf_nbuf_is_queue_empty(&peer->tidq[OL_TX_PFLOW_CTRL_PRIORITY_TID].nbufq)) {
        /* Take first skb frame from tidq */
        netbuf = qdf_nbuf_queue_first(&peer->tidq[OL_TX_PFLOW_CTRL_PRIORITY_TID].nbufq);
        if(netbuf) {
            tx_desc = ol_tx_desc_alloc(pdev);
            if(tx_desc) {
                /* Pop netbuf from tidq */
                netbuf = qdf_nbuf_queue_remove(&peer->tidq[OL_TX_PFLOW_CTRL_PRIORITY_TID].nbufq);

                vdev = (ol_txrx_vdev_handle )qdf_nbuf_get_vdev_ctx(netbuf);
                peer->tidq[tid].byte_count -= qdf_nbuf_len(netbuf);
                pdev->pflow_ctl_total_dequeue_cnt++;
                pdev->pflow_ctl_global_queue_cnt--;
                peer->tidq[tid].dequeue_cnt++;

                if (ol_tx_ll_cachedhdr_prep(vdev, netbuf, peer_id, tid, tx_desc)) {
                    ol_tx_desc_free(pdev, tx_desc);
                    continue;
                }
                if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
                    qdf_nbuf_set_next(netbuf, NULL);

                    if (tnetbuf == NULL) {
                        tnetbuf = netbuf;
                        hnetbuf = netbuf;
                    } else {
                        qdf_nbuf_set_next(tnetbuf, netbuf);
                        tnetbuf = netbuf;
                    }
                }
            } else { /* !tx_desc */
                TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
                TXRX_PRINT(TXRX_PRINT_LEVEL_INFO2,
                        "%s desc allocation failed \n", __func__);
                status = OL_TX_PFLOW_CTRL_DEQ_DESC_FAIL;
                OL_TX_PEER_UNLOCK(pdev, peer_id);
                goto out;
            }
        } else { /* !netbuf */
            if (!qdf_nbuf_is_queue_empty(&peer->tidq[OL_TX_PFLOW_CTRL_PRIORITY_TID].nbufq))  {
                qdf_print("%s queue not empty %d , but nbuf NULL \n", __func__, qdf_nbuf_queue_len(&peer->tidq[OL_TX_PFLOW_CTRL_PRIORITY_TID].nbufq));
            }

            TXRX_PRINT(TXRX_PRINT_LEVEL_INFO1,
                    "%s no packets in queue \n", __func__);
            status = OL_TX_PFLOW_CTRL_DEQ_REMAINING;
            goto out;
        }
    }// while

out:
    ol_tx_pflow_ctrl_queue_map_update(pdev, peer, peer_id, tid);
    if(hnetbuf) {
        hif_batch_send(pdev->htt_pdev->osc, hnetbuf, vdev->epid, vdev->downloadlen, 0);
    }

    return status;
}

static inline uint32_t
ol_tx_pflow_ctrl_dequeue_send(struct ol_txrx_pdev_t *pdev, uint32_t *fetch_ind_rec, uint32_t *num_tx_pkts, uint32_t *num_tx_bytes)
{
    struct ol_txrx_peer_t *peer;
    u_int16_t tx_pkts;
    uint32_t pkt_download_len, pkt_len;
    uint32_t ep_id, status;
    uint8_t ftype;
    u_int32_t len = 0, delta_msecs;
    ol_txrx_vdev_handle vdev;

    u_int32_t peer_id;
    u_int8_t tid;
    u_int32_t max_bytes, tx_bytes;
    u_int16_t max_pkts;

    struct ol_tx_desc_t *tx_desc = NULL;
    qdf_nbuf_t netbuf = NULL;
    qdf_nbuf_t tnetbuf = NULL;
    qdf_nbuf_t hnetbuf = NULL;
#if ATH_DATA_TX_INFO_EN
    struct ol_ath_softc_net80211 *scn = (struct ol_ath_softc_net80211 *)(pdev->ctrl_pdev);
#endif

    peer_id = HTT_T2H_TX_FETCH_IND_PEERID_GET(*fetch_ind_rec);
    tid = HTT_T2H_TX_FETCH_IND_TIDNO_GET(*fetch_ind_rec);
    max_pkts = HTT_T2H_TX_FETCH_IND_MAX_PKTS_GET(*fetch_ind_rec);
    max_bytes = *(fetch_ind_rec + 1);
    tx_bytes = 0;

    PFLOW_CTRL_PDEV_STATS_ADD(pdev, DEQUEUE_REQ_BYTECNT, max_bytes);
    PFLOW_CTRL_PDEV_STATS_ADD(pdev, DEQUEUE_REQ_PKTCNT, max_pkts);

#if PEER_FLOW_CONTROL_DEBUG
    qdf_print("%s peer_id = %d tid = %d max_pkts = %d max_bytes = %d \n", __func__, peer_id, tid, max_pkts, max_bytes);
#endif

    peer = ol_txrx_peer_find_by_id(pdev, peer_id);

    if(!peer) {
        TXRX_PRINT(TXRX_PRINT_LEVEL_INFO1,
                "Couldn't find peer from ID %d - %s\n", peer_id, __func__);
        return OL_TX_PFLOW_CTRL_DEQ_DONE;
    }

    PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_DEQUEUE_REQ_BYTECNT, max_bytes);
    PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_DEQUEUE_REQ_PKTCNT, max_pkts);

    tx_pkts = 0;
    status = OL_TX_PFLOW_CTRL_DEQ_DONE;

    if (!qdf_nbuf_is_queue_empty(&peer->tidq[OL_TX_PFLOW_CTRL_PRIORITY_TID].nbufq)) {
        status = ol_tx_pflow_dequeue_priority_tid(pdev, peer, peer_id, OSDEP_EAPOL_TID);
        if(status == OL_TX_PFLOW_CTRL_DEQ_DESC_FAIL)
            goto out;
    }

    while (tx_pkts <= max_pkts) {

        OL_TX_PEER_LOCK(pdev, peer_id);

        if (qdf_nbuf_is_queue_empty(&peer->tidq[tid].nbufq)) {
            OL_TX_PEER_UNLOCK(pdev, peer_id);
            status = OL_TX_PFLOW_CTRL_DEQ_REMAINING;
            goto out;
        }

        netbuf = qdf_nbuf_queue_first(&peer->tidq[tid].nbufq);

        if (netbuf) {
            vdev = (ol_txrx_vdev_handle )qdf_nbuf_get_vdev_ctx(netbuf);
            qdf_assert_always(vdev);
            qdf_assert_always(vdev->pdev);

            ftype = qdf_nbuf_get_ftype(netbuf);
            if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
#if HOST_SW_TSO_SG_ENABLE
                if(ftype == CB_FTYPE_TSO_SG) {
                    len = ol_tx_tso_sg_get_nbuf_len(netbuf);
                } else
#endif
                {
                    len = qdf_nbuf_len(netbuf);
                }
            } else {
                len = ol_tx_rawmode_nbuf_len(netbuf);
            }

            tx_bytes += len;

            if (tx_bytes > max_bytes) {
                tx_bytes -= len;
                OL_TX_PEER_UNLOCK(pdev, peer_id);
                goto out;
            }

            tx_desc = ol_tx_desc_alloc(pdev);
            if (tx_desc) {
                if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
                    netbuf = qdf_nbuf_queue_remove(&peer->tidq[tid].nbufq);
#if HOST_SW_TSO_SG_ENABLE
                    if(ftype == CB_FTYPE_TSO_SG) {
                        pkt_len = ol_tx_tso_sg_get_nbuf_len(netbuf);
                    } else
#endif
                    {
                        pkt_len = qdf_nbuf_len(netbuf);
                    }
                } else {
                    pkt_len = ol_tx_rawmode_deque_pflow_ctl_helper(pdev, peer, netbuf, tid);
                }

                if (pdev->prof_trigger) {
                    delta_msecs  = ktime_to_ms(net_timedelta(netbuf->tstamp));

                    PFLOW_CTRL_PDEV_STATS_ADD(pdev, QUEUE_OCCUPANCY, delta_msecs);

                    if(delta_msecs < 2)
                        PFLOW_CTRL_PDEV_STATS_ADD(pdev, QUEUE_OCCUPANCY_BIN2, 1);
                    else if(delta_msecs < 8)
                        PFLOW_CTRL_PDEV_STATS_ADD(pdev, QUEUE_OCCUPANCY_BIN8, 1);
                    else if(delta_msecs < 16)
                        PFLOW_CTRL_PDEV_STATS_ADD(pdev, QUEUE_OCCUPANCY_BIN16, 1);
                    else if(delta_msecs < 32)
                        PFLOW_CTRL_PDEV_STATS_ADD(pdev, QUEUE_OCCUPANCY_BIN32, 1);
                    else
                        PFLOW_CTRL_PDEV_STATS_ADD(pdev, QUEUE_OCCUPANCY_BINHIGH, 1);
                }

                peer->tidq[tid].byte_count -= pkt_len;
                pdev->pflow_ctl_total_dequeue_cnt++;
                pdev->pflow_ctl_global_queue_cnt--;
                peer->tidq[tid].dequeue_cnt++;
                tx_pkts++;
                peer->tidq[tid].avg_size_compute_cnt++;

                ol_tx_pflow_ctrl_queue_map_update(pdev, peer, peer_id, tid);

                OL_TX_PEER_UNLOCK(pdev, peer_id);

                PFLOW_CTRL_PDEV_STATS_ADD(pdev, DEQUEUE_CNT, 1);
                PFLOW_CTRL_PDEV_STATS_ADD(pdev, DEQUEUE_BYTECNT, pkt_len);
                PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_DEQUEUE_CNT, 1);
                PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_DEQUEUE_BYTECNT, pkt_len);



                if ( ( ftype == CB_FTYPE_MCAST2UCAST)
#if HOST_SW_TSO_SG_ENABLE
                        || ( ftype == CB_FTYPE_TSO_SG )
#endif
#if HOST_SW_SG_ENABLE
                        || ( ftype == CB_FTYPE_SG)
#endif
                   ) {
                    /*
                     * Before sending the mcast2ucast packet through CE_Send_fast,
                     * send the packets in the backlog
                     */
                    if(hnetbuf) {
                        hif_batch_send(pdev->htt_pdev->osc, hnetbuf, vdev->epid, vdev->downloadlen, 0);
                        tnetbuf = NULL;
                        hnetbuf = NULL;
                    }

                    pkt_download_len = ((struct htt_pdev_t *)(pdev->htt_pdev))->download_len;
                    ep_id = HTT_EPID_GET(pdev->htt_pdev);

                    status = ol_tx_prepare_ll_fast(pdev, vdev, netbuf, pkt_download_len, ep_id, tx_desc);

                    if((status == OL_TXRX_SUCCESS) && (peer_id != HTT_INVALID_PEER)){
                        htt_tx_desc_set_peer_id((u_int32_t *)(tx_desc->htt_tx_desc), peer_id);
                    }

                    if(tid != HTT_TX_EXT_TID_INVALID) {
                        htt_tx_desc_tid(pdev->htt_pdev, tx_desc->htt_tx_desc, tid);
                    }

#if ATH_DATA_TX_INFO_EN
                    if(scn->enable_perpkt_txstats) {
                        QDF_NBUF_CB_TX_SUBMIT_TS(netbuf) = OS_GET_TIMESTAMP();
                    }
#endif

                    if ((status == OL_TXRX_SUCCESS) &&
                            (1 == hif_send_fast(pdev->htt_pdev->osc, netbuf, ep_id, pkt_download_len))) {

                    } else {
                            /*Failed to send frame, free all associated memory*/
                        if (ftype == CB_FTYPE_MCAST2UCAST) {
                            TXRX_STATS_ADD(pdev, pub.mcast_enhance.num_me_dropped_i, 1);
                            ol_tx_me_free_buf(pdev, (struct ol_tx_me_buf_t *) qdf_nbuf_get_fctx(netbuf));
                        }
                        ol_tx_desc_free(pdev, tx_desc);
                        qdf_nbuf_free(netbuf);
                    }
                } else { /* Not a MCAST2UCAST/TSO_SG/SG packet */
                    if (ol_tx_ll_cachedhdr_prep(vdev, netbuf, peer_id, tid, tx_desc)) {
                        ol_tx_desc_free(pdev, tx_desc);
                        continue;
                    }
                    if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
                        qdf_nbuf_set_next(netbuf, NULL);

                        if (tnetbuf == NULL) {
                            tnetbuf = netbuf;
                            hnetbuf = netbuf;
                        } else {
                            qdf_nbuf_set_next(tnetbuf, netbuf);
                            tnetbuf = netbuf;
                        }

                    } else {

                        /* in Mixed VAP case existing packets added to the list
                            for other VAP , get transmitted before raw mode packet is sent.*/
                        if(hnetbuf) {
                            hif_batch_send(pdev->htt_pdev->osc, hnetbuf, vdev->epid, vdev->downloadlen, 0);
                            hnetbuf = NULL;
                            tnetbuf = NULL;
                        }

                        pkt_download_len = vdev->downloadlen;
                        if (len + HTC_HTT_TRANSFER_HDRSIZE < pkt_download_len) {
                            pkt_download_len = len + HTC_HTT_TRANSFER_HDRSIZE;
                        }
#if ATH_DATA_TX_INFO_EN
                        if(scn->enable_perpkt_txstats) {
                            QDF_NBUF_CB_TX_SUBMIT_TS(netbuf) = OS_GET_TIMESTAMP();
                        }
#endif
                        if(hif_send_single(pdev->htt_pdev->osc, netbuf, vdev->epid, pkt_download_len)){
                            qdf_nbuf_t tmpbuf = NULL;
                            ol_tx_desc_free(pdev, tx_desc);
                            while (netbuf != NULL) {
                                qdf_nbuf_unmap_single( vdev->osdev, (qdf_nbuf_t) netbuf, QDF_DMA_TO_DEVICE);
                                tmpbuf = netbuf;
                                netbuf = netbuf->next;
                                qdf_nbuf_free(tmpbuf);
                            }
                        }
                    }
                }
            } else { /* if (!tx_desc)  */

                TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
                TXRX_PRINT(TXRX_PRINT_LEVEL_INFO2,
                        "%s desc allocation failed \n", __func__);
                status = OL_TX_PFLOW_CTRL_DEQ_DESC_FAIL;
                PFLOW_CTRL_PDEV_STATS_ADD(pdev, TX_DESC_FAIL, 1);
                /*update to per-peer stats too*/
                PFLOW_CTRL_PEER_STATS_ADD(peer, tid, PEER_TX_DESC_FAIL, 1);
                OL_TX_PEER_UNLOCK(pdev, peer_id);
                goto out;
            }
        } else { /* if (!netbuf) */
            if (!qdf_nbuf_is_queue_empty(&peer->tidq[tid].nbufq))  {
                qdf_print("%s queue not empty %d , but nbuf NULL \n", __func__, qdf_nbuf_queue_len(&peer->tidq[tid].nbufq));
            } else {
                QUEUE_MAP_CNT_RESET(pdev, peer_id, tid);
            }

            TXRX_PRINT(TXRX_PRINT_LEVEL_INFO1,
                    "%s no packets in queue \n", __func__);
            status = OL_TX_PFLOW_CTRL_DEQ_REMAINING;
            OL_TX_PEER_UNLOCK(pdev, peer_id);
            goto out;
        }
    }

out:
    if(hnetbuf)
        hif_batch_send(pdev->htt_pdev->osc, hnetbuf, vdev->epid, vdev->downloadlen, 0);

    *num_tx_pkts = tx_pkts;
    *num_tx_bytes = tx_bytes;


    return status;
}


void ol_tx_ll_pflow_sched(struct ol_txrx_pdev_t *pdev, uint16_t peer_id, uint8_t tid, uint32_t num_pkts)
{
    uint32_t fetch_ind_record[2];
    uint32_t dummy = 0;

    fetch_ind_record[0] = (peer_id | (tid << 12) | (num_pkts << 16));
    fetch_ind_record[1] = 0xffffffff;

    ol_tx_pflow_ctrl_dequeue_send(pdev,fetch_ind_record, &dummy, &dummy);

    return;
}

u_int32_t
ol_tx_ll_fetch_sched(struct ol_txrx_pdev_t *pdev, uint32_t num_records, uint32_t *fetch_ind_record, qdf_nbuf_t fetch_resp_buf)
{
    int i;
    u_int32_t status;
    u_int32_t num_tx_bytes;
    u_int32_t num_tx_pkts;
    u_int32_t *fetch_resp_record;
    u_int32_t *curr_record;
    u_int16_t peerid, tid;

    i = 0;
    num_tx_pkts = 0;
    num_tx_bytes = 0;
    curr_record = fetch_ind_record;
    OL_TX_PDEV_LOCK(&pdev->tx_lock);
    while(i < num_records) {
        peerid = HTT_T2H_TX_FETCH_IND_PEERID_GET(*curr_record);
        tid    = HTT_T2H_TX_FETCH_IND_TIDNO_GET(*curr_record);

        if(unlikely((peerid >= OL_TXRX_MAX_PEER_IDS) || (tid >= OL_TX_PFLOW_CTRL_MAX_TIDS))) {
            TXRX_PRINT(TXRX_PRINT_LEVEL_INFO1,
                    ("%s peer_id %d or tid %d out of range \n", __func__, peerid, tid));
            goto out;
        }

        status = ol_tx_pflow_ctrl_dequeue_send(pdev, curr_record, &num_tx_pkts, &num_tx_bytes);

        fetch_resp_record = (u_int32_t *) qdf_nbuf_put_tail(fetch_resp_buf,8);
        if (fetch_resp_record == NULL) {
            qdf_print("%s: Failed to expand head\n", __func__);
            goto out;
        }

        *fetch_resp_record = (peerid & HTT_H2T_TX_FETCH_RESP_PEERID_M) |
            ((tid << HTT_H2T_TX_FETCH_RESP_TIDNO_S ) & HTT_H2T_TX_FETCH_RESP_TIDNO_M) |
            ((num_tx_pkts << HTT_H2T_TX_FETCH_RESP_NUM_PKTS_S) & HTT_H2T_TX_FETCH_RESP_NUM_PKTS_M);

        *(fetch_resp_record + 1)  =  num_tx_bytes;

#if PEER_FLOW_CONTROL_DEBUG
        printk("%s dequeued %d tx_pkts and %d tx_bytes \n", num_tx_pkts, num_tx_bytes);
#endif

        if (unlikely(status == OL_TX_PFLOW_CTRL_DEQ_DESC_FAIL)) {
            TXRX_PRINT(TXRX_PRINT_LEVEL_INFO2,
                    "%s num_requested not existing \n", __func__);
            goto out;
        }

        i++;
        curr_record += 2;
    }

out:
    OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
    return i;
}
#else /* PEER_FLOW_CONTROL */

void
ol_tx_ll_sched(struct ol_txrx_pdev_t *pdev)
{
    struct ol_tx_desc_t *tx_desc = NULL;
    ol_txrx_vdev_handle vdev;
    qdf_nbuf_t netbuf = NULL;
    uint32_t pkt_download_len;
    uint32_t ep_id, status;
    uint16_t msdu_len;
    uint8_t ftype;
    struct ol_ath_softc_net80211 *scn = (struct ol_ath_softc_net80211 *)(pdev->ctrl_pdev);
    struct ieee80211com *ic = &(scn->sc_ic);

    qdf_nbuf_t tnetbuf = NULL;
    qdf_nbuf_t hnetbuf = NULL;

    do {
        OL_TX_FLOW_CTRL_LOCK(&pdev->acnbufqlock);
        if (pdev->acqcnt) {
            tx_desc = ol_tx_desc_alloc(pdev);
            if (tx_desc) {
                netbuf = qdf_nbuf_queue_remove(&pdev->acnbufq);
                pdev->acqcnt--;
            } else {
                TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
                OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
                break;
            }
        }
        OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);

        if (netbuf) {
            vdev = (ol_txrx_vdev_handle )qdf_nbuf_get_vdev_ctx(netbuf);

            qdf_assert_always(vdev);
            qdf_assert_always(vdev->pdev);
            ftype = qdf_nbuf_get_ftype(netbuf);

            if (ftype == CB_FTYPE_MCAST2UCAST) {
                struct ol_tx_me_buf_t *mc_uc_buf = (struct ol_tx_me_buf_t *)
                    qdf_nbuf_get_fctx(netbuf);

                /*
                 * Before sending the mcast2ucast packet through CE_Send_fast,
                 * send the packets in the backlog
                */
                if(hnetbuf) {
                    hif_batch_send(pdev->htt_pdev->osc, hnetbuf, vdev->epid, vdev->downloadlen, 0);
                    tnetbuf = NULL;
                    hnetbuf = NULL;
                }

                pkt_download_len = ((struct htt_pdev_t *)(pdev->htt_pdev))->download_len;

                ep_id = HTT_EPID_GET(pdev->htt_pdev);

                status = ol_tx_prepare_ll_fast(pdev, vdev, netbuf, pkt_download_len, ep_id, tx_desc);
#if ATH_DATA_TX_INFO_EN
                if(scn->enable_perpkt_txstats) {
                    QDF_NBUF_CB_TX_SUBMIT_TS(netbuf) = OS_GET_TIMESTAMP();
                }
#endif
                if ((status == OL_TXRX_SUCCESS) &&
                        (1 == hif_send_fast(pdev->htt_pdev->osc, netbuf, ep_id, pkt_download_len))) {
                    /*Succesfully sent frame. It will be freed in tx_complete*/
                    continue;
                } else {
                    /*Failed to send frame, free all associated memory*/
                    if (ftype == CB_FTYPE_MCAST2UCAST) {
                        TXRX_STATS_ADD(pdev, pub.mcast_enhance.num_me_dropped_i, 1);
                        ol_tx_me_free_buf(pdev, mc_uc_buf);
                    }
                    ol_tx_desc_free(pdev, tx_desc);
                    qdf_nbuf_free(netbuf);
                }
                continue;
            }

            if (ol_tx_ll_cachedhdr_prep(vdev, netbuf, HTT_INVALID_PEER, HTT_TX_EXT_TID_INVALID, tx_desc)) {
                ol_tx_desc_free(pdev, tx_desc);
                continue;
            }

            msdu_len =  qdf_nbuf_len(netbuf);
            pkt_download_len = vdev->downloadlen;
            if ( msdu_len + vdev->htc_htt_hdr_size < pkt_download_len) {
                /*
                 * This case of packet length being less than the nominal download
                 * length can happen for a couple reasons:
                 * In HL, the nominal download length is a large artificial value.
                 * In LL, the frame may not have the optional header fields
                 * accounted for in the nominal download size (LLC/SNAP header,
                 * IPv4 or IPv6 header).
                 */
                pkt_download_len = msdu_len + vdev->htc_htt_hdr_size;
            }

            qdf_nbuf_set_next(netbuf, NULL);
            if (tnetbuf == NULL) {
                tnetbuf = netbuf;
                hnetbuf = netbuf;
            } else {
                qdf_nbuf_set_next(tnetbuf, netbuf);
                tnetbuf = netbuf;
            }
        }
    } while (pdev->acqcnt);

    if(hnetbuf)
    {
        hif_batch_send(pdev->htt_pdev->osc, hnetbuf, vdev->epid, vdev->downloadlen, 0);
    }
    if ((pdev->acqcnt_len - pdev->acqcnt) >= OL_TX_FLOW_CTRL_QUEUE_WAKEUP_THRESOLD)
    {
        /* Resume the OS net device if it is paused */
        uint32_t num_vaps = 0;
        ieee80211_iterate_vap_list_internal(ic, ol_wakeup_txq, NULL, num_vaps);
    }
}
#endif /* PEER_FLOW_CONTROL */

#if QCA_OL_TX_PDEV_LOCK
void  ol_ll_pdev_tx_lock(void* ptr){
    struct ol_txrx_vdev_t* vdev = (struct ol_txrx_vdev_t*)ptr;
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    qdf_spin_lock_bh(&pdev->tx_lock);

}

void  ol_ll_pdev_tx_unlock(void* ptr){

    struct ol_txrx_vdev_t* vdev = (struct ol_txrx_vdev_t*)ptr;

    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    qdf_spin_unlock_bh(&pdev->tx_lock);

}
#endif

int
ol_tx_ll_cachedhdr(ol_txrx_vdev_handle vdev,
#if PEER_FLOW_CONTROL
		qdf_nbuf_t netbuf ,uint16_t peer_id, uint8_t tid)
#else

		qdf_nbuf_t netbuf)
#endif
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    uint32_t pkt_download_len, headroom;
    struct ol_tx_desc_t *tx_desc = NULL;
    u_int16_t msdu_len ;

#if !PEER_FLOW_CONTROL
    u_int32_t status;
#endif
    uint32_t arp_src = 0;
    uint32_t arp_dst = 0;
    uint32_t arp_cmd = 0;
    struct iphdr *iph = NULL;
    struct icmphdr *icmp = NULL;
    struct ol_ath_softc_net80211 *scn = (struct ol_ath_softc_net80211 *)pdev->ctrl_pdev;
    osif_dev  *osdev = (osif_dev *)vdev->osif_vdev;
    struct ieee80211vap *vap = osdev->os_if;

    ATH_DEBUG_SET_RTSCTS_ENABLE((osif_dev *)vdev->osif_vdev);

    qdf_nbuf_num_frags_init(netbuf);
    /* Initializing CB_FTYPE with 0 */
    qdf_nbuf_set_fctx_type(netbuf, 0, 0);

    if (qdf_unlikely((headroom = qdf_nbuf_headroom(netbuf))< vdev->htc_htt_hdr_size)){
        netbuf = qdf_nbuf_realloc_headroom(netbuf, vdev->htc_htt_hdr_size);
        if (netbuf == NULL ){
            qdf_print("Realloc failed \n");
            goto out;
        }
    }

    if (scn->sc_arp_dbg_conf) {
        struct ethhdr * eh;
        eh = (struct ethhdr *)(netbuf->data + vap->mhdr_len);
        iph = (struct iphdr*)(((uint8_t *)eh) + sizeof(struct ethhdr));
        /*
         * Debug prints for ICMP Packets
         */
        if (iph->protocol == IPPROTO_ICMP ) {
            icmp = (struct icmphdr*)(((uint8_t *)iph) + (iph->ihl * 4));
            if ((iph->saddr == ntohl(scn->sc_arp_dbg_srcaddr)) && (iph->daddr == ntohl(scn->sc_arp_dbg_dstaddr))) {
                printk("%s: ICMP type: %d: Sequence: %u From Source Address: 0x%08x To Destination Address: 0x%08x \n", __func__, icmp->type, icmp->un.echo.sequence
                        , iph->saddr , iph->daddr);
            }
        }

        /*
         * Deriving the source address and dest address from netbuf->data
         *
         */
#define ARP_SRC_OFFSET 28
#define ARP_DESTINATION_OFFSET 40
#define ARP_DEST_SHIFT 16
#define ARP_DEST_SHIFT_OFFSET 38
#define ARP_CMD_OFFSET 20

        arp_src = *(uint32_t *)((uint8_t *)netbuf->data + ARP_SRC_OFFSET + vap->mhdr_len);
        arp_dst = *(uint16_t *)((uint8_t *)netbuf->data + ARP_DESTINATION_OFFSET + vap->mhdr_len);
        arp_dst <<= ARP_DEST_SHIFT;
        arp_dst |= *(uint16_t *)((uint8_t *)netbuf->data + ARP_DEST_SHIFT_OFFSET + vap->mhdr_len);
        arp_cmd = *(uint16_t *)((uint8_t *)netbuf->data + ARP_CMD_OFFSET + vap->mhdr_len);

#undef ARP_SRC_OFFSET
#undef ARP_DESTINATION_OFFSET
#undef ARP_DEST_SHIFT
#undef ARP_DEST_SHIFT_OFFSET
#undef ARP_CMD_OFFSET

        if (qdf_ntohs(eh->h_proto) == ETHERTYPE_ARP) {
            /* need to add ipaddr filter logic */
#define ARP_REQ 0x100
#define ARP_RESP 0x200

            if ((arp_dst == ntohl(scn->sc_arp_dbg_dstaddr)) && (arp_src == ntohl(scn->sc_arp_dbg_srcaddr))) {
                if( arp_cmd == ARP_REQ) {
                    scn->sc_tx_arp_req_count++;
                    printk("\nARP_DEBUG : In function %s Tx ARP Request Packet from SourceIP %x to destination IP %x, ethertype %x \n", __func__,ntohl(scn->sc_arp_dbg_srcaddr),ntohl(scn->sc_arp_dbg_dstaddr), qdf_ntohs(eh->h_proto));
                }
                else if (arp_cmd == ARP_RESP) {
                    scn->sc_tx_arp_resp_count++;
                    printk("\nARP_DEBUG : In function %s Tx ARP Response Packet from SourceIP %x to destination IP %x, ethertype %x \n",
                            __func__, ntohl(scn->sc_arp_dbg_srcaddr), ntohl(scn->sc_arp_dbg_dstaddr), qdf_ntohs(eh->h_proto));

                }
            }
#undef ARP_REQ
#undef ARP_RESP
        }
    }

#if !PEER_FLOW_CONTROL
    /* flow ctrl for RAW mode is TBD */
    if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw))
    {
        /*
         * check if flow ctrl queue is empty or not, if not empty
         * it indicates that the tx_descriptors are exhausted,
         * hence enqueue the pkt to flow ctrl queue and return.
         * if flow ctrl queue is empty go ahead and allocate a
         * tx_descriptor.
         */
        qdf_nbuf_set_vdev_ctx(netbuf, (void *)vdev);
        status = ol_tx_check_enqueue_flow_ctrl(pdev, netbuf);
        if (qdf_likely(status == OL_TX_FLOW_CTRL_ENQ_DONE) ||
                (status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD)) {
            if(qdf_unlikely(status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD))
            {
                /* request OS not to forward more packets untill requested */
                netif_stop_queue(((osif_dev *)vdev->osif_vdev)->netdev);
            }
            goto out;
        } else if (status == OL_TX_FLOW_CTRL_Q_FULL) {
            /* Normally, It should not land here */
            ol_vap_txdiscard_stats_update(vdev->osif_vdev, netbuf);
            qdf_nbuf_free(netbuf);
            goto out;
        }
    }
#endif

    tx_desc = ol_tx_desc_alloc(pdev);

    if (qdf_unlikely(!tx_desc)) {
#if ATH_TX_OVERFLOW_IND
        ol_vap_send_overflow_event(vdev);
#endif
#if !PEER_FLOW_CONTROL
        /* flow ctrl for RAW mode is TBD */
        if (OL_CFG_NONRAW_TX_LIKELINESS(vdev->tx_encap_type != htt_pkt_type_raw)) {
            /*
             * enqueue the pkt to flow ctrl queue as tx_descriptor
             * allocation failed
             */
            qdf_nbuf_set_vdev_ctx(netbuf,(void *)vdev);
            status = ol_tx_enqueue_flow_ctrl(pdev, netbuf);
            if (qdf_likely(status == OL_TX_FLOW_CTRL_ENQ_DONE) ||
                    (status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD)) {
                if(qdf_unlikely(status == OL_TX_FLOW_CTRL_ENQ_DONE_PAUSE_THRESOLD))
                {
                    /* request OS not to forward more packets untill requested */
                    netif_stop_queue(((osif_dev *)vdev->osif_vdev)->netdev);
                }
                goto out;
            } else if (status == OL_TX_FLOW_CTRL_Q_FULL) {
                /* Normally, It should not land here */
                ol_vap_txdiscard_stats_update(vdev->osif_vdev, netbuf);
                qdf_nbuf_free(netbuf);
                goto out;
            }
        }
        else
#endif
        {
            TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
#if !PEER_FLOW_CONTROL
            qdf_nbuf_free(netbuf);
            goto out;
#else
            return OL_TXRX_FAILURE;
#endif

        }
    }

    TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_avoid, 1);

#if !PEER_FLOW_CONTROL
    if (ol_tx_ll_cachedhdr_prep(vdev, netbuf, HTT_INVALID_PEER, HTT_TX_EXT_TID_INVALID, tx_desc)) {
#else
    if (ol_tx_ll_cachedhdr_prep(vdev, netbuf, peer_id, tid, tx_desc)) {
#endif
        ol_tx_desc_free(pdev, tx_desc);
        goto out ;
    }

    msdu_len =  qdf_nbuf_len(netbuf);
    pkt_download_len = vdev->downloadlen;

    if ( msdu_len + vdev->htc_htt_hdr_size < pkt_download_len) {
        /*
         * This case of packet length being less than the nominal download
         * length can happen for a couple reasons:
         * In HL, the nominal download length is a large artificial value.
         * In LL, the frame may not have the optional header fields
         * accounted for in the nominal download size (LLC/SNAP header,
         * IPv4 or IPv6 header).
         */
        pkt_download_len = msdu_len + vdev->htc_htt_hdr_size;
    }
#if ATH_DATA_TX_INFO_EN
    if(scn->enable_perpkt_txstats) {
        QDF_NBUF_CB_TX_SUBMIT_TS(netbuf) = OS_GET_TIMESTAMP();
    }
#endif
    if(hif_send_single(pdev->htt_pdev->osc, netbuf, vdev->epid, pkt_download_len)){
        ol_tx_desc_free(pdev, tx_desc);
        qdf_nbuf_unmap_single( vdev->osdev, (qdf_nbuf_t) netbuf, QDF_DMA_TO_DEVICE);
        ol_vap_txdiscard_stats_update(vdev->osif_vdev, netbuf);
        qdf_nbuf_free(netbuf);
    }

out :
    ATH_HYFI_NOTIFY_LOW_ON_BUFFER(pdev->ctrl_pdev, (pdev->tx_desc.pool_size - pdev->stats.pub.tx.desc_in_use));
    return OL_TXRX_SUCCESS;
}

qdf_nbuf_t ol_tx_reinject_cachedhdr(
        struct ol_txrx_vdev_t *vdev,
        qdf_nbuf_t msdu, uint32_t peer_id)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    uint32_t peer_id_classify;

#if QCA_PARTNER_DIRECTLINK_TX
    if (CE_is_directlink(pdev->ce_tx_hdl)) {
        ol_tx_partner(msdu, OSIF_TO_NETDEV(vdev->osif_vdev), peer_id);
    } else
#endif /* QCA_PARTNER_DIRECTLINK_TX */
    {
        struct ol_tx_desc_t *tx_desc = NULL;

#if PEER_FLOW_CONTROL
        struct ol_tx_nbuf_classify nbuf_class;
#endif

        peer_id_classify = peer_id;

#if PEER_FLOW_CONTROL
        if (peer_id_classify == HTT_INVALID_PEER) {
            ol_tx_classify(vdev, msdu, &nbuf_class);

            if(nbuf_class.peer_id == HTT_INVALID_PEER){
                qdf_nbuf_free(msdu);
                goto out ;
            }

            peer_id_classify = nbuf_class.peer_id;
        }
#endif

        tx_desc = ol_tx_desc_alloc(pdev);
        if (qdf_unlikely(!tx_desc)) {
            TXRX_STATS_ADD(pdev, pub.tx.desc_alloc_fails, 1);
            ol_vap_txdiscard_stats_update(vdev->osif_vdev, msdu);
            qdf_nbuf_free(msdu);
            goto out ;
        }

        if (ol_tx_ll_cachedhdr_prep(vdev, msdu, peer_id_classify, HTT_TX_EXT_TID_INVALID, tx_desc)) {
            ol_tx_desc_free(pdev, tx_desc);
            goto out;
        }

        if(hif_send_single(pdev->htt_pdev->osc, 	msdu,	vdev->epid, vdev->downloadlen)){
            ol_tx_desc_free(pdev, tx_desc);
            qdf_nbuf_unmap_single( vdev->osdev, (qdf_nbuf_t)msdu, QDF_DMA_TO_DEVICE);
            qdf_nbuf_free(msdu);
        }
    }
out :
    return OL_TXRX_SUCCESS;
}

#endif /* QCA_OL_TX_CACHEDHDR*/
#endif /* WLAN_FEATURE_FASTPATH */

static inline int
OL_TXRX_TX_IS_RAW(enum ol_txrx_osif_tx_spec tx_spec)
{
    return
        tx_spec &
        (ol_txrx_osif_tx_spec_raw |
         ol_txrx_osif_tx_spec_no_aggr |
         ol_txrx_osif_tx_spec_no_encrypt);
}

static inline u_int8_t
OL_TXRX_TX_RAW_SUBTYPE(enum ol_txrx_osif_tx_spec tx_spec)
{
    u_int8_t sub_type = 0x1; /* 802.11 MAC header present */

    if (tx_spec & ol_txrx_osif_tx_spec_no_aggr) {
        sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_AGGR_S;
    }
    if (tx_spec & ol_txrx_osif_tx_spec_no_encrypt) {
        sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_ENCRYPT_S;
    }
    if (tx_spec & ol_txrx_osif_tx_spect_nwifi_no_encrypt) {
        sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_ENCRYPT_S;
    }
    return sub_type;
}

qdf_nbuf_t
ol_tx_non_std_ll(
    ol_txrx_vdev_handle vdev,
    u_int8_t ext_tid,
    enum ol_txrx_osif_tx_spec tx_spec,
    qdf_nbuf_t msdu_list)
{
    qdf_nbuf_t msdu = msdu_list;
    htt_pdev_handle htt_pdev = vdev->pdev->htt_pdev;

#if HOST_SW_TSO_ENABLE
    /* Detect TSO Enabled packet  and do TCP segmentation
       by cloning of multiple buffers*/

    if(tx_spec & ol_txrx_osif_tx_spec_tso) {
	    TXRX_STATS_MSDU_INCR(vdev->pdev, tx.tso.tso_pkts, msdu_list);
	    msdu = ol_tx_tso_segment(vdev, msdu_list);

	    /* Multiple buffers are created(one per segment) and linked with each other
	     *  for further TX processing,
	     * Original buffer used as header buffer, freed at later stage of TX
	     * If NULL is returned, error encountered in segmentation
	     * else proceed processing of individual TCP segments */
	    if (qdf_unlikely(!msdu))  return NULL;
    }
#endif /* HOST_SW_TSO_ENABLE */

    /*
     * The msdu_list variable could be used instead of the msdu var,
     * but just to clarify which operations are done on a single MSDU
     * vs. a list of MSDUs, use a distinct variable for single MSDUs
     * within the list.
     */
    while (msdu) {
        qdf_nbuf_t next;
        struct ol_tx_desc_t *tx_desc;

#if HOST_SW_TSO_ENABLE
        if (tx_spec & ol_txrx_osif_tx_spec_tso) {
	    /* msdu list is returned back incase of error condition,
	     * same needs to be returned back to the caller for freeing */
	    if( ol_tx_tso_prepare_ll(&tx_desc, vdev, msdu) )
		    return msdu;
	    if(qdf_unlikely(tx_desc == NULL))
		    return msdu;
    } else
#endif /* HOST_SW_TSO_ENABLE */

        {
        ol_tx_prepare_ll(tx_desc, vdev, msdu);
        }

        /*
         * The netbuf may get linked into a different list inside the
         * ol_tx_send function, so store the next pointer before the
         * tx_send call.
         */
        next = qdf_nbuf_next(msdu);

        if (tx_spec != ol_txrx_osif_tx_spec_std) {
            if (tx_spec & ol_txrx_osif_tx_spec_tso) {
                tx_desc->pkt_type = ol_tx_frm_tso;
            } else if (tx_spec & ol_txrx_osif_tx_spect_nwifi_no_encrypt) {
                u_int8_t sub_type = OL_TXRX_TX_RAW_SUBTYPE(tx_spec);
                htt_tx_desc_type(
                    htt_pdev, tx_desc->htt_tx_desc,
                    htt_pkt_type_native_wifi, sub_type);
            } else if (OL_TXRX_TX_IS_RAW(tx_spec)) {
                /* different types of raw frames */
                u_int8_t sub_type = OL_TXRX_TX_RAW_SUBTYPE(tx_spec);
                htt_tx_desc_type(
                    htt_pdev, tx_desc->htt_tx_desc,
                    htt_pkt_type_raw, sub_type);
            }
        }
        /* explicitly specify the TID and the limit
         * it to the 0-15 value of the QoS TID and
         * 19 of the non-pause TID.
         */

        if ((ext_tid >= HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST) &&
          (ext_tid != HTT_TX_EXT_TID_NONPAUSE)) {
            ext_tid = HTT_TX_EXT_TID_DEFAULT;
        }
        htt_tx_desc_tid(htt_pdev, tx_desc->htt_tx_desc, ext_tid);

        /*
         * If debug display is enabled, show the meta-data being
         * downloaded to the target via the HTT tx descriptor.
         */
        htt_tx_desc_display(htt_pdev, tx_desc->htt_tx_desc);
        ol_tx_send(vdev, tx_desc, msdu);
        msdu = next;
    }
    return NULL; /* all MSDUs were accepted */
}

qdf_nbuf_t
ol_tx_hl(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
    qdf_nbuf_t msdu;

    /*
     * TBDXXX - TXRX module for now only downloads number
     * of descriptors that target could accept.
     */
    if (qdf_atomic_read(&vdev->pdev->target_tx_credit) <= 0) {
        return msdu_list;
    }

    /*
     * The msdu_list variable could be used instead of the msdu var,
     * but just to clarify which operations are done on a single MSDU
     * vs. a list of MSDUs, use a distinct variable for single MSDUs
     * within the list.
     */
    msdu = msdu_list;
    while (msdu) {
        qdf_nbuf_t next;
        struct ol_tx_desc_t *tx_desc;
        tx_desc = ol_tx_desc_hl(vdev->pdev, vdev, msdu);
        if (! tx_desc) {
            TXRX_STATS_MSDU_LIST_INCR(vdev->pdev, tx.dropped.host_reject, msdu);
            return msdu; /* the list of unaccepted MSDUs */
        }
        OL_TXRX_PROT_AN_LOG(vdev->pdev->prot_an_tx_sent, msdu);
        /*
         * If debug display is enabled, show the meta-data being
         * downloaded to the target via the HTT tx descriptor.
         */
        htt_tx_desc_display(vdev->pdev->htt_pdev, tx_desc->htt_tx_desc);
        /*
         * The netbuf will get stored into a (peer-TID) tx queue list
         * inside the ol_tx_classify_store function, so store the next
         * pointer before the tx_classify_store call.
         */
        next = qdf_nbuf_next(msdu);
/*
 * FIX THIS:
 * 2.  Call tx classify to determine peer and TID.
 * 3.  Store the frame in a peer-TID queue.
 */
//        ol_tx_classify_store(vdev, tx_desc, msdu);
        /*
         * FIX THIS
         * temp FIFO for bringup
         */
        ol_tx_send(vdev, tx_desc, msdu);
        msdu = next;
    }
/*
 * FIX THIS:
 * 4.  Invoke the download scheduler.
 */
//    ol_tx_dl_sched(vdev->pdev);

    return NULL; /* all MSDUs were accepted */
}

qdf_nbuf_t
ol_tx_non_std_hl(
    ol_txrx_vdev_handle data_vdev,
    u_int8_t ext_tid,
    enum ol_txrx_osif_tx_spec tx_spec,
    qdf_nbuf_t msdu_list)
{
    /* FILL IN HERE */
    qdf_assert(0);
    return NULL;
}

void
ol_txrx_mgmt_tx_cb_set(
    ol_txrx_pdev_handle pdev,
    u_int8_t type,
    ol_txrx_mgmt_tx_cb download_cb,
    ol_txrx_mgmt_tx_cb ota_ack_cb,
    void *ctxt)
{
    TXRX_ASSERT1(type < OL_TXRX_MGMT_NUM_TYPES);
    qdf_print(">>>> CB Set %p\n", download_cb);
    pdev->tx_mgmt.callbacks[type].download_cb = download_cb;
    pdev->tx_mgmt.callbacks[type].ota_ack_cb = ota_ack_cb;
    pdev->tx_mgmt.callbacks[type].ctxt = ctxt;
}


int
ol_txrx_mgmt_send(
    ol_txrx_vdev_handle vdev,
    qdf_nbuf_t tx_mgmt_frm,
    u_int8_t type)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    A_UINT8  frm_hdr[HTT_MGMT_FRM_HDR_DOWNLOAD_LEN];
    A_UINT8 *frm_buf;

    frm_buf = (A_UINT8 *)wbuf_header(tx_mgmt_frm);
    OS_MEMCPY(frm_hdr, frm_buf, HTT_MGMT_FRM_HDR_DOWNLOAD_LEN);

    return htt_h2t_mgmt_tx(pdev,
                   tx_mgmt_frm,
                   wbuf_get_pktlen(tx_mgmt_frm),
                   vdev->vdev_id,
                   frm_hdr,
                   vdev);
}

int
ol_txrx_mgmt_send_ext(ol_txrx_vdev_handle vdev,
			 qdf_nbuf_t tx_mgmt_frm,
			 uint8_t type, uint8_t use_6mbps, uint16_t chanfreq)
{
    return 0;
}

void
ol_txrx_sync(ol_txrx_pdev_handle pdev, u_int8_t sync_cnt)
{
    htt_h2t_sync_msg(pdev->htt_pdev, sync_cnt);
}

qdf_nbuf_t ol_tx_reinject(
    struct ol_txrx_vdev_t *vdev,
    qdf_nbuf_t msdu, uint32_t peer_id)
{
#if QCA_PARTNER_DIRECTLINK_TX
    struct ol_txrx_pdev_t *pdev = vdev->pdev;

    if (CE_is_directlink(pdev->ce_tx_hdl)) {
        ol_tx_partner(msdu, OSIF_TO_NETDEV(vdev->osif_vdev), peer_id);
    } else
#endif /* QCA_PARTNER_DIRECTLINK_TX */

{
	struct ol_tx_desc_t *tx_desc;
    ol_tx_prepare_ll(tx_desc, vdev, msdu);
    HTT_TX_DESC_POSTPONED_SET(*((u_int32_t *)(tx_desc->htt_tx_desc)), TRUE);

    htt_tx_desc_set_peer_id((u_int32_t *)(tx_desc->htt_tx_desc), peer_id);

    ol_tx_send(vdev, tx_desc, msdu);
}

    return NULL;
}

#if PEER_FLOW_CONTROL

char *pflow_ctrl_pdev_stats_str[] =  {
    "HTTFetch",
    "HTTFetch_NoId",
    "HTTFetch_Conf",
    "HTTDesc_Alloc",
    "HTTDesc_Alloc_Fail",
    "HTTFetch_Resp",
    "Dequeue_Cnt",
    "Enqueue_Cnt",
    "Queue_Bypass_Cnt",
    "Enqueue_Fail_Cnt",
    "Dequeue_Bytecnt",
    "Enqueue_Bytecnt",
    "Peer_q_Full",
    "Dequeue_Req_Bytecnt",
    "Dequeue_Req_Cnt",
    "Queue_Depth",
    "Tx_Compl_ind",
    "Tx_Desc_fail",
    /* Add all Latency at the end,
     *  bcoz formmating will be different
     */
    "Intr_Taskletbin500",
    "Intr_Taskletbin1000",
    "Intr_Taskletbin2000",
    "Intr_Taskletbin4000",
    "Intr_Taskletbin6000",
    "Intr_Taskletbinhigh",
    "HTTFetchbin500",
    "HTTFetchbin1000",
    "HTTFetchbin2000",
    "HTTFetchbinhigh",
    "Queue_Occupancy",
    "Queue_Occupancy_Bin2",
    "Queue_Occupancy_Bin8",
    "Queue_Occupancy_Bin16",
    "Queue_Occupancy_Bin32",
    "Queue_Occupancy_Binhigh",
};

char *pflow_ctrl_peer_stats_str[] = {
    "Enqueue_Cnt",
    "Dequeue_Cnt",
    "Dequeue_Bytecnt",
    "Enqueue_Bytecnt",
    "Dequeue_Req_bytecnt",
    "Dequeue_Req_pktcnt",
    "Peer_q_full",
    "Msdu_TTL_Expiry",
    "Hol_Cnt",
    "Peer_Tx_Desc_Fail",
};

void
pflow_ctl_display_pdev_stats(struct ol_txrx_pdev_t *pdev)
{
    int i;
    for(i=0;i < PFLOW_CTRL_GLOBAL_STATS_MAX;i++) {
        qdf_print("%s ==>: %d\n",pflow_ctrl_pdev_stats_str[i], pdev->pstats[i].value);
        pdev->pstats[i].value = 0;
    }
}

void
pflow_ctl_display_tidq_stats(struct ol_txrx_peer_t *peer, u_int8_t tid)
{
    int i;
    for(i=0;i < PFLOW_CTRL_TIDQ_STATS_MAX;i++) {
        qdf_print("%s :  %d\n",pflow_ctrl_peer_stats_str[i], peer->tidq[tid].stats[i]);
        peer->tidq[tid].stats[i] = 0;
    }
    return;
}

void
ol_tx_flow_ctrl_tidq_stats(struct ol_txrx_pdev_t *pdev, u_int32_t peer_id, u_int8_t tid)
{
    struct ol_txrx_peer_t *peer;
    peer = ol_txrx_peer_find_by_id(pdev, peer_id);
    if (!peer) {
        qdf_print("%s peer=NULL %d\n", __func__, peer_id);
        return;
    }
    qdf_print("----------------------\n");
    qdf_print(" peer_id %d, tid %d\n", peer_id, tid);
    qdf_print("---------------+------\n");
    /*
    This byte count is actually an encoded byte count; Top 2 bits are to represent the quantum multiple that the remaining 6 bits hold.
     0 – 64 byte quantum, 1 – 512 byte quantum, 2 – 1024 byte quantum, 3 – 2048 byte quantum.
    This is sent in queue map to target firmware and hence this compression.
    */
#ifdef BIG_ENDIAN_HOST
    qdf_print("encoded_byte_cnt   | 0x%x\n", QUEUE_MAP_BYTE_CNT_GET(pdev,peer_id,tid));
#else
    qdf_print("encoded_byte_cnt   | 0x%x\n", pdev->pmap->byte_cnt[tid][peer_id]);
#endif
    qdf_print("bytes         | %u\n", peer->tidq[tid].byte_count);
    qdf_print("queue_len     | %d\n", qdf_nbuf_queue_len(&peer->tidq[tid].nbufq));
    qdf_print("max_queue_len | %d\n", pdev->pflow_ctl_queue_max_len[peer_id][tid]);
    qdf_print("high_watermark| %u\n", peer->tidq[tid].high_watermark);
    pflow_ctl_display_tidq_stats(peer, tid);
    return;
}

void
ol_tx_pflow_ctrl_stats_timer(void *arg)
{
    struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *) arg;
    u_int32_t peer_bitmap, peer_id;
    u_int8_t bitmap_idx, bit_idx, tid;

    qdf_print("------- Global Stats -----------------\n");
    pflow_ctl_display_pdev_stats(pdev);

    qdf_print("------- per-Peer/TIDQ Stats ----------------\n");
    tid = 0;
    while(tid < OL_TX_PFLOW_CTRL_MAX_TIDS) {

        for (bitmap_idx = 0; bitmap_idx < (OL_TXRX_MAX_PEER_IDS >> 5); bitmap_idx++) {
            peer_bitmap = pdev->pmap->tid_peermap[tid][bitmap_idx];
            if (peer_bitmap) {
                for (bit_idx = 0; bit_idx < 32; bit_idx++) {
                    if (peer_bitmap & (1 << bit_idx)) {
                        peer_id = (bitmap_idx << 5) + bit_idx;
                        ol_tx_flow_ctrl_tidq_stats(pdev, peer_id, tid);
                    }
                }
            }
        }
        tid++;
    }

    qdf_print("---------------------------------\n");
    qdf_timer_mod(&pdev->pflow_ctl_stats_timer, pdev->pflow_ctl_stats_timer_interval);
}

void
ol_tx_pflow_ctrl_cong_ctrl_timer(void *arg)
{
    struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *) arg;
    struct ol_txrx_peer_t *peer;
    u_int32_t peer_bitmap, peer_id, queue_max_len;
    u_int8_t bitmap_idx, bit_idx, tid;
    u_int32_t total_dequeue_cnt = pdev->pflow_ctl_total_dequeue_cnt;

    pdev->pflow_ttl_cntr++;
    if(!pdev->pflow_ctl_global_queue_cnt) {
        goto out;
    }

    if(total_dequeue_cnt) {
        pdev->pflow_ctl_total_dequeue_cnt  = 0;
        pdev->pflow_ctl_total_dequeue_byte_cnt  = 0;
    }

    tid = 0;
    while(tid < OL_TX_PFLOW_CTRL_MAX_TIDS) {
        for (bitmap_idx = 0; bitmap_idx < (OL_TXRX_MAX_PEER_IDS >> 5); bitmap_idx++) {
            peer_bitmap = pdev->pmap->tid_peermap[tid][bitmap_idx];
            if (peer_bitmap) {
                for (bit_idx = 0; bit_idx < 32; bit_idx++) {
                    if (peer_bitmap & (1 << bit_idx)) {
                        peer_id = (bitmap_idx << 5) + bit_idx;
                        peer = ol_txrx_peer_find_by_id(pdev, peer_id);
                        if (!peer) {
                            continue;
                        }

                        if(total_dequeue_cnt) {
                            queue_max_len  =
                                ((pdev->pflow_ctl_max_buf_global * peer->tidq[tid].dequeue_cnt)/total_dequeue_cnt);
                            queue_max_len  = MIN(pdev->pflow_ctl_max_queue_len, queue_max_len);
                            queue_max_len  = MAX(pdev->pflow_ctl_min_queue_len, queue_max_len);
                            pdev->pflow_ctl_queue_max_len[peer_id][tid] = queue_max_len;
                            peer->tidq[tid].dequeue_cnt = 0;
                        }


                        /* TTL Handler */
                        if (pdev->pflow_ttl_cntr == pdev->pflow_msdu_ttl) {
                            ol_tx_pflow_ctrl_ttl(pdev, peer_id, tid);
                        }

                    }
                }
            }
        }
        tid++;
    }

out:
    if (pdev->pflow_ttl_cntr == pdev->pflow_msdu_ttl) {
        pdev->pflow_ttl_cntr = 0;
    }
    if (pdev->pflow_cong_ctrl_timer_interval) {
        qdf_timer_mod(&pdev->pflow_ctl_cong_timer, pdev->pflow_cong_ctrl_timer_interval);
    }
}

void ol_txrx_per_peer_stats(struct ol_txrx_pdev_t *pdev, char *addr)
{
    struct ol_txrx_ast_entry_t *ast_entry;
    u_int16_t peer_id=HTT_INVALID_PEER;
    u_int16_t tid;
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
    int status;
#endif
    struct ieee80211_node *ni = NULL;
    struct ol_ath_softc_net80211 *scn = (struct ol_ath_softc_net80211 *)(pdev->ctrl_pdev);
    struct ieee80211com *ic = &(scn->sc_ic);

    ast_entry = ol_txrx_ast_find_hash_find(pdev, addr, 1);
    if (!ast_entry) {
        return;
    }
    peer_id = ast_entry->peer_id;
    printk("Found peer_id=%d\n", peer_id);

    if (peer_id == HTT_INVALID_PEER) {
        printk("No peer found\n");
        return;
    }

    if (peer_id >= OL_TXRX_MAX_PEER_IDS) {
        printk("Invalid Peer Id=%d\n", peer_id);
        return;
    }

    ni = ieee80211_find_node(&ic->ic_sta, addr);
    if (ni == NULL) {
        qdf_print("Node not found!\n");
        return;
    }

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
    status = osif_nss_peer_stats_frame_send(pdev, peer_id);
    if (status != 0) {
        qdf_print("NSS command send failed for peer stats \n");
        ieee80211_free_node(ni);
        return;
    }
#endif

    qdf_print("------- Assoc ID = %d ----------------\n", IEEE80211_AID(ni->ni_associd));
    ieee80211_free_node(ni);

    qdf_print("------- per-Peer/TIDQ Stats ----------------\n");
    tid = 0;
    while(tid < OL_TX_PFLOW_CTRL_MAX_TIDS) {

        ol_tx_flow_ctrl_tidq_stats(pdev, peer_id, tid);

        tid++;
    }

    qdf_print("---------------------------------\n");
}

    void
ol_dump_tidq(void *arg)
{
    struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *) arg;
    uint8_t tid,i;

    for(tid=0;tid < 8;tid++) {
        uint32_t *ptr = (uint32_t *)pdev->pmap->tid_peermap[tid];
        qdf_print("\n TidBmap Tid %d\n", tid);
        for(i=0;i < MAX_PEER_MAP_WORD;i++) {
            qdf_print("%08x ", *ptr);
            ptr++;
        }
    }

}


    void
ol_tx_pflow_ctrl_ttl(struct ol_txrx_pdev_t *pdev, u_int32_t peer_id, u_int8_t tid)
{
    struct ol_txrx_peer_t *peer;
    qdf_nbuf_t netbuf;
    uint32_t qdepth = 0;

    /*
     * First check BSS Peer. Bcoz Mcast pkts uses slowest data rate
     *
     */
    OL_TX_PDEV_LOCK(&pdev->tx_lock);

    peer = ol_txrx_peer_find_by_id(pdev, peer_id);

    if(!peer) {
        goto out;
    }

    if(qdf_nbuf_is_queue_empty(&peer->tidq[tid].nbufq)) {
        goto out;
    }

    qdepth = qdf_nbuf_queue_len(&peer->tidq[tid].nbufq);

    if (qdepth > peer->tidq[tid].enq_cnt) {
        /* Traverse until expiry */
        uint32_t ttl_expired_pkt_cnt = qdepth - peer->tidq[tid].enq_cnt;
        while(ttl_expired_pkt_cnt != 0) {
            netbuf = qdf_nbuf_queue_remove(&peer->tidq[tid].nbufq);
            if (netbuf) {

                /* update counters */
                peer->tidq[tid].byte_count -= qdf_nbuf_len(netbuf);
                pdev->pflow_ctl_global_queue_cnt--;
                ol_tx_free_buf_generic(pdev, netbuf);
                pdev->pflow_msdu_ttl_cnt++;
                ttl_expired_pkt_cnt--;

                PFLOW_CTRL_PEER_STATS_ADD(peer, tid, TIDQ_MSDU_TTL_EXPIRY, 1);

            } else { /* if (!netbuf) */
                if (!qdf_nbuf_is_queue_empty(&peer->tidq[tid].nbufq))  {
                    qdf_print("%s queue not empty %d , but nbuf NULL \n", __func__, qdf_nbuf_queue_len(&peer->tidq[tid].nbufq));
                } else {
                    QUEUE_MAP_CNT_RESET(pdev, peer_id, tid);
                }

                TXRX_PRINT(TXRX_PRINT_LEVEL_INFO1,
                        "%s no packets in queue \n", __func__);
                goto out;
            }
        }//while
    }//if condition

out:
    if(peer) {
        ol_tx_pflow_ctrl_queue_map_update(pdev, peer, peer_id, tid);
        peer->tidq[tid].enq_cnt = 0;
    }
    OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
}

u_int32_t
ol_tx_pflow_enqueue_nawds(void *arg, qdf_nbuf_t nbuf, uint8_t tid, uint8_t nodrop, uint8_t hol)
{
    struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *) arg;
    struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *) vdev->pdev;
    struct ol_txrx_peer_t *peer;
    qdf_nbuf_t netbuf_copy;
    u_int32_t status;

    TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
        if(peer && peer->peer_ids[0] != HTT_INVALID_PEER &&  (peer->bss_peer || peer->nawds_enabled)) {
            netbuf_copy = qdf_nbuf_copy(nbuf);
            if (netbuf_copy) {
                qdf_nbuf_reset_ctxt(netbuf_copy);
                qdf_nbuf_set_vdev_ctx(netbuf_copy, (void *)vdev);
                status = ol_tx_enqueue_pflow_ctrl(vdev, netbuf_copy, peer->peer_ids[0], tid, nodrop, hol);
                if ((status != OL_TX_PFLOW_CTRL_ENQ_DONE) && (status != OL_TX_PFLOW_CTRL_DESC_AVAIL)) {
                    qdf_nbuf_free(netbuf_copy);
                    TXRX_STATS_ADD(pdev, pub.tx.fl_ctrl.fl_ctrl_discard, 1);
                }
            }
        }
    }

    return OL_TX_PFLOW_CTRL_ENQ_DONE;
}

u_int32_t
ol_tx_pflow_is_nawds(void *arg)
{
    struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *) arg;
    struct ol_txrx_peer_t *peer;
    int found = 0;

    TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
        if(peer && peer->nawds_enabled)
            found = 1;
    }
    return found;
}

void
ol_tx_flush_tid_queue_pflow_ctrl(struct ol_txrx_pdev_t *pdev, u_int16_t peer_id, u_int8_t tid)
{
    qdf_nbuf_t netbuf;
    struct ol_txrx_peer_t *peer;

    OL_TX_PEER_LOCK(pdev, peer_id);

    peer = ol_txrx_peer_find_by_id(pdev, peer_id);

    if(!peer) {
        goto out;
    }

    if(peer->peer_ids[0] != peer_id) {
        goto out;
    }

    if(tid != OL_TX_PFLOW_CTRL_PRIORITY_TID) {
#ifdef BIG_ENDIAN_HOST
	QUEUE_MAP_BYTE_CNT_SET(pdev,peer_id,tid,0);
#else
        pdev->pmap->byte_cnt[tid][peer_id] = 0;
#endif
        peer->tidq[tid].byte_count = 0;
    }

    while(!qdf_nbuf_is_queue_empty(&peer->tidq[tid].nbufq)) {
        netbuf = qdf_nbuf_queue_remove(&peer->tidq[tid].nbufq);
        if (netbuf)
            ol_tx_free_buf_generic(pdev, netbuf);
        pdev->pflow_ctl_global_queue_cnt--;
    }

    /*
     * Reset TID Active Bitmap
     */
    if(tid != OL_TX_PFLOW_CTRL_PRIORITY_TID) {
        QUEUE_MAP_CNT_RESET(pdev, peer_id, tid);
    }

out:
    OL_TX_PEER_UNLOCK(pdev, peer_id);
    return;
}

void
ol_tx_flush_peer_queue_pflow_ctrl(struct ol_txrx_pdev_t *pdev, u_int16_t peer_id)
{
    qdf_nbuf_t netbuf;
    u_int8_t tid;

    struct ol_txrx_peer_t *peer;

    if (peer_id >= OL_TXRX_MAX_PEER_IDS){
        return;
    }
    peer = ol_txrx_peer_find_by_id(pdev, peer_id);
    if(!peer) {
        return;
    }   /*
     * Both these locks are mutually exclusive;
     * When TX PDEV Lock is enabled, Peer lock is compile-time disabled
     * and vice-versa
     */
    OL_TX_PDEV_LOCK(&pdev->tx_lock);
    OL_TX_PEER_LOCK(pdev, peer_id);

    for(tid = 0; tid < OL_TX_PFLOW_CTRL_HOST_MAX_TIDS; tid++) {

        while(!qdf_nbuf_is_queue_empty(&peer->tidq[tid].nbufq)) {
            netbuf = qdf_nbuf_queue_remove(&peer->tidq[tid].nbufq);
            if (netbuf)
                ol_tx_free_buf_generic(pdev, netbuf);
            pdev->pflow_ctl_global_queue_cnt--;
        }

        if(tid != OL_TX_PFLOW_CTRL_PRIORITY_TID) {
#ifdef BIG_ENDIAN_HOST
	    QUEUE_MAP_BYTE_CNT_SET(pdev,peer_id,tid,0);
#else
            pdev->pmap->byte_cnt[tid][peer_id] = 0;
#endif
            peer->tidq[tid].byte_count = 0;

            /*
             * Reset TID Active Bitmap
             */
            QUEUE_MAP_CNT_RESET(pdev, peer_id, tid);
        }
    }

    OL_TX_PEER_UNLOCK(pdev, peer_id);
    OL_TX_PDEV_UNLOCK(&pdev->tx_lock);

    return;
}

void
ol_tx_flush_buffers_pflow_ctrl(struct ol_txrx_vdev_t *vdev)
{
    u_int16_t peer_id;
    u_int8_t tid;
    struct ol_txrx_peer_t *peer;
    struct ol_txrx_pdev_t *pdev = vdev->pdev;

    OL_TX_PDEV_LOCK(&pdev->tx_lock);

    if (pdev->pflow_ctl_global_queue_cnt == 0) {
        /* Nothing to free , return */
        OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
        return;
    }

    for(tid = 0; tid < OL_TX_PFLOW_CTRL_HOST_MAX_TIDS; tid++) {

        peer_id = 0;
        while(peer_id < OL_TXRX_MAX_PEER_IDS) {
            peer = ol_txrx_peer_find_by_id(pdev, peer_id);
            if(peer && (peer->vdev == vdev)) {
               ol_tx_flush_tid_queue_pflow_ctrl(pdev, peer_id, tid);
            }
            peer_id++;
        }

    }

    OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
    return;
}


void ol_tx_flush_buffers(struct ol_txrx_vdev_t *vdev)
{
#if !PEER_FLOW_CONTROL
    {
        struct ol_txrx_pdev_t *pdev = vdev->pdev;
        qdf_nbuf_queue_t tmpq;
        u_int32_t scn_qcnt = 0;
        qdf_nbuf_t netbuf;

        /* Init temp queue */
        qdf_nbuf_queue_init(&tmpq);
        OL_TX_PDEV_LOCK(&pdev->tx_lock);
        if (pdev->acqcnt) {
            OL_TX_FLOW_CTRL_LOCK(&pdev->acnbufqlock);
            /* Copy acnbufq into tmpq */
            (void)qdf_nbuf_queue_append(&tmpq, &pdev->acnbufq);
            scn_qcnt = pdev->acqcnt;
            /* Mark acnbufq as empty */
            pdev->acqcnt = 0;
            qdf_nbuf_queue_init(&pdev->acnbufq);
            OL_TX_FLOW_CTRL_UNLOCK(&pdev->acnbufqlock);
        }
        OL_TX_PDEV_UNLOCK(&pdev->tx_lock);

        while (scn_qcnt) {
            netbuf = qdf_nbuf_queue_remove(&tmpq);
            if (netbuf) {
                ol_tx_free_buf_generic(pdev,netbuf);
            } else {
                printk("%s: Error: Broken acnbufq, remaining qlen: %d\n",
                    __func__, scn_qcnt);
                break;
            }
            scn_qcnt--;
        }
    }
#else
    ol_tx_flush_buffers_pflow_ctrl(vdev);
#endif
}

void
ol_tx_switch_mode_flush_buffers(struct ol_txrx_pdev_t *pdev)
{
    u_int16_t peer_id;
    u_int8_t tid;
    struct ol_txrx_peer_t *peer;

    OL_TX_PDEV_LOCK(&pdev->tx_lock);

    if (pdev->pflow_ctl_global_queue_cnt == 0) {
        /* Nothing to free , return */
        OL_TX_PDEV_UNLOCK(&pdev->tx_lock);
        return;
    }

    tid = 0;

    while (tid < OL_TX_PFLOW_CTRL_HOST_MAX_TIDS) {
        peer_id = 0;
        while(peer_id < OL_TXRX_MAX_PEER_IDS) {
            peer = ol_txrx_peer_find_by_id(pdev, peer_id);
            /* flush pkts to target */
            if(peer) {
                if(tid != OL_TX_PFLOW_CTRL_PRIORITY_TID) {
                    ol_tx_ll_pflow_sched(pdev, peer_id, tid, qdf_nbuf_queue_len(&peer->tidq[tid].nbufq));
                }
                /* Drop pkts */
                if(qdf_nbuf_queue_len(&peer->tidq[tid].nbufq))
                    ol_tx_flush_tid_queue_pflow_ctrl(pdev, peer_id, tid);
            }

            peer_id++;
        }
        tid++;
    }

    OL_TX_PDEV_UNLOCK(&pdev->tx_lock);

    return;
}

#endif /*PEER_FLOW_CONTROL*/

void
ol_pdev_set_tid_override_queue_mapping(struct ol_txrx_pdev_t *pdev, int value)
{
    pdev->tid_override_queue_mapping = !!value;
}

int
ol_pdev_get_tid_override_queue_mapping(struct ol_txrx_pdev_t *pdev)
{
    return pdev->tid_override_queue_mapping;
}

