/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< trdstypes.h >**********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TRDSTypes - Qualcomm Technologies Bluetooth Transport Discovery Service   */
/*              Types.                                                        */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/26/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __TRDSTYPEH__
#define __TRDSTYPEH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.   */
#include "BTPSKRNL.h"     /* BTPS Kernel Prototypes/Constants.                */

   /* The following define the defined TDS Error Codes that may be sent */
   /* in a GATT Error Response.                                         */
#define TRDS_ERROR_CODE_SUCCESS                          0x00

   /* The following defines common attribute protocol error codes use by*/
   /* this service that may be needed by the Application.               */
#define TRDS_ERROR_CODE_INSUFFICIENT_AUTHENTICATION      (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION)
#define TRDS_ERROR_CODE_INSUFFICIENT_AUTHORIZATION       (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION)
#define TRDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION          (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION)
#define TRDS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED       (ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)
#define TRDS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS    (ATT_PROTOCOL_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)
#define TRDS_ERROR_CODE_UNLIKELY_ERROR                   (ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR)

   /* The following MACRO is a utility MACRO that assigns the Automation*/
   /* IO Service 16 bit UUID to the specified UUID_16_t variable.  This */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the TDS UUID Constant value.          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TRDS_ASSIGN_TRDS_SERVICE_UUID_16(_x)             ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x24)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TDS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* TDS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the TDS Service UUID.                               */
#define TRDS_COMPARE_TRDS_SERVICE_UUID_TO_UUID_16(_x)    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x24)

   /* The following defines the TDS UUID that is used when building the */
   /* TDS Service Table.                                                */
#define TRDS_SERVICE_BLUETOOTH_UUID_CONSTANT             { 0x24, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the TDS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the TDS UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TRDS_ASSIGN_TRDS_SERVICE_SDP_UUID_16(_x)         ASSIGN_SDP_UUID_16((_x), 0x18, 0x24)

   /* The following MACRO is a utility MACRO that assigns the TDS       */
   /* Control Point Characteristic 16 bit UUID to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the TDS Control Point       */
   /* Characteristic UUID Constant value.                               */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TRDS_ASSIGN_CONTROL_POINT_CHARACTERISTIC_UUID_16(_x)           ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xBC)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TDS Control Point Characteristic UUID in   */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the Control Point Characteristic UUID (MACRO */
   /* returns boolean result) NOT less than/greater than.  The first    */
   /* parameter is the UUID_16_t variable to compare to the TDS Control */
   /* Point Characteristic UUID.                                        */
#define TRDS_COMPARE_CONTROL_POINT_CHARACTERISTIC_UUID_TO_UUID_16(_x)  COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xBC)

   /* The following defines the TDS Control Point Characteristic UUID   */
   /* that is used when building the TDS Service Table.                 */
#define TRDS_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT      { 0xBC, 0x2A }

   /* The following defines the valid value for the TDS Transport       */
   /* Discovery Data AD Type Code.                                      */
#define TRDS_TRANSPORT_DISCOVERY_DATA_AD_TYPE_CODE       0x26

   /* The following defines the valid values that may be set for the    */
   /* Organization ID field of the Transpot Block.                      */
   /* * NOTE * Please refer to Bluetooth SIG Assigned Numbers for a     */
   /*          complete list of assigned numbers.                       */
#define TRDS_ORGANIZATION_ID_BT_SIG                      0x01

   /* The following structure represents the TDS Transport Block.       */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTRDS_Transport_Block_t
{
   NonAlignedByte_t Organization_ID;
   NonAlignedByte_t Flags;
   NonAlignedByte_t Transport_Data_Length;
   NonAlignedByte_t Transport_Data[1];
} __PACKED_STRUCT_END__ TRDS_Transport_Block_t;

#define TRDS_TRANSPORT_BLOCK_SIZE(_x)                    (BTPS_STRUCTURE_OFFSET(TRDS_Transport_Block_t, Transport_Data) + (_x))

   /* The following defines the valid values that may be set for the Op */
   /* Code field of the TDS Control Point.                              */
#define TRDS_CONTROL_POINT_OP_CODE_ACTIVATE_TRANSPORT     0x01

   /* The following structure represents the TDS Control Point          */
   /* Characteristic (Request).                                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTRDS_Control_Point_Request_t
{
   NonAlignedByte_t Op_Code;
   NonAlignedByte_t Organization_ID;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ TRDS_Control_Point_Request_t;

#define TRDS_CONTROL_POINT_REQUEST_SIZE(_x)              (BTPS_STRUCTURE_OFFSET(TRDS_Control_Point_Request_t, Variable_Data) + (_x))

   /* The following defines the valid values that may be set for the    */
   /* Result Code field of the TDS Control Point.                       */
#define TRDS_CONTROL_POINT_RESULT_CODE_SUCCESS                     0x00
#define TRDS_CONTROL_POINT_RESULT_CODE_OP_CODE_NOT_SUPPORTED       0x01
#define TRDS_CONTROL_POINT_RESULT_CODE_INVALID_PARAMETER           0x02
#define TRDS_CONTROL_POINT_RESULT_CODE_UNSUPPORTED_ORGANIZATION_ID 0x03
#define TRDS_CONTROL_POINT_RESULT_CODE_OPERATION_FAILED            0x04

   /* The following structure represents the TDS Control Point          */
   /* Characteristic (Response).                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTRDS_Control_Point_Response_t
{
   NonAlignedByte_t Request_Op_Code;
   NonAlignedByte_t Result_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ TRDS_Control_Point_Response_t;

#define TRDS_CONTROL_POINT_RESPONSE_SIZE(_x)             (BTPS_STRUCTURE_OFFSET(TRDS_Control_Point_Response_t, Variable_Data) + (_x))

  /* The following defines the valid values (bit mask) that may be set  */
  /* for Client Characteristic Configuration Descriptors (CCCDs).       */
  /* * NOTE * Both cannot be set simultaneously.                        */
#define TRDS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE    (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
#define TRDS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE  (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)

   /* The following defines the TDS GATT Service Flags MASK that should */
   /* be passed into GATT_Register_Service when the TDS Service is      */
   /* registered.                                                       */
#define TRDS_SERVICE_FLAGS_LE                            (GATT_SERVICE_FLAGS_LE_SERVICE)
#define TRDS_SERVICE_FLAGS_BR_EDR                        (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define TRDS_SERVICE_FLAGS_DUAL_MODE                     (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
