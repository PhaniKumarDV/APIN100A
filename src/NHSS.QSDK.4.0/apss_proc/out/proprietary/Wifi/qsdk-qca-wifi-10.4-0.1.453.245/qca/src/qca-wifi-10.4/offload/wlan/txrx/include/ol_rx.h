/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
#ifndef _OL_RX__H_
#define _OL_RX__H_

#include <qdf_nbuf.h> /* qdf_nbuf_t */
#include <ol_txrx_types.h> /* ol_txrx_vdev_t, etc. */

void
ol_rx_deliver(
    struct ol_txrx_vdev_t *vdev,
    struct ol_txrx_peer_t *peer,
    unsigned tid,
    qdf_nbuf_t head_msdu);

void
ol_rx_discard(
    struct ol_txrx_vdev_t *vdev,
    struct ol_txrx_peer_t *peer,
    unsigned tid,
    qdf_nbuf_t head_msdu);

void 
ol_rx_frames_free(
    htt_pdev_handle htt_pdev, 
    qdf_nbuf_t frames);

void
ol_rx_peer_init(struct ol_txrx_pdev_t *pdev, struct ol_txrx_peer_t *peer);

void
ol_rx_peer_cleanup(struct ol_txrx_vdev_t *vdev, struct ol_txrx_peer_t *peer);

#endif /* _OL_RX__H_ */
