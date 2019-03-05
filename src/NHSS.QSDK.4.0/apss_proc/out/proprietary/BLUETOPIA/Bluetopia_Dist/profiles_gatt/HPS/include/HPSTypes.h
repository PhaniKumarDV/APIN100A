/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< hpstypes.h >***********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HPSTypes - Qualcomm Technologies Bluetooth HTTP Proxy Service Types.      */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/17/16  R. McCord      Initial Creation.                               */
/******************************************************************************/
#ifndef __HPSTYPESH__
#define __HPSTYPESH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.   */
#include "BTPSKRNL.h"     /* BTPS Kernel Prototypes/Constants.                */

   /* The following defines the attribute protocol Application error    */
   /* codes.                                                            */
   /* * NOTE * HPS_ERROR_CODE_SUCCESS should be used for all response   */
   /*          API's to indicate a successful response.                 */
#define HPS_ERROR_CODE_SUCCESS                                 0x00
#define HPS_ERROR_CODE_INVALID_REQUEST                         0x81
#define HPS_ERROR_CODE_NETWORK_NOT_AVAILABLE                   0x82

   /* The following defines common attribute protocol error codes use by*/
   /* this service that may be needed by the Application.               */
#define HPS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED             (ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)
#define HPS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS          (ATT_PROTOCOL_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)
#define HPS_ERROR_CODE_UNLIKELY_ERROR                         (ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR)

   /* The following MACRO is a utility MACRO that assigns the HPS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable. This     */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the HPS UUID Constant value.          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define HPS_ASSIGN_HPS_SERVICE_UUID_16(_x)                     ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x23)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined HPS Service UUID in UUID16 form. This      */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* HPS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than. The first parameter is the UUID_16_t variable  */
   /* to compare to the HPS Service UUID.                               */
#define HPS_COMPARE_HPS_SERVICE_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x23)

   /* The following MACRO is a utility MACRO that assigns the HPS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the HPS UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define HPS_ASSIGN_HPS_SERVICE_SDP_UUID_16(_x)                 ASSIGN_SDP_UUID_16((_x), 0x18, 0x23)

   /* The following defines the HPS Service UUID that is used when      */
   /* building the HPS Service Table.                                   */
#define HPS_SERVICE_CHARACTERISTIC_UUID_CONSTANT               { 0x23, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the HPS       */
   /* Uniform Resource Indentifier (URI) Characteristic 16 bit UUID to  */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* HPS Uniform Resource Indentifier (URI) Characteristic UUID        */
   /* Constant Value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define HPS_ASSIGN_URI_UUID_16(_x)                             ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xB6)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined HPS Uniform Resource Indentifier           */
   /* Characteristic UUID in UUID16 form.  This MACRO only returns      */
   /* whether the UUID_16_t variable is equal to the HPS Uniform        */
   /* Resource Indentifier (URI) Characteristic UUID (MACRO returns     */
   /* boolean result) NOT less than/greater than.  The first parameter  */
   /* is the UUID_16_t variable to compare to the HPS Uniform Resource  */
   /* Indentifier (URI) Characteristic UUID.                            */
#define HPS_COMPARE_URI_UUID_TO_UUID_16(_x)                    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xB6)

   /* The following defines the HPS Uniform Resource Indentifier (URI)  */
   /* Characteristic UUID that is used when building the HPS Service    */
   /* Table.                                                            */
#define HPS_URI_CHARACTERISTIC_UUID_CONSTANT                   { 0xB6, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the HPS HTTP  */
   /* Headers Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the HPS HTTP Headers Characteristic   */
   /* UUID Constant Value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define HPS_ASSIGN_HTTP_HEADERS_UUID_16(_x)                    ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xB7)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined HPS HTTP Headers Characteristic UUID in    */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the HPS HTTP Headers Characteristic UUID     */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the HPS   */
   /* HTTP Headers Characteristic UUID.                                 */
#define HPS_COMPARE_HTTP_HEADERS_UUID_TO_UUID_16(_x)           COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xB7)

   /* The following defines the HPS HTTP Headers Characteristic UUID    */
   /* that is used when building the HPS Service Table.                 */
#define HPS_HTTP_HEADERS_CHARACTERISTIC_UUID_CONSTANT          { 0xB7, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the HPS HTTP  */
   /* Entity Body Characteristic 16 bit UUID to the specified UUID_16_t */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the HPS HTTP Entity Body              */
   /* Characteristic UUID Constant Value.                               */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define HPS_ASSIGN_HTTP_ENTITY_BODY_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xB9)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined HPS HTTP Entity Body Characteristic UUID in*/
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the HPS HTTP Entity Body Characteristic UUID */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the HPS   */
   /* HTTP Entity Body Characteristic UUID.                             */
#define HPS_COMPARE_HTTP_ENTITY_BODY_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xB9)

   /* The following defines the HPS HTTP Entity Body Characteristic UUID*/
   /* that is used when building the HPS Service Table.                 */
#define HPS_HTTP_ENTITY_BODY_CHARACTERISTIC_UUID_CONSTANT      { 0xB9, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the HPS HTTP  */
   /* Control point Characteristic 16 bit UUID to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the HPS HTTP Control point  */
   /* Characteristic UUID Constant Value.                               */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define HPS_ASSIGN_HTTP_CONTROL_POINT_UUID_16(_x)              ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xBA)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined HPS HTTP Control point Characteristic UUID */
   /* in UUID16 form.  This MACRO only returns whether the UUID_16_t    */
   /* variable is equal to the HPS HTTP Control point Characteristic    */
   /* UUID (MACRO returns boolean result) NOT less than/greater than.   */
   /* The first parameter is the UUID_16_t variable to compare to the   */
   /* HPS HTTP Control point Characteristic UUID.                       */
#define HPS_COMPARE_HTTP_CONTROL_POINT_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xBA)

   /* The following defines the HPS HTTP Control point Characteristic   */
   /* UUID that is used when building the HPS Service Table.            */
#define HPS_HTTP_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT    { 0xBA, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the HPS HTTP  */
   /* Status Code Characteristic 16 bit UUID to the specified UUID_16_t */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the HPS HTTP Status Code              */
   /* Characteristic UUID Constant Value.                               */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define HPS_ASSIGN_HTTP_STATUS_CODE_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xB8)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined HPS HTTP Status Code Characteristic UUID in*/
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the HPS HTTP Status Code Characteristic UUID */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the HPS   */
   /* HTTP Status Code Characteristic UUID.                             */
#define HPS_COMPARE_HTTP_STATUS_CODE_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xB8)

   /* The following defines the HPS HTTP Status Code Characteristic UUID*/
   /* that is used when building the HPS Service Table.                 */
#define HPS_HTTP_STATUS_CODE_CHARACTERISTIC_UUID_CONSTANT      { 0xB8, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the HPS HTTPS */
   /* Security Characteristic 16 bit UUID to the specified UUID_16_t    */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the HPS HTTPS Security Characteristic */
   /* UUID Constant Value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define HPS_ASSIGN_HTTPS_SECURITY_UUID_16(_x)                  ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xBB)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined HPS HTTPS Security Characteristic UUID in  */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the HPS HTTPS Security Characteristic UUID   */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the HPS   */
   /* HTTPS Security Characteristic UUID.                               */
#define HPS_COMPARE_HTTPS_SECURITY_UUID_TO_UUID_16(_x)         COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xBB)

   /* The following defines the HPS HTTPS Security Characteristic UUID  */
   /* that is used when building the HPS Service Table.                 */
#define HPS_HTTPS_SECURITY_CHARACTERISTIC_UUID_CONSTANT        { 0xBB, 0x2A }

   /* The following defines the valid values that may be set for the    */
   /* Op_Code field of the HPS_HTTP_Control_Point_t structure below.    */
   /* * NOTE * Values (0;12-255) are reserved for future use.           */
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_GET_REQUEST        (1)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_HEAD_REQUEST       (2)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_POST_REQUEST       (3)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_PUT_REQUEST        (4)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_DELETE_REQUEST     (5)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_GET_REQUEST       (6)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_HEAD_REQUEST      (7)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_POST_REQUEST      (8)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_PUT_REQUEST       (9)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_DELETE_REQUEST    (10)
#define HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_CANCEL_REQUEST     (11)

   /* The following defines the valid bit mask values that may be set   */
   /* for the Data_Status field of the HPS_HTTP_Status_Code_t structure */
   /* below.                                                            */
   /* * NOTE * Bits (4-7) are reserved for future use.                  */
#define HPS_DATA_STATUS_HEADER_RECEIVED                        (0x01)
#define HPS_DATA_STATUS_HEADER_TRUNCATED                       (0x02)
#define HPS_DATA_STATUS_BODY_RECEIVED                          (0x04)
#define HPS_DATA_STATUS_BODY_TRUNCATED                         (0x08)

   /* The following defines the format of the HTTP Status Code          */
   /* Characteristic structure.                                         */
   /* * NOTE * The Status_Code field should be set to the Status-Line of*/
   /*          the first line of the HTTP Response Message.             */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHPS_HTTP_Status_Code_t
{
   NonAlignedWord_t  Status_Code;
   NonAlignedByte_t  Data_Status;
} __PACKED_STRUCT_END__ HPS_HTTP_Status_Code_t;

#define HPS_HTTP_STATUS_CODE_SIZE                              (sizeof(HPS_HTTP_Status_Code_t))

  /* The following defines the valid values (bit mask) that may be set  */
  /* for HTTP Control Point Client Characteristic Configuration         */
  /* Descriptor (CCCD).                                                 */
  /* * NOTE * The bit value                                             */
  /*          HPS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE is*/
  /*          NOT VALID for the HTTP Control Point, however it has been */
  /*          included for reference.                                   */
#define HPS_CLIENT_CHARACTERISTIC_CONFIGURATION_DISABLED         (0)
#define HPS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE    (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
#define HPS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE  (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)

   /* The following defines the HPS Service Flags MASK that should be   */
   /* passed into the GATT_Register_Service() function to configure the */
   /* transport the service will use when it is registered with GATT.   */
#define HPS_SERVICE_FLAGS_LE                                   (GATT_SERVICE_FLAGS_LE_SERVICE)
#define HPS_SERVICE_FLAGS_BR_EDR                               (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define HPS_SERVICE_FLAGS_DUAL_MODE                            (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
