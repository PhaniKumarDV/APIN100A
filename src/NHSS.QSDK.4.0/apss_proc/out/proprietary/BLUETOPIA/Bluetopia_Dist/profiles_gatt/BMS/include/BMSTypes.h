/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< bmstypes.h >***********************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BMSTypes - Qualcomm Technologies Bluetooth Bond Management Service Types. */
/*                                                                            */
/*  Author:  Michael Rougeux                                                  */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/05/15  M. Rougeux     Initial creation.                               */
/******************************************************************************/
#ifndef __BMSTYPESH__
#define __BMSTYPESH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.   */
#include "BTPSKRNL.h"     /* BTPS Kernel Prototypes/Constants.                */

   /* The following defines the attribute protocol error codes.  These  */
   /* error codes may be set for the ResponseCode field of the          */
   /* BMS_BM_Control_Point_Response().                                  */
#define BMS_ERROR_CODE_SUCCESS                                 0x00
#define BMS_ERROR_CODE_OPCODE_NOT_SUPPORTED                    0x80
#define BMS_ERROR_CODE_OPERATION_FAILED                        0x81

   /* The following MACRO is a utility MACRO that assigns the BMS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable. This     */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the BMS UUID Constant value.          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define BMS_ASSIGN_BMS_SERVICE_UUID_16(_x)                     ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x1E)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined BMS Service UUID in UUID16 form. This      */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* BMS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than. The first parameter is the UUID_16_t variable  */
   /* to compare to the BMS Service UUID.                               */
#define BMS_COMPARE_BMS_SERVICE_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x1E)

   /* The following defines the BMS Service UUID that is used when      */
   /* building the BMS Service Table.                                   */
#define BMS_SERVICE_UUID_CONSTANT                              { 0x1E, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the BMS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the BMS UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define BMS_ASSIGN_BMS_SERVICE_SDP_UUID_16(_x)                 ASSIGN_SDP_UUID_16((_x), 0x18, 0x1E)

   /* The following MACRO is a utility MACRO that assigns the BM        */
   /* Control point Characteristic 16 bit UUID to the specified         */
   /* UUID_16_t variable. This MACRO accepts one parameter which is the */
   /* UUID_16_t variable that is to receive the BMS BM CONTROL POINT    */
   /* UUID Constant Value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define BMS_ASSIGN_BM_CONTROL_POINT_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xA4)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined BM Control Point UUID in UUID16 form. This */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* BM Control Point UUID (MACRO returns boolean result) NOT less     */
   /* than/greater than. The first parameter is the UUID_16_t variable  */
   /* to compare to the BM Control Point UUID.                          */
#define BMS_COMPARE_BM_CONTROL_POINT_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xA4)

   /* The following defines the BM Control Point Characteristic UUID    */
   /* that is used when building the BMS Service Table.                 */
#define BMS_BM_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT      { 0xA4, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the BMS       */
   /* Feature Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable. This MACRO accepts one parameter which is the UUID_16_t */
   /* variable that is to receive the BMS feature UUID Constant value.  */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define BMS_ASSIGN_BM_FEATURE_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xA5)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined BMS BM Feature UUID in UUID16 form. This   */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* BMS BM Feature UUID (MACRO returns boolean result) NOT less       */
   /* than/greater than. The first parameter is the UUID_16_t variable  */
   /* to compare to the BMS Feature UUID.                               */
#define BMS_COMPARE_BM_FEATURE_UUID_TO_UUID_16(_x)             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xA5)

   /* The following defines the BMS BM Feature Characteristic UUID that */
   /* is used when building the BMS Service Table.                      */
#define BMS_BM_FEATURE_CHARACTERISTIC_UUID_CONSTANT            { 0xA5, 0x2A }

   /* Defines the maximum length of a BMS Authorization Code per BMS    */
   /* Bluetooth service specification.                                  */
#define BMS_MAXIMUM_AUTHORIZATION_CODE_LENGTH                  511

   /* The following defines the values that may be used as the Flags of */
   /* a BM Feature Characteristic bitmask.                              */
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE         0x00000001
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE_AUTH    0x00000002
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR            0x00000004
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_AUTH       0x00000008
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_LE               0x00000010
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_LE_AUTH          0x00000020
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_LE               0x00000040
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_LE_AUTH          0x00000080
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR                  0x00000100
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_AUTH             0x00000200
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_LE                     0x00000400
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_LE_AUTH                0x00000800
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_LE             0x00001000
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_LE_AUTH        0x00002000
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR                0x00004000
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_AUTH           0x00008000
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_LE                   0x00010000
#define BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_LE_AUTH              0x00020000

#define DEFAULT_BM_LE_FEATURES_BIT_MASK         ((BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_LE) | (BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_LE_AUTH) | \
                                                 (BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_LE)       | (BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_LE_AUTH)       | \
                                                 (BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_LE)     | (BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_LE_AUTH))

#define DEFAULT_BM_BR_EDR_FEATURES_BIT_MASK     ((BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR) | (BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_AUTH) | \
                                                 (BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR)       | (BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_AUTH)       | \
                                                 (BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR)     | (BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_AUTH))

#define DEFAULT_BM_DUAL_MODE_FEATURES_BIT_MASK  ((BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE) | (BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE_AUTH) | \
                                                 (BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_LE)       | (BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_LE_AUTH)       | \
                                                 (BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_LE)     | (BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_LE_AUTH))

   /* The following defines the values that may be set for the          */
   /* CommandType field of BM Control Point Format Data.                */
#define BMS_BM_CONTROL_POINT_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE        0x01
#define BMS_BM_CONTROL_POINT_DELETE_BOND_REQUESTING_DEVICE_BREDR           0x02
#define BMS_BM_CONTROL_POINT_DELETE_BOND_REQUESTING_DEVICE_LE              0x03
#define BMS_BM_CONTROL_POINT_DELETE_BOND_ALL_DEVICES_BREDR_LE              0x04
#define BMS_BM_CONTROL_POINT_DELETE_BOND_ALL_DEVICES_BREDR                 0x05
#define BMS_BM_CONTROL_POINT_DELETE_BOND_ALL_DEVICES_LE                    0x06
#define BMS_BM_CONTROL_POINT_DELETE_BOND_OTHER_DEVICES_BREDR_LE            0x07
#define BMS_BM_CONTROL_POINT_DELETE_BOND_OTHER_DEVICES_BREDR               0x08
#define BMS_BM_CONTROL_POINT_DELETE_BOND_OTHER_DEVICES_LE                  0x09

   /* The following structure defines the format of the                 */
   /* BM_Control_Point structure. This structure will be used for both  */
   /* Control Point request and response purposes.                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBMS_BM_Control_Point_t
{
   NonAlignedByte_t Op_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ BMS_BM_Control_Point_t;

#define BMS_BM_CONTROL_POINT_SIZE(_x)                          (BTPS_STRUCTURE_OFFSET(BMS_BM_Control_Point_t, Variable_Data) + _x)

#define BMS_ATT_MTU_SIZE                                       28

   /* The following defines the BMS GATT Service Flags MASK that should */
   /* be passed into GATT_Register_Service when the BM Service is       */
   /* registered.                                                       */
#define BMS_SERVICE_FLAGS_LE                                  (GATT_SERVICE_FLAGS_LE_SERVICE)
#define BMS_SERVICE_FLAGS_BR_EDR                              (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define BMS_SERVICE_FLAGS_DUAL_MODE                           (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
