/*
 * Copyright (c) 2010, Atheros Communications Inc. 
 * All Rights Reserved.
 * 
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * 
 */

#include "ieee80211_mlme_priv.h"

#if UMAC_SUPPORT_BTAMP

void ieee80211_recv_beacon_btamp(struct ieee80211_node *ni, wbuf_t wbuf, int subtype, 
                                 struct ieee80211_rx_status *rs, ieee80211_scan_entry_t scan_entry)
{
    struct ieee80211vap       *tmpvap, *vap = ni->ni_vap;
    struct ieee80211com       *ic = ni->ni_ic;
    u_int16_t                 erp;
    u_int8_t                  *htcap = NULL, *htinfo = NULL;
    u_int8_t                  *wme;
    u_int8_t                  *rates, *xrates, i;
    struct ieee80211_frame    *wh;

    wh = (struct ieee80211_frame *)wbuf_header(wbuf);
	erp = ieee80211_scan_entry_erpinfo(scan_entry);

    if (ni != vap->iv_bss) {

        /* Beacon from peer, setup node for association */
        ni->ni_chan = ieee80211_scan_entry_channel(scan_entry);
        ni->ni_intval = ieee80211_scan_entry_beacon_interval(scan_entry);
        ni->ni_capinfo = ieee80211_scan_entry_capinfo(scan_entry);

        /* update WMM capability */
        if (((wme = ieee80211_scan_entry_wmeinfo_ie(scan_entry))  != NULL) ||
            ((wme = ieee80211_scan_entry_wmeparam_ie(scan_entry)) != NULL)) {
            ni->ni_ext_caps |= IEEE80211_NODE_C_QOS;
        }

        /* Parse HT capability */
        htcap  = ieee80211_scan_entry_htcap(scan_entry);
        htinfo = ieee80211_scan_entry_htinfo(scan_entry);
        if (htcap) {
            ieee80211_parse_htcap(ni, htcap);
        }
        if (htinfo) {
            ieee80211_parse_htinfo(ni, htinfo);
        }

        /* NB: must be after ni_chan is setup */
        rates = ieee80211_scan_entry_rates(scan_entry);
        xrates = ieee80211_scan_entry_xrates(scan_entry);
        if (rates)
            ieee80211_setup_rates(ni, rates, xrates, IEEE80211_F_DOXSECT);
        if (htcap != NULL)
            ieee80211_setup_ht_rates(ni, htcap, IEEE80211_F_DOXSECT);
        if (htinfo != NULL)
            ieee80211_setup_basic_ht_rates(ni, htinfo);
 
        /* Find min basic supported rate */
        ni->ni_minbasicrate = 0;
        for (i=0; i < ni->ni_rates.rs_nrates; i++) {
            if ((ni->ni_minbasicrate == 0) ||
                ((ni->ni_minbasicrate & IEEE80211_RATE_VAL) > (ni->ni_rates.rs_rates[i] & IEEE80211_RATE_VAL))) {
                ni->ni_minbasicrate = ni->ni_rates.rs_rates[i];
            }
        }

        /* parse WPA/RSN IE and setup RSN info */
        if (ni->ni_capinfo & IEEE80211_CAPINFO_PRIVACY) {
            struct ieee80211_rsnparms rsn;
            u_int8_t *rsn_ie;
            int status = IEEE80211_STATUS_SUCCESS;

            rsn = ni->ni_rsn;
            rsn_ie = ieee80211_scan_entry_rsn(scan_entry);

            if (rsn_ie != NULL)
                status = ieee80211_parse_rsn(vap, rsn_ie, &rsn);
            
            /* if a RSN IE was not there, or it's not valid, check the WPA IE */
            if ((rsn_ie == NULL) || (status != IEEE80211_STATUS_SUCCESS)) {
                IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE,
                                  "%s: invalid security settings for node %s\n",
                                  __func__, ether_sprintf(ni->ni_macaddr));
            }
            ni->ni_rsn = rsn;   /* update rsn parameters */
        }

        /* tsf of last beacon */
        OS_MEMCPY(ni->ni_tstamp.data, ieee80211_scan_entry_tsf(scan_entry), sizeof(ni->ni_tstamp));

        /* record absolute time of last beacon */
        vap->iv_last_beacon_time = OS_GET_TIMESTAMP();

        ic->ic_beacon_update(ni, rs->rs_rssi);

        /*
         * check for ERP change
         */
        if (ni->ni_erp != erp) {
            IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "erp change: was 0x%x, now 0x%x", ni->ni_erp, erp);
            if (ic->ic_reg_parm.ampIgnoreNonERP == 0 &&
                (erp & IEEE80211_ERP_USE_PROTECTION) && (!IEEE80211_IS_PROTECTION_ENABLED(ic))) {
                IEEE80211_DPRINTF(vap,IEEE80211_MSG_INPUT,
                                  "setting protection bit (beacon from %s)\n",
                                  ether_sprintf(wh->i_addr2));
                IEEE80211_ENABLE_PROTECTION(ic);
		        TAILQ_FOREACH(tmpvap, &(ic)->ic_vaps, iv_next) {
			        ieee80211_vap_erpupdate_set(tmpvap);
                }
                ic->ic_update_protmode(ic);
                ieee80211_set_shortslottime(ic, 0); /* Disable Short Slot Time */

                if (erp & IEEE80211_ERP_LONG_PREAMBLE)
                    IEEE80211_ENABLE_BARKER(ic);
                else
                    IEEE80211_DISABLE_BARKER(ic);
            }
            ni->ni_erp = erp;
        }

        if (IEEE80211_ADDR_EQ(wh->i_addr3, ieee80211_node_get_bssid(ni)) &&
            (!ni->ni_associd)) {
            ieee80211_mlme_join_complete_btamp(ni);
        }

    } else {

        /* Update protection mode when in 11G mode */
        if (IEEE80211_IS_CHAN_ANYG(ic->ic_curchan) ||
            IEEE80211_IS_CHAN_11NG(ic->ic_curchan)) {
            if (ic->ic_reg_parm.ampIgnoreNonERP == 0 &&
                ic->ic_protmode != IEEE80211_PROT_NONE && (erp & IEEE80211_ERP_NON_ERP_PRESENT)) {
                if (!IEEE80211_IS_PROTECTION_ENABLED(ic)) {
                    IEEE80211_DPRINTF(vap,IEEE80211_MSG_INPUT,
                                      "setting protection bit "
                                      "(beacon from %s)\n",
                                      ether_sprintf(wh->i_addr2));
                    IEEE80211_ENABLE_PROTECTION(ic);
		            TAILQ_FOREACH(tmpvap, &(ic)->ic_vaps, iv_next) {
			            ieee80211_vap_erpupdate_set(tmpvap);
                    }
                    ic->ic_update_protmode(ic);
                    ieee80211_set_shortslottime(ic, 0);
                }
            }
            ic->ic_last_nonerp_present = OS_GET_TIMESTAMP();
        }
    }
}

#endif
