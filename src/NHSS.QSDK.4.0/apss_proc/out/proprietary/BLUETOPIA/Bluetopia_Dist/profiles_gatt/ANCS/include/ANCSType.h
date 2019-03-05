/*****< ancstype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANCSTYPE - Stonestreet One Bluetooth Stack Apple Notification Center      */
/*             Service Types.                                                 */
/*                                                                            */
/*  Author:  Matt Buckley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/27/13  M. Buckley     Initial creation.                               */
/******************************************************************************/
#ifndef __ANCSTYPEH__
#define __ANCSTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following constatns define the ANCS error codes.  These may   */
   /* exist in a GATT Error Response.  These error codes are defined in */
   /* the ANCS specification.                                           */
#define ANCS_ERROR_CODE_UNKNOWN_COMMAND                  0xA0
#define ANCS_ERROR_CODE_INVALID_COMMAND                  0xA1
#define ANCS_ERROR_CODE_INVALID_PARAMETER                0xA2

   /* The following MACRO is a utility MACRO that assigns the ANCS      */
   /* Service 128 bit UUID to the specified UUID_128_t variable.  This  */
   /* MACRO accepts one parameter which is a pointer to a UUID_128_t    */
   /* variable that is to receive the ANCS UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define ANCS_ASSIGN_ANCS_SERVICE_UUID_128(_x)            ASSIGN_BLUETOOTH_UUID_128(*((UUID_128_t *)(_x)), 0x79, 0x05, 0xF4, 0x31, 0xB5, 0xCE, 0x4E, 0x99, 0xA4, 0x0F, 0x4B, 0x1E, 0x12, 0x2D, 0x00, 0xD0)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 128 to the defined ANCS Service UUID in UUID16 form.  This   */
   /* MACRO only returns whether the UUID_128_t variable is equal to the*/
   /* ANCS Service UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_128_t variable*/
   /* to compare to the ANCS Service UUID.                              */
#define ANCS_COMPARE_ANCS_SERVICE_UUID_TO_UUID_128(_x)   COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x79, 0x05, 0xF4, 0x31, 0xB5, 0xCE, 0x4E, 0x99, 0xA4, 0x0F, 0x4B, 0x1E, 0x12, 0x2D, 0x00, 0xD0)

   /* The following defines the ANCS Service UUID that is used when     */
   /* building the ANCS Service Table.                                  */
#define ANCS_SERVICE_BLUETOOTH_UUID_CONSTANT             { 0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79 }

   /* The following MACRO is a utility MACRO that assigns the ANCS      */
   /* Notification Source Characteristic 128 bit UUID to the specified  */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the ANCS Notification  */
   /* Source UUID Constant value.                                       */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define ANCS_ASSIGN_NOTIFICATION_SOURCE_UUID_128(_x)     ASSIGN_BLUETOOTH_UUID_128((_x), 0x9F, 0xBF, 0x12, 0x0D, 0x63, 0x01, 0x42, 0xD9, 0x8C, 0x58, 0x25, 0xE6, 0x99, 0xA2, 0x1D, 0xBD )

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 128 to the defined ANCS Notification Source UUID in UUID128  */
   /* form.  This MACRO only returns whether the UUID_128_t variable is */
   /* equal to the Notification Source UUID (MACRO returns boolean      */
   /* result) NOT less than/greater than.                               */
   /* The first parameter is the UUID_128_t variable to compare to the  */
   /* ANCS Notification Source UUID.                                    */
#define ANCS_COMPARE_NOTIFICATION_SOURCE_UUID_TO_UUID_128(_x)  COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x9F, 0xBF, 0x12, 0x0D, 0x63, 0x01, 0x42, 0xD9, 0x8C, 0x58, 0x25, 0xE6, 0x99, 0xA2, 0x1D, 0xBD )

   /* The following defines the ANCS Notification Source Characteristic */
   /* UUID that is used when building the ANCS Service Table.           */
#define ANCS_NOTIFICATION_SOURCE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0x8C, 0xD9, 0x42, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F }

   /* The following MACRO is a utility MACRO that assigns the ANCS      */
   /* Control Point Characteristic 128 bit UUID to the specified        */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the ANCS Control Point */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define ANCS_ASSIGN_CONTROL_POINT_UUID_128(_x)           ASSIGN_BLUETOOTH_UUID_128((_x), 0x69, 0xD1, 0xD8, 0xF3, 0x45, 0xE1, 0x49, 0xA8, 0x98, 0x21, 0x9B, 0xBD, 0xFD, 0xAA, 0xD9, 0xD9 )

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 128 to the defined ANCS Control Point UUID in UUID128        */
   /* form.  This MACRO only returns whether the UUID_128_t variable is */
   /* equal to the Control Point UUID (MACRO returns boolean result)    */
   /* NOT less than/greater than.                                       */
   /* The first parameter is the UUID_128_t variable to compare to the  */
   /* ANCS Control Point UUID.                                          */
#define ANCS_COMPARE_CONTROL_POINT_UUID_TO_UUID_128(_x)  COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x69, 0xD1, 0xD8, 0xF3, 0x45, 0xE1, 0x49, 0xA8, 0x98, 0x21, 0x9B, 0xBD, 0xFD, 0xAA, 0xD9, 0xD9 )

   /* The following defines the ANCS Control Point Characteristic UUID  */
   /* that is used when building the ANCS Service Table.                */
#define ANCS_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xD9, 0xD9, 0xAA, 0xFD, 0xBD, 0x9B, 0x21, 0x98, 0xA8, 0x49, 0xE1, 0x45, 0xF3, 0xD8, 0xD1, 0x69 }

   /* The following MACRO is a utility MACRO that assigns the ANCS      */
   /* Data Source Characteristic 128 bit UUID to the specified          */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the ANCS Data Source   */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define ANCS_ASSIGN_DATA_SOURCE_UUID_128(_x)             ASSIGN_BLUETOOTH_UUID_128((_x), 0x22, 0xEA, 0xC6, 0xE9, 0x24, 0xD6, 0x4B, 0xB5, 0xBE, 0x44, 0xB3, 0x6A, 0xCE, 0x7C, 0x7B, 0xFB )

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 128 to the defined ANCS Data Source UUID in UUID128 form.    */
   /* This MACRO only returns whether the UUID_128_t variable is equal  */
   /* to the Data Source UUID (MACRO returns boolean result) NOT less   */
   /* than/greater than.                                                */
   /* The first parameter is the UUID_128_t variable to compare to the  */
   /* ANCS Data Source UUID.                                            */
#define ANCS_COMPARE_DATA_SOURCE_UUID_TO_UUID_128(_x)    COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x22, 0xEA, 0xC6, 0xE9, 0x24, 0xD6, 0x4B, 0xB5, 0xBE, 0x44, 0xB3, 0x6A, 0xCE, 0x7C, 0x7B, 0xFB )

   /* The following defines the ANCS Data Source Characteristic UUID    */
   /* that is used when building the ANCS Service Table.                */
#define ANCS_DATA_SOURCE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xFB, 0x7B, 0x7C, 0xCE, 0x6A, 0xB3, 0x44, 0xBE, 0xB5, 0x4B, 0xD6, 0x24, 0xE9, 0xC6, 0xEA, 0x22 }

   /* The following two constants represent the different Event Flags   */
   /* that can be used.                                                 */
#define ANCS_EVENT_FLAG_SILENT                           0x00000001
#define ANCS_EVENT_FLAG_IMPORTANT                        0x00000002

   /* The following constants represent the different Category IDs that */
   /* can be used.                                                      */
#define ANCS_CATEGORY_ID_OTHER                           0
#define ANCS_CATEGORY_ID_INCOMING_CALL                   1
#define ANCS_CATEGORY_ID_MISSED_CALL                     2
#define ANCS_CATEGORY_ID_VOICEMAIL                       3
#define ANCS_CATEGORY_ID_SOCIAL                          4
#define ANCS_CATEGORY_ID_SCHEDULE                        5
#define ANCS_CATEGORY_ID_EMAIL                           6
#define ANCS_CATEGORY_ID_NEWS                            7
#define ANCS_CATEGORY_ID_HEALTH_AND_FITNESS              8
#define ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE            9
#define ANCS_CATEGORY_ID_LOCATION                        10
#define ANCS_CATEGORY_ID_ENTERTAINMENT                   11

   /* The following constants represent the different Event IDs that    */
   /* can be used.                                                      */
#define ANCS_EVENT_ID_NOTIFICATION_ADDED                 0
#define ANCS_EVENT_ID_NOTIFICATION_MODIFIED              1
#define ANCS_EVENT_ID_NOTIFICATION_REMOVED               2

   /* The following constants represent the different Command IDs that  */
   /* can be used.                                                      */
#define ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES      0
#define ANCS_COMMAND_ID_GET_APP_ATTRIBUTES               1

   /* The following constants represent the different Notification      */
   /* Attribute IDs that can be used.                                   */
#define ANCS_NOTIFICATION_ATTRIBUTE_ID_APP_IDENTIFIER    0
#define ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE             1
#define ANCS_NOTIFICATION_ATTRIBUTE_ID_SUBTITLE          2
#define ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE           3
#define ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE_SIZE      4
#define ANCS_NOTIFICATION_ATTRIBUTE_ID_DATE              5

   /* The following constants represent the different App Attribute IDs */
   /* that can be used.                                                 */
#define ANCS_APP_ATTRIBUTE_ID_DISPLAY_NAME               0

   /* The following packed structure contains the format in which an    */
   /* ANCS notification will be received over the Notification Source   */
   /* characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANCS_Notification_Packet_t
{
   NonAlignedByte_t  EventID;
   NonAlignedByte_t  EventFlags;
   NonAlignedByte_t  CategoryID;
   NonAlignedByte_t  CategoryCount;
   NonAlignedDWord_t NotificationUID;
} __PACKED_STRUCT_END__ ANCS_Notification_Packet_t;

#define ANCS_NOTIFICATION_PACKET_SIZE                    (sizeof(ANCS_Notification_Packet_t))

   /* The following packed structure describes the format by which a    */
   /* Get Notification Attribute command must be written to the Control */
   /* Point characteristic.                                             */
   /* Note: The format for the Attribute Request Data is specified in   */
   /* the ANCS Service Specification.                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANCS_Notification_Attribute_Request_Packet_t
{
   NonAlignedByte_t  CommandID;
   NonAlignedDWord_t NotificationUID;
   NonAlignedByte_t  AttributeRequestData[1];
} __PACKED_STRUCT_END__ ANCS_Notification_Attribute_Request_Packet_t;

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of an ANCS Notification Attribute Request  */
   /* Packet based on the length of its Attribute Request Data.  The    */
   /* first parameter to this MACRO is the length of the member         */
   /* AttributeRequestData in bytes.                                    */
#define ANCS_NOTIFICATION_ATTRIBUTE_REQUEST_PACKET_SIZE(_x)       (BTPS_STRUCTURE_OFFSET(ANCS_Notification_Attribute_Request_Packet_t, AttributeRequestData) + ((_x)*BYTE_SIZE))

   /* The following packed structure describes the format in which a    */
   /* response to a Get Notification Attribute command will be received */
   /* over the Data Source characteristic.                              */
   /* Note: The format for the Attribute Response Data is specified in  */
   /* the ANCS Service Specification.                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANCS_Notification_Attribute_Response_Packet_t
{
   NonAlignedByte_t  CommandID;
   NonAlignedDWord_t NotificationUID;
   NonAlignedByte_t  AttributeResponseData[1];
} __PACKED_STRUCT_END__ ANCS_Notification_Attribute_Response_Packet_t;

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of an ANCS Notification Attribute Response */
   /* Packet based on the length of its Attribute Response Data.  The   */
   /* first parameter to this MACRO is the length of the member         */
   /* AttributeResponseData in bytes.                                   */
#define ANCS_NOTIFICATION_ATTRIBUTE_RESPONSE_PACKET_SIZE(_x)      (BTPS_STRUCTURE_OFFSET(ANCS_Notification_Attribute_Response_Packet_t, AttributeResponseData) + ((_x)*BYTE_SIZE))

   /* The following packed structure describes the format by which a    */
   /* Get App Attribute command must be written to the Control Point    */
   /* characteristic.                                                   */
   /* Note: The format for the Attribute Request Data is specified in   */
   /* the ANCS Service Specification.                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANCS_App_Attribute_Request_Packet_t
{
   NonAlignedByte_t CommandID;
   NonAlignedByte_t AppIdAndAttributeData[1];
} __PACKED_STRUCT_END__ ANCS_App_Attribute_Request_Packet_t;

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of an ANCS App Attribute Request Packet    */
   /* based on the length of its App Identifier and Attribute Request   */
   /* Data.  The first parameter to this MACRO is the length of the     */
   /* member AppIdentifier in bytes.  The second parameter to this      */
   /* MACRO is the length of the member AttributeRequestData in bytes.  */
#define ANCS_APP_ATTRIBUTE_REQUEST_PACKET_SIZE(_x, _y)   (BTPS_STRUCTURE_OFFSET(ANCS_App_Attribute_Request_Packet_t, AppIdAndAttributeData) + ((_x)*BYTE_SIZE) + ((_y)*BYTE_SIZE))

   /* The following packed structure describes the format in which a    */
   /* response to a Get App Attribute command will be received over the */
   /* Data Source characteristic.                                       */
   /* Note: The format for the Attribute Response Data is specified in  */
   /* the ANCS Service Specification.                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANCS_App_Attribute_Response_Packet_t
{
   NonAlignedByte_t CommandID;
   NonAlignedByte_t AppIdAndAttributeResponseData[1];
} __PACKED_STRUCT_END__ ANCS_App_Attribute_Response_Packet_t;

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of an ANCS App Attribute Response Packet   */
   /* based on the length of its App Identifier and Attribute Response  */
   /* Data.  The first parameter to this MACRO is the length of the     */
   /* member AppIdentifier in bytes.  The second parameter to this      */
   /* MACRO is the length of the member AttributeResponseData in bytes. */
#define ANCS_APP_ATTRIBUTE_RESPONSE_PACKET_SIZE(_x, _y)  (BTPS_STRUCTURE_OFFSET(ANCS_App_Attribute_Response_Packet_t, AppIdAndAttributeResponseData) + ((_x)*BYTE_SIZE) + ((_y)*BYTE_SIZE))

#endif
