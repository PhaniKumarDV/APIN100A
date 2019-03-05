/*
 *  Copyright (c) 2008 Atheros Communications Inc.
 * All Rights Reserved.
 *
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 */

#include <ieee80211_var.h>
#include <ieee80211_channel.h>
#include <ieee80211_api.h>
#include <ieee80211_rateset.h>
#include <if_smart_ant.h>
#include <qdf_lock.h>

int ieee80211_ucfg_set_chanswitch(wlan_if_t vaphandle, u_int8_t chan, u_int8_t tbtt, u_int16_t ch_width);

const char *ieee80211_phymode_name[] = {
    "auto",             /* IEEE80211_MODE_AUTO */
    "11a",              /* IEEE80211_MODE_11A */
    "11b",              /* IEEE80211_MODE_11B */
    "11g",              /* IEEE80211_MODE_11G */
    "FH",               /* IEEE80211_MODE_FH */
    "turboA",           /* IEEE80211_MODE_TURBO_A */
    "turboG",           /* IEEE80211_MODE_TURBO_G */
    "11naht20",         /* IEEE80211_MODE_11NA_HT20 */
    "11nght20",         /* IEEE80211_MODE_11NG_HT20 */
    "11naht40plus",     /* IEEE80211_MODE_11NA_HT40PLUS */
    "11naht40minus",    /* IEEE80211_MODE_11NA_HT40MINUS */
    "11nght40plus",     /* IEEE80211_MODE_11NG_HT40PLUS */
    "11nght40minus",    /* IEEE80211_MODE_11NG_HT40MINUS */
    "11nght40",         /* IEEE80211_MODE_11NG_HT40 */
    "11naht40",         /* IEEE80211_MODE_11NA_HT40 */
    "11acvht20",        /* IEEE80211_MODE_11AC_VHT20 */
    "11acvht40plus",    /* IEEE80211_MODE_11AC_VHT40PLUS */
    "11acvht40minus",   /* IEEE80211_MODE_11AC_VHT40MINUS */
    "11acvht40",        /* IEEE80211_MODE_11AC_VHT40 */
    "11acvht80",        /* IEEE80211_MODE_11AC_VHT80 */
    "11acvht160",       /* IEEE80211_MODE_11AC_VHT160 */
    "11acvht80_80",     /* IEEE80211_MODE_11AC_VHT80_80 */
};

/*
 * Return the phy mode for with the specified channel.
 */
enum ieee80211_phymode
ieee80211_chan2mode(const struct ieee80211_channel *chan)
{
    if (IEEE80211_IS_CHAN_108G(chan))
        return IEEE80211_MODE_TURBO_G;
    else if (IEEE80211_IS_CHAN_TURBO(chan))
        return IEEE80211_MODE_TURBO_A;
    else if (IEEE80211_IS_CHAN_A(chan))
        return IEEE80211_MODE_11A;
    else if (IEEE80211_IS_CHAN_ANYG(chan))
        return IEEE80211_MODE_11G;
    else if (IEEE80211_IS_CHAN_B(chan))
        return IEEE80211_MODE_11B;
    else if (IEEE80211_IS_CHAN_FHSS(chan))
        return IEEE80211_MODE_FH;
    else if (IEEE80211_IS_CHAN_11NA_HT20(chan))
        return IEEE80211_MODE_11NA_HT20;
    else if (IEEE80211_IS_CHAN_11NG_HT20(chan))
        return IEEE80211_MODE_11NG_HT20;
    else if (IEEE80211_IS_CHAN_11NA_HT40PLUS(chan))
        return IEEE80211_MODE_11NA_HT40PLUS;
    else if (IEEE80211_IS_CHAN_11NA_HT40MINUS(chan))
        return IEEE80211_MODE_11NA_HT40MINUS;
    else if (IEEE80211_IS_CHAN_11NG_HT40PLUS(chan))
        return IEEE80211_MODE_11NG_HT40PLUS;
    else if (IEEE80211_IS_CHAN_11NG_HT40MINUS(chan))
        return IEEE80211_MODE_11NG_HT40MINUS;
    else if (IEEE80211_IS_CHAN_11AC_VHT20(chan))
        return IEEE80211_MODE_11AC_VHT20;
    else if (IEEE80211_IS_CHAN_11AC_VHT40PLUS(chan))
        return IEEE80211_MODE_11AC_VHT40PLUS;
    else if (IEEE80211_IS_CHAN_11AC_VHT40MINUS(chan))
        return IEEE80211_MODE_11AC_VHT40MINUS;
    else if (IEEE80211_IS_CHAN_11AC_VHT80(chan))
        return IEEE80211_MODE_11AC_VHT80;
    else if (IEEE80211_IS_CHAN_11AC_VHT160(chan))
        return IEEE80211_MODE_11AC_VHT160;
    else if (IEEE80211_IS_CHAN_11AC_VHT80_80(chan))
        return IEEE80211_MODE_11AC_VHT80_80;

    /* NB: should not get here */
    printk("%s: cannot map channel to mode; freq %u flags 0x%x\n",
           __func__, chan->ic_freq, chan->ic_flags);
    return IEEE80211_MODE_11B;
}

char
channel_type(wlan_chan_t c)
{
    if (IEEE80211_IS_CHAN_ST(c))
        return 'S';
    if (IEEE80211_IS_CHAN_108A(c))
        return 'T';
    if (IEEE80211_IS_CHAN_108G(c))
        return 'G';
    if (IEEE80211_IS_CHAN_A(c))
        return 'a';
    if (IEEE80211_IS_CHAN_ANYG(c))
        return 'g';
    if (IEEE80211_IS_CHAN_11NA(c))
        return 'n';
    if (IEEE80211_IS_CHAN_11NG(c))
        return 'n';
    if (IEEE80211_IS_CHAN_B(c))
        return 'b';
    return 'f';
}

/*
 * Check for Channel and Mode consistency. If channel and mode mismatches, return error.
 */
#define IEEE80211_MODE_TURBO_STATIC_A   IEEE80211_MODE_MAX
int
ieee80211_check_chan_mode_consistency(struct ieee80211com *ic,int mode,struct ieee80211_channel *c)
{
    if (c == IEEE80211_CHAN_ANYC) return 0;

    switch (mode)
    {
    case IEEE80211_MODE_11B:
        if(IEEE80211_IS_CHAN_B(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11G:
        if(IEEE80211_IS_CHAN_ANYG(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11A:
        if(IEEE80211_IS_CHAN_A(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_TURBO_STATIC_A:
        if(IEEE80211_IS_CHAN_A(c) && IEEE80211_IS_CHAN_STURBO(c) )
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_AUTO:
        return 0;
        break;

    case IEEE80211_MODE_11NG_HT20:
        if(IEEE80211_IS_CHAN_11NG_HT20(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11NG_HT40PLUS:
        if(IEEE80211_IS_CHAN_11NG_HT40PLUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11NG_HT40MINUS:
        if(IEEE80211_IS_CHAN_11NG_HT40MINUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11NG_HT40:
        if(IEEE80211_IS_CHAN_11NG_HT40MINUS(c) || IEEE80211_IS_CHAN_11NG_HT40PLUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11NA_HT20:
        if(IEEE80211_IS_CHAN_11NA_HT20(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11NA_HT40PLUS:
        if(IEEE80211_IS_CHAN_11NA_HT40PLUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11NA_HT40MINUS:
        if(IEEE80211_IS_CHAN_11NA_HT40MINUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11NA_HT40:
        if(IEEE80211_IS_CHAN_11NA_HT40MINUS(c) || IEEE80211_IS_CHAN_11NA_HT40PLUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11AC_VHT20:
        if(IEEE80211_IS_CHAN_11AC_VHT20(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11AC_VHT40PLUS:
        if(IEEE80211_IS_CHAN_11AC_VHT40PLUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11AC_VHT40MINUS:
        if(IEEE80211_IS_CHAN_11AC_VHT40MINUS(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11AC_VHT40:
        if(IEEE80211_IS_CHAN_11AC_VHT40(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11AC_VHT80:
        if(IEEE80211_IS_CHAN_11AC_VHT80(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11AC_VHT160:
        if(IEEE80211_IS_CHAN_11AC_VHT160(c))
            return 0;
        else
            return -EINVAL;
        break;

    case IEEE80211_MODE_11AC_VHT80_80:
        if(IEEE80211_IS_CHAN_11AC_VHT80_80(c))
            return 0;
        else
            return -EINVAL;
        break;
    }
    return -EINVAL;
}
#undef  IEEE80211_MODE_TURBO_STATIC_A

/*
 * Convert MHz frequency to IEEE channel number.
 */
u_int
ieee80211_mhz2ieee(struct ieee80211com *ic, u_int freq, u_int flags)
{
    /* XXX: The only reason to call low-level driver here is to
     * let HAL take care of public safety channel configurations. */
    return ic->ic_mhz2ieee(ic, freq, flags);
}

/*
 * Convert IEEE channel number to MHz frequency.
 */
u_int
ieee80211_ieee2mhz(struct ieee80211com *ic, u_int chan, u_int flags)
{
    struct ieee80211_channel *c;
    int i;
    /* If it is current channel avoid complete channel list search */
    c = ic->ic_curchan;
    if ((c != NULL) && (c->ic_ieee == chan)) {
        return c->ic_freq;
    }

    /* brute force search */
    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];

        if (c->ic_ieee == chan) {
            return c->ic_freq;
        }
    }
    /* If freq is not found in the channel list, then return freq based on
       channel number */
    if (flags & IEEE80211_CHAN_2GHZ) {  /* 2GHz band */
        if (chan == 14)
            return 2484;
        if (chan < 14)
            return 2407 + chan*5;
        else
            return 2512 + ((chan-15)*20);
    } else if (flags & IEEE80211_CHAN_5GHZ) {/* 5Ghz band */
        return 5000 + (chan*5);
    } else {                /* either, guess */
        if (chan == 14)
            return 2484;
        if (chan < 14)          /* 0-13 */
            return 2407 + chan*5;
        if (chan < 27)          /* 15-26 */
            return 2512 + ((chan-15)*20);
        return 5000 + (chan*5);
    }
}

u_int32_t
wlan_mhz2ieee(wlan_dev_t devhandle, u_int32_t freq, u_int32_t flags)
{
    struct ieee80211com *ic = devhandle;

    return ieee80211_mhz2ieee(ic, freq, flags);
}

u_int32_t
wlan_ieee2mhz(wlan_dev_t devhandle, int ieeechan)
{
    struct ieee80211com *ic = (struct ieee80211com *)devhandle;
    return ieee80211_ieee2mhz(ic, ieeechan, 0);
}

/*
 * Locate a channel given a frequency+flags.  We cache
 * the previous lookup to optimize swithing between two
 * channels--as happens with dynamic turbo.
 * This verifies that found channels have not been excluded because of 11d.
 */
struct ieee80211_channel *
ieee80211_find_channel(struct ieee80211com *ic, int freq, u_int8_t des_cfreq2, u_int32_t flags)
{
    struct ieee80211_channel *c;
    int i;

    flags &= IEEE80211_CHAN_ALLTURBO;
    c = ic->ic_prevchan;
    if ((c != NULL) &&
            (! IEEE80211_IS_CHAN_11D_EXCLUDED(c)) &&
            (c->ic_freq == freq) &&
            ((c->ic_flags & IEEE80211_CHAN_ALLTURBO) == flags)) {
        if (flags == IEEE80211_CHAN_11AC_VHT80_80) {
            if (c->ic_vhtop_ch_freq_seg2 == des_cfreq2) {
                return c;
            }
        }
        else {
            return c;
        }
    }

    /* brute force search */
    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];

        if ((! IEEE80211_IS_CHAN_11D_EXCLUDED(c)) &&
                (c->ic_freq == freq) &&
                ((c->ic_flags & IEEE80211_CHAN_ALLTURBO) == flags)) {
            if (flags == IEEE80211_CHAN_11AC_VHT80_80) {
                if (des_cfreq2 == 0 || c->ic_vhtop_ch_freq_seg2 == des_cfreq2) {
                    return c;
                }
            }
            else {
                return c;
            }
        }
    }

    return NULL;
}
#ifdef __linux__
//#ifndef ATH_WLAN_COMBINE
#endif
struct ieee80211_channel *
ieee80211_doth_findchan(struct ieee80211vap *vap, u_int8_t chan)
{
    struct ieee80211com *ic = vap->iv_ic;
    struct ieee80211_channel *c;
    u_int32_t flags, freq;

    /* NB: try first to preserve turbo */
    flags = vap->iv_bsschan->ic_flags & IEEE80211_CHAN_ALL;
    freq = ieee80211_ieee2mhz(ic, chan, 0);
    c = ieee80211_find_channel(ic, freq, vap->iv_des_cfreq2, flags);
    if (c == NULL)
        c = ieee80211_find_channel(ic, freq, 0, 0);
    return c;
}

/*
 * Locate the channel given channel number and mode
 */
struct ieee80211_channel *
ieee80211_find_dot11_channel(struct ieee80211com *ic, int ieee, u_int8_t des_cfreq2, enum ieee80211_phymode mode)
{
    /* TODO: When making regulatory changes for 11AC please, please optimize this function */

    static const u_int chanflags[] = {
        0,                              /* IEEE80211_MODE_AUTO */
        IEEE80211_CHAN_A,               /* IEEE80211_MODE_11A */
        IEEE80211_CHAN_B,               /* IEEE80211_MODE_11B */
        IEEE80211_CHAN_PUREG,           /* IEEE80211_MODE_11G */
        IEEE80211_CHAN_FHSS,            /* IEEE80211_MODE_FH */
        IEEE80211_CHAN_108A,            /* IEEE80211_MODE_TURBO_A */
        IEEE80211_CHAN_108G,            /* IEEE80211_MODE_TURBO_G */
        IEEE80211_CHAN_11NA_HT20,       /* IEEE80211_MODE_11NA_HT20 */
        IEEE80211_CHAN_11NG_HT20,       /* IEEE80211_MODE_11NG_HT20 */
        IEEE80211_CHAN_11NA_HT40PLUS,   /* IEEE80211_MODE_11NA_HT40PLUS */
        IEEE80211_CHAN_11NA_HT40MINUS,  /* IEEE80211_MODE_11NA_HT40MINUS */
        IEEE80211_CHAN_11NG_HT40PLUS,   /* IEEE80211_MODE_11NG_HT40PLUS */
        IEEE80211_CHAN_11NG_HT40MINUS,  /* IEEE80211_MODE_11NG_HT40MINUS */
        0,                              /* IEEE80211_MODE_11NG_HT40 */
        0,                              /* IEEE80211_MODE_11NA_HT40 */
        IEEE80211_CHAN_11AC_VHT20,      /* IEEE80211_MODE_11AC_VHT20 */
        IEEE80211_CHAN_11AC_VHT40PLUS,  /* IEEE80211_MODE_11AC_VHT40PLUS */
        IEEE80211_CHAN_11AC_VHT40MINUS, /* IEEE80211_MODE_11AC_VHT40MINUS */
        0,                              /* IEEE80211_MODE_11AC_VHT40 */
        IEEE80211_CHAN_11AC_VHT80,      /* IEEE80211_MODE_11AC_VHT80 */
        IEEE80211_CHAN_11AC_VHT160,     /* IEEE80211_MODE_11AC_VHT160 */
        IEEE80211_CHAN_11AC_VHT80_80,   /* IEEE80211_MODE_11AC_VHT80_80 */
        IEEE80211_CHAN_ST,              /* IEEE80211_MODE_TURBO_STATIC_A */
    };
    u_int modeflags;
    int i;

    wlan_if_t vap;
    modeflags = mode & (IEEE80211_CHAN_HALF | IEEE80211_CHAN_QUARTER);
    mode &= ~modeflags;
    modeflags |= chanflags[mode];
    vap =  TAILQ_FIRST(&(ic)->ic_vaps);

    for (i = 0; i < ic->ic_nchans; i++) {
        struct ieee80211_channel *c = &ic->ic_channels[i];
        struct ieee80211_channel *nc;

        if (ieee && (c->ic_ieee != ieee))
            continue;

        if (mode == IEEE80211_MODE_AUTO) {
            if (IEEE80211_IS_CHAN_TURBO(c)) {
                /* ignore turbo channels for autoselect */
                continue;
            }

            if (IEEE80211_IS_CHAN_2GHZ(c)) {
                /*
                 * Precedence is 11NG channels first, then 11G, then 11B.
                 * HT40 channels are ignored when IEEE80211_MODE_AUTO is used.
                 */
                if ((ic->ic_modecaps & (1<<IEEE80211_MODE_11NG_HT20)) &&
                   ((vap->iv_des_hw_mode >= IEEE80211_MODE_11NG_HT20)) &&
                    ((ic->ic_opmode != IEEE80211_M_IBSS) || ieee80211_ic_ht20Adhoc_is_set(ic)) &&
                    ((nc = ieee80211_find_channel(ic, c->ic_freq, 0, IEEE80211_CHAN_11NG_HT20)) != NULL)) {
                    return nc;
                } else if ((ic->ic_modecaps & (1<<IEEE80211_MODE_11G)) &&
                          ((vap->iv_des_hw_mode >= IEEE80211_MODE_11G)) &&
                           ((nc = ieee80211_find_channel(ic, c->ic_freq, 0, IEEE80211_CHAN_PUREG)) != NULL)) {
                    return nc;
                } else if ((ic->ic_modecaps & (1<<IEEE80211_MODE_11G)) &&
                          ((vap->iv_des_hw_mode >= IEEE80211_MODE_11G)) &&
                           ((nc = ieee80211_find_channel(ic, c->ic_freq, 0, IEEE80211_CHAN_G)) != NULL)) {
                    return nc;
                } else if ((ic->ic_modecaps & (1<<IEEE80211_MODE_11B)) &&
                           ((nc = ieee80211_find_channel(ic, c->ic_freq, 0, IEEE80211_CHAN_B)) != NULL)) {
                    return nc;
                }
            } else {
                /* Precedence is 11AC,11NA then 11A */
                if ((ic->ic_modecaps & (1<<IEEE80211_MODE_11AC_VHT20)) &&
                    ((vap->iv_des_hw_mode >= IEEE80211_MODE_11AC_VHT20)) &&
                    ((ic->ic_opmode != IEEE80211_M_IBSS) || ieee80211_ic_ht20Adhoc_is_set(ic)) &&
                    ((nc = ieee80211_find_channel(ic, c->ic_freq, 0, IEEE80211_CHAN_11AC_VHT20)) != NULL)) {
                    return nc;
                } else if ((ic->ic_modecaps & (1<<IEEE80211_MODE_11NA_HT20)) &&
                          ((vap->iv_des_hw_mode >= IEEE80211_MODE_11NA_HT20)) &&
                    ((ic->ic_opmode != IEEE80211_M_IBSS) || ieee80211_ic_ht20Adhoc_is_set(ic)) &&
                    ((nc = ieee80211_find_channel(ic, c->ic_freq, 0, IEEE80211_CHAN_11NA_HT20)) != NULL)) {
                    return nc;
                } else if ((ic->ic_modecaps & (1<<IEEE80211_MODE_11A)) &&
                           ((nc = ieee80211_find_channel(ic, c->ic_freq, 0, IEEE80211_CHAN_A)) != NULL)) {
                    return nc;
                }
            }
        } else if (mode == IEEE80211_MODE_11NG_HT40) {
            if (IEEE80211_IS_CHAN_2GHZ(c)) {
                 /* Precedence is 11NG PLUS channels first */

                if ((c->ic_flags & IEEE80211_CHAN_11NG_HT40PLUS) == IEEE80211_CHAN_11NG_HT40PLUS)
                    return c;

                if ((c->ic_flags & IEEE80211_CHAN_11NG_HT40MINUS) == IEEE80211_CHAN_11NG_HT40MINUS)
                    return c;
            }
        } else if (mode == IEEE80211_MODE_11NA_HT40) {
            if (IEEE80211_IS_CHAN_5GHZ(c)) {
                 /* Precedence is 11NA PLUS channels first */

                if ((c->ic_flags & IEEE80211_CHAN_11NA_HT40PLUS) == IEEE80211_CHAN_11NA_HT40PLUS)
                    return c;

                if ((c->ic_flags & IEEE80211_CHAN_11NA_HT40MINUS) == IEEE80211_CHAN_11NA_HT40MINUS)
                    return c;
            }
        } else if (mode == IEEE80211_MODE_11NG_HT40MINUS) {
            if ((c->ic_flags & IEEE80211_CHAN_11NG_HT40MINUS) == IEEE80211_CHAN_11NG_HT40MINUS)
		    return c;
        } else if (mode == IEEE80211_MODE_11NG_HT40PLUS) {
            if ((c->ic_flags & IEEE80211_CHAN_11NG_HT40PLUS) == IEEE80211_CHAN_11NG_HT40PLUS)
		    return c;
        } else if (mode == IEEE80211_MODE_11NA_HT40MINUS) {
            if ((c->ic_flags & IEEE80211_CHAN_11NA_HT40MINUS) == IEEE80211_CHAN_11NA_HT40MINUS)
		    return c;
        } else if (mode == IEEE80211_MODE_11NA_HT40PLUS) {
            if ((c->ic_flags & IEEE80211_CHAN_11NA_HT40PLUS) == IEEE80211_CHAN_11NA_HT40PLUS)
		    return c;
        } else if (mode == IEEE80211_MODE_11AC_VHT20) {
            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT20) == IEEE80211_CHAN_11AC_VHT20)
		    return c;
        } else if (mode == IEEE80211_MODE_11AC_VHT40) {

            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT40PLUS) == IEEE80211_CHAN_11AC_VHT40PLUS)
		    return c;

            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT40MINUS) == IEEE80211_CHAN_11AC_VHT40MINUS)
		    return c;

        } else if (mode == IEEE80211_MODE_11AC_VHT40PLUS) {
            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT40PLUS) == IEEE80211_CHAN_11AC_VHT40PLUS)
		    return c;
        } else if (mode == IEEE80211_MODE_11AC_VHT40MINUS) {
            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT40MINUS) == IEEE80211_CHAN_11AC_VHT40MINUS)
		    return c;
        } else if (mode == IEEE80211_MODE_11AC_VHT80) {
            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT80) == IEEE80211_CHAN_11AC_VHT80) {
		    return c;
            }
        } else if (mode == IEEE80211_MODE_11AC_VHT160) {
            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT160) == IEEE80211_CHAN_11AC_VHT160) {
		    return c;
            }
        } else if (mode == IEEE80211_MODE_11AC_VHT80_80) {
            if ((c->ic_flags & IEEE80211_CHAN_11AC_VHT80_80) == IEEE80211_CHAN_11AC_VHT80_80) {
                if (des_cfreq2 == 0 || c->ic_vhtop_ch_freq_seg2 == des_cfreq2) {
                    return c;
                }
            }
        } else {
            if ((c->ic_flags & modeflags) == modeflags)
                return c;
        }

    }
    return NULL;
}

#if UMAC_SUPPORT_DFS

#define IS_CHANNEL_WEATHER_RADAR(freq) ((freq >= 5600) && (freq <= 5650))
#define ADJACENT_WEATHER_RADAR_CHANNEL   5580
#define CH100_START_FREQ                 5490
#define CH100                            100
/*
 * Print a console message with the device name prepended.
 */
static void
if_printf( osdev_t dev, const char *fmt, ...)
{
    va_list ap;
    char buf[512];              /* XXX */

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    printk("\n %s\n", buf);     /* TODO: print device name also */
}
static int find_any_valid_channel(struct ieee80211com *ic,  u_int32_t chan_mode)
{
    int i;
    u_int32_t chan_flags = 0;
    u_int32_t chan_flagext = 0;
    int ret_val = -1;

   /* find a valid channel in alt_chan_mode */

    for (i = 0; i < ic->ic_nchans; i++) {
        chan_flags = ic->ic_channels[i].ic_flags;
        chan_flagext = ic->ic_channels[i].ic_flagext;

        /* skip the channel if this is not desired mode */
        if ((chan_flags & chan_mode) == 0) {
            continue;
        }
        /* skip if weathere channles are not allowed */
        if(ic->ic_no_weather_radar_chan) {
            u_int32_t freq = ieee80211_chan2freq(ic,&ic->ic_channels[i]);
            if(IS_CHANNEL_WEATHER_RADAR(freq)) {
                continue;
            }
        }

        /* skip if radar was found in the channel */
        if (chan_flags & IEEE80211_CHAN_RADAR) {
            continue;
        }

        /* skip if DFS channles are blocked */

        if ((chan_flagext & IEEE80211_CHAN_DFS) &&
            (ic->ic_flags_ext & IEEE80211_FEXT_BLKDFSCHAN)) {
            continue;
        }
        /* we found a channel */
        ret_val = i;
        break;
    }
    return ret_val;
}
static int find_alternate_mode_channel(struct ieee80211com *ic, u_int32_t alt_chan_mode, int chan_count)
{
    u_int32_t   random_byte = 0;
    int n;
    int i;
    int count = 0;
    int ret_val = -1;

    int use_lower_5g_only = 0;
    int use_upper_5g_only = 0;
    u_int32_t chan_flags = 0;
    u_int32_t chan_flagext = 0;

    struct ieee80211_channel *ptarget_channel = NULL;

    osdev_t dev = ic->ic_osdev;
    OS_GET_RANDOM_BYTES(&random_byte,1);
    n = (random_byte + OS_GET_TICKS() ) % chan_count;

    /* FR 27305:
       In Japan domain, if current channel is below channel 100
       then find a new channel that is below 100. Similarly
       if the current channel is 100 or higher then pick a channel
       that is 100 or higher
    */
    if (DFS_MKK4_DOMAIN == ic->ic_get_dfsdomain(ic)) {
        if (IEEE80211_IS_CHAN_11AC_VHT80_80(ic->ic_curchan)) {
            /* no action required for now */
            use_lower_5g_only = 0;
            use_upper_5g_only = 0;
        } else if (IEEE80211_IS_CHAN_11AC_VHT160(ic->ic_curchan)){
            /* no action required for now */
            use_lower_5g_only = 0;
            use_upper_5g_only = 0;
        } else {
            if (ic->ic_curchan->ic_freq < CH100_START_FREQ) {
                use_lower_5g_only = 1;
                use_upper_5g_only = 0;
            } else {
                use_lower_5g_only = 0;
                use_upper_5g_only = 1;
            }
        }
    }

   /* find n-th valid channel in alt_chan_mode */

    for (i = 0; i < ic->ic_nchans; i++) {
        chan_flags = ic->ic_channels[i].ic_flags;
        chan_flagext = ic->ic_channels[i].ic_flagext;

        /* skip the channel if this is not desired alternate mode */
        if ((chan_flags & alt_chan_mode) == 0) {
            continue;
        }

        /* skip if weathere channles are not allowed */
        if(ic->ic_no_weather_radar_chan) {
            u_int32_t freq = ieee80211_chan2freq(ic,&ic->ic_channels[i]);
            if(IS_CHANNEL_WEATHER_RADAR(freq)) {
                continue;
            }
        }

        /* skip if radar was found in the channel */
        if (chan_flags & IEEE80211_CHAN_RADAR) {
            continue;
        }

        /* skip if DFS channles are blocked */

        if ((chan_flagext & IEEE80211_CHAN_DFS) &&
            (ic->ic_flags_ext & IEEE80211_FEXT_BLKDFSCHAN)) {
            continue;
        }
        /* FR 27305:
           In Japan domain, if current channel is below channel 100
           then find a new channel that is below 100. Similarly
           if the current channel is 100 or higher then pick a channel
           that is 100 or higher
        */

        if (use_lower_5g_only) {
            if (IEEE80211_IS_CHAN_11AC_VHT80_80(&ic->ic_channels[i])) {
                if ((ic->ic_channels[i].ic_freq > CH100_START_FREQ) ||
                    (ic->ic_channels[i].ic_vhtop_ch_freq_seg2 > CH100)) {
                    /* skip this channel */
                    continue;
                }
            } else {
                if (ic->ic_channels[i].ic_freq > CH100_START_FREQ) {
                    /* skip this channel */
                    continue;
                }
            }
        }

        if (use_upper_5g_only) {
            if (IEEE80211_IS_CHAN_11AC_VHT80_80(&ic->ic_channels[i])) {
                if ((ic->ic_channels[i].ic_freq < CH100_START_FREQ) ||
                    (ic->ic_channels[i].ic_vhtop_ch_freq_seg2 < CH100)) {
                    /* skip this channel */
                    continue;
                }
            } else {
                if (ic->ic_channels[i].ic_freq < CH100_START_FREQ) {
                    /* skip this channel */
                    continue;
                }
            }
        }

        if (count == n) {
            ret_val = i;
            ptarget_channel = &ic->ic_channels[i];

            if (IEEE80211_IS_CHAN_11AC_VHT160(ptarget_channel)) {
                if_printf (dev, "%s: VHT160 channel %d(%dMHz)\n",
                           __func__,
                           ptarget_channel->ic_ieee,
                           ptarget_channel->ic_freq);
            } else if (IEEE80211_IS_CHAN_11AC_VHT80_80(ptarget_channel)) {
                if_printf (dev, "%s: VHT80_80 primary channel %d(%dMHz), extension channel %d(%dMHz)\n",
                           __func__,
                           ptarget_channel->ic_ieee,
                           ptarget_channel->ic_freq,
                           ptarget_channel->ic_vhtop_ch_freq_seg2,
                           ieee80211_ieee2mhz(ic, ptarget_channel->ic_vhtop_ch_freq_seg2, ptarget_channel->ic_flags));
            } else if (IEEE80211_IS_CHAN_11AC_VHT80(ptarget_channel)) {
                if_printf (dev, "%s: VHT80 channel %d(%dMHz)\n",
                           __func__,
                           ptarget_channel->ic_ieee,
                           ptarget_channel->ic_freq);
            }
            break;
        } else {
            count++;
        }
    }
    return ret_val;
}
#endif
int ieee80211_random_channel(struct ieee80211com *ic, u_int8_t is_select_nondfs, u_int8_t skip_curchan)
{
    int chanStart,n=0;
    u_int32_t curChanFlags, chan_flags, chan_flagext=0;

    int numGChannels=0;
    int numAChannels=0;
    int j;

    int ht160_count = 0;
    int ht80_80_count = 0;
    int ht80_count = 0;
    int ht40plus_count = 0;
    int ht40minus_count = 0;
    int ht20_count = 0;
    int use_lower_5g_only = 0;
    int use_upper_5g_only = 0;

    /* IR: 107025 -- Random channel selction not correct             */
    /* instead of u_int8_t available_chan_idx[IEEE80211_CHAN_MAX+1]  */
    /* use int *available_chan_idx and dynamically allocate it       */
    /* storing int charStart in byte array available_chan_idx[] is   */
    /* reason for random channel selection failure when number of    */
    /* max channel (IEEE80211_CHAN_MAX) is more than 255             */
    int *available_chan_idx;
    int available_chan_count = 0;
    int ret_val = -1;
    u_int32_t alt_chan_mode = 0;
    int       chan_count    = 0;

    osdev_t dev = ic->ic_osdev;
    if ((available_chan_idx = OS_MALLOC(ic->ic_osdev, (IEEE80211_CHAN_MAX+1) * sizeof(int) , GFP_KERNEL)) == NULL) {
        printk("%s: cannot allocate memory\n", __func__);
        return ret_val;
    }

    /* FR 27305:
       In Japan domain, if current channel is below channel 100
       then find a new channel that is below 100. Similarly
       if the current channel is 100 or higher then pick a channel
       that is 100 or higher
    */
    if (DFS_MKK4_DOMAIN == ic->ic_get_dfsdomain(ic)) {
        if (IEEE80211_IS_CHAN_11AC_VHT80_80(ic->ic_curchan)) {
            /* no action required for now */
            use_lower_5g_only = 0;
            use_upper_5g_only = 0;
            if_printf(dev,"%s -- MMK4 domain, HT80_80, no restriction on using upper or lower 5G channel\n", __func__);
        } else if (IEEE80211_IS_CHAN_11AC_VHT160(ic->ic_curchan)){
            /* no action required for now */
            use_lower_5g_only = 0;
            use_upper_5g_only = 0;
            if_printf(dev,"%s -- MMK4 domain, HT160, will look for HT160. if can't find no restriction on using upper or lower 5G channel\n", __func__);
        } else {
            if (ic->ic_curchan->ic_freq < CH100_START_FREQ) {
                use_lower_5g_only = 1;
                use_upper_5g_only = 0;
                if_printf(dev,"%s -- MMK4 domain, search for lower 5G (less than 5490 MHz) channels\n", __func__);
            } else {
                use_lower_5g_only = 0;
                use_upper_5g_only = 1;
                if_printf(dev,"%s -- MMK4 domain, search for upper 5G (more than 5490 MHz) channels\n", __func__);
            }
        }
    }


    /* Pick a random channel */

    /* Find how many G channels are present in the channel list
    * Assuming all G channels are present at the begining of
    * the list, followed by all A channels
     */

    for (j = 0; j < ic->ic_nchans; j++)
    {
        chan_flags = ic->ic_channels[j].ic_flags;
        if(chan_flags & IEEE80211_CHAN_2GHZ)
        {
            numGChannels++;
            continue;
        }
        else
            break;
    }

    numAChannels = (ic->ic_nchans - numGChannels);
    chanStart = numGChannels ; //+ ( OS_GET_TICKS() % numAChannels);

    curChanFlags = (ic->ic_curchan->ic_flags) & IEEE80211_CHAN_ALL;
    if ( ic->ic_flags_ext & IEEE80211_FEXT_BLKDFSCHAN )
	curChanFlags &= ~IEEE80211_CHAN_DFS;

    for (n = 0; n < ic->ic_nchans; chanStart++, n++)
	{
	    if (chanStart == ic->ic_nchans)
		chanStart = 0;
	    chan_flags = ic->ic_channels[chanStart].ic_flags;
	    chan_flagext = ic->ic_channels[chanStart].ic_flagext;

        if(skip_curchan)
        {
            /* Skip curchan when choosing apriori random channel */
            if(ic->ic_channels[chanStart].ic_freq == ic->ic_curchan->ic_freq)
                continue;

        }
        /* these channels have CAC of 10 minutes so skipping these */

        if(ic->ic_no_weather_radar_chan) {
        /*we should also avoid this channel in
          HT40 mode as extension channel will be on 5600 */
            u_int32_t freq = ieee80211_chan2freq(ic,&ic->ic_channels[chanStart]);

            if(((IS_CHANNEL_WEATHER_RADAR(freq))
                        || ((IEEE80211_CHAN_11NA_HT40PLUS & chan_flags)
                            && (ADJACENT_WEATHER_RADAR_CHANNEL == freq)))
                    && (DFS_ETSI_DOMAIN == ic->ic_get_dfsdomain(ic)))
            {
				continue;
			}
#undef ADJACENT_WEATHER_RADAR_CHANNEL
		}

	    /*
	     * (1) skip static turbo channel as it will require STA to be in
	     * static turbo to work.
	     * (2) skip channel which's marked with radar detction
	     * (3) WAR: we allow user to config not to use any DFS channel
	     */
	    /* When we pick a channel, skip excluded 11D channels. See bug 3124 */

	    if ((chan_flags & IEEE80211_CHAN_STURBO) ||
		(chan_flags & IEEE80211_CHAN_RADAR)  ||
		(chan_flagext & IEEE80211_CHAN_11D_EXCLUDED) ||
		(chan_flagext & IEEE80211_CHAN_DFS &&
		 ic->ic_flags_ext & IEEE80211_FEXT_BLKDFSCHAN )||
		(chan_flagext & IEEE80211_CHAN_DFS &&
		 is_select_nondfs))
		continue;


            /* FR 27305:
               In Japan domain, if current channel is below channel 100
               then find a new channel that is below 100. Similarly
               if the current channel is 100 or higher then pick a channel
               that is 100 or higher
            */

            if (use_lower_5g_only) {
                if (IEEE80211_IS_CHAN_11AC_VHT80_80(&ic->ic_channels[chanStart])) {
                    if ((ic->ic_channels[chanStart].ic_freq > CH100_START_FREQ) ||
                        (ic->ic_channels[chanStart].ic_vhtop_ch_freq_seg2 > CH100)) {
                        /* skip this channel */
                        continue;
                    }
                } else {
                    if (ic->ic_channels[chanStart].ic_freq > CH100_START_FREQ) {
                        /* skip this channel */
                        continue;
                    }
                }
            }

            if (use_upper_5g_only) {
                if (IEEE80211_IS_CHAN_11AC_VHT80_80(&ic->ic_channels[chanStart])) {
                    if ((ic->ic_channels[chanStart].ic_freq < CH100_START_FREQ) ||
                        (ic->ic_channels[chanStart].ic_vhtop_ch_freq_seg2 < CH100)) {
                        /* skip this channel */
                        continue;
                    }
                } else {
                    if (ic->ic_channels[chanStart].ic_freq < CH100_START_FREQ) {
                        /* skip this channel */
                        continue;
                    }
                }
            }

            /* keep a count of VHT160, VHT80_80 and VHT80 channels
               so that we can move from VHT160 to VHT80_80 to VHT80
               if we cannot find a channel in current mode
            */

            if (chan_flags & IEEE80211_CHAN_VHT20) {
                ht20_count++;
            } else if (chan_flags & IEEE80211_CHAN_VHT40PLUS) {
                ht40plus_count++;
            } else if (chan_flags & IEEE80211_CHAN_VHT40MINUS) {
                ht40minus_count++;
            } else if (chan_flags & IEEE80211_CHAN_VHT80) {
                ht80_count++;
            } else if (chan_flags & IEEE80211_CHAN_VHT80_80) {
                ht80_80_count++;
            } else if (chan_flags & IEEE80211_CHAN_VHT160) {
                ht160_count++;
            }
	    if ((chan_flags & IEEE80211_CHAN_ALL) == curChanFlags) {
		//break;
             available_chan_idx[available_chan_count++] = chanStart;
             if (available_chan_count >= IEEE80211_CHAN_MAX + 1)
                break;
           }
	}
    if(available_chan_count)
    {
        u_int32_t   random_byte = 0;
        OS_GET_RANDOM_BYTES(&random_byte,1);
        j = (random_byte + OS_GET_TICKS() ) % available_chan_count;
        chanStart = (available_chan_idx[j]);
#if 0 //DEBUG code

        for (n = 0; n < available_chan_count; n++) {
        printk("%s[%d] idx %d chan %d flags 0x%08x\n", __func__, __LINE__,
                available_chan_idx[n], ic->ic_channels[available_chan_idx[n]].ic_ieee, ic->ic_channels[available_chan_idx[n]].ic_flags);
        }
        printk("%s[%d] random %d avail_count %d rand_idx %d\n", __func__, __LINE__, random_byte, available_chan_count, j);
        printk("%s[%d] Selected chan %d flags 0x%08x chanStart %d \n", __func__, __LINE__,
                ic->ic_channels[chanStart].ic_ieee, ic->ic_channels[chanStart].ic_flags, chanStart);
 #endif
        ret_val = chanStart;
    } else {
        if_printf (dev, "%s: Cannot find a channel, looking for channel in other mode. ht80_count=%d, ht80_80_count=%d, ht160_count=%d\n", __func__, ht80_count, ht80_80_count, ht160_count);

        /* we need to handle HT160/HT80_80 in a special way   */
        /* HT160 has only two channels available. We will     */
        /* try to change to HT80_80 if we cannot find any     */
        /* 160 MHz contiguous channel. If there is no HT80_80 */
        /* channel then we will look for HT80 channel. Also   */
        /* we will change HT80_80 to HT80 in case we can't    */
        /* find a HT80_80 channel. This can happen in some    */
        /* design with two 5G radios where one radio operates */
        /* in channel 36 through 64.                          */
        /* The same could be done for other 11AC modes but    */
        /* we have plenty of HT80, HT40 and HT20 channels     */
        /* The following code can also be enhanced to switch  */
        /* automatically to a wider channel whenever one      */
        /* is present                                         */

        if (ht160_count > 0) {
            alt_chan_mode = IEEE80211_CHAN_VHT160;
            chan_count    = ht160_count;
        } else if (ht80_80_count > 0) {
            alt_chan_mode = IEEE80211_CHAN_VHT80_80;
            chan_count    = ht80_80_count;
        } else if (ht80_count > 0) {
            alt_chan_mode = IEEE80211_CHAN_VHT80;
            chan_count    = ht80_count;
        } else if (ht40plus_count > 0) {
            alt_chan_mode = IEEE80211_CHAN_VHT40PLUS;
            chan_count    = ht40plus_count;
        } else if (ht40minus_count > 0) {
            alt_chan_mode = IEEE80211_CHAN_VHT40MINUS;
            chan_count    = ht40minus_count;
        } else if (ht20_count > 0) {
            alt_chan_mode = IEEE80211_CHAN_VHT20;
            chan_count    = ht20_count;
        }
        if (chan_count) {
            ret_val = find_alternate_mode_channel(ic, alt_chan_mode, chan_count);
            if (ret_val == -1) {
                /* last attempt to get a valid channel */
                if_printf(dev, "%s: Cannot find a channel. Forcing to first available HT20 channel\n", __func__);
                ret_val = find_any_valid_channel(ic, IEEE80211_CHAN_VHT20);
            }
        } else {
            /* When all the DFS channels are in NOL and there is no DFS channel,
             * chan_count is 0 and observed 'Division by zero in kernel'
             */
            ret_val = -1;
        }
    }
    OS_FREE(available_chan_idx);
    return ret_val;

}

/*
 * Check if two channels are in the same frequency band.
 */
bool
ieee80211_is_same_frequency_band(const struct ieee80211_channel *chan1, const struct ieee80211_channel *chan2)
{
    if (chan2 == NULL) {
        return false;
    }

    if (IEEE80211_IS_CHAN_2GHZ(chan1)) {
        /*
         * Channel1 is 2GHz, return TRUE only if channel2 is also 2GHz
         */
        return IEEE80211_IS_CHAN_2GHZ(chan2);
    }
    else {
        /*
         * Channel1 is 5GHz, return TRUE only if channel2 is also 5GHz
         */
        return IEEE80211_IS_CHAN_5GHZ(chan2);
    }
}

/*
 * Update channel list and associated PHY mode bitmask
 */
void
ieee80211_update_channellist(struct ieee80211com *ic, int exclude_11d)
{
    struct ieee80211_channel *c;
    int i;
    u_int32_t modcapmask;

    /*
     * Fill in 802.11 available channel set, mark
     * all available channels as active, and pick
     * a default channel if not already specified.
     */
    KASSERT(0 < ic->ic_nchans && ic->ic_nchans <= IEEE80211_CHAN_MAX,
            ("invalid number of channels specified: %u", ic->ic_nchans));
    OS_MEMZERO(ic->ic_chan_avail, sizeof(ic->ic_chan_avail));

    modcapmask = (1 << IEEE80211_MODE_MAX) -1;
    ic->ic_modecaps &= (~modcapmask);
    ic->ic_modecaps |= 1<<IEEE80211_MODE_AUTO;

    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];
        KASSERT(c->ic_flags != 0, ("channel with no flags"));
        KASSERT(c->ic_ieee < IEEE80211_CHAN_MAX,
                ("channel with bogus ieee number %u", c->ic_ieee));

        if (exclude_11d && IEEE80211_IS_CHAN_11D_EXCLUDED(c))
            continue;

        setbit(ic->ic_chan_avail, c->ic_ieee);

        /*
         * Identify mode capabilities.
         */
        if (IEEE80211_IS_CHAN_A(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11A;
        if (IEEE80211_IS_CHAN_B(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11B;
        if (IEEE80211_IS_CHAN_PUREG(c) || IEEE80211_IS_CHAN_G(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11G;
        if (IEEE80211_IS_CHAN_FHSS(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_FH;
        if (IEEE80211_IS_CHAN_108A(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_TURBO_A;
        if (IEEE80211_IS_CHAN_108G(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_TURBO_G;
        if (IEEE80211_IS_CHAN_11NA_HT20(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11NA_HT20;
        if (IEEE80211_IS_CHAN_11NG_HT20(c))
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11NG_HT20;
        if (IEEE80211_IS_CHAN_11NA_HT40PLUS(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11NA_HT40PLUS;
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11NA_HT40;
        }
        if (IEEE80211_IS_CHAN_11NA_HT40MINUS(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11NA_HT40MINUS;
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11NA_HT40;
        }

        /*
         * HT40 in 2GHz allowed only if user enabled it.
         */
        if (ic->ic_reg_parm.enable2GHzHt40Cap) {
            if (IEEE80211_IS_CHAN_11NG_HT40PLUS(c)) {
                ic->ic_modecaps |= 1<<IEEE80211_MODE_11NG_HT40PLUS;
                ic->ic_modecaps |= 1<<IEEE80211_MODE_11NG_HT40;
            }
            if (IEEE80211_IS_CHAN_11NG_HT40MINUS(c)) {
                ic->ic_modecaps |= 1<<IEEE80211_MODE_11NG_HT40MINUS;
                ic->ic_modecaps |= 1<<IEEE80211_MODE_11NG_HT40;
            }
        }

        /* VHT */
        if (IEEE80211_IS_CHAN_11AC_VHT20(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT20;
        }
        if (IEEE80211_IS_CHAN_11AC_VHT40PLUS(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT40PLUS;
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT40;
        }
        if (IEEE80211_IS_CHAN_11AC_VHT40MINUS(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT40MINUS;
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT40;
        }
        if (IEEE80211_IS_CHAN_11AC_VHT80(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT80;
        }

        if (IEEE80211_IS_CHAN_11AC_VHT160(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT160;
    }
        if (IEEE80211_IS_CHAN_11AC_VHT80_80(c)) {
            ic->ic_modecaps |= 1<<IEEE80211_MODE_11AC_VHT80_80;
    }
    }

    /* initialize candidate channels to all available */
    OS_MEMCPY(ic->ic_chan_active, ic->ic_chan_avail,
              sizeof(ic->ic_chan_avail));

    if (exclude_11d)
        ieee80211_scan_update_channel_list(ic->ic_scanner);

    /* bug #103186
     * after re-setting the country code, if curchan is
     * not availble anymore in the new country, then
     * let's find another channel.
     */
    if( (exclude_11d) && (ic->ic_curchan != NULL)) {
        struct ieee80211_channel *first_valid_c = NULL;
        for(i=0; i<ic->ic_nchans; i++){
            c = &ic->ic_channels[i];
            if(isset(ic->ic_chan_avail, c->ic_ieee)){
                /* the ic_curchan is still valid, break */
                if( ic->ic_curchan == c)
                    break;
                if( first_valid_c == NULL)
                    first_valid_c = c;
            }
        }
        if( i >= ic->ic_nchans)
            /* the ic_curchan is not valid, set a valid channel */
            ieee80211_set_channel(ic, first_valid_c);
    }
}

#if UMAC_SUPPORT_IBSS
struct ieee80211_channel *
ieee80211_autoselect_adhoc_channel(struct ieee80211com *ic)
{
    struct ieee80211_channel *c;
    int i;

    /*
     * If 11n adhoc is enabled, We do 11NG_HT20, then G, then B modes.
     */
    if (ieee80211_ic_ht20Adhoc_is_set(ic)) {
        c = ieee80211_find_dot11_channel(ic, IEEE80211_CHAN_ADHOC_DEFAULT1, 0, IEEE80211_MODE_11NG_HT20);
        if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
            return c;
        c = ieee80211_find_dot11_channel(ic, IEEE80211_CHAN_ADHOC_DEFAULT2, 0, IEEE80211_MODE_11NG_HT20);
        if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
            return c;

        for (i = 0; i < ic->ic_nchans; i++) {
            c = ieee80211_find_dot11_channel(ic, ic->ic_channels[i].ic_ieee, 0, IEEE80211_MODE_11NG_HT20);
            if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
                return c;
        }
    }

    /*
     * Give preference to 802.11g channel 10 because it is the safest channel
     * to be usded in ad hoc, i.e. least regulartory domain consideration.
     */
    c = ieee80211_find_dot11_channel(ic, IEEE80211_CHAN_ADHOC_DEFAULT1, 0, IEEE80211_MODE_11G);
    if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
        return c;

    /* Then 802.11g channel 11  */
    c = ieee80211_find_dot11_channel(ic, IEEE80211_CHAN_ADHOC_DEFAULT2, 0, IEEE80211_MODE_11G);
    if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
        return c;

    /* Then any 802.11g channel */
    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];
        if (IEEE80211_IS_CHAN_ANYG(c) && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
            return c;
    }

    /* Then 802.11b channel 10  */
    c = ieee80211_find_dot11_channel(ic, IEEE80211_CHAN_ADHOC_DEFAULT1, 0, IEEE80211_MODE_11B);
    if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
        return c;

    /* Then 802.11b channel 11  */
    c = ieee80211_find_dot11_channel(ic, IEEE80211_CHAN_ADHOC_DEFAULT2, 0, IEEE80211_MODE_11B);
    if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
        return c;

    /* Then any 802.11b channel */
    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];
        if (IEEE80211_IS_CHAN_B(c) && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
            return c;
    }

    /*
     * If 11n adhoc is enabled, We do 11NA_HT20, then A mode.
     */
    if (ieee80211_ic_ht20Adhoc_is_set(ic)) {
        for (i = 0; i < ic->ic_nchans; i++) {
            c = ieee80211_find_dot11_channel(ic, ic->ic_channels[i].ic_ieee, 0, IEEE80211_MODE_11NA_HT20);
            if (c && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
                return c;
        }
    }

    /* Then any 802.11a channel */
    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];
        if (IEEE80211_IS_CHAN_A(c) && !IEEE80211_IS_CHAN_DISALLOW_ADHOC(c))
            return c;
    }

    return NULL;
}
#endif

/*
 * Set current channel
 */
int
ieee80211_set_channel(struct ieee80211com *ic, struct ieee80211_channel *chan)
{
    int error;
    ic->ic_prevchan = ic->ic_curchan;
    ic->ic_curchan = chan;
    error = ic->ic_set_channel(ic);
    ic->ic_curmode = ieee80211_chan2mode(ic->ic_curchan);
#ifdef ATH_SUPPORT_DFS
    ieee80211_dfs_cac_valid_reset(ic);
#endif
#if UNIFIED_SMARTANTENNA
    ieee80211_smart_ant_channel_change(ic);
#endif
    /* Note: In VHT mode the right channel center frequency index will be available
     * from the regulatory domain table */

    /*
     * When there is radar detect in Repeater, repeater sends RCSAs, CSAs and
     * switches to new next channel, in ind rpt case repeater AP could start
     * beaconing before Root comes up, next channel needs to be changed
     *
     * For offload, resmgr will trigger channel change and fw resp of vdev up will
     * update dfs channel
     */
    if(!ic->ic_is_mode_offload(ic))
    {
        if(IEEE80211_IS_CHAN_DFS(ic->ic_curchan))
        {
                if(!ic->ic_tx_next_ch || ic->ic_curchan == ic->ic_tx_next_ch)
                {
                    ieee80211_update_dfs_next_channel(ic);
                }
        }
        else {
            ic->ic_tx_next_ch = NULL;
        }
    }
    return error;
}
#if ATH_SUPPORT_WIFIPOS
/*
 * Fast Set current channel
 * Functionality: This uses optimal functionality to do channel change.
 */
int
ieee80211_lean_set_channel(struct ieee80211com *ic, struct ieee80211_channel *chan)
{
    int error;
    ic->ic_prevchan = ic->ic_curchan;
    ic->ic_curchan = chan;
    error = ic->ic_lean_set_channel(ic);
    // restoring the channel information
    if(error) {
        ic->ic_curchan = ic->ic_prevchan;
    }
    ic->ic_curmode = ieee80211_chan2mode(ic->ic_curchan);
    return error;
}
#endif

/*
 * Set the current phy mode.
 */
int ieee80211_setmode(struct ieee80211com *ic,
                      enum ieee80211_phymode mode,
                      enum ieee80211_opmode opmode)
{
    ieee80211_reset_erp(ic, mode, opmode); /* reset ERP state */
    ic->ic_curmode = mode;      /* NB: must do post reset_erp */
    return 0;
}

const char *ieee80211_phymode_to_name( enum ieee80211_phymode mode)
{
    static const char unknown_mode[]="UNKNWON";
    if (mode >= sizeof(ieee80211_phymode_name)/sizeof(char *)) {
        return unknown_mode;
    }
    return (ieee80211_phymode_name[mode]);
}

int
wlan_get_supported_phymodes(wlan_dev_t devhandle,
                            enum ieee80211_phymode *modes,
                            u_int16_t *nmodes,
                            u_int16_t len)
{
    struct ieee80211com *ic = devhandle;
    int n = 0;
#define ADD_PHY_MODE(_m) do {                       \
    if (IEEE80211_SUPPORT_PHY_MODE(ic, (_m))) {     \
        if (len < (n+1))                            \
            goto bad;                               \
        modes[n++] = (_m);                          \
    }                                               \
} while (0)

    /*
     * NB: we fill in the modes array in certain order to be
     * compatible with Win 7/Vista SP1 driver.
     */
    ADD_PHY_MODE(IEEE80211_MODE_11B);
    ADD_PHY_MODE(IEEE80211_MODE_11A);
    ADD_PHY_MODE(IEEE80211_MODE_11G);
    ADD_PHY_MODE(IEEE80211_MODE_11NG_HT40PLUS);
    ADD_PHY_MODE(IEEE80211_MODE_11NG_HT40MINUS);
    ADD_PHY_MODE(IEEE80211_MODE_11NG_HT20);
    ADD_PHY_MODE(IEEE80211_MODE_11NA_HT40PLUS);
    ADD_PHY_MODE(IEEE80211_MODE_11NA_HT40MINUS);
    ADD_PHY_MODE(IEEE80211_MODE_11NA_HT20);
    ADD_PHY_MODE(IEEE80211_MODE_11AC_VHT80_80);
    ADD_PHY_MODE(IEEE80211_MODE_11AC_VHT160);
    ADD_PHY_MODE(IEEE80211_MODE_11AC_VHT80);
    ADD_PHY_MODE(IEEE80211_MODE_11AC_VHT40PLUS);
    ADD_PHY_MODE(IEEE80211_MODE_11AC_VHT40MINUS);
    ADD_PHY_MODE(IEEE80211_MODE_11AC_VHT20);

    *nmodes = n;
    return 0;

bad:
    *nmodes = IEEE80211_MODE_MAX;
    return -EOVERFLOW;
#undef ADD_PHY_MODE
}

int
wlan_set_desired_phylist(wlan_if_t vaphandle, enum ieee80211_phymode *phylist, u_int16_t nphy)
{
    struct ieee80211vap *vap = vaphandle;
    int i;

    if (nphy > IEEE80211_MODE_MAX || nphy < 1) {
        return -EINVAL;
    }

    vap->iv_des_modecaps = 0;

    for (i = 0; i < nphy; i++) {
        if (phylist[i] == IEEE80211_MODE_AUTO) {
            vap->iv_des_modecaps = (1 << IEEE80211_MODE_AUTO);
            return 0;
        }

        if (!IEEE80211_SUPPORT_PHY_MODE(vap->iv_ic, phylist[i])) {
            return -EINVAL;
        }

        vap->iv_des_modecaps |= (1 << phylist[i]);
    }

    return 0;
}

int
wlan_get_desired_phylist(wlan_if_t vaphandle,
                         enum ieee80211_phymode *phylist,
                         u_int16_t *nphy,
                         u_int16_t len)
{
    struct ieee80211vap *vap = vaphandle;
    enum ieee80211_phymode m;
    u_int16_t count;

    if (len < 1) {
        *nphy = IEEE80211_MODE_MAX;
        return -EOVERFLOW;
    }

    /* return AUTO if we accept any PHY modes */
    if (IEEE80211_ACCEPT_ANY_PHY_MODE(vap)) {
        phylist[0] = IEEE80211_MODE_AUTO;
        *nphy = 1;
        return 0;
    }

    count = 0;
    for (m = IEEE80211_MODE_AUTO + 1; m < IEEE80211_MODE_MAX; m++) {
        if (IEEE80211_ACCEPT_PHY_MODE(vap, m)) {
            /* is input big enough */
            if (len < (count+1))
                return -EINVAL;

            phylist[count++] = m;
        }
    }

    *nphy = count;
    return 0;
}

enum ieee80211_phymode
wlan_get_desired_phymode(wlan_if_t vaphandle)
{
    struct ieee80211vap *vap = vaphandle;
    return vap->iv_des_mode;
}

int
wlan_set_desired_phymode(wlan_if_t vaphandle, enum ieee80211_phymode mode)
{
    struct ieee80211vap *vap = vaphandle;
    wlan_chan_t chan = wlan_get_current_channel(vap, false);
    struct ieee80211vap *tmpvap = NULL;
    struct ieee80211com *ic = vap->iv_ic;
    int numvaps_up = 0;

    if (!IEEE80211_SUPPORT_PHY_MODE(vap->iv_ic, mode)) {
            printk("Rejecting mode %d: This mode may be invalid or may be unavailable for the requested values of parameters such as channel and chainmasks\n", mode);
            return -EINVAL;
    }

    /* in multivap case, should we check if this is in agreement with other vaps */
    if (chan != IEEE80211_CHAN_ANYC) {
        /* If any vap is running, find the same frequency channel with vap desired mode */
        numvaps_up = ieee80211_get_num_vaps_up(ic);

        if ((numvaps_up) && (ic->ic_curchan != chan)) {
            chan = ic->ic_curchan;
        }
        chan = ieee80211_find_dot11_channel(ic, chan->ic_ieee, chan->ic_vhtop_ch_freq_seg2, mode);
    }

    if ((mode == IEEE80211_MODE_AUTO) || (vap->iv_opmode == IEEE80211_M_STA)) {
        vap->iv_des_mode = mode;
        vap->iv_des_hw_mode = mode;
        if (chan != IEEE80211_CHAN_ANYC && chan) {
            vap->iv_des_chan[mode] = chan;
        }
    } else {
        TAILQ_FOREACH(tmpvap, &ic->ic_vaps, iv_next) {
            tmpvap->iv_des_mode = mode;
            tmpvap->iv_des_hw_mode = mode;
            if (chan != IEEE80211_CHAN_ANYC && chan) {
                tmpvap->iv_des_chan[mode] = chan;
            }
        }
    }

    return 0;
}

int
wlan_set_desired_ibsschan(wlan_if_t vaphandle, int channum)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211_channel *channel = NULL;
    struct ieee80211com *ic = vap->iv_ic;

    if (channum > IEEE80211_CHAN_MAX)
        return -EINVAL;

    if(channum == IEEE80211_CHAN_ANY) {
            channel = vap->iv_des_chan[vap->iv_des_mode];
            if(channel == IEEE80211_CHAN_ANYC)
                return -EINVAL;
            channum = channel->ic_ieee;
    } else {
        channel = ieee80211_find_dot11_channel(ic, channum, vap->iv_des_cfreq2, vap->iv_des_mode | ic->ic_chanbwflag);
        if (channel == NULL) {
            channel = ieee80211_find_dot11_channel(ic, channum, 0, IEEE80211_MODE_AUTO);
            if (channel == NULL)
                return -EINVAL;
        }
    }

    if (IEEE80211_IS_CHAN_RADAR(channel)) {
        return -EINVAL;
    }

    vap->iv_des_ibss_chan = channum;

    return 0;
}


enum ieee80211_phymode
wlan_get_current_phymode(wlan_if_t vaphandle)
{
    return ieee80211_get_current_phymode(vaphandle->iv_ic);
}

enum ieee80211_phymode
wlan_get_bss_phymode(wlan_if_t vaphandle)
{
    return ieee80211_chan2mode(vaphandle->iv_bsschan);
}

wlan_chan_t
wlan_get_current_channel(wlan_if_t vaphandle, bool hwChan)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;

    /*
     * If the VAP is in RUN state, return the current channel.
     * Otherwise, return the desired channel of the desired phymode.
     */
    if (ieee80211_vap_ready_is_set(vap) || hwChan) {
        return ic->ic_curchan;
    } else {
        return vap->iv_des_chan[vap->iv_des_mode];
    }
}

wlan_chan_t
wlan_get_dev_current_channel(wlan_dev_t devhandle)
{
    struct ieee80211com *ic = devhandle;

    return (ic->ic_curchan == IEEE80211_CHAN_ANYC)? NULL: ic->ic_curchan;
}

wlan_chan_t
wlan_get_bss_channel(wlan_if_t vaphandle)
{
    struct ieee80211vap *vap = vaphandle;

    return vap->iv_bsschan;
}

wlan_chan_t
wlan_get_des_channel(wlan_if_t vaphandle)
{
    struct ieee80211vap *vap = vaphandle;

    return vap->iv_des_chan[vap->iv_des_mode];
}


int
wlan_get_channel_list(wlan_dev_t devhandle, enum ieee80211_phymode phymode,
                      wlan_chan_t chanlist[], u_int32_t n)
{
    struct ieee80211com *ic = devhandle;
    struct ieee80211_channel *chan;
    int i, nchan = 0;
    for (i = 0; i < ic->ic_nchans; i++) {
        chan = &ic->ic_channels[i];

        /*
         * only active channels
         */
        if (isclr(ic->ic_chan_avail, chan->ic_ieee)) {
            continue;
        }

        if (phymode == IEEE80211_MODE_AUTO ||
            phymode == ieee80211_chan2mode(chan)) {
            /*
             * found a matching channel
             */
            if (n < (nchan + 1))
                return -EOVERFLOW;

            chanlist[nchan++] = chan;
        }
    }

    return nchan;
}

void
wlan_get_noise_floor(wlan_if_t vaphandle, int16_t *nfBuf)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;

    ic->ic_get_chainnoisefloor(ic, ic->ic_curchan, nfBuf);
}

int16_t
wlan_get_chan_noise_floor(wlan_if_t vaphandle, u_int16_t freq, u_int32_t flags)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;
    struct ieee80211_channel chan;

    chan.ic_freq  = freq;
    chan.ic_flags = flags;

    return ic->ic_get_noisefloor(ic, &chan, 0);
}

u_int32_t
wlan_channel_frequency(wlan_chan_t chan)
{
    return (chan == IEEE80211_CHAN_ANYC) ?  IEEE80211_CHAN_ANY : chan->ic_freq;
}

u_int32_t
wlan_channel_ieee(wlan_chan_t chan)
{
    return (chan == IEEE80211_CHAN_ANYC) ?  IEEE80211_CHAN_ANY : chan->ic_ieee;
}

enum ieee80211_phymode
wlan_channel_phymode(wlan_chan_t chan)
{
    return ieee80211_chan2mode(chan);
}

int8_t
wlan_channel_maxpower(wlan_chan_t chan)
{
    return chan->ic_maxregpower;
}

u_int32_t
wlan_channel_flags(wlan_chan_t chan)
{
    return chan->ic_flags;
}

bool
wlan_channel_is_passive(wlan_chan_t chan)
{
    return (IEEE80211_IS_CHAN_PASSIVE(chan));
}

bool
wlan_channel_is_5GHzOdd(wlan_chan_t chan)
{
    return (IEEE80211_IS_CHAN_ODD(chan));
}

bool
wlan_channel_is_dfs(wlan_chan_t chan, bool flagOnly)
{
    if (flagOnly)
        return (IEEE80211_IS_CHAN_DFSFLAG(chan));
    else
        return (IEEE80211_IS_CHAN_DFS(chan));
}


/*
 * Auto Channel Select handler used for interface up.
 */
static void spectral_eacs_event_handler(void *arg, wlan_chan_t channel)
{
    struct ieee80211vap *vap = (struct ieee80211vap *)arg;
    int chan = wlan_channel_ieee(channel);
    int error = 0;
    /* Skip channel set, if channel is ANYC */
    if ((!channel) || (channel == IEEE80211_CHAN_ANYC)) {
        goto done;
    }

    IEEE80211_DPRINTF(vap, IEEE80211_MSG_ACS,"%s channel selected is %d\n", __func__,chan);
    error = wlan_set_channel(vap, chan, channel->ic_vhtop_ch_freq_seg2);
    if (error !=0) {
        IEEE80211_DPRINTF(vap, IEEE80211_MSG_ANY,
            "%s : failed to set channel with error code %d\n",
            __func__, error);
    }
done:
    wlan_autoselect_unregister_event_handler(vap, &spectral_eacs_event_handler, vap);
}


/**
* @brief            set known ieee802111_channel to vap
*
* @param vaphandle  handle to vap for which channel has to be set
* @param channel    handle to ieee80211_channel
*
* @return:          0 for success, -ve error code other wise.
*/
int
wlan_set_ieee80211_channel(wlan_if_t vaphandle,
                           struct ieee80211_channel *channel)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;

    if (!channel) {
        IEEE80211_DPRINTF(vap, IEEE80211_MSG_ACS,
                "%s: vap-%d NULL channel specified\n", __func__, vap->iv_unit);
        return -EINVAL;
    }

#if ATH_SUPPORT_VOW_DCS
        if ( !ic->ic_eacs_done ){
            /* disable dcs when channel is not chosen by EACS */
            ic->ic_disable_dcsim(ic);
        } else {
            ic->ic_eacs_done = 0;
        }
#endif
    if ((vap->iv_opmode == IEEE80211_M_HOSTAP) &&
        (ieee80211_check_chan_mode_consistency(ic,vap->iv_des_mode,channel))) {
        return -EINVAL;
    }

    /* don't allow to change to channel with radar found */
    if (IEEE80211_IS_CHAN_RADAR(channel)) {
        return -EINVAL;
    }

    /* we are ready to change channel, restore some ic flag , EV#133678 */
    if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
        IEEE80211_DISABLE_PROTECTION(ic);
        ic->ic_update_protmode(ic);
        if (ic->ic_caps & IEEE80211_C_SHPREAMBLE) {
                IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE,"%s: restore short preamble setting\n", __func__);
                ic->ic_flags |= IEEE80211_F_SHPREAMBLE;
                ic->ic_flags &= ~IEEE80211_F_USEBARKER;
        }
    }

    /* mark desired channel */
    vap->iv_des_chan[vap->iv_des_mode] = channel;

    /*
     * 2.4 GHz Channel Switch Announcement would be disabled in default.
     * Due to maybe some STAs do not handle the CSA in 2.4 GHz very well.
     * Also make sure the channel change is not due to interference. In this
     * case, do not wait for CSA
     */
    if ((ieee80211_ic_2g_csa_is_set(ic)) && ieee80211_vap_ready_is_set(vap) && !(ic->cw_inter_found)) {
        ieee80211_ucfg_set_chanswitch(vap, channel->ic_ieee, IEEE80211_2GCSA_TBTTCOUNT, 0);
        return 0;
    }

    /*
     * Do a channel change only when we're already in the RUN or DFSWAIT state.
     * The MLME will pickup the desired channel later.
     */

    /*
     * TBD: If curchan = channel, still need to set channel again to pass
     * SendRecv_ext in ndistest.
     */
    if (ieee80211_vap_ready_is_set(vap) || ieee80211_vap_dfswait_is_set(vap) ||
        (ic->cw_inter_found)) {// && ic->ic_curchan != channel) {
        ieee80211_set_channel(ic, channel);
        {
            /* In case of multi BSSID, make sure all the VAPs change channel */
            struct ieee80211vap *tmp_vap = NULL;
            TAILQ_FOREACH(tmp_vap, &ic->ic_vaps, iv_next) {
                /* clear erpupdate bit */
                ieee80211_vap_erpupdate_clear(tmp_vap);
                tmp_vap->iv_bsschan = ic->ic_curchan;
                /* This is needed to make the beacon is re-initlized */
                tmp_vap->channel_change_done = 1;
#if ATH_SUPPORT_VOW_DCS
                if ( (ic->cw_inter_found) && (ieee80211_vap_ready_is_set(vap) ||
                    ieee80211_vap_dfswait_is_set(vap) )) {
                    if (ieee80211_sta_power_unpause(vap) != EOK) {
                        IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s failed to send fakewakeup\n",
                            __func__);
                    }
                }
#endif
                IEEE80211_DPRINTF(tmp_vap, IEEE80211_MSG_STATE, "%s switch channel %d\n",
                                    __func__, ic->ic_curchan->ic_ieee);
            }
        }
    }

    return 0;
}

int
wlan_set_channel(wlan_if_t vaphandle, int chan, u_int8_t des_cfreq2)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;
    struct ieee80211_channel *channel;

    if (chan > IEEE80211_CHAN_MAX)
        return -EINVAL;

    if (chan == IEEE80211_CHAN_ANY) {

        if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
            /* allow IEEE80211_CHAN_ANYC for auto channel select in AP mode*/
            vap->iv_des_chan[vap->iv_des_mode] = IEEE80211_CHAN_ANYC;
            /* Trigger EACS only when vap is ready or channel change issued by DCS module */
            if (ieee80211_vap_ready_is_set(vap) || (ic->cw_inter_found)) {
              wlan_autoselect_register_event_handler(vap, &spectral_eacs_event_handler, (void *)vap);
              wlan_autoselect_find_infra_bss_channel(vap);
            }
            return 0;
        } else {
            /* select the desired channel for the desired PHY mode */
            channel = vap->iv_des_chan[vap->iv_des_mode];
            if(channel == IEEE80211_CHAN_ANYC)
                return -EINVAL;
        }
    } else {
        /*
         * find the corresponding channel object for the desired PHY mode.
         */
        channel = ieee80211_find_dot11_channel(ic, chan, des_cfreq2, vap->iv_des_mode | ic->ic_chanbwflag);
        if (channel == NULL) {
            channel = ieee80211_find_dot11_channel(ic, chan, 0, IEEE80211_MODE_AUTO);
            if (channel == NULL)
                return -EINVAL;
        }
    }

    return wlan_set_ieee80211_channel(vap, channel);
}

#if ATH_SUPPORT_WIFIPOS
/*
 * Function:        wlan_pause_node
 * param:           vaphandle, macaddr which is not to be pasued,
                    pauae(1 => to pasue, 0 =>unpause)
 * Functionality:   This function is a hook to call node_pause_control
 *                  This is for pasuing all the nodes which are not
 *                  being probed.
 */
int
wlan_pause_node(wlan_if_t vaphandle, struct ieee80211_node *ni, bool pause)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;
    ic->ic_pause_node(ic, ni, pause);
    return 0;
}
int
wlan_resched_txq(wlan_if_t vaphandle)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;
    ic->ic_resched_txq(ic);
    return 0;
}

/*  \Function:      wlan_lean_set_channel
 *  \param:         vapahndle, channel number
 *  \Functionality: This function is a hook to the call for fast
 *                  channel change which doesnot include any txq drain
 *                  Hence to use this channel change one must and
 *                  use node pause before calling this function once
 *                  coming back to the channel unpause the nodes and
 *                  jumpStartTx (this is needed since there will be
 *                  no frame in HW queue and this will cause stuck TX
 *                  path.
 *  \Note:          This is currently implemented for Wifi positioning and
 *                  is an optimized for this application only. This need to
 *                  evolve with additional functionality as the use case
 *                  increases.
 */
int
wlan_lean_set_channel(wlan_if_t vaphandle, int chan)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;
    struct ieee80211_channel *channel;
    int error = -EINVAL;

    if (chan > IEEE80211_CHAN_MAX)
        return -EINVAL;

    /*
     * find the corresponding channel object for the desired PHY mode.
     */
    channel = ieee80211_find_dot11_channel(ic, chan, vap->iv_des_cfreq2, vap->iv_des_mode | ic->ic_chanbwflag);
    if (channel == NULL) {
        channel = ieee80211_find_dot11_channel(ic, chan, 0, IEEE80211_MODE_AUTO);

        if (channel == NULL)
            return -EINVAL;
    }

    if ((vap->iv_opmode == IEEE80211_M_HOSTAP) &&
        (ieee80211_check_chan_mode_consistency(ic,vap->iv_des_mode,channel))) {
            return -EINVAL;
    }

    /* don't allow to change to channel with radar found */
    if (IEEE80211_IS_CHAN_RADAR(channel)) {
        return -EINVAL;
    }

    /* mark desired channel */
    vap->iv_des_chan[vap->iv_des_mode] = channel;

    /*
     * 2.4 GHz Channel Switch Announcement would be disabled in default.
     * Due to maybe some STAs do not handle the CSA in 2.4 GHz very well.
     */
    if ((ieee80211_ic_2g_csa_is_set(ic)) && ieee80211_vap_ready_is_set(vap)) {
        ieee80211_ucfg_set_chanswitch(vap, chan, IEEE80211_2GCSA_TBTTCOUNT, 0);
        return 0;
    }

    /*
     * Do a channel change only when we're already in the RUN or DFSWAIT state.
     * The MLME will pickup the desired channel later.
     */

    /*
     * TBD: If curchan = channel, still need to set channel again to pass
     * SendRecv_ext in ndistest.
     */
    if (ieee80211_vap_ready_is_set(vap) || ieee80211_vap_dfswait_is_set(vap)) {// && ic->ic_curchan != channel) {
        error = ieee80211_lean_set_channel(ic, channel);
        vap->iv_bsschan = ic->ic_curchan;

        ic->ic_chanchange_chan = ic->ic_curchan->ic_ieee;
        vap->iv_chanchange_count = ic->ic_chanchange_tbtt;

        IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s switch channel %d\n",
                         __func__, ic->ic_curchan->ic_ieee);
        vap->iv_flags |= IEEE80211_F_CHANSWITCH;
    }
    return error;
}
#endif

#ifdef ATH_SUPPORT_HTC
int
wlan_reset_iv_chan(wlan_if_t vaphandle, int chan)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211com *ic = vap->iv_ic;
    struct ieee80211_channel *channel;

    if ((chan != 1) && (chan != 6) && (chan != 11))
        return -EINVAL;

    /*
     * find the corresponding channel object for the desired PHY mode.
     */
    channel = ieee80211_find_dot11_channel(ic, chan, vap->iv_des_cfreq2, vap->iv_des_mode);
    if (channel == NULL) {
        channel = ieee80211_find_dot11_channel(ic, chan, 0, IEEE80211_MODE_AUTO);
        if (channel == NULL) {
            return -EINVAL;
        }
    }
    vap->iv_bsschan = channel;
    return 0;
}
#endif

/* Structure used in the ieee80211_vap_iter_get_bss_chan() to get the BSS frequencies */
struct ieee80211_iter_vaps_get_bss_chan_arg {
    int         max_list_size;
    int         current_list_size;
    u_int32_t   *bss_freq_list;
    int         ret_param;
};

static void ieee80211_vap_iter_get_bss_chan(void *arg, wlan_if_t vap)
{
    struct ieee80211_iter_vaps_get_bss_chan_arg *params;

    params = (struct ieee80211_iter_vaps_get_bss_chan_arg *) arg;

    if (params->ret_param != EOK) {
        return;
    }

    if (ieee80211_vap_ready_is_set(vap)) {
        /* Get the BSS channel and its frequency */
        wlan_chan_t     bsschan;
        int             i;
        u_int32_t       bss_freq;

        bsschan = vap->iv_bsschan;
        if (bsschan && (bsschan != IEEE80211_CHAN_ANYC)) {
            bss_freq = wlan_channel_frequency(bsschan);
        }
        else {
            return;
        }

        /* Check whether it is a duplicate */
        for (i = 0; i < params->current_list_size; i++) {
            if (params->bss_freq_list[i] == bss_freq) {
                /* A match */
                return;
            }
        }
        /* Else no match */
        params->current_list_size++;
        if (params->current_list_size >= params->max_list_size) {
            /* No more space */
            params->ret_param = -EOVERFLOW;
            return;
        }
        params->bss_freq_list[params->current_list_size - 1] = bss_freq;
    }
}

/*
 * Get the list of BSS channel used by other active VAPs. Note that as VAPs go up and down,
 * this list can change with time.
 * bss_freq_list contains an array of frequencies to return the BSS channels used by
 * other active VAPs. On function entry, list_size contains the size of array bss_freq_list.
 * On exit, list_size contains the size of BSS frequency list filled. If no other active VAPs,
 * then list_size will be zero.
 */
int
wlan_get_bss_chan_list(wlan_dev_t devhandle, u_int32_t *bss_freq_list, int *list_size)
{
    struct ieee80211_iter_vaps_get_bss_chan_arg params;

    if (*list_size <= 0) {
        /* Empty list */
        return -EOVERFLOW;
    }

    OS_MEMZERO(&params, sizeof(params));

    params.max_list_size = *list_size;
    params.bss_freq_list = bss_freq_list;
    params.ret_param = EOK;

    wlan_iterate_vap_list(devhandle, ieee80211_vap_iter_get_bss_chan,(void *) &params);

    /* Return the number of BSS channels found */
    *list_size = params.current_list_size;

    return params.ret_param;
}

/*
 * Calculate the channel width of the given umac channel.
 *
 * XXX verify that half/quarter rates are always 5/10MHz, or
 *     whether it should take the actual channel width
 *     (20/40/80) into account (ie, by dividing it by 2 or 4.)
 */
int
ieee80211_get_chan_width(struct ieee80211_channel *chan)
{

	/*
	 * Since VHT channels have HT flags and HT channels
	 * have legacy flags, start at VHT, then do HT,
	 * then check legacy.
	 *
	 * Or, check the PHY mode for the given channel.
	 */

	/*
	 * XXX should these just shift the PHY width by 2 or 4?
	 * (eg, would we ever have a HT40 channel in 5MHz?)
	 */
	if (IEEE80211_IS_CHAN_QUARTER(chan))
		return (5);
	else if (IEEE80211_IS_CHAN_HALF(chan))
		return (10);

	switch (ieee80211_chan2mode(chan)) {
        /* 160 */
		case IEEE80211_MODE_11AC_VHT160:
			return (160);
        /* 80 + 80 */
		case IEEE80211_MODE_11AC_VHT80_80:
			return (80+80);
		/* 80 */
		case IEEE80211_MODE_11AC_VHT80:
			return (80);

		/* 40 */
		case IEEE80211_MODE_11AC_VHT40PLUS:
		case IEEE80211_MODE_11AC_VHT40MINUS:
		case IEEE80211_MODE_11NA_HT40PLUS:
		case IEEE80211_MODE_11NA_HT40MINUS:
		case IEEE80211_MODE_11NA_HT40:
		case IEEE80211_MODE_11NG_HT40PLUS:
		case IEEE80211_MODE_11NG_HT40MINUS:
		case IEEE80211_MODE_11NG_HT40:
			return (40);

		default:
			return (20);
	}
}

/*
 * Calculate the centre frequency of the given channel.
 *
 * XXX doesn't know about VHT 80+80 yet!
 * XXX half/quarter rates!
 */
int
ieee80211_get_chan_centre_freq(struct ieee80211com *ic,
    struct ieee80211_channel *chan)
{
	int chan_centre, chan_offset, chan_width;

	/*
	 * XXX Only VHT80 channels have freq1/freq2 setup.
	 */
	if ((IEEE80211_IS_CHAN_11AC_VHT80(chan)) || (IEEE80211_IS_CHAN_11AC_VHT80_80(chan))) {
		/* 11AC, so cfreq1/cfreq2 are setup */

		/*
		 * XXX center frequency of only first segment is provided even for 80+80
		 */
		chan_centre = ieee80211_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg1,
		    chan->ic_flags);
    } else if (IEEE80211_IS_CHAN_11AC_VHT160(chan)) {

        /* For VHT160, centre frequency is ic_vhtop_ch_freq_seg2 */
        chan_centre = ieee80211_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg2, chan->ic_flags);
    } else {
		/* HT20/HT40, VHT20/VHT40 */

		/*
		 * XXX this is hard-coded - it should be 5 or 10 for
		 * half/quarter appropriately.
		 */
		chan_width = 20;

		/* Grab default channel centre */
		chan_centre = ieee80211_chan2freq(ic, chan);

		/* Calculate offset based on HT40U/HT40D */
		/* XXX and also VHT40 for now */
		/* XXX add flag check methods to umac */
		if (chan->ic_flags & IEEE80211_CHAN_VHT40PLUS)
			chan_offset = chan_width;
		else if (chan->ic_flags & IEEE80211_CHAN_VHT40MINUS)
			chan_offset = -chan_width;
		else if (IEEE80211_IS_CHAN_11N_HT40PLUS(chan))
			chan_offset = chan_width;
		else if (IEEE80211_IS_CHAN_11N_HT40MINUS(chan))
			chan_offset = -chan_width;
		else
			/* HT20 */
			chan_offset = 0;

		/* Calculate new _real_ channel centre */
		chan_centre += (chan_offset / 2);
	}

	/*
	 * XXX TODO: add half/quarter rate support!
	 */

	return (chan_centre);
}

/*
 * Check whether cp falls within p1 and p2, exclusively.
 */
static int
ol_check_intersect_excl(int p1, int p2, int cp)
{

	return (cp > p1 && cp < p2);
}

/*
 * Check whether two frequency centre/channel width entries
 * overlap in any way.
 *
 * This is an exclusive overlap - ie, two touching edges
 * do not match.
 *
 * XXX very likely not optimal at all!
 */
static int
ol_check_overlap(int f1, int w1, int f2, int w2)
{
	int f1_h, f1_l;
	int f2_h, f2_l;
	int ret;

	/* Calculate low/high frequency ranges */
	f1_l = f1 - (w1 / 2);
	f1_h = f1 + (w1 / 2);

	f2_l = f2 - (w2 / 2);
	f2_h = f2 + (w2 / 2);

	/*
	 * Since the intersect check is exclusive, two
	 * exact overlaps won't actually work as each
	 * edge only touches the other edge.  It's not
	 * considered inside a range.  Hence why there's
	 * a separate "is totally equal" check.
	 */
	ret = (
	    ((f1 == f2) && (w1 == w2)) ||
	    ol_check_intersect_excl(f1_l, f1_h, f2_l) ||
	    ol_check_intersect_excl(f1_l, f1_h, f2_h) ||
	    ol_check_intersect_excl(f2_l, f2_h, f1_l) ||
	    ol_check_intersect_excl(f2_l, f2_h, f1_h)
	);

	return (ret);
}

/*
 * Check whether the given channel overlaps (exclusive, not inclusive)
 * the given centre frequency and width.
 *
 * This has to take the VHT flags into account.
 *
 * XXX TODO: 80 + 80 requires two checks!
 * XXX TODO: no 160MHz support!
 */
int
ieee80211_check_channel_overlap(struct ieee80211com *ic,
    struct ieee80211_channel *chan, int nol_freq, int nol_width)
{
	int chan_freq;
	int chan_width;
        int overlap = 0;

	/* Calculate centre frequency */
	chan_freq = ieee80211_get_chan_centre_freq(ic, chan);

	/* Grab width */
	chan_width = ieee80211_get_chan_width(chan);

#if 0
	qdf_print("%s: checking ic_freq=%d, chan_freq=%d, chan_width=%d; freq=%d"
	    " against nol_freq=%d, nol_width=%d\n",
	    __func__,
	    chan->ic_freq,
	    chan_freq,
	    chan_width,
	    chan->ic_vhtop_ch_freq_seg1,
	    nol_freq,
	    nol_width);
#endif
        if (IEEE80211_IS_CHAN_11AC_VHT80_80 (chan)) {
            /* HT80_80 mode has 2 segments and */
            /* each segment must be checked    */
            /* check for control channel first */
	    overlap = ol_check_overlap(chan_freq, chan_width/2, nol_freq, nol_width);
            /* check extension channel */
            chan_freq = ieee80211_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg2, chan->ic_flags);
	    overlap += ol_check_overlap(chan_freq, chan_width/2, nol_freq, nol_width);
        } else {
	    /* Return if there's overlap */
	    overlap = ol_check_overlap(chan_freq, chan_width, nol_freq, nol_width);
        }
        return overlap;
}

void
ieee80211_get_extchaninfo(struct ieee80211com *ic,
        struct ieee80211_channel *chan,
        struct ieee80211_channel_list *chan_info)
{
    int chan_center;

    if (IEEE80211_IS_CHAN_11AC_VHT40PLUS(chan) ||
            IEEE80211_IS_CHAN_11NA_HT40PLUS(chan)) {
        chan_info->cl_nchans = 2;
        chan_info->cl_channels[0] = chan;
        chan_info->cl_channels[1] = ic->ic_find_channel(ic, chan->ic_freq + 20, 0, IEEE80211_CHAN_11NA_HT20);
    } else if (IEEE80211_IS_CHAN_11AC_VHT40MINUS(chan) ||
            IEEE80211_IS_CHAN_11NA_HT40MINUS(chan)) {
        chan_info->cl_nchans = 2;
        chan_info->cl_channels[0] = chan;
        chan_info->cl_channels[1] = ic->ic_find_channel(ic, chan->ic_freq - 20, 0, IEEE80211_CHAN_11NA_HT20);
    } else if (IEEE80211_IS_CHAN_11AC_VHT80(chan)) {
        chan_center = ic->ic_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg1, chan->ic_flags);
        chan_info->cl_nchans = 4;
        chan_info->cl_channels[0] = ic->ic_find_channel(ic, chan_center - 30, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[1] = ic->ic_find_channel(ic, chan_center - 10, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[2] = ic->ic_find_channel(ic, chan_center + 10, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[3] = ic->ic_find_channel(ic, chan_center + 30, 0, IEEE80211_CHAN_11NA_HT20);
    } else if (IEEE80211_IS_CHAN_11AC_VHT160(chan)) {

        chan_center = ic->ic_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg2, chan->ic_flags);
        chan_info->cl_nchans = 8;
        chan_info->cl_channels[0] = ic->ic_find_channel(ic, chan_center - 70, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[1] = ic->ic_find_channel(ic, chan_center - 50, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[2] = ic->ic_find_channel(ic, chan_center - 30, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[3] = ic->ic_find_channel(ic, chan_center - 10, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[4] = ic->ic_find_channel(ic, chan_center + 70, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[5] = ic->ic_find_channel(ic, chan_center + 50, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[6] = ic->ic_find_channel(ic, chan_center + 30, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[7] = ic->ic_find_channel(ic, chan_center + 10, 0, IEEE80211_CHAN_11NA_HT20);
    } else if (IEEE80211_IS_CHAN_11AC_VHT80_80(chan)) {

        chan_center = ic->ic_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg1, chan->ic_flags);
        chan_info->cl_nchans = 8;
        chan_info->cl_channels[0] = ic->ic_find_channel(ic, chan_center - 30, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[1] = ic->ic_find_channel(ic, chan_center - 10, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[2] = ic->ic_find_channel(ic, chan_center + 10, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[3] = ic->ic_find_channel(ic, chan_center + 30, 0, IEEE80211_CHAN_11NA_HT20);

        chan_center = ic->ic_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg2, chan->ic_flags);

        chan_info->cl_channels[4] = ic->ic_find_channel(ic, chan_center - 30, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[5] = ic->ic_find_channel(ic, chan_center - 10, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[6] = ic->ic_find_channel(ic, chan_center + 10, 0, IEEE80211_CHAN_11NA_HT20);
        chan_info->cl_channels[7] = ic->ic_find_channel(ic, chan_center + 30, 0, IEEE80211_CHAN_11NA_HT20);
    } else  {
        chan_info->cl_nchans = 1;
        chan_info->cl_channels[0] = chan;
    }
    return;
}


/***
  Check whether missing extchan is 144
**/
u_int8_t ieee80211_is_extchan_144(struct ieee80211com *ic,
           struct ieee80211_channel *chan, u_int8_t extchan)
{
   u_int16_t chan_center = 0;
   u_int16_t chan_center2 = 0;
   u_int16_t chan_freq = chan->ic_freq;

   if(IEEE80211_IS_CHAN_11AC_VHT80(chan)) {
      chan_center = ic->ic_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg1, chan->ic_flags);
   }
   if(IEEE80211_IS_CHAN_11AC_VHT80_80(chan)) {
      chan_center = ic->ic_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg1, chan->ic_flags);
      chan_center2 = ic->ic_ieee2mhz(ic, chan->ic_vhtop_ch_freq_seg2, chan->ic_flags);
   }
   /* Check whether the channel freq is with in the range */
   /* if it is primary, extchan 144 index is 3 */
   /* if it is secondary, extchan 144 index is 7 */
   if(((chan_center == 5690) && (extchan == 3))||
      ((chan_center2 == 5690) && (extchan == 7))) {
      return 1;
   }
   /* For HT40+, extchan 144 index is 1 */
   if (chan_freq == 5700) {
      if((IEEE80211_IS_CHAN_11NA_HT40PLUS(chan) ||
            IEEE80211_IS_CHAN_11AC_VHT40PLUS(chan)) &&
         (extchan == 1)) {
         return 1;
      }
   }
   return 0;
}

/**
 * @brief finds the offset for secondary twenty channel
 *
 * @param chan: channel for which secondary offset to find
 */

int8_t
ieee80211_secondary20_channel_offset(struct ieee80211_channel *chan)
{

    int8_t pri_center_ch_diff, sec_level;
    u_int16_t pri_chan_40_center;

    if (!chan || IEEE80211_IS_CHAN_A(chan) ||
        IEEE80211_IS_CHAN_B(chan) || IEEE80211_IS_CHAN_G(chan) ||
        IEEE80211_IS_CHAN_PUREG(chan) || IEEE80211_IS_CHAN_ANYG(chan) ||
        IEEE80211_IS_CHAN_11N_HT20(chan) || IEEE80211_IS_CHAN_11AC_VHT20(chan)) {
        /* No secondary channel */
        return 0;
    }

    if (IEEE80211_IS_CHAN_11AC_VHT40PLUS(chan) || IEEE80211_IS_CHAN_11NG_HT40PLUS(chan)
            || IEEE80211_IS_CHAN_11NA_HT40PLUS(chan)) {
            return 1;
    }

    if (IEEE80211_IS_CHAN_11AC_VHT40MINUS(chan) || IEEE80211_IS_CHAN_11NG_HT40MINUS(chan)
            || IEEE80211_IS_CHAN_11NA_HT40MINUS(chan)) {
            return -1;
    }

    if (IEEE80211_IS_CHAN_11AC_VHT80(chan) || IEEE80211_IS_CHAN_11AC_VHT80_80(chan)) {
        /* The following logic generates the extension channel offset from
         * the primary channel(ic_ieee) and 80M channel central frequency.
         * The channelization for 80M is as following:
         * | 20M  20M  20M  20M | with the following example | 36 40 44 48 |
         * |         80M        |                |     80M     |
         * The central frequency is 42 in the example.
         * If the primary channel is 36 and 44, the extension channel is 40PLUS.
         * If the primary channel is 40 and 48 the extension channel is 40MINUS.
         */

        if (chan->ic_ieee < chan->ic_vhtop_ch_freq_seg1) {
            if ((chan->ic_vhtop_ch_freq_seg1 - chan->ic_ieee) > 4) {
                return 1;
            } else {
                return -1;
            }
        } else {
            if ((chan->ic_ieee - chan->ic_vhtop_ch_freq_seg1) > 4) {
                return -1;
            } else {
                return 1;
            }
        }
    }

    if (IEEE80211_IS_CHAN_11AC_VHT160(chan)) {
        /* The channelization of 160M is as following:
         * | 20M 20M 20M 20M 20M 20M 20M 20M | with following example |
         * 36 40 44 48 52 56 60 64 | The center frequency is 40 in this example.
         * If primary channel is 36, 44, 52 or 60, the extension channel is 40PLUS.
         * If primary channel is 40, 48, 56 or 64, the extension channel is 40MINUS.
         */

        pri_center_ch_diff = chan->ic_ieee - chan->ic_vhtop_ch_freq_seg2;

        if (pri_center_ch_diff > 0) {
            sec_level = -1;
        } else {
            sec_level = 1;
        }

        if (sec_level*pri_center_ch_diff < -6) {
            pri_chan_40_center = chan->ic_vhtop_ch_freq_seg2 - (2*sec_level*6);
        } else {
            pri_chan_40_center = chan->ic_vhtop_ch_freq_seg2 - (2*sec_level*2);
        }

        if (pri_chan_40_center > chan->ic_ieee) {
            return 1;
        } else {
            return -1;
        }
    }

    return 0;
}

uint8_t
ieee80211_sec_chan_offset(struct ieee80211_channel *chan)
{
    int     offset      = 0;
    uint8_t secoffset   = 0;

    offset = ieee80211_secondary20_channel_offset(chan);
    switch (offset) {
        case 1:
            secoffset = IEEE80211_SEC_CHAN_OFFSET_SCA;
            break;
        case -1:
            secoffset = IEEE80211_SEC_CHAN_OFFSET_SCB;
            break;
        case 0:
        default:
            secoffset = IEEE80211_SEC_CHAN_OFFSET_SCN;
            break;
    }
    return secoffset;
}


/**
* @brief finds current mode of underlying
         radio interface (like a/b/g/n/ac etc)
*
* @param ic pointer to struct ieee80211com
*
* @return current phymode or invalid
*/
enum ieee80211_mode ieee80211_get_mode(struct ieee80211com *ic)
{
    struct ieee80211_channel *ch = ic->ic_curchan;
    enum ieee80211_mode mode = IEEE80211_MODE_INVALID;

    if (IEEE80211_IS_CHAN_A(ch)) {
        mode = IEEE80211_MODE_A;
    } else if (IEEE80211_IS_CHAN_B(ch)) {
        mode = IEEE80211_MODE_B;
    } else if (IEEE80211_IS_CHAN_G(ch) ||
            IEEE80211_IS_CHAN_PUREG(ch) ||
            IEEE80211_IS_CHAN_ANYG(ch)) {
        mode = IEEE80211_MODE_G;
    } else if (IEEE80211_IS_CHAN_11NG(ch) ||
               IEEE80211_IS_CHAN_11NA(ch)) {
        mode = IEEE80211_MODE_N;
    } else if (IEEE80211_IS_CHAN_VHT(ch)) {
        mode = IEEE80211_MODE_AC;
    }

    return mode;
}


/**
* @brief calclulates composite phymode from given mode,
         chwidth and secondary channel offset.
*
* @param mode       :   a/b/g/n/ac etc
* @param chwidth    :   channel width
* @param secchanoffset: secondary channel offset.
*
* @return ieee80211_phymode
*/
enum ieee80211_phymode ieee80211_get_composite_phymode (enum ieee80211_mode mode,
        enum ieee80211_cwm_width chwidth, uint8_t secchanoffset)
{
    enum ieee80211_phymode phymode = IEEE80211_MODE_AUTO;

    switch(mode) {
        case IEEE80211_MODE_AC:
        {
            if (chwidth == IEEE80211_CWM_WIDTH20) {
                phymode = IEEE80211_MODE_11AC_VHT20;
            } else if (chwidth == IEEE80211_CWM_WIDTH40) {
                if (secchanoffset == IEEE80211_SEC_CHAN_OFFSET_SCA) {
                    phymode = IEEE80211_MODE_11AC_VHT40PLUS;
                } else if (secchanoffset == IEEE80211_SEC_CHAN_OFFSET_SCB) {
                    phymode = IEEE80211_MODE_11AC_VHT40MINUS;
                }
            } else if (chwidth == IEEE80211_CWM_WIDTH80) {
                phymode = IEEE80211_MODE_11AC_VHT80;
            } else if (chwidth == IEEE80211_CWM_WIDTH160) {
                phymode = IEEE80211_MODE_11AC_VHT160;
            } else if (chwidth == IEEE80211_CWM_WIDTH80_80) {
                phymode = IEEE80211_MODE_11AC_VHT80_80;
            }
            break;
        }
        case IEEE80211_MODE_NA:
        {
            if (chwidth == IEEE80211_CWM_WIDTH20) {
                phymode = IEEE80211_MODE_11NA_HT20;
            } else if (chwidth == IEEE80211_CWM_WIDTH40) {
                if (secchanoffset == IEEE80211_SEC_CHAN_OFFSET_SCA) {
                    phymode = IEEE80211_MODE_11NA_HT40PLUS;
                } else if (secchanoffset == IEEE80211_SEC_CHAN_OFFSET_SCB) {
                    phymode = IEEE80211_MODE_11NA_HT40MINUS;
                }
            }
            break;
        }
        case IEEE80211_MODE_NG:
        {
            if (chwidth == IEEE80211_CWM_WIDTH20) {
                phymode = IEEE80211_MODE_11NG_HT20;
            } else if (chwidth == IEEE80211_CWM_WIDTH40) {
                if (secchanoffset == IEEE80211_SEC_CHAN_OFFSET_SCA) {
                    phymode = IEEE80211_MODE_11NG_HT40PLUS;
                } else if (secchanoffset == IEEE80211_SEC_CHAN_OFFSET_SCB) {
                    phymode = IEEE80211_MODE_11NG_HT40MINUS;
                }
            }
            break;
        }
        case IEEE80211_MODE_A:
        {
            if (chwidth == IEEE80211_CWM_WIDTH20) {
                phymode = IEEE80211_MODE_11A;
            }
            break;
        }
        case IEEE80211_MODE_G:
        {
            if (chwidth == IEEE80211_CWM_WIDTH20) {
                phymode = IEEE80211_MODE_11G;
            }
            break;
        }
        case IEEE80211_MODE_B:
        {
            if (chwidth == IEEE80211_CWM_WIDTH20) {
                phymode = IEEE80211_MODE_11B;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return phymode;
}

enum ieee80211_cwm_width
ieee80211_get_cwm_width_from_channel(struct ieee80211_channel *chan)
{
    /* enum ieee80211_cwm_width = IEEE80211_CWM_WIDTHINVALID; */

    if (IEEE80211_IS_CHAN_11AC(chan)) {
        if (IEEE80211_IS_CHAN_11AC_VHT80_80(chan)) {
            return IEEE80211_CWM_WIDTH80_80;
        }
        if (IEEE80211_IS_CHAN_11AC_VHT160(chan)) {
            return IEEE80211_CWM_WIDTH160;
        }
        if (IEEE80211_IS_CHAN_11AC_VHT80(chan)) {
            return IEEE80211_CWM_WIDTH80;
        }
        if (IEEE80211_IS_CHAN_11AC_VHT40(chan)) {
            return IEEE80211_CWM_WIDTH40;
        }
        if (IEEE80211_IS_CHAN_11AC_VHT20(chan)) {
            return IEEE80211_CWM_WIDTH20;
        }
    }

    if (IEEE80211_IS_CHAN_11N(chan)) {
        if (IEEE80211_IS_CHAN_11NA_HT20(chan) || IEEE80211_IS_CHAN_11NG_HT20(chan)) {
            return IEEE80211_CWM_WIDTH20;
        } else {
            return IEEE80211_CWM_WIDTH40;
        }
    }

    return IEEE80211_CWM_WIDTH20;
}

#if ATH_SUPPORT_DFS && ATH_SUPPORT_STA_DFS
void
ieee80211_print_nolhistory(struct ieee80211com *ic)
{
    struct ieee80211_channel *c;
    int i,j=0;

    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];
        if(IEEE80211_IS_CHAN_HISTORY_RADAR(c)) {
            printk("nolhistory:%d channel=%d MHz Flags=%X \n",j,c->ic_freq,c->ic_flags);
            j++;
        }
    }
    return;
}

void
ieee80211_clear_nolhistory(struct ieee80211com *ic)
{
    struct ieee80211_channel *c;
    int i;

    for (i = 0; i < ic->ic_nchans; i++) {
        c = &ic->ic_channels[i];
        IEEE80211_CHAN_CLR_HISTORY_RADAR(c);
    }
    return;
}
#endif
