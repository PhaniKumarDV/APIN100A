/*
 *  Copyright (c) 2008 Atheros Communications Inc.
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
#ifdef ATH_WPS_IE
#include "ieee80211_var.h"
#include "ieee80211_api.h"

int
wlan_frm_haswscie(const wbuf_t wbuf)
{
    const u_int8_t *frm = ((u_int8_t *)wbuf_header(wbuf) + sizeof(struct ieee80211_frame));
    const u_int8_t *efrm = ((u_int8_t *)wbuf_header(wbuf) + wbuf_get_pktlen(wbuf));
    while (frm < efrm) {
        switch (*frm) {
            case IEEE80211_ELEMID_VENDOR:
                if (iswpsoui((u_int8_t *)frm))
                    return 1;
                break;
                default:
                        break;
        }
        frm += frm[1] + 2;
    }
    return 0;
}
#endif /*ATH_WPS_IE*/
