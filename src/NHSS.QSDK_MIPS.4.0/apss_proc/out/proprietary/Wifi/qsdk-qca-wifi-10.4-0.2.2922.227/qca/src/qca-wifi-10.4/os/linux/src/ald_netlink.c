/*
 * Copyright (c) 2015 Qualcomm Atheros, Inc.
 *
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

/*
 *  Copyright (c) 2010, Atheros Communications Inc.
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
#include "linux/if.h"
#include "linux/socket.h"
#include <net/rtnetlink.h>
#include <net/sock.h>

#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/cache.h>
#include <linux/proc_fs.h>

#include "if_athvar.h"
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
#include "ald_netlink.h"
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
#include <osif_nss_wifiol_vdev_if.h>
#endif


struct ald_netlink *ald_nl = NULL;

int ald_init_netlink(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION (3,10,0)
    struct netlink_kernel_cfg cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.groups = 1;
    cfg.input = &ald_nl_receive;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
    struct netlink_kernel_cfg cfg = {
    .groups = 1,
    .input  = ald_nl_receive,
    };
#endif

    if (ald_nl == NULL) {
        ald_nl = (struct ald_netlink *)kzalloc(sizeof(struct ald_netlink), GFP_KERNEL);

        if(ald_nl == NULL)
            return -ENODEV;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)/* Also for >= 3,10,0 */
        ald_nl->ald_sock = (struct sock *)netlink_kernel_create(&init_net, NETLINK_ALD, &cfg);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
        ald_nl->ald_sock = (struct sock *)netlink_kernel_create(&init_net, NETLINK_ALD,
                                   THIS_MODULE, &cfg);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION (2,6,24)
        ald_nl->ald_sock = (struct sock *)netlink_kernel_create(&init_net, NETLINK_ALD,
                                   1, &ald_nl_receive, NULL, THIS_MODULE);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION (2,6,22)
        ald_nl->ald_sock = (struct sock *)netlink_kernel_create(NETLINK_ALD,
                                   1, &ald_nl_receive, (struct mutex *) NULL, THIS_MODULE);
#else
        ald_nl->ald_sock = (struct sock *)netlink_kernel_create(NETLINK_ALD,
                                   1, &ald_nl_receive, THIS_MODULE);
#endif

        if (ald_nl->ald_sock == NULL) {
            kfree(ald_nl);
            ald_nl = NULL;

            printk("%s NETLINK_KERNEL_CREATE FAILED\n", __func__);

            return -ENODEV;
        }

        atomic_set(&ald_nl->ald_refcnt, 1);
    } else {
        atomic_inc(&ald_nl->ald_refcnt);
    }

    return 0;
}

int ald_destroy_netlink(void)
{
    if (ald_nl == NULL) {
        printk("\n%s ald_nl is NULL\n", __func__);
        return -ENODEV;
    }

    if (!atomic_dec_return(&ald_nl->ald_refcnt)) {
        if (ald_nl->ald_sock)
            sock_release(ald_nl->ald_sock->sk_socket);

        kfree(ald_nl);
        ald_nl = NULL;
    }

    return 0;
}


static void ald_notify(wlan_if_t vap, u_int32_t info_cmd, u_int32_t info_len, void *info_data)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh = NULL;
    u_int8_t *nldata = NULL;
    u_int32_t pid;

    if (ald_nl == NULL || vap == NULL)
        return;

    pid = vap->ald_pid;

    if (pid == WLAN_DEFAULT_NETLINK_PID) return;

    skb = nlmsg_new(info_len, GFP_ATOMIC);
    if (!skb) {
        printk("%s: No memory, info_cmd = %d\n", __func__, info_cmd);
        return;
    }

    nlh = nlmsg_put(skb, pid, 0, info_cmd, info_len, 0);
    if (!nlh) {
        printk("%s: nlmsg_put() failed, info_cmd = %d\n", __func__, info_cmd);
        kfree_skb(skb);
        return;
    }

    nldata = NLMSG_DATA(nlh);
    memcpy(nldata, info_data, info_len);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
    NETLINK_CB(skb).pid = 0;        /* from kernel */
#else
    NETLINK_CB(skb).portid = 0;  /* from kernel */
#endif

    NETLINK_CB(skb).dst_group = 0;  /* unicast */
    netlink_unicast(ald_nl->ald_sock, skb, pid, MSG_DONTWAIT);
}

static void ald_send_info_iter_func(wlan_if_t vap)
{
    osif_dev *osifp;
    struct net_device *dev;
    struct ald_stat_info *info = NULL;

    info = qdf_mem_malloc(sizeof(struct ald_stat_info));

    if (info == NULL) {
            return;
    }

    osifp = (osif_dev *)wlan_vap_get_registered_handle(vap);

    if (osifp->is_up && vap->ald_pid != WLAN_DEFAULT_NETLINK_PID) {
        vap->iv_ald->staticp = info;

        info->cmd = IEEE80211_ALD_ALL;

        dev = ((osif_dev *)vap->iv_ifp)->netdev;
        strcpy(info->name, dev->name);

        wlan_ald_get_statistics(vap, NULL);

        ald_notify(vap, IEEE80211_ALD_ALL, sizeof(struct ald_stat_info), info);
    }
    qdf_mem_free(info);
}

static void ald_nl_hifitbl_update(wlan_if_t vap, struct nlmsghdr *nlh)
{
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
    struct ieee80211com *ic = vap->iv_ic;
#endif
    write_lock_bh(&vap->iv_me->me_hifi_lock);
    if (nlh->nlmsg_len <= NLMSG_LENGTH(0)) {
        vap->iv_me->me_hifi_table.entry_cnt = 0;
    } else {
        unsigned int hifi_table_size = sizeof(vap->iv_me->me_hifi_table);

        if (nlh->nlmsg_len - NLMSG_HDRLEN < hifi_table_size)
            hifi_table_size = nlh->nlmsg_len - NLMSG_HDRLEN;

        memcpy(&vap->iv_me->me_hifi_table, NLMSG_DATA(nlh), hifi_table_size);

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
    if (ic->nss_funcs) {
	ic->nss_funcs->ic_osif_nss_vdev_me_reset_snooplist(vap->iv_txrx_handle);
	ic->nss_funcs->ic_osif_nss_vdev_me_update_hifitlb(vap->iv_txrx_handle, &vap->iv_me->me_hifi_table);
    }
#endif
    }
#if ATH_SUPPORT_ME_FW_BASED
	if(vap->iv_ic->ic_is_mode_offload(vap->iv_ic))
		vap->iv_ic->ic_hifitbl_update_target(vap);
#endif
    write_unlock_bh(&vap->iv_me->me_hifi_lock);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION (2,6,24)
static void ald_nl_receive(struct sk_buff *__skb)
#else
static void ald_nl_receive(struct sock *sk, int len)
#endif
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    wlan_if_t vap;
    u_int32_t pid;
    int32_t ifindex;
    struct net_device *dev;
    osif_dev  *osifp;

#if LINUX_VERSION_CODE >= KERNEL_VERSION (2,6,24)
    if ((skb = skb_get(__skb)) != NULL) {
#else
    if ((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL) {
#endif
        nlh = nlmsg_hdr(skb);
        pid = nlh->nlmsg_pid;
        ifindex = nlh->nlmsg_flags;

        dev = dev_get_by_index(&init_net, ifindex);
        if (!dev) {
            printk("%s: Invalid interface index:%d\n", __func__, ifindex);
            kfree_skb(skb);
            return;
        }

        osifp = ath_netdev_priv(dev);
        if (osifp ->is_deleted)
        {
            printk("%s: vap[%s] has been deleted\n", __func__, dev->name);
            goto out;
        }

        vap = osifp->os_if;
        if(vap == NULL) {
            goto out;
        }

        if(ald_nl == NULL) {
            goto out;
        }

        if (vap->ald_pid  !=  pid) {
            vap->ald_pid = pid;
            printk("[%s] Band steering events being sent to PID:%d\n",dev->name,vap->ald_pid);
        }

        if (nlh->nlmsg_type == IEEE80211_ALD_ALL)
            ald_send_info_iter_func(vap);
        else if (nlh->nlmsg_type == IEEE80211_ALD_MCTBL_UPDATE) {
            ald_nl_hifitbl_update(vap, nlh);
        }

out:
        dev_put(dev);
        kfree_skb(skb);
    }
}

int ald_assoc_notify(wlan_if_t vap, u_int8_t *macaddr, u_int8_t aflag)
{
    struct net_device *dev;
    wlan_chan_t chan;
    struct ald_assoc_info info;

    info.cmd = IEEE80211_ALD_ASSOCIATE;
    dev = ((osif_dev *)vap->iv_ifp)->netdev;
    strcpy(info.name, dev->name);

    chan = wlan_get_current_channel(vap, true);
    if(chan->ic_freq * 100000 < 500000)
        info.afreq = ALD_FREQ_24G;
    else
        info.afreq = ALD_FREQ_5G;
    info.aflag = aflag;
    memcpy(info.macaddr, macaddr, IEEE80211_ADDR_LEN);

    ald_notify(vap, IEEE80211_ALD_ASSOCIATE, sizeof(info), &info);
    return 0;
}

/* function for 75% buffers full warning */
int ald_buffull_notify(wlan_if_t vap)
{
    struct net_device *dev;
    struct ald_buffull_info info;

    info.cmd = IEEE80211_ALD_BUFFULL_WRN;
    dev = ((osif_dev *)vap->iv_ifp)->netdev;
    strcpy(info.name, dev->name);

    info.resv = ATH_TXBUF;

    ald_notify(vap, IEEE80211_ALD_BUFFULL_WRN, sizeof(info), &info);
    return 0;

}

int
ieee80211_ioctl_ald_getStatistics(struct net_device *dev, struct iw_request_info *info,
            void *w, char *extra)
{
    osif_dev *osifp = ath_netdev_priv(dev);
    wlan_if_t vap = osifp->os_if;
    int retv = 0;
    struct ald_stat_info *param = NULL;

    param = (struct ald_stat_info *)kmalloc(sizeof(struct ald_stat_info), GFP_KERNEL);
    if (!param)
        return -ENOMEM;

    if (copy_from_user(param, ((union iwreq_data *)w)->data.pointer, sizeof(struct ald_stat_info))) {
        OS_FREE(param);
        return -EFAULT;
    }
    IEEE80211_DPRINTF(vap, IEEE80211_MSG_DEBUG,
            "%s parameter is %s 0x%x get1\n", __func__, param->name, param->cmd);
    retv = wlan_ald_get_statistics(vap, param);
    copy_to_user(((union iwreq_data *)w)->data.pointer, param, sizeof(struct ald_stat_info));
    kfree(param);
    return retv;
}

void
ieee80211_buffull_handler(struct ieee80211com *ic)
{
    struct ieee80211vap *vap;
    TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
        if (vap) {
            IEEE80211_DELIVER_EVENT_BUFFULL(vap);
            break;
        }
    }

}
EXPORT_SYMBOL(ieee80211_buffull_handler);

#endif
