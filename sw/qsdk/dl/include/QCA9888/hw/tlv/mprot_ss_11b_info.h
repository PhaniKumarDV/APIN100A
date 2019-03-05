// Copyright (c) 2014 Qualcomm Atheros, Inc.  All rights reserved.
// $ATH_LICENSE_HW_HDR_C$
//
// DO NOT EDIT!  This file is automatically generated
//               These definitions are tied to a particular hardware layout


#ifndef _MPROT_SS_11B_INFO_H_
#define _MPROT_SS_11B_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct l_sig_b l_sig_b_bw20;
//	1	struct l_sig_b l_sig_b_bw40;
//	2	struct tx_service tx_service_bw20;
//	3	struct tx_service tx_service_bw40;
//	4	duration_bw20[15:0], duration_bw40[31:16]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_MPROT_SS_11B_INFO 5

struct mprot_ss_11b_info {
    struct            l_sig_b                       l_sig_b_bw20;
    struct            l_sig_b                       l_sig_b_bw40;
    struct            tx_service                       tx_service_bw20;
    struct            tx_service                       tx_service_bw40;
    volatile uint32_t duration_bw20                   : 16, //[15:0]
                      duration_bw40                   : 16; //[31:16]
};

/*

struct l_sig_b l_sig_b_bw20
			
			This field has exactly the same contents as the L_SIG_B
			TLV (without the tag/length word)

struct l_sig_b l_sig_b_bw40
			
			This field has exactly the same contents as the L_SIG_B
			TLV (without the tag/length word)

struct tx_service tx_service_bw20
			
			This field has exactly the same contents as the
			TX_SERVICE TLV (without the tag/length word)

struct tx_service tx_service_bw40
			
			This field has exactly the same contents as the
			TX_SERVICE TLV (without the tag/length word)

duration_bw20
			
			Duration in the generated RTS/CTS frame for 20 MHz
			transmission.

duration_bw40
			
			Duration in the generated RTS/CTS frame for 40 MHz
			transmission.
*/

#define MPROT_SS_11B_INFO_0_L_SIG_B_L_SIG_B_BW20_OFFSET              0x00000000
#define MPROT_SS_11B_INFO_0_L_SIG_B_L_SIG_B_BW20_LSB                 16
#define MPROT_SS_11B_INFO_0_L_SIG_B_L_SIG_B_BW20_MASK                0xffffffff
#define MPROT_SS_11B_INFO_1_L_SIG_B_L_SIG_B_BW40_OFFSET              0x00000004
#define MPROT_SS_11B_INFO_1_L_SIG_B_L_SIG_B_BW40_LSB                 16
#define MPROT_SS_11B_INFO_1_L_SIG_B_L_SIG_B_BW40_MASK                0xffffffff
#define MPROT_SS_11B_INFO_2_TX_SERVICE_TX_SERVICE_BW20_OFFSET        0x00000008
#define MPROT_SS_11B_INFO_2_TX_SERVICE_TX_SERVICE_BW20_LSB           16
#define MPROT_SS_11B_INFO_2_TX_SERVICE_TX_SERVICE_BW20_MASK          0xffffffff
#define MPROT_SS_11B_INFO_3_TX_SERVICE_TX_SERVICE_BW40_OFFSET        0x0000000c
#define MPROT_SS_11B_INFO_3_TX_SERVICE_TX_SERVICE_BW40_LSB           16
#define MPROT_SS_11B_INFO_3_TX_SERVICE_TX_SERVICE_BW40_MASK          0xffffffff

/* Description		MPROT_SS_11B_INFO_4_DURATION_BW20
			
			Duration in the generated RTS/CTS frame for 20 MHz
			transmission.
*/
#define MPROT_SS_11B_INFO_4_DURATION_BW20_OFFSET                     0x00000010
#define MPROT_SS_11B_INFO_4_DURATION_BW20_LSB                        0
#define MPROT_SS_11B_INFO_4_DURATION_BW20_MASK                       0x0000ffff

/* Description		MPROT_SS_11B_INFO_4_DURATION_BW40
			
			Duration in the generated RTS/CTS frame for 40 MHz
			transmission.
*/
#define MPROT_SS_11B_INFO_4_DURATION_BW40_OFFSET                     0x00000010
#define MPROT_SS_11B_INFO_4_DURATION_BW40_LSB                        16
#define MPROT_SS_11B_INFO_4_DURATION_BW40_MASK                       0xffff0000


#endif // _MPROT_SS_11B_INFO_H_
