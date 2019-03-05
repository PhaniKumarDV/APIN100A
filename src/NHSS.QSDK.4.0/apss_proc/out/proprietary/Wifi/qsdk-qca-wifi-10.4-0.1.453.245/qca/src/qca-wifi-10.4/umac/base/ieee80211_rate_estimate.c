/*
 * Copyright (c) 2015-2016 Qualcomm Atheros, Inc.
 *
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <ieee80211_var.h>
#include <ieee80211_channel.h>
#include <ieee80211_rateset.h>
#include <ieee80211_rate_estimate.h>

typedef struct SNRToPhyRateEntry {
    u_int8_t  snr;
    u_int16_t phyRate;
} SNRToPhyRateEntry_t;

#define MIN_NSS          1
#define MAX_NSS          4
#define MAX_RATES        10
#define MAX_SNR          0xFF
#define INVALID_LINK_CAP 0x0

const SNRToPhyRateEntry_t SNRToPhyRateTable[wlan_phymode_invalid]
                              [wlan_chwidth_invalid]
                              [MAX_NSS]
                              [MAX_RATES] =
{
    // Data is extracted from SNR_table_WHC_v01.xlsx, using the SNR and Rates
    // worksheets. When dealing with fractional values, the ceiling is used
    // for the SNR and the floor is used for the rate (to be conservative).

    // 802.11g/a mode - derived from 11n with 51% efficiency (due to
    //                  no AMPDU)
    // ================================================================
    {
        // 20 MHz
        {
            // 1 spatial stream
            {
                { 7  /* snr */,    3  /* rate */ },
                { 10 /* snr */,    6  /* rate */ },
                { 13 /* snr */,    9  /* rate */ },
                { 14 /* snr */,    13 /* rate */ },
                { 18 /* snr */,    19 /* rate */ },
                { 22 /* snr */,    26 /* rate */ },
                { 23 /* snr */,    29 /* rate */ },
                { 25 /* snr */,    33 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },
        // 40 MHz - not valid for this mode
        {
            // 1 spatial stream
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },
        // 80 MHz - not valid for this mode
        {
            // 1 spatial stream
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },
        // 160 MHz - not valid for this mode
        {
            // 1 spatial stream
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams (invalid for this mode)
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },
    },

    // 802.11n mode - note that MCS8 and 9 are always invalid
    // ================================================================
    {
        // 20 MHz
        {
            // 1 spatial stream
            {
                { 7 /* snr */,     6  /* rate */ },
                { 10 /* snr */,    13 /* rate */ },
                { 13 /* snr */,    19 /* rate */ },
                { 14 /* snr */,    26 /* rate */ },
                { 18 /* snr */,    39 /* rate */ },
                { 22 /* snr */,    52 /* rate */ },
                { 23 /* snr */,    58 /* rate */ },
                { 25 /* snr */,    65 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams
            {
                { 7 /* snr */,     13  /* rate */ },
                { 11 /* snr */,    26  /* rate */ },
                { 14 /* snr */,    39  /* rate */ },
                { 18 /* snr */,    52  /* rate */ },
                { 21 /* snr */,    78  /* rate */ },
                { 26 /* snr */,    104 /* rate */ },
                { 28 /* snr */,    117 /* rate */ },
                { 31 /* snr */,    130 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 7 /* snr */,     13  /* rate */ },
                { 11 /* snr */,    26  /* rate */ },
                { 14 /* snr */,    39  /* rate */ },
                { 18 /* snr */,    52  /* rate */ },
                { 21 /* snr */,    78  /* rate */ },
                { 26 /* snr */,    104 /* rate */ },
                { 28 /* snr */,    117 /* rate */ },
                { 31 /* snr */,    130 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 7 /* snr */,     13  /* rate */ },
                { 11 /* snr */,    26  /* rate */ },
                { 14 /* snr */,    39  /* rate */ },
                { 18 /* snr */,    52  /* rate */ },
                { 21 /* snr */,    78  /* rate */ },
                { 26 /* snr */,    104 /* rate */ },
                { 28 /* snr */,    117 /* rate */ },
                { 31 /* snr */,    130 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },

        // 40 MHz
        {
            // 1 spatial stream
            {
                { 6 /* snr */,     13  /* rate */ },
                { 9 /* snr */,     27  /* rate */ },
                { 12 /* snr */,    40  /* rate */ },
                { 13 /* snr */,    54  /* rate */ },
                { 17 /* snr */,    81  /* rate */ },
                { 21 /* snr */,    108 /* rate */ },
                { 22 /* snr */,    121 /* rate */ },
                { 24 /* snr */,    135 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams
            {
                { 6 /* snr */,     27  /* rate */ },
                { 10 /* snr */,    54  /* rate */ },
                { 13 /* snr */,    81  /* rate */ },
                { 17 /* snr */,    108 /* rate */ },
                { 21 /* snr */,    162 /* rate */ },
                { 25 /* snr */,    216 /* rate */ },
                { 27 /* snr */,    243 /* rate */ },
                { 30 /* snr */,    270 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 6 /* snr */,     27  /* rate */ },
                { 10 /* snr */,    54  /* rate */ },
                { 13 /* snr */,    81  /* rate */ },
                { 17 /* snr */,    108 /* rate */ },
                { 21 /* snr */,    162 /* rate */ },
                { 25 /* snr */,    216 /* rate */ },
                { 27 /* snr */,    243 /* rate */ },
                { 30 /* snr */,    270 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 6 /* snr */,     27  /* rate */ },
                { 10 /* snr */,    54  /* rate */ },
                { 13 /* snr */,    81  /* rate */ },
                { 17 /* snr */,    108 /* rate */ },
                { 21 /* snr */,    162 /* rate */ },
                { 25 /* snr */,    216 /* rate */ },
                { 27 /* snr */,    243 /* rate */ },
                { 30 /* snr */,    270 /* rate */ },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },

        // 80 MHz - not valid for 802.11n
        {
            // 1 spatial stream
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },

        // 160 MHz - not valid for 802.11n
        {
            // 1 spatial stream
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 2 spatial streams
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 3 spatial streams
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },

            // 4 spatial streams
            {
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
                { MAX_SNR,     INVALID_LINK_CAP },
            },
        },
    },

    // 802.11ac mode - identical to 802.11n but with MCS8 and 9 populated
    // ================================================================
    {
        // 20 MHz
        {
            // 1 spatial stream
            {
                { 7 /* snr */,     6  /* rate */ },
                { 10 /* snr */,    13 /* rate */ },
                { 13 /* snr */,    19 /* rate */ },
                { 14 /* snr */,    26 /* rate */ },
                { 18 /* snr */,    39 /* rate */ },
                { 22 /* snr */,    52 /* rate */ },
                { 23 /* snr */,    58 /* rate */ },
                { 25 /* snr */,    65 /* rate */ },
                { 31 /* snr */,    78 /* rate */ },
                { 32 /* snr */,    78 /* rate */ },
            },

            // 2 spatial streams
            {
                { 7 /* snr */,     13  /* rate */ },
                { 11 /* snr */,    26  /* rate */ },
                { 14 /* snr */,    39  /* rate */ },
                { 18 /* snr */,    52  /* rate */ },
                { 21 /* snr */,    78  /* rate */ },
                { 26 /* snr */,    104 /* rate */ },
                { 28 /* snr */,    117 /* rate */ },
                { 31 /* snr */,    130 /* rate */ },
                { 35 /* snr */,    156 /* rate */ },
                { 37 /* snr */,    156 /* rate */ },
            },

            // 3 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 7 /* snr */,     13  /* rate */ },
                { 11 /* snr */,    26  /* rate */ },
                { 14 /* snr */,    39  /* rate */ },
                { 18 /* snr */,    52  /* rate */ },
                { 21 /* snr */,    78  /* rate */ },
                { 26 /* snr */,    104 /* rate */ },
                { 28 /* snr */,    117 /* rate */ },
                { 31 /* snr */,    130 /* rate */ },
                { 35 /* snr */,    156 /* rate */ },
                { 37 /* snr */,    156 /* rate */ },
            },

            // 4 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 7 /* snr */,     13  /* rate */ },
                { 11 /* snr */,    26  /* rate */ },
                { 14 /* snr */,    39  /* rate */ },
                { 18 /* snr */,    52  /* rate */ },
                { 21 /* snr */,    78  /* rate */ },
                { 26 /* snr */,    104 /* rate */ },
                { 28 /* snr */,    117 /* rate */ },
                { 31 /* snr */,    130 /* rate */ },
                { 35 /* snr */,    156 /* rate */ },
                { 37 /* snr */,    156 /* rate */ },
            },
        },

        // 40 MHz
        {
            // 1 spatial stream
            {
                { 6 /* snr */,     13  /* rate */ },
                { 9 /* snr */,     27  /* rate */ },
                { 12 /* snr */,    40  /* rate */ },
                { 13 /* snr */,    54  /* rate */ },
                { 17 /* snr */,    81  /* rate */ },
                { 21 /* snr */,    108 /* rate */ },
                { 22 /* snr */,    121 /* rate */ },
                { 24 /* snr */,    135 /* rate */ },
                { 30 /* snr */,    162 /* rate */ },
                { 31 /* snr */,    180 /* rate */ },
            },

            // 2 spatial streams
            {
                { 6 /* snr */,     27  /* rate */ },
                { 10 /* snr */,    54  /* rate */ },
                { 13 /* snr */,    81  /* rate */ },
                { 17 /* snr */,    108 /* rate */ },
                { 21 /* snr */,    162 /* rate */ },
                { 25 /* snr */,    216 /* rate */ },
                { 27 /* snr */,    243 /* rate */ },
                { 30 /* snr */,    270 /* rate */ },
                { 34 /* snr */,    324 /* rate */ },
                { 36 /* snr */,    360 /* rate */ },
            },

            // 3 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 6 /* snr */,     27  /* rate */ },
                { 10 /* snr */,    54  /* rate */ },
                { 13 /* snr */,    81  /* rate */ },
                { 17 /* snr */,    108 /* rate */ },
                { 21 /* snr */,    162 /* rate */ },
                { 25 /* snr */,    216 /* rate */ },
                { 27 /* snr */,    243 /* rate */ },
                { 30 /* snr */,    270 /* rate */ },
                { 34 /* snr */,    324 /* rate */ },
                { 36 /* snr */,    360 /* rate */ },
            },

            // 4 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 6 /* snr */,     27  /* rate */ },
                { 10 /* snr */,    54  /* rate */ },
                { 13 /* snr */,    81  /* rate */ },
                { 17 /* snr */,    108 /* rate */ },
                { 21 /* snr */,    162 /* rate */ },
                { 25 /* snr */,    216 /* rate */ },
                { 27 /* snr */,    243 /* rate */ },
                { 30 /* snr */,    270 /* rate */ },
                { 34 /* snr */,    324 /* rate */ },
                { 36 /* snr */,    360 /* rate */ },
            },
        },

        // 80 MHz
        {
            // 1 spatial stream
            {
                { 5 /* snr */,     29  /* rate */ },
                { 8 /* snr */,     58  /* rate */ },
                { 11 /* snr */,    87  /* rate */ },
                { 12 /* snr */,    117 /* rate */ },
                { 16 /* snr */,    175 /* rate */ },
                { 20 /* snr */,    234 /* rate */ },
                { 21 /* snr */,    263 /* rate */ },
                { 23 /* snr */,    292 /* rate */ },
                { 29 /* snr */,    351 /* rate */ },
                { 30 /* snr */,    390 /* rate */ },
            },

            // 2 spatial streams
            {
                { 5 /* snr */,     58  /* rate */ },
                { 9 /* snr */,     117 /* rate */ },
                { 12 /* snr */,    175 /* rate */ },
                { 16 /* snr */,    234 /* rate */ },
                { 20 /* snr */,    351 /* rate */ },
                { 24 /* snr */,    468 /* rate */ },
                { 26 /* snr */,    526 /* rate */ },
                { 29 /* snr */,    585 /* rate */ },
                { 33 /* snr */,    702 /* rate */ },
                { 35 /* snr */,    780 /* rate */ },
            },

            // 3 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 5 /* snr */,     58  /* rate */ },
                { 9 /* snr */,     117 /* rate */ },
                { 12 /* snr */,    175 /* rate */ },
                { 16 /* snr */,    234 /* rate */ },
                { 20 /* snr */,    351 /* rate */ },
                { 24 /* snr */,    468 /* rate */ },
                { 26 /* snr */,    526 /* rate */ },
                { 29 /* snr */,    585 /* rate */ },
                { 33 /* snr */,    702 /* rate */ },
                { 35 /* snr */,    780 /* rate */ },
            },

            // 4 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 5 /* snr */,     58  /* rate */ },
                { 9 /* snr */,     117 /* rate */ },
                { 12 /* snr */,    175 /* rate */ },
                { 16 /* snr */,    234 /* rate */ },
                { 20 /* snr */,    351 /* rate */ },
                { 24 /* snr */,    468 /* rate */ },
                { 26 /* snr */,    526 /* rate */ },
                { 29 /* snr */,    585 /* rate */ },
                { 33 /* snr */,    702 /* rate */ },
                { 35 /* snr */,    780 /* rate */ },
            },
        },

        // 160 MHz - assumed to double the data rate of 80 MHz
        {
            // 1 spatial stream
            {
                { 5 /* snr */,     58  /* rate */ },
                { 8 /* snr */,     116 /* rate */ },
                { 11 /* snr */,    174 /* rate */ },
                { 12 /* snr */,    234 /* rate */ },
                { 16 /* snr */,    350 /* rate */ },
                { 20 /* snr */,    468 /* rate */ },
                { 21 /* snr */,    526 /* rate */ },
                { 23 /* snr */,    584 /* rate */ },
                { 29 /* snr */,    702 /* rate */ },
                { 30 /* snr */,    780 /* rate */ },
            },

            // 2 spatial streams
            {
                { 5 /* snr */,     116  /* rate */ },
                { 9 /* snr */,     234  /* rate */ },
                { 12 /* snr */,    350  /* rate */ },
                { 16 /* snr */,    468  /* rate */ },
                { 20 /* snr */,    702  /* rate */ },
                { 24 /* snr */,    936  /* rate */ },
                { 26 /* snr */,    1052 /* rate */ },
                { 29 /* snr */,    1170 /* rate */ },
                { 33 /* snr */,    1404 /* rate */ },
                { 35 /* snr */,    1560 /* rate */ },
            },

            // 3 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 5 /* snr */,     116  /* rate */ },
                { 9 /* snr */,     234  /* rate */ },
                { 12 /* snr */,    350  /* rate */ },
                { 16 /* snr */,    468  /* rate */ },
                { 20 /* snr */,    702  /* rate */ },
                { 24 /* snr */,    936  /* rate */ },
                { 26 /* snr */,    1052 /* rate */ },
                { 29 /* snr */,    1170 /* rate */ },
                { 33 /* snr */,    1404 /* rate */ },
                { 35 /* snr */,    1560 /* rate */ },
            },

            // 4 spatial streams - assumed to be the same as 2 NSS due to no
            // data available at this time
            {
                { 5 /* snr */,     116  /* rate */ },
                { 9 /* snr */,     234  /* rate */ },
                { 12 /* snr */,    350  /* rate */ },
                { 16 /* snr */,    468  /* rate */ },
                { 20 /* snr */,    702  /* rate */ },
                { 24 /* snr */,    936  /* rate */ },
                { 26 /* snr */,    1052 /* rate */ },
                { 29 /* snr */,    1170 /* rate */ },
                { 33 /* snr */,    1404 /* rate */ },
                { 35 /* snr */,    1560 /* rate */ },
            },
        },

    }
};

u_int16_t ieee80211_SNRToPhyRateTablePerformLookup(
        struct ieee80211vap *vap,
        u_int8_t snr, int nss,
        wlan_phymode_e phyMode,
        wlan_chwidth_e chwidth) {
    size_t i;
    const SNRToPhyRateEntry_t *entries;

    if(phyMode >= wlan_phymode_invalid ||
            chwidth >= wlan_chwidth_invalid ||
            nss > MAX_NSS ||
            nss < MIN_NSS)
        return INVALID_LINK_CAP;

    entries = SNRToPhyRateTable[phyMode][chwidth][nss - 1];

    // Although this could be done with a binary search, for the small
    // size of this array, it is not likely worth the complexity. Thus,
    // we search through the array until we find an entry whose SNR is
    // larger than the client one. The entry right before this is the
    // one to use.
    for (i = 0; i < MAX_RATES; ++i) {
        if (snr < entries[i].snr) {
            break;
        }
    }

    if (0 == i) {
        return INVALID_LINK_CAP;
    } else {
        return entries[i - 1].phyRate;
    }
}

