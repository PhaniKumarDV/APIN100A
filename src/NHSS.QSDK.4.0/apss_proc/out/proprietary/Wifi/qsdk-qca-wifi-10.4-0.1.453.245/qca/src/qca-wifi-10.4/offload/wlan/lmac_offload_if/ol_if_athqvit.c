/*
 * Copyright (c) 2011, Atheros Communications Inc.
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

#include "ol_if_athvar.h"
#include "ol_defines.h"
#include "sw_version.h"
#include "targaddrs.h"
#include "ol_helper.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "qdf_types.h" /* qdf_vprint */

#ifdef  QVIT
#include <qvit/QMF.h>
#include <qvit/qvit_defs.h>
int qvit_check_message(ol_scn_t scn, u_int8_t *data);
void qvit_process_swba_event(ol_scn_t scn, QVIT_Msg_Async_SWBA_Event *swba_event);
void qvit_process_tbtt_event(ol_scn_t scn, QVIT_Msg_Async_SWBA_Event *swba_event);

int qvit_check_message(ol_scn_t scn, u_int8_t *data)
{
    u_int8_t *cptr = data;
    QVIT_Msg_Async_SWBA_Event *swba_event;
    cptr += sizeof(QVIT_SEG_HDR_INFO_STRUCT);
    swba_event = (QVIT_Msg_Async_SWBA_Event *)cptr;

    if (swba_event->hdr.msgCode == QVIT_MSG_ASYNC_SWBA_EVENT)
    {
#ifdef QVIT_DEBUG_SWBA_EVENT
        printk(KERN_INFO "QVIT: %s() msgCode [0x%x]\n", __FUNCTION__, swba_event->hdr.msgCode);
        printk(KERN_INFO "QVIT: %s: Start\n", __FUNCTION__);
        qvit_hexdump((unsigned char *)cptr, swba_event->hdr.msgLen);
        printk(KERN_INFO "QVIT: %s: End\n", __FUNCTION__);
#endif
        qvit_process_swba_event(scn, swba_event);
        return 0;
    }

    if (swba_event->hdr.msgCode == QVIT_MSG_ASYNC_TBTT_EVENT)
    {
#ifdef QVIT_DEBUG_TBTT_EVENT
        printk(KERN_INFO "QVIT: %s() msgCode [0x%x]\n", __FUNCTION__, swba_event->hdr.msgCode);
        printk(KERN_INFO "QVIT: %s: Start\n", __FUNCTION__);
        qvit_hexdump((unsigned char *)cptr, swba_event->hdr.msgLen);
        printk(KERN_INFO "QVIT: %s: End\n", __FUNCTION__);
#endif
        qvit_process_tbtt_event(scn, swba_event);
        return 0;
    }
    return 1;
}


static int ol_ath_qvit_event(ol_scn_t scn,
                             u_int8_t *data,
                             u_int32_t datalen)
{
    QVIT_SEG_HDR_INFO_STRUCT segHdrInfo;
    u_int8_t totalNumOfSegments,currentSeq;

    segHdrInfo = *(QVIT_SEG_HDR_INFO_STRUCT *)&(data[0]);

    scn->utf_event_info.currentSeq = (segHdrInfo.segmentInfo & 0xF);

    currentSeq = (segHdrInfo.segmentInfo & 0xF);
    totalNumOfSegments = (segHdrInfo.segmentInfo >>4)&0xF;

    datalen = datalen - sizeof(segHdrInfo);
#ifdef QVIT_DEBUG_EVENT
    printf(KERN_INFO "QVIT: %s: totalNumOfSegments [%d]\n", __FUNCTION__, totalNumOfSegments);
#endif
    if ( currentSeq == 0 )
    {
        scn->utf_event_info.expectedSeq = 0;
        scn->utf_event_info.offset = 0;
    }
    else
    {
        if ( scn->utf_event_info.expectedSeq != currentSeq )
        {
            printk(KERN_ERR "QVIT: Mismatch in expecting seq expected Seq %d got seq %d\n",scn->utf_event_info.expectedSeq,currentSeq);
        }
    }

    OS_MEMCPY(&scn->utf_event_info.data[scn->utf_event_info.offset],&data[sizeof(segHdrInfo)],datalen);
    scn->utf_event_info.offset = scn->utf_event_info.offset + datalen;
    scn->utf_event_info.expectedSeq++;
#ifdef QVIT_DEBUG_EVENT
    printk(KERN_INFO "QVIT: %s: scn->utf_event_info.expectedSeq [%d]\n", __FUNCTION__, scn->utf_event_info.expectedSeq);
#endif
    if ( scn->utf_event_info.expectedSeq == totalNumOfSegments )
    {
        if( scn->utf_event_info.offset != segHdrInfo.len )
        {
            printk(KERN_ERR "QVIT: All segs received total len mismatch .. len %d total len %d\n",scn->utf_event_info.offset,segHdrInfo.len);
        }
#ifdef QVIT_DEBUG_EVENT
        qvit_hexdump((unsigned char *)data, datalen + 4);
#endif
        /* Now we have whole message */
        /* inspect it to find if it is ASYNC_SWBA_EVENT */
        /* qMsgPtr->msgCode >= QVIT_MSG_ASYNC_FA_HA_START &&
                  qMsgPtr->msgCode <= QVIT_MSG_ASYNC_FA_HA_END) */
        if (qvit_check_message(scn, data))
            scn->utf_event_info.length = scn->utf_event_info.offset;
    }

    return 0;
}

void ol_ath_qvit_detach(struct ol_ath_softc_net80211 *scn)
{

#ifdef QVIT_debug
    printk(KERN_INFO "QVIT: %s: called\n", __FUNCTION__);
#endif
    OS_FREE(scn->utf_event_info.data);
    wmi_unified_unregister_event_handler(scn->wmi_handle, wmi_pdev_qvit_event_id);
    scn->utf_event_info.data = NULL;
    scn->utf_event_info.length = 0;
}

void ol_ath_qvit_attach(struct ol_ath_softc_net80211 *scn)
{
#ifdef QVIT_DEBUG
    printk(KERN_INFO "QVIT: %s: called\n", __FUNCTION__);
#endif
    scn->utf_event_info.data = (unsigned char *)OS_MALLOC((void*)scn->sc_osdev,MAX_QVIT_EVENT_LENGTH,GFP_KERNEL);
    scn->utf_event_info.length = 0;

    wmi_unified_register_event_handler(scn->wmi_handle, wmi_pdev_qvit_event_id,
                                       ol_ath_qvit_event,
                                       WMI_RX_UMAC_CTX);
}

int ol_ath_pdev_qvit_cmd(struct ol_ath_softc_net80211 *scn,
                              u_int8_t *utf_payload,
                              u_int32_t len)
{
    struct pdev_qvit_params param;

    qdf_mem_set(&param, sizeof(param), 0);
    param.utf_payload = utf_payload;
    param.len = len;

#ifdef QVIT_DEBUG
    printk(KERN_INFO "QVIT: %s: called\n", __FUNCTION__);
#endif

    return wmi_unified_pdev_qvit_cmd_send(scn->wmi_handle, &param);
}

int ol_ath_qvit_cmd(ol_scn_t scn_handle, u_int8_t *data, u_int16_t len)
{
    struct ol_ath_softc_net80211 *scn = (struct ol_ath_softc_net80211 *)scn_handle;

    scn->utf_event_info.length = 0;

#ifdef QVIT_DEBUG
    printk(KERN_INFO "QVIT: %s called\n", __FUNCTION__);
#endif
    return wmi_unified_pdev_qvit_cmd(scn,data,len);
}


int ol_ath_qvit_rsp(ol_scn_t scn, u_int8_t *payload)
{
    int ret = -1;
    if ( scn->utf_event_info.length )
    {
        ret = 0;

        *(A_UINT32*)&(payload[0]) = scn->utf_event_info.length;
        OS_MEMCPY((payload+4), scn->utf_event_info.data, scn->utf_event_info.length);

#ifdef QVIT_DEBUG_RSP
        printk(KERN_INFO "QVIT: %s: Start\n", __FUNCTION__);
        qvit_hexdump((unsigned char *)payload, (unsigned int)(scn->utf_event_info.length + 4));
        printk(KERN_INFO "QVIT: %s: End\n", __FUNCTION__);
#endif
        scn->utf_event_info.length = 0;
    }
#ifdef QVIT_DEBUG_RSP1
    printf(KERN_INFO "QVIT: %s: ret = %d\n", __FUNCTION__, ret);
#endif
    return ret;
}




#endif
