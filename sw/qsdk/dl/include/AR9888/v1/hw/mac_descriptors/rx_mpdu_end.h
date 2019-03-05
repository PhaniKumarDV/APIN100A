// Copyright (c) 2011 Qualcomm Atheros, Inc.  All rights reserved.
// $ATH_LICENSE_HW_HDR_C$
//
// DO NOT EDIT!  These definitions are tied to a particular hardware layout

#ifndef _RX_MPDU_END_H_
#define _RX_MPDU_END_H_
#if !defined(__ASSEMBLER__)
#endif

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	reserved_0[12:0], overflow_err[13], last_mpdu[14], post_delim_err[15], post_delim_cnt[27:16], mpdu_length_err[28], tkip_mic_err[29], decrypt_err[30], fcs_err[31]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_MPDU_END 1

struct rx_mpdu_end {
    volatile uint32_t reserved_0                      : 13, //[12:0]
                      overflow_err                    :  1, //[13]
                      last_mpdu                       :  1, //[14]
                      post_delim_err                  :  1, //[15]
                      post_delim_cnt                  : 12, //[27:16]
                      mpdu_length_err                 :  1, //[28]
                      tkip_mic_err                    :  1, //[29]
                      decrypt_err                     :  1, //[30]
                      fcs_err                         :  1; //[31]
};

/*

reserved_0
			Reserved

overflow_err
			PCU Receive FIFO does not have enough space to store the
			full receive packet.  Enough space is reserved in the
			receive FIFO for the status is written.  This MPDU remaining
			packets in the PPDU will be filtered and no Ack response
			will be transmitted.

last_mpdu
			Indicates that this is the last MPDU of a PPDU.

post_delim_err
			Indicates that a delimiter FCS error occurred after this
			MPDU before the next MPDU.  Only valid when last_msdu is
			set.

post_delim_cnt
			Count of the delimiters after this MPDU.  This requires the
			last MPDU to be held until all the EOF descriptors have been
			received.  This may be inefficient in the future when
			ML-MIMO is used.  Only valid when last_mpdu is set.

mpdu_length_err
			See definition in RX attention descriptor

tkip_mic_err
			See definition in RX attention descriptor

decrypt_err
			See definition in RX attention descriptor

fcs_err
			See definition in RX attention descriptor
*/


/* Description		RX_MPDU_END_0_RESERVED_0
			Reserved
*/
#define RX_MPDU_END_0_RESERVED_0_OFFSET                              0x00000000
#define RX_MPDU_END_0_RESERVED_0_LSB                                 0
#define RX_MPDU_END_0_RESERVED_0_MASK                                0x00001fff

/* Description		RX_MPDU_END_0_OVERFLOW_ERR
			PCU Receive FIFO does not have enough space to store the
			full receive packet.  Enough space is reserved in the
			receive FIFO for the status is written.  This MPDU remaining
			packets in the PPDU will be filtered and no Ack response
			will be transmitted.
*/
#define RX_MPDU_END_0_OVERFLOW_ERR_OFFSET                            0x00000000
#define RX_MPDU_END_0_OVERFLOW_ERR_LSB                               13
#define RX_MPDU_END_0_OVERFLOW_ERR_MASK                              0x00002000

/* Description		RX_MPDU_END_0_LAST_MPDU
			Indicates that this is the last MPDU of a PPDU.
*/
#define RX_MPDU_END_0_LAST_MPDU_OFFSET                               0x00000000
#define RX_MPDU_END_0_LAST_MPDU_LSB                                  14
#define RX_MPDU_END_0_LAST_MPDU_MASK                                 0x00004000

/* Description		RX_MPDU_END_0_POST_DELIM_ERR
			Indicates that a delimiter FCS error occurred after this
			MPDU before the next MPDU.  Only valid when last_msdu is
			set.
*/
#define RX_MPDU_END_0_POST_DELIM_ERR_OFFSET                          0x00000000
#define RX_MPDU_END_0_POST_DELIM_ERR_LSB                             15
#define RX_MPDU_END_0_POST_DELIM_ERR_MASK                            0x00008000

/* Description		RX_MPDU_END_0_POST_DELIM_CNT
			Count of the delimiters after this MPDU.  This requires the
			last MPDU to be held until all the EOF descriptors have been
			received.  This may be inefficient in the future when
			ML-MIMO is used.  Only valid when last_mpdu is set.
*/
#define RX_MPDU_END_0_POST_DELIM_CNT_OFFSET                          0x00000000
#define RX_MPDU_END_0_POST_DELIM_CNT_LSB                             16
#define RX_MPDU_END_0_POST_DELIM_CNT_MASK                            0x0fff0000

/* Description		RX_MPDU_END_0_MPDU_LENGTH_ERR
			See definition in RX attention descriptor
*/
#define RX_MPDU_END_0_MPDU_LENGTH_ERR_OFFSET                         0x00000000
#define RX_MPDU_END_0_MPDU_LENGTH_ERR_LSB                            28
#define RX_MPDU_END_0_MPDU_LENGTH_ERR_MASK                           0x10000000

/* Description		RX_MPDU_END_0_TKIP_MIC_ERR
			See definition in RX attention descriptor
*/
#define RX_MPDU_END_0_TKIP_MIC_ERR_OFFSET                            0x00000000
#define RX_MPDU_END_0_TKIP_MIC_ERR_LSB                               29
#define RX_MPDU_END_0_TKIP_MIC_ERR_MASK                              0x20000000

/* Description		RX_MPDU_END_0_DECRYPT_ERR
			See definition in RX attention descriptor
*/
#define RX_MPDU_END_0_DECRYPT_ERR_OFFSET                             0x00000000
#define RX_MPDU_END_0_DECRYPT_ERR_LSB                                30
#define RX_MPDU_END_0_DECRYPT_ERR_MASK                               0x40000000

/* Description		RX_MPDU_END_0_FCS_ERR
			See definition in RX attention descriptor
*/
#define RX_MPDU_END_0_FCS_ERR_OFFSET                                 0x00000000
#define RX_MPDU_END_0_FCS_ERR_LSB                                    31
#define RX_MPDU_END_0_FCS_ERR_MASK                                   0x80000000


#endif // _RX_MPDU_END_H_
