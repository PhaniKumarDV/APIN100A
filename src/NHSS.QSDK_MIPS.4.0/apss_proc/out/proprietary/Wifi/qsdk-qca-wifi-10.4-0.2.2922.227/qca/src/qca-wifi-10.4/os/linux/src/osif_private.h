/*
 * Copyright (c) 2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef _OSIF_PRIVATE_H_
#define _OSIF_PRIVATE_H_

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,38)
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/sysctl.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/sysfs.h>

#include <net/iw_handler.h>
#include <linux/wireless.h>
#include <linux/if_arp.h>		/* XXX for ARPHRD_* */
#if ATH_SUPPORT_FLOWMAC_MODULE
#include <net/pkt_sched.h>
#endif
#include <asm/uaccess.h>

#include "if_media.h"
#include "if_upperproto.h"

#include "ieee80211.h"
#include "_ieee80211.h"
#include <ieee80211_api.h>
#include "ieee80211_defines.h"
#include <ieee80211_ioctl.h>
#include <ieee80211_sme_api.h>
#include <ieee80211P2P_api.h>

#if ATH_PERF_PWR_OFFLOAD
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_pflow.h>
#include <cdp_txrx_raw.h>
#include <cdp_txrx_me.h>
#endif
#if UMAC_SUPPORT_ACFG
#include <acfg_api_types.h>
#endif

#ifdef ATH_SUPPORT_LINUX_VENDOR
#include <ioctl_vendor.h>      /* Vendor Include */
#endif

#define OS_SIWSCAN_TIMEOUT ((12000 * HZ) / 1000)
#define SIWSCAN_TIME_ENH_IND_RPT	 5
#define OSIF_MAX_CONNECTION_ATTEMPT      3
#define OSIF_MAX_CONNECTION_STOP_TIMEOUT 20
#define OSIF_MAX_CONNECTION_TIMEOUT      0xFFFFFFFF
#define OSIF_MAX_DELETE_VAP_TIMEOUT      30
#define OSIF_MAX_STOP_VAP_TIMEOUT_CNT    300
#define OSIF_MAX_START_VAP_TIMEOUT_CNT   300

#define OSIF_CONNECTION_TIMEOUT          ((CONVERT_SEC_TO_SYSTEM_TIME(1)/20) + 1) /*50 msec */
#define OSIF_DELETE_VAP_TIMEOUT          CONVERT_SEC_TO_SYSTEM_TIME(1)
#define OSIF_DISCONNECT_TIMEOUT       ((CONVERT_SEC_TO_SYSTEM_TIME(1)/100) + 1) /* 10 msec */
#define OSIF_STOP_VAP_TIMEOUT         ((CONVERT_SEC_TO_SYSTEM_TIME(1)/40) + 1) /* 25 msec */
#define OSIF_VAPUP_TICK                (HZ/50) /* 20 ms */
#define OSIF_VAPUP_TIMEOUT_COUNT       150 /* 3 seconds(20 ms *150) */
#define OSIF_START_VAP_TIMEOUT         ((CONVERT_SEC_TO_SYSTEM_TIME(1)/100) + 1) /* 10 msec */
/* make sure the above constant does not return 0 */
#define OSIF_TXCHANSWITCH_LOOPCOUNT 100

#define OSIF_VAP_STOP_TIMEBUFF_US     (5000)
#define OSIF_VAP_OPEN_TIMEBUFF_US     (5000)

int osif_ioctl_create_vap(struct net_device *dev, struct ifreq *ifr,
							 struct ieee80211_clone_params cp,
							 osdev_t os_handle);
int osif_ioctl_delete_vap(struct net_device *dev);
int osif_ioctl_switch_vap(struct net_device *dev, enum ieee80211_opmode opmode);
void osif_attach(struct net_device *dev);
void osif_detach(struct net_device *dev);
void osif_notify_push_button(osdev_t osdev, u_int32_t push_time);
int osif_vap_init(struct net_device *dev, int forcescan);
int osif_vap_stop(struct net_device *dev);
int osif_restart_vaps(struct ieee80211com *ic);
int osif_restart_for_config(struct ieee80211com *ic, int (*config_callback)(struct ieee80211com *ic));
void osif_deliver_data(os_if_t osif, struct sk_buff *skb);
int osif_vap_hardstart(struct sk_buff *skb,struct net_device *dev);
void osif_forward_mgmt_to_app(os_if_t osif, wbuf_t wbuf,
                                        u_int16_t type, u_int16_t subtype, ieee80211_recv_status *rs);
int osif_ioctl_get_vap_info (struct net_device *dev,
                            struct ieee80211_profile *profile);

void osif_vap_acs_cancel(struct net_device *dev, u_int8_t wlanscan);
void delete_default_vap_keys(struct ieee80211vap *vap);
#if UMAC_SUPPORT_ACFG
int osif_set_vap_vendor_param(struct net_device *dev, acfg_vendor_param_req_t *req);
int osif_get_vap_vendor_param(struct net_device *dev, acfg_vendor_param_req_t *req);
#endif

#if UMAC_PER_PACKET_DEBUG
int  osif_atoi_proc(char *buf);
ssize_t osif_proc_pppdata_write(struct file *filp, const char __user *buff,
                unsigned long len, void *data );
#endif

void osif_notify_scan_done(struct net_device *dev, ieee80211_scan_completion_reason reason);
#if ATH_SUPPORT_SPECTRAL
int osif_ioctl_eacs(struct net_device *dev, struct ifreq *ifr, osdev_t os_handle);
#endif
#if ATH_SUPPORT_VLAN
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
void			qdf_net_vlan_attach(struct net_device *dev,struct net_device_ops *osif_dev_ops);
#else
void			qdf_net_vlan_attach(struct net_device *dev);
#endif
void			qdf_net_vlan_detach(struct net_device *dev);
#endif

#if ATH_RXBUF_RECYCLE
void ath_rxbuf_recycle_init(osdev_t osdev);
void ath_rxbuf_recycle_destroy(osdev_t osdev);
#endif /* ATH_RXBUF_RECYCLE */

/* TGf l2_update_frame  format */
struct l2_update_frame
{
    struct ether_header eh;
    u_int8_t dsap;
    u_int8_t ssap;
    u_int8_t control;
    u_int8_t xid[3];
}  __packed;

#define IEEE80211_L2UPDATE_CONTROL 0xf5
#define IEEE80211_L2UPDATE_XID_0 0x81
#define IEEE80211_L2UPDATE_XID_1 0x80
#define IEEE80211_L2UPDATE_XID_2 0x0

#if WLAN_FEATURE_FASTPATH
#define MAX_MSDUS_ACC           1
#define MAX_MSDUS_ACC_MASK      (MAX_MSDUS_ACC - 1)
#define MSDUS_ARRAY_SIZE        1
#define MSDUS_ARRAY_SIZE_MASK   (MSDUS_ARRAY_SIZE - 1)
#endif /* WLAN_FEATURE_FASTPATH */

typedef struct _osif_dev {
    osdev_t  os_handle;
    struct net_device *netdev;

#if WLAN_FEATURE_FASTPATH
    qdf_nbuf_t  *nbuf_arr;      /* array to accumulate packets from the OS */
    uint16_t    nbuf_arr_pi;    /* producer index for nbuf_arr */
    uint16_t    nbuf_arr_ci;    /* consumer index for nbuf_arr */
    spinlock_t nbuf_arr_lock;   /* lock to access nbuf_arr (first cut) */
#endif /* WLAN_FEATURE_FASTPATH */
#if QCA_NSS_PLATFORM
    void *nss_redir_ctx;              /* QCA NSS platform specific handler */
#endif
    bool nss_nwifi;		/* check if NSS decapsulates native wifi packets */
    bool is_ar900b;		/* is this a beeliner or peregrine */

    wlan_if_t os_if;
    bool    osif_is_mode_offload;   /* = 1 for offload, = 0 for direct attach */
    struct list_head pending_rx_frames; /* stuff too big for wext events, send by ioctl */
    spinlock_t list_lock;

    wlan_dev_t os_devhandle;
    struct net_device *os_comdev;
    struct ifmedia os_media;    /* interface media config */
    u_int8_t  os_unit;
#if ATH_SUPPORT_VLAN
	struct vlan_group	*vlgrp;
	unsigned short		vlanID;
#endif
#if ATH_SUPPORT_WRAP
    TAILQ_ENTRY(_osif_dev)  osif_dev_list;      /*entry for wrap oma dev list*/
    LIST_ENTRY(_osif_dev)   osif_dev_hash;      /*entry for wrap oma hash list*/
    TAILQ_ENTRY(_osif_dev)  osif_dev_list_vma;  /*entry for wrap vma dev list*/
    LIST_ENTRY(_osif_dev)   osif_dev_hash_vma;  /*entry for wrap vma hash list*/
    struct wrap_com     *wrap_handle;           /*ptr to wrap common structure*/
    unsigned char       osif_dev_oma[ETH_ALEN]; /* dev oma mac address */
    unsigned char       osif_dev_vma[ETH_ALEN]; /* dev vma mac address */
    int                 osif_dev_port_type;     /*wrap dev port type*/
    unsigned long       osif_dev_cnt_stl;       /*wrap dev count stolen*/
    unsigned long       osif_dev_cnt_drp;       /*wrap dev count dropped*/
    unsigned long       osif_dev_cnt_ovrd;      /*wrap dev count overide br decision*/
#endif
    wlan_connection_sm_t sm_handle;
    wlan_ibss_sm_t       sm_ibss_handle;
    enum ieee80211_opmode os_opmode;
    struct net_device_stats os_devstats;
    struct iw_statistics    os_iwstats; /* wireless statistics block */

    /* cache for the security config */
    ieee80211_cipher_type uciphers[IEEE80211_CIPHER_MAX];
    u_int16_t u_count;
    ieee80211_cipher_type mciphers[IEEE80211_CIPHER_MAX];
    u_int16_t m_count;
    IEEE80211_SCAN_REQUESTOR    scan_requestor;
    IEEE80211_SCAN_ID           scan_id;
    IEEE80211_SCAN_REQUESTOR    offchan_scan_requestor;      /* scan requestor for offchan scan */
    IEEE80211_SCAN_ID           offchan_scan_id;             /* scan id for offchan scan */
    u_int32_t       user_scan_wait:1,
                    is_up:1,
                    is_stop_event_pending:1,
                    is_deleted:1,
                    is_ibss_create:1,
                    is_delete_in_progress:1,
                    is_restart:1,
                    is_vap_pending:1,
                    use_p2p_go:1,
                    use_p2p_client:1,
                    is_p2p_interface:1, /* use_p2p_go | use_p2p_client */
                    is_bss_started:1,
                    disable_ibss_create:1,
                    is_scan_chevent:1;  /* Is delivery of Scan Channel Events enabled during
                                           802.11 scans. Applicable only for 11ac offload.
                                           Currently takes effect only for IEEE80211_M_HOSTAP
                                           operating mode since the Scan Channel Events
                                           are of use only in that mode.
                                           XXX - Add for other operating modes if use
                                           cases arise.
                                           XXX - Currently, the Events are not delivered
                                           upwards to the application layer. They are
                                           used internally in the driver. If delivery
                                           to application layer is desired, it can be
                                           added using IWEVCUSTOM. */

    u_int32_t       no_stop_disassoc:1;
    u_int8_t        os_giwscan_count;
    unsigned long   os_last_siwscan;    /* time last set scan request */
#define OSIF_SCAN_BAND_ALL            (0)
#define OSIF_SCAN_BAND_2G_ONLY        (1)
#define OSIF_SCAN_BAND_5G_ONLY        (2)
    u_int8_t        os_scan_band;       /* only scan channels of requested band */

    u_int8_t authmode;
    u_int32_t app_filter;
#ifdef ATHEROS_LINUX_PERIODIC_SCAN
#define OSIF_PERIODICSCAN_DEF_PERIOD        (0)
#define OSIF_PERIODICSCAN_MIN_PERIOD        (30000)
#define OSIF_PERIODICSCAN_COEXT_PERIOD      (180000)

    u_int32_t   os_periodic_scan_period;    /* 0 means off Periodic Scan */
    os_timer_t  os_periodic_scan_timer;
#endif
    spinlock_t  tx_lock; /* lock for the tx path */
#ifdef ATH_SUPPORT_LINUX_VENDOR
#define OSIF_VENDOR_RAWDATA_SIZE        (24)
    u_int8_t os_vendor_specific[OSIF_VENDOR_RAWDATA_SIZE];    /* Some vendor ioctl() maybe cache something. */
#endif
#if ATH_SUPPORT_IWSPY
    struct 	iw_spy_data 		  spy_data;	/* iwspy support */
#endif
#if ATH_SUPPORT_WAPI
    u_int32_t    os_wapi_rekey_period;
    os_timer_t    os_wapi_rekey_timer;
#endif
#if ATH_PERF_PWR_OFFLOAD
    ol_txrx_vdev_handle iv_txrx_handle;
    ol_txrx_tx_fp iv_vap_send;
#endif /* ATH_PERF_PWR_OFFLOAD */
#if UMAC_VOW_DEBUG
    #define MAX_VOW_CLIENTS_DBG_MONITOR 8
    u_int8_t vow_dbg_en;
    unsigned long umac_vow_counter;
    unsigned long tx_dbg_vow_counter[MAX_VOW_CLIENTS_DBG_MONITOR];
    u_int8_t tx_dbg_vow_peer[MAX_VOW_CLIENTS_DBG_MONITOR][2];
    unsigned long tx_prev_pkt_count[MAX_VOW_CLIENTS_DBG_MONITOR];
#endif
#if UMAC_SUPPORT_VI_DBG
    u_int8_t vi_dbg;
#endif

#if QCA_PARTNER_DIRECTLINK_TX
    bool is_directlink;
#endif /* QCA_PARTNER_DIRECTLINK_TX */

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
    void *nss_wifiol_ctx;
    uint32_t radio_id;	       /* Wifi radio to which this vap belongs to */
    uint32_t nss_ifnum;	       /* NSS interface number associated with vap */
    spinlock_t queue_lock;    /* Lock for the vdev tx queue */
    struct sk_buff_head wifiol_txqueue; /* qwrap and extap tx/rx queues for bursty traffic handling */
    uint32_t stale_pkts_timer_interval;
    os_timer_t wifiol_stale_pkts_timer;
#endif

} osif_dev;

#define DIRECT_ATTACH 1
#define OFFLOAD       2

struct wifi_radio_t  {
    u_int8_t wifi_radio_type;
    void *sc;
};

#define NUM_MAX_RADIOS 3

struct g_wifi_info {
    u_int8_t num_radios;
    struct wifi_radio_t wifi_radios[NUM_MAX_RADIOS];
};

#define OSIF_WAPI_REKEY_TIMEOUT 5000  /*ms*/

#ifndef ETH_P_80211_RAW
#define ETH_P_80211_RAW (ETH_P_ECONET + 1)
#endif

enum
{
        DIDmsg_lnxind_wlansniffrm               = 0x00000044,
        DIDmsg_lnxind_wlansniffrm_hosttime      = 0x00010044,
        DIDmsg_lnxind_wlansniffrm_mactime       = 0x00020044,
        DIDmsg_lnxind_wlansniffrm_channel       = 0x00030044,
        DIDmsg_lnxind_wlansniffrm_rssi          = 0x00040044,
        DIDmsg_lnxind_wlansniffrm_sq            = 0x00050044,
        DIDmsg_lnxind_wlansniffrm_signal        = 0x00060044,
        DIDmsg_lnxind_wlansniffrm_noise         = 0x00070044,
        DIDmsg_lnxind_wlansniffrm_rate          = 0x00080044,
        DIDmsg_lnxind_wlansniffrm_istx          = 0x00090044,
        DIDmsg_lnxind_wlansniffrm_frmlen        = 0x000A0044,
        DIDmsg_lnxind_wlansniffrm_rate_phy1     = 0x000B0044,
        DIDmsg_lnxind_wlansniffrm_rate_phy2     = 0x000C0044,
        DIDmsg_lnxind_wlansniffrm_rate_phy3     = 0x000D0044,
};

enum
{
    P80211ENUM_msgitem_status_no_value  = 0x00
};
enum
{
    P80211ENUM_truth_false              = 0x00
};

/*
 * Some data is too big to return in a wireless extension event.
 * So instead, the event signals the availability of data an IOCTL is used
 * to fetch data - which calls the fetch_p2p_mgmt list access routine.
 */
struct pending_rx_frames_list {
    struct list_head list;
    struct wlan_p2p_rx_frame rx_frame; /* data to be returned via ioctl */
    u_int32_t  freq;        /* pre-pend frequency to data on fx_frames */
    u_int32_t  frame_type;  /* pre-pend type to data */
    int    extra[1];        /* variable length extra data */
};
struct pending_rx_frames_list *osif_fetch_p2p_mgmt(struct net_device *dev);
#ifdef HOST_OFFLOAD
int osif_is_pending_frame_list_empty(struct net_device *dev);
#endif
void osif_p2p_rx_frame_handler(osif_dev *osifp, wlan_p2p_event *event,
                                int frame_type);

#ifdef LIMIT_MTU_SIZE
#define LIMITED_MTU 1368
#include <net/icmp.h>
#include <linux/icmp.h>
#include <net/route.h>


#define LIMIT_PATH_MTU_TX(_skb,_Fake_rtable)                            \
        if (_skb->len >=  LIMITED_MTU + ETH_HLEN) {                     \
            _skb->h.raw = _skb->nh.raw = _skb->data +ETH_HLEN ;         \
            _skb->dst = (struct dst_entry *)&_Fake_rtable;              \
            _skb->pkt_type = PACKET_HOST;                               \
            dst_hold(_skb->dst);                                        \
            icmp_send(_skb, ICMP_DEST_UNREACH,                          \
                     ICMP_FRAG_NEEDED, htonl(LIMITED_MTU - 4 ));        \
            dev_kfree_skb_any(_skb);                                    \
            return 0;                                                   \
        }

#define LIMIT_PATH_MTU_RX(_skb,_Fake_rtable)                            \
        if (_skb->len >=  LIMITED_MTU) {                                \
            _skb->h.raw = _skb->nh.raw = _skb->data  ;                  \
            _skb->dst = (struct dst_entry *)&_Fake_rtable;              \
            _skb->pkt_type = PACKET_HOST;                               \
            dst_hold(_skb->dst);                                        \
            icmp_send(_skb, ICMP_DEST_UNREACH,                          \
                     ICMP_FRAG_NEEDED, htonl(LIMITED_MTU - 4 ));        \
            dev_kfree_skb_any(_skb);                                    \
            return;                                                     \
        }
#endif

#define IS_IFUP(_dev) \
    (((_dev)->flags & (IFF_RUNNING|IFF_UP)) == (IFF_RUNNING|IFF_UP))
#define IS_UP(_dev) \
    (((_dev)->flags & (IFF_RUNNING|IFF_UP)) == (IFF_RUNNING|IFF_UP))
#define RESCAN  1

#define IS_CONNECTED(osp) ((((osp)->os_opmode==IEEE80211_M_STA) || \
                        ((u_int8_t)(osp)->os_opmode == IEEE80211_M_P2P_CLIENT))? \
                        wlan_connection_sm_is_connected((osp)->sm_handle): \
                        (osp)->is_bss_started )

#define NETDEV_TO_VAP(_dev) (((osif_dev *)netdev_priv(_dev))->os_if)
#define OSIF_TO_NETDEV(_osif) (((osif_dev *)(_osif))->netdev)

#if ATH_PERF_PWR_OFFLOAD
/* Packet format configuration for a given interface or radio.
 * This must correspond to ordering in the enumeration htt_pkt_type.
 */
enum osif_pkt_type {
    osif_pkt_type_raw = 0,
    osif_pkt_type_native_wifi = 1,
    osif_pkt_type_ethernet = 2,
};
#endif /* ATH_PERF_PWR_OFFLOAD */

#define OSIF_NETDEV_TYPE_DA 0
#define OSIF_NETDEV_TYPE_OL 1
#define OSIF_NETDEV_TYPE_NSS_WIFIOL 2
int osif_register_dev_ops_xmit(int (*vap_hardstart)(struct sk_buff *skb, struct net_device *dev), int netdev_type);

int  osif_ol_wrap_tx_process(osif_dev **osifp, struct ieee80211vap *vap, struct sk_buff **skb);
int  osif_ol_wrap_rx_process(osif_dev **osifp, struct net_device **dev, wlan_if_t vap, struct sk_buff *skb);
void osif_mon_add_radiotap_header(os_if_t osif, struct sk_buff *skb, ieee80211_recv_status *rs);
void osif_mon_add_prism_header(os_if_t osif, struct sk_buff *skb, ieee80211_recv_status *rs);
#endif /* _OSIF_PRIVATE_H_ */
