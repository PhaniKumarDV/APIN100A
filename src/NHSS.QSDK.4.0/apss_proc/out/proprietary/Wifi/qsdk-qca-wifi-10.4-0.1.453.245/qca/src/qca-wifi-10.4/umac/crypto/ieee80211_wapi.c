/*
 * Copyright (c) 2010, Atheros Communications Inc. 
 * All Rights Reserved.
 * 
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * 
 */
#include <osdep.h>

#if ATH_SUPPORT_WAPI
#include "ieee80211_var.h"
#include "ieee80211_api.h"
#include "ieee80211_ioctl.h"

void longint_add(u_int32_t *a, u_int32_t b, u_int16_t len)
{
	int i = 0;
	u_int32_t carry = 0;
	u_int32_t *a1 = NULL;
	u_int32_t ca1 = 0;
	u_int32_t ca2 = 0;
	u_int32_t ca3 = 0;
	u_int32_t bb = b;

	for(i=len - 1; i>=0; i--) {	
		a1=a+i; 
		ca1 = (*a1)&0x80000000;	
		ca2 = bb&0x80000000;
		*a1 += bb + carry;
		bb = 0;
		ca3 = (*a1)&0x80000000;	
		if(ca1==0x80000000 && ca2==0x80000000)  carry=1; 
		else if(ca1!=ca2 && ca3==0)  carry=1; 
        	else carry=0;
		a1++;
	}
}

static void  initialize_send_iv(u_int32_t *iv, u_int8_t *init_iv)
{
	int i = 0;
	
	for(i=0; i<WPI_IV_LEN/4; i++)
	{
		*(iv+i) =  be32toh(*(((u_int32_t *)(init_iv))+i));
	}
}

int
sta_wapi_init(struct ieee80211vap *vap, struct ieee80211_key *k)
{
    unsigned char iv_init_AP[16]    = { 0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x36,
                                             0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x37};
    unsigned char iv_init_STA[16]   = { 0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x36,
                                             0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x36};

    if(vap->iv_opmode == IEEE80211_M_HOSTAP)
    {   
        /* initialize receive iv */
        /* During Tx PN should be increment and 
           send but as per our implementation we increment only after Tx complete 
           So First packet PN check will be failed.
           To compensate increment the PN here by 2*/
        iv_init_AP[15] += 2; 
        OS_MEMCPY(k->wk_recviv, iv_init_STA, WPI_IV_LEN);
        /*initialize send iv */
        OS_MEMSET((char *)(k->wk_txiv), 0, WPI_IV_LEN);
        initialize_send_iv(k->wk_txiv, iv_init_AP);
    } else {
        /*initialize receive iv */
        /* During Tx PN should be increment and
        send but as per our implementation we increment only after Tx complete
        So First packet PN check will be failed.
        To compensate increment the PN here by 2*/
        iv_init_STA[15] += 2;
        OS_MEMCPY(k->wk_recviv, iv_init_AP, WPI_IV_LEN);
        /*initialize send iv */
        OS_MEMSET((char *)(k->wk_txiv), 0, WPI_IV_LEN);	
        initialize_send_iv(k->wk_txiv, iv_init_STA);
    }       
    return 0;
}

void *wlan_wapi_callback_begin(wlan_if_t vaphandle, u_int8_t *macaddr, int *msg_len, int msg_type)
{
    struct ieee80211vap *vap = vaphandle;
    struct wapi_sta_msg_t *sta_msg = NULL;
    struct ieee80211_node *ni;
    u_int8_t *ie;
    int ie_len = 0;
    
    if (ieee80211_vap_wapi_is_set(vap)) {
        sta_msg = (struct wapi_sta_msg_t *)OS_MALLOC(vap->iv_ic->ic_osdev, sizeof(struct wapi_sta_msg_t), GFP_KERNEL);
        if (sta_msg) {
            switch (msg_type) {
                case WAPI_STA_AGING:
                case WAPI_UNICAST_REKEY:
                case WAPI_MULTI_REKEY:
                    break;
                case WAPI_WAI_REQUEST:
                    ni = ieee80211_vap_find_node(vap, macaddr);
                    if(ni == NULL || ni->ni_wpa_ie == NULL) {
                        OS_FREE(sta_msg);
                        if (ni) {
                            ieee80211_free_node(ni);
                        }
                        return NULL;
                    }
			        ni->ni_flags &= ~IEEE80211_NODE_AUTH;
                    ie = ni->ni_wpa_ie;
                    OS_MEMCPY(sta_msg->gsn, vap->iv_nw_keys[vap->iv_def_txkey].wk_txiv, WPI_IV_LEN);
            		sta_msg->datalen += WPI_IV_LEN;
            		ie_len = ie[1] + 2;
            		OS_MEMCPY(sta_msg->wie, ie, ie_len);
            		sta_msg->datalen += ie_len;
                    /* decrease node refcnt */
                    ieee80211_free_node(ni);
                    break;
                default:
                    break;    
            }
        	OS_MEMCPY(sta_msg->vap_mac, vap->iv_myaddr, IEEE80211_ADDR_LEN);
        	sta_msg->datalen += IEEE80211_ADDR_LEN + 2;
        	sta_msg->msg_type = msg_type;

        	OS_MEMCPY(sta_msg->sta_mac, macaddr, IEEE80211_ADDR_LEN);
        	sta_msg->msg_type = msg_type;
        	sta_msg->datalen += IEEE80211_ADDR_LEN + 2;
            *msg_len = sta_msg->datalen + 4;
        }
    }
    return (u_int8_t *)sta_msg;
}

void wlan_wapi_callback_end(u_int8_t *sta_msg)
{
    if (sta_msg)
        OS_FREE(sta_msg);
    return;    
}

static void wlan_wapi_unicast_update(struct ieee80211vap* vap, struct ieee80211_node *ni)
{
    
    if (!ni || ni->ni_associd == 0)	/* only associated stations */
        return;

    ni->ni_wapi_rekey_pkthresh = 
        ni->ni_stats.ns_tx_ucast + vap->iv_wapi_urekey_pkts;
   
}	

void wlan_wapi_unicast_rekey(struct ieee80211vap* vap, struct ieee80211_node *ni)
{
    
    if (!ni || ni->ni_associd == 0)	/* only associated stations */
        return;

//  IEEE80211_DPRINTF(vap, IEEE80211_MSG_CRYPTO,"ni unicast:%d, threshold:%d\n",ni->ni_stats.ns_tx_ucast ,ni->ni_wapi_rekey_pkthresh);
    if( ni->ni_wapi_rekey_pkthresh == 0)
    {
        ni->ni_wapi_rekey_pkthresh = 
        	ni->ni_stats.ns_tx_ucast + vap->iv_wapi_urekey_pkts;	
        return;
    }
		
    if(ni->ni_stats.ns_tx_ucast >= ni->ni_wapi_rekey_pkthresh)
    {
        IEEE80211_DELIVER_EVENT_STA_REKEYING(vap, ni->ni_macaddr);
        ni->ni_wapi_rekey_pkthresh = 
        	ni->ni_stats.ns_tx_ucast + vap->iv_wapi_urekey_pkts;
    }
    
    
}	

void wlan_wapi_multicast_rekey(struct ieee80211vap* vap, struct ieee80211_node *ni)
{
    uint8_t bcast_addr[IEEE80211_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
    if( ni )
    {
//      IEEE80211_DPRINTF(vap, IEEE80211_MSG_CRYPTO,"ni mcast:%d, threshold:%d\n",ni->ni_stats.ns_tx_mcast ,ni->ni_wapi_rekey_pkthresh);    
        if( ni->ni_wapi_rekey_pkthresh == 0)
        {
            ni->ni_wapi_rekey_pkthresh = 
            	ni->ni_stats.ns_tx_mcast + vap->iv_wapi_mrekey_pkts;
            return;
        }
		
        if(ni->ni_stats.ns_tx_mcast >= ni->ni_wapi_rekey_pkthresh)
        {
            IEEE80211_DELIVER_EVENT_STA_REKEYING(vap, bcast_addr);
            ni->ni_wapi_rekey_pkthresh = 
            	ni->ni_stats.ns_tx_mcast + vap->iv_wapi_mrekey_pkts;
        }
    }
}

int wlan_get_wapirekey_unicast(wlan_if_t vaphandle)
{
    struct ieee80211vap *vap = vaphandle;
    return (int)vap->iv_wapi_urekey_pkts;
}

int wlan_get_wapirekey_multicast(wlan_if_t vaphandle)
{
    struct ieee80211vap *vap = vaphandle;
    return (int)vap->iv_wapi_mrekey_pkts;

}

int wlan_set_wapirekey_unicast(wlan_if_t vaphandle, int value)
{
    struct ieee80211vap *vap = vaphandle;

    vap->iv_wapi_urekey_pkts = (u32) value;
    if( vap->iv_wapi_urekey_pkts !=0)
    {
        wlan_iterate_station_list(vap, 
		(ieee80211_sta_iter_func)wlan_wapi_unicast_update, (void*)vap);	
    }
    return 0;
}

int wlan_set_wapirekey_multicast(wlan_if_t vaphandle, int value)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211_node *ni;
		
    vap->iv_wapi_mrekey_pkts = (u32) value;
    ni = vap->iv_bss;
    if (vap->iv_wapi_mrekey_pkts && ni) 
    {
        ni->ni_wapi_rekey_pkthresh = 
        	ni->ni_stats.ns_tx_mcast + vap->iv_wapi_mrekey_pkts;
    }
    return 0;
}

int wlan_set_wapirekey_update(wlan_if_t vaphandle, unsigned char* macaddr)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211_node *ni;
	
    IEEE80211_DPRINTF(vap, IEEE80211_MSG_CRYPTO,
		"wapi rekey update for mac %s\n",ether_sprintf(macaddr));			
    ni = ieee80211_vap_find_node(vap, macaddr);
    if (ni == NULL)
        return -EINVAL;
    
    wlan_wapi_unicast_update(vap,ni);
    ieee80211_free_node(ni);
    return 0;
}

#if 0
int wapi_send_msg(struct ieee80211vap *vap, u_int8_t *msg, int len)
{
	struct net_device *dev = vap->iv_dev;
	union iwreq_data wreq;

	OS_MEMSET(&wreq, 0, sizeof(wreq));
	wreq.data.length = len;
	WIRELESS_SEND_EVENT(dev, IWEVCUSTOM, &wreq, msg);
	return 0;
}

int wapi_callback_wai(struct ieee80211vap *vap, 
			u_int8_t *addr, u_int8_t *ie, 
			u_int16_t msg_type)
{
	u_int8_t ie_len = 0;
	struct wapi_sta_msg_t sta_msg;
	
	OS_MEMSET(&sta_msg, 0, sizeof(struct wapi_sta_msg_t));	

	switch(msg_type){
	case WAPI_STA_AGING:
	case WAPI_UNICAST_REKEY:
	case WAPI_MUTIL_REKEY:
	case WAPI_STA_STATS:
		break;
	case WAPI_WAI_REQUEST:
	{
        OS_MEMCPY(sta_msg.gsn, vap->iv_wapimsk_keys.wk_txiv, IV_LEN);
		sta_msg.datalen += IV_LEN;
		ie_len = ie[1] + 2;
		OS_MEMCPY(sta_msg.wie, ie, ie_len);
		sta_msg.datalen += ie_len;
		break;
	}
	default:	
		return CALL_WAI_ERR_MSGTYPE;
	}
	OS_MEMCPY(sta_msg.vap_mac, vap->iv_myaddr, 6);
	sta_msg.datalen += 6 + 2;
	sta_msg.msg_type = msg_type;

	OS_MEMCPY(sta_msg.sta_mac, addr, 6);
	sta_msg.msg_type = msg_type;
	sta_msg.datalen += 6 + 2;
	return wapi_send_msg(vap, (u_int8_t *)&(sta_msg), sta_msg.datalen+4);
}

void wapi_notify_node_join(struct ieee80211_node *ni)
{
	int ret = 0;
	struct ieee80211vap *vap = ni->ni_vap;
	
	if ((vap->iv_opmode == IEEE80211_M_HOSTAP) &&
		(ieee80211_vap_wapi_is_set(vap))&&
		(ni != vap->iv_bss)&&
		(ni->ni_wpa_ie != NULL))
		{
			ni->ni_wkused = 0xFF;
			ni->ni_flags &= ~IEEE80211_NODE_AUTH;
			if(ni->ni_wpa_ie[1] + 2 >=256)
			{
				return;
			}
			ret = wapi_callback_wai(vap, ni->ni_macaddr, ni->ni_wpa_ie, WAPI_WAI_REQUEST);
		}
}

void wapi_notify_node_leave(struct ieee80211_node *ni)
{
	int ret = 0;
	struct ieee80211vap *vap = ni->ni_vap;
	if ((vap->iv_opmode == IEEE80211_M_HOSTAP) &&
		(ieee80211_vap_wapi_is_set(vap))&&
		(ni != vap->iv_bss))
		{
			ret =wapi_callback_wai(vap , ni->ni_macaddr, NULL, WAPI_STA_AGING);
		}
}
#endif

int wlan_setup_wapi(wlan_if_t vaphandle, int value)
{
    struct ieee80211vap *vap = vaphandle;
	struct ieee80211_rsnparms *rsn = &vap->iv_rsn;
    ieee80211_auth_mode authmode;

	/*1. set IEEE80211_PARAM_PRIVACY:*/
	IEEE80211_DPRINTF(vap, IEEE80211_MSG_CRYPTO,
		"set IEEE80211_FEXT_WAPI_ENABLE = %d\n", value);
	if (value) 
	{
		/* XXX check for key state? */
        IEEE80211_VAP_PRIVACY_ENABLE(vap);
		vap->iv_bss->ni_authmode = IEEE80211_AUTH_WAPI;
	} 
	else
	{	
        IEEE80211_VAP_PRIVACY_DISABLE(vap);
		vap->iv_bss->ni_authmode = IEEE80211_AUTH_OPEN;
		return 0;
	}
	
	if(	((value & WAPI_MCAST_SUITE_POLICY) == 0) ||
		((value & WAPI_UCAST_SUITE_POLICY) == 0) ||
		((value & WAPI_KEYMGT_ALGS_POLICY) == 0))
	{
		return -EINVAL;
	}
	/*
	 * 2.set IEEE80211_PARAM_MCASTCIPHER:
	 * WAPI says the multicast cipher is the lowest unicast
	 * cipher supported.  
	 */
    RSN_SET_MCAST_CIPHER(rsn, IEEE80211_CIPHER_WAPI);
	rsn->rsn_mcastkeylen = 128 / NBBY;
	
	/*
	 * 3.set IEEE80211_PARAM_UCASTCIPHERS:
	 *
	 * Default unicast cipher to WPI_SMS4 for WAPI use.  If
	 * WAPI is enabled the management code will set these
	 * values to reflect.
	 */

    RSN_SET_UCAST_CIPHER(rsn, IEEE80211_CIPHER_WAPI);
	rsn->rsn_ucastkeylen = 128 / NBBY;

	/*4.set IEEE80211_PARAM_KEYMGTALGS:*/
	/*
	 * We support both WAPI-PSK and WAPI-CERT; the one used
	 * is determined by the authentication mode and the
	 * setting of the PSK state.
	 */
	rsn->rsn_keymgmtset = 0; 
	if(value & BIT(2))
	{
		rsn->rsn_keymgmtset |= WAPI_ASE_WAI_PSK;
	}

	if(value & BIT(3))
	{
		rsn->rsn_keymgmtset |= WAPI_ASE_WAI_UNSPEC;
	}
	
	if(value & BIT(4))
	{
		rsn->rsn_caps = 1;
	}

    /*
     * 4. set auth mode
     */
    authmode = IEEE80211_AUTH_WAPI;
    wlan_set_authmodes((wlan_if_t)vap, &authmode, 1);

	/*5.IEEE80211_PARAM_WPA:*/
	vap->iv_flags &= ~IEEE80211_F_WPA;
    ieee80211_vap_wapi_set(vap);
	return 0;
}

#endif /*ATH_SUPPORT_WAPI*/

