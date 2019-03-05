/*
 *  Copyright (c) 2008 Atheros Communications Inc. 
 * All Rights Reserved.
 * 
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * 
 */

#ifndef _NET80211_IEEE80211_REGDMN_H
#define _NET80211_IEEE80211_REGDMN_H

#include <ieee80211_var.h>

#ifdef UMAC_SUPPORT_REGDMN

void ieee80211_mark_dfs(struct ieee80211com *ic, struct ieee80211_channel *ichan);
#if ATH_SUPPORT_ZERO_CAC_DFS
void ieee80211_mark_precac_dfs(struct ieee80211com *ic, uint8_t is_radar_found_on_secondary_seg);
bool ieee80211_is_precac_timer_running(struct ieee80211com *ic);
struct ieee80211_channel * ieee80211_dfs_find_precac_secondary_vht80_chan(struct ieee80211com *ic);
#endif
void ieee80211_send_rcsa(struct ieee80211com *ic, struct ieee80211vap *vap, u_int8_t rcsa_count, u_int8_t ieee_chan);
int ieee80211_dfs_action(struct ieee80211vap *vap, struct ieee80211_channelswitch_ie *pcsaie);
int ieee80211_set_country_code(struct ieee80211com *ic, char* isoName, u_int16_t cc, enum ieee80211_clist_cmd cmd);
void ieee80211_update_spectrumrequirement(struct ieee80211vap *vap);
int ieee80211_update_country_allowed(struct ieee80211com *ic);
void ieee80211_set_regclassids(struct ieee80211com *ic, const u_int8_t *regclassids, u_int nregclass);
#if ATH_SUPPORT_IBSS_DFS
void ieee80211_ibss_beacon_update_start(struct ieee80211com *ic);
void ieee80211_ibss_beacon_update_stop(struct ieee80211com *ic);
#endif /* ATH_SUPPORT_IBSS_DFS */
#else

#define ieee80211_mark_dfs(ic,ichan)                         /**/
#define ieee80211_send_rcsa(ic,vap,rcsa_count,ieee_chan)     /**/
#define ieee80211_dfs_action(struct ieee80211vap *vap, struct ieee80211_channelswitch_ie *pcsaie) /**/
#define ieee80211_set_country_code(ic,isoName)                0
#define ieee80211_update_spectrumrequirement(vap)            /**/
#define ieee80211_update_country_allowed(ic)                  0
#define ieee80211_set_regclassids(ic,regclassids,nregclass)  /**/
#if ATH_SUPPORT_IBSS_DFS
#define ieee80211_ibss_beacon_update_start(ic)               /**/
#define ieee80211_ibss_beacon_update_stop(ic)                /**/
#endif /* ATH_SUPPORT_IBSS_DFS */
#endif

#define MAX_CHANNELS_PER_OPERATING_CLASS  24

typedef struct regdmn_op_class_map {
    uint8_t op_class;
    enum ieee80211_cwm_width ch_width;
    uint8_t sec20_offset;
    uint8_t ch_set[MAX_CHANNELS_PER_OPERATING_CLASS];
} regdmn_op_class_map_t;

uint8_t
regdmn_get_new_opclass_from_channel(struct ieee80211com *ic,
                                    struct ieee80211_channel *channel);

#endif /* _NET80211_IEEE80211_REGDMN_H */
