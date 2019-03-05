/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< wsstypes.h >***********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  WSSTypes - Qualcomm Technologies Bluetooth Stack Weight Scale Service     */
/*             Types.                                                         */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/25/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __WSSTYPEH__
#define __WSSTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.          */
#include "GATTType.h"           /* Bluetooth GATT Type Definitions.     */

   /* The following defines the attribute protocol Application error    */
   /* codes.                                                            */
   /* * NOTE * WSS_ERROR_CODE_SUCCESS should be used for all response   */
   /*          API's to indicate a successful response.                 */
#define WSS_ERROR_CODE_SUCCESS                                 0x00

   /* The following defines common attribute protocol error codes use by*/
   /* this service that may be needed by the Application.               */
#define WSS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED             (ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)
#define WSS_ERROR_CODE_UNLIKELY_ERROR                         (ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR)

   /* The following MACRO is a utility MACRO that assigns the Immediate */
   /* Alert Service 16 bit UUID to the specified UUID_16_t              */
   /* variable.  This MACRO accepts one parameter which is a pointer to */
   /* a UUID_16_t variable that is to receive the WSS UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define WSS_ASSIGN_WSS_SERVICE_UUID_16(_x)                     ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x1D)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined WSS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* WSS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the WSS Service UUID.                               */
#define WSS_COMPARE_WSS_SERVICE_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x1D)

   /* The following MACRO is a utility MACRO that assigns the WSS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the WSS UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define WSS_ASSIGN_WSS_SERVICE_SDP_UUID_16(_x)                 ASSIGN_SDP_UUID_16((_x), 0x18, 0x1D)

   /* The following defines the WSS Parameter Service UUID that is      */
   /* used when building the WSS Service Table.                         */
#define WSS_SERVICE_BLUETOOTH_UUID_CONSTANT                    { 0x1D, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the WSS       */
   /* Feature Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the WSS Feature UUID Constant value.  */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define WSS_ASSIGN_WEIGHT_SCALE_FEATURE_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x9E)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined WSS Feature UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* WSS Feature UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the WSS Feature UUID.                               */
#define WSS_COMPARE_WEIGHT_SCALE_FEATURE_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x9E)

   /* The following defines the WSS Feature Characteristic UUID that is */
   /* used when building the WSS Service Table.                         */
#define WSS_FEATURE_CHARACTERISTIC_UUID_CONSTANT               { 0x9E, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the WSS       */
   /* Measurement Characteristic 16 bit UUID to the specified UUID_16_t */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the WSS Measurement UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define WSS_ASSIGN_WEIGHT_MEASUREMENT_UUID_16(_x)              ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x9D)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined WSS Measurement UUID in UUID16 form.  This */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* WSS Measurement UUID (MACRO returns boolean result) NOT less      */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the WSS Measurement UUID.                           */
#define WSS_COMPARE_WEIGHT_MEASUREMENT_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x9D)

   /* The following defines the WSS Measurement Characteristic UUID that*/
   /* is used when building the WSS Service Table.                      */
#define WSS_WEIGHT_MEASUREMENT_CHARACTERISTIC_UUID_CONSTANT    { 0x9D, 0x2A }

   /* The following defines the valid values of WSS Weight Scale Feature*/
   /* bits that may be set in the Feature field of WSS Feature          */
   /* characteristic.                                                   */
   /* ** NOTE ** KG and LB in Macros are (kg) and (lb) units.  POINT    */
   /*            indicates a decimal placeholder.                       */
   /* ** NOTE ** M and INCH in Macros are (m) and (inch) units.  POINT  */
   /*            indicates a decimal placeholder.                       */
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_TIME_STAMP_SUPPORTED                       0x00000001
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_MULTIPLE_USERS_SUPPORTED                   0x00000002
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_BMI_SUPPORTED                              0x00000004

#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_NOT_SPECIFIED                0x00000000
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_POINT_5_KG_OR_1_LB           0x00000080
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_POINT_2_KG_OR_POINT_5_LB     0x00000100
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_POINT_1_KG_OR_POINT_2_LB     0x00000180
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_POINT_05_KG_OR_POINT_1_LB    0x00000200
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_POINT_02_KG_OR_POINT_05_LB   0x00000280
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_POINT_01_KG_OR_POINT_02_LB   0x00000400
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_WM_RESOLUTION_POINT_005_KG_OR_POINT_01_LB  0x00000480

#define WSS_WEIGHT_SCALE_FEATURE_FLAG_HM_RESOLUTION_NOT_SPECIFIED                0x00000000
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_HM_RESOLUTION_POINT_01_M_OR_1_INCH         0x00080000
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_HM_RESOLUTION_POINT_005_M_OR_POINT_5_INCH  0x00100000
#define WSS_WEIGHT_SCALE_FEATURE_FLAG_HM_RESOLUTION_POINT_0001_M_OR_POINT_1_INCH 0x00180000

   /* The following defines the valid WSS Measurement Flags bits that   */
   /* may be set in the Flags field of a WSS Measurement characteristic.*/
#define WSS_WEIGHT_MEASUREMENT_FLAG_MEASUREMENT_UNITS_IMPERIAL  0x01
#define WSS_WEIGHT_MEASUREMENT_FLAG_TIME_STAMP_PRESENT          0x02
#define WSS_WEIGHT_MEASUREMENT_FLAG_USER_ID_PRESENT             0x04
#define WSS_WEIGHT_MEASUREMENT_FLAG_BMI_HEIGHT_PRESENT          0x08
#define WSS_WEIGHT_MEASUREMENT_FLAG_RESERVED                    0xF0

   /* The following structure defines the format of a WSS Measurement   */
   /* value that must always be specified in the WSS Measurement        */
   /* characteristic value.                                             */
typedef __PACKED_STRUCT_BEGIN__ struct _tagWSS_Weight_Measurement_t
{
   NonAlignedByte_t Flags;
   NonAlignedWord_t Weight;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ WSS_Weight_Measurement_t;

#define WSS_WEIGHT_MEASUREMENT_SIZE(_x)                        (BTPS_STRUCTURE_OFFSET(WSS_Weight_Measurement_t, Variable_Data) + (_x))

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the minimum size of a WSS Weight Measurement value    */
   /* based on the WSS Weight Measurement Flags.  The only parameter to */
   /* this MACRO is the WSS Weight Measurement Flags.                   */
   /* * NOTE * We combine the sizes for BMI and Height to get size of   */
   /*          DWORD.                                                   */
#define WSS_WEIGHT_MEASUREMENT_MINIMUM_LENGTH(_x)              (WSS_WEIGHT_MEASUREMENT_SIZE(0)                                                                            + \
                                                               (((_x) & WSS_WEIGHT_MEASUREMENT_FLAG_TIME_STAMP_PRESENT)   ? GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE : 0)  + \
                                                               (((_x) & WSS_WEIGHT_MEASUREMENT_FLAG_USER_ID_PRESENT)      ? NON_ALIGNED_BYTE_SIZE : 0)                    + \
                                                               (((_x) & WSS_WEIGHT_MEASUREMENT_FLAG_BMI_HEIGHT_PRESENT)   ? NON_ALIGNED_DWORD_SIZE : 0))

    /* The following definition is set for a WSS Measurement if it is   */
    /* unsuccessful.                                                    */
#define WSS_WEIGHT_MEASUREMENT_UNSUCCESSFUL                    (0xFFFF)

    /* The following definition is set for a WSS User ID that is        */
    /* unknown.                                                         */
#define WSS_USER_ID_UNKNOWN                                    (0xFF)

  /* The following defines the valid values (bit mask) that may be set  */
  /* for Weight Measurement Client Characteristic Configuration         */
  /* Descriptor (CCCD).                                                 */
  /* * NOTE * The bit value                                             */
  /*          HPS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE is  */
  /*          NOT VALID for the CCCD, however it has been included for  */
  /*          reference.                                                */
#define WSS_CLIENT_CHARACTERISTIC_CONFIGURATION_DISABLED         (0)
#define WSS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE    (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
#define WSS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE  (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)

   /* The following defines the WSS GATT Service Flags MASK that should */
   /* be passed into GATT_Register_Service when the WSS Service is      */
   /* registered.                                                       */
#define WSS_SERVICE_FLAGS_LE                                   (GATT_SERVICE_FLAGS_LE_SERVICE)
#define WSS_SERVICE_FLAGS_BR_EDR                               (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define WSS_SERVICE_FLAGS_DUAL_MODE                            (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
