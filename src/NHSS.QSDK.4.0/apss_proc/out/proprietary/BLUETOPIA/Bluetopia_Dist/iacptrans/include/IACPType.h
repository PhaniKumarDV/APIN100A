/*****< iacptype.h >***********************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  IACPTYPE - Apple Authentication Coprocessor Type Definitions, Register    */
/*             Definitions, and Constants.                                    */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/08/11  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __IACPTYPEH__
#define __IACPTYPEH__

   /* The following constants represent the I2C Read/Write addresses    */
   /* that the Authentication Coprocessor can be configured to use.     */
   /* * NOTE * Prior to Coprocessor version 2.0C, these were selected   */
   /*          via Mode0/Mode1 pins.                                    */
#define IACP_I2C_WRITE_ADDRESS_OPTION_RANGE_0x20_0x21                0x20
#define IACP_I2C_READ_ADDRESS_OPTION_RANGE_0x20_0x21                 0x21

#define IACP_I2C_WRITE_ADDRESS_OPTION_RANGE_0x22_0x23                0x22
#define IACP_I2C_READ_ADDRESS_OPTION_RANGE_0x22_0x23                 0x23

   /* The following defines the register addresses that are present in  */
   /* the Authentication Coprocessor.                                   */
#define IACP_DEVICE_VERSION_REGISTER                                 0x00
#define IACP_FIRMWARE_VERSION_REGISTER                               0x01
#define IACP_AUTHENTICATION_PROTOCOL_MAJOR_VERSION_REGISTER          0x02
#define IACP_AUTHENTICATION_PROTOCOL_MINOR_VERSION_REGISTER          0x03
#define IACP_DEVICE_ID_REGISTER                                      0x04
#define IACP_ERROR_CODE_REGISTER                                     0x05
#define IACP_AUTHENTICATION_CONTROL_STATUS_REGISTER                  0x10
#define IACP_SIGNATURE_DATA_LENGTH_REGISTER                          0x11
#define IACP_SIGNATURE_DATA_REGISTER                                 0x12
#define IACP_CHALLENGE_DATA_LENGTH_REGISTER                          0x20
#define IACP_CHALLENGE_DATA_REGISTER                                 0x21
#define IACP_ACCESSORY_CERTIFICATE_DATA_LENGTH_REGISTER              0x30
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_01_REGISTER             0x31
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_02_REGISTER             0x32
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_03_REGISTER             0x33
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_04_REGISTER             0x34
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_05_REGISTER             0x35
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_06_REGISTER             0x36
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_07_REGISTER             0x37
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_08_REGISTER             0x38
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_09_REGISTER             0x39
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_10_REGISTER             0x3A
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_11_REGISTER             0x3B
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_12_REGISTER             0x3C
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_13_REGISTER             0x3D
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_14_REGISTER             0x3E
#define IACP_ACCESSORY_CERTIFICATE_DATA_PAGE_15_REGISTER             0x3F
#define IACP_SELF_TEST_CONTROL_STATUS_REGISTER                       0x40
#define IACP_IPOD_CERTIFICATE_DATA_LENGTH_REGISTER                   0x50
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_01_REGISTER                  0x51
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_02_REGISTER                  0x52
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_03_REGISTER                  0x53
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_04_REGISTER                  0x54
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_05_REGISTER                  0x55
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_06_REGISTER                  0x56
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_07_REGISTER                  0x57
#define IACP_IPOD_CERTIFICATE_DATA_PAGE_08_REGISTER                  0x58

   /* The following defines the values that may be presented via the    */
   /* Error Code register.                                              */
#define IACP_ERROR_CODE_NO_ERROR                                        0
#define IACP_ERROR_CODE_INVALID_READ_REGISTER                           1
#define IACP_ERROR_CODE_INVALID_WRITE_REGISTER                          2
#define IACP_ERROR_CODE_INVALID_SIGNATURE_LENGTH                        3
#define IACP_ERROR_CODE_INVALID_CHALLENGE_LENGTH                        4
#define IACP_ERROR_CODE_INVALID_CERTIFICATE_LENGTH                      5
#define IACP_ERROR_CODE_INTERNAL_ERROR_DURING_SIGNATURE_GENERATION      6
#define IACP_ERROR_CODE_INTERNAL_ERROR_DURING_CHALLENGE_GENERATION      7
#define IACP_ERROR_CODE_INTERNAL_ERROR_DURING_SIGNATURE_VERIFICATION    8
#define IACP_ERROR_CODE_INTERNAL_ERROR_DURING_CHALLENGE_VALIDATION      9
#define IACP_ERROR_CODE_INVALID_PROCESS_CONTROL                        10

   /* The following define the command values that can be written to the*/
   /* Authentication Control and Status register.                       */
#define IACP_PROC_CONTROL_NO_EFFECT                                  0x00
#define IACP_PROC_CONTROL_START_SIGNATURE_GENERATION_PROCESS         0x01
#define IACP_PROC_CONTROL_START_CHALLENGE_GENERATION_PROCESS         0x02
#define IACP_PROC_CONTROL_START_SIGNATURE_VERIFICATION_PROCESS       0x03
#define IACP_PROC_CONTROL_START_CHALLENGE_VALIDATION_PROCESS         0x04
#define IACP_PROC_CONTROL_FORCE_ACP_TO_SLEEP                         0x05

   /* The following define the status values that can be read from the  */
   /* Authentication Control and Status register.                       */
#define IACP_PROC_RESULT_VALUE_MASK                                  0x70
#define IACP_PROC_RESULT_ERROR_MASK                                  0x80
#define IACP_PROC_RESULT_NO_RESULT_PRODUCED                          0x00
#define IACP_PROC_RESULT_SIGNATURE_SUCCESSFULLY_GENERATED            0x10
#define IACP_PROC_RESULT_CHALLENGE_SUCCESSFULLY_GENERATED            0x20
#define IACP_PROC_RESULT_SIGNATURE_SUCCESSFULLY_VERIFIED             0x30
#define IACP_PROC_RESULT_CERTIFICATE_SUCCESSFULLY_VALIDATED          0x40

   /* The following define the command values that can be written to the*/
   /* Selt-Test Control and Status register.                            */
#define IACP_SELF_TEST_PROC_CONTROL_NONE                             0x00
#define IACP_SELF_TEST_PROC_CONTROL_RUN_CERTIFICATE_AND_KEY_TEST     0x01

   /* The following define the status values that can be read from the  */
   /* Self-Test Control and Status register.                            */
#define IACP_SELF_TEST_PROC_RESULT_VALUE_MASK                        0xF0
#define IACP_SELF_TEST_PROC_RESULT_CERTIFICATE_NOT_FOUND             0x70
#define IACP_SELF_TEST_PROC_RESULT_PRIVATE_KEY_NOT_FOUND             0x60

#endif
