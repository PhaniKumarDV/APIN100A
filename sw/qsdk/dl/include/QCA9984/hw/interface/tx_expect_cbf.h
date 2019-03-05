// Copyright (c) 2014 Qualcomm Atheros, Inc.  All rights reserved.
// $ATH_LICENSE_HW_HDR_C$
//
// DO NOT EDIT!  This file is automatically generated
//               These definitions are tied to a particular hardware layout


#ifndef _TX_EXPECT_CBF_H_
#define _TX_EXPECT_CBF_H_
#if !defined(__ASSEMBLER__)
#endif

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	user_number[1:0], reserved[31:2]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_TX_EXPECT_CBF 1

struct tx_expect_cbf {
    volatile uint32_t user_number                     :  2, //[1:0]
                      reserved                        : 30; //[31:2]
};

/*

user_number
			
			Defines the source of the expected CBF. The user number
			refers to the hardware user location, not the MU group
			location. <legal 0-3>

reserved
			
			Reserved - MAC to set this field to 0, PHY to ignore
			this field.  <legal 0>
*/


/* Description		TX_EXPECT_CBF_0_USER_NUMBER
			
			Defines the source of the expected CBF. The user number
			refers to the hardware user location, not the MU group
			location. <legal 0-3>
*/
#define TX_EXPECT_CBF_0_USER_NUMBER_OFFSET                           0x00000000
#define TX_EXPECT_CBF_0_USER_NUMBER_LSB                              0
#define TX_EXPECT_CBF_0_USER_NUMBER_MASK                             0x00000003

/* Description		TX_EXPECT_CBF_0_RESERVED
			
			Reserved - MAC to set this field to 0, PHY to ignore
			this field.  <legal 0>
*/
#define TX_EXPECT_CBF_0_RESERVED_OFFSET                              0x00000000
#define TX_EXPECT_CBF_0_RESERVED_LSB                                 2
#define TX_EXPECT_CBF_0_RESERVED_MASK                                0xfffffffc


#endif // _TX_EXPECT_CBF_H_
