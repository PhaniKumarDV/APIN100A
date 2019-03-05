/*****< MCAPType.h >***********************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MCAPTYPE - Bluetooth Multi-Channel Adaptation Protocol (MCAP)             */
/*             Type Definitions/Constants.                                    */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/09/09  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __MCAPTYPEH__
#define __MCAPTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following defines the Minimum value of a Dynamically allocated*/
   /* Data Link ID.                                                     */
#define MIN_DATA_LINK_DYNAMIC_ID_VALUE                            0x0001

   /* The following defines the Maximum value of a Dynamically allocated*/
   /* Data Link ID.                                                     */
#define MAX_DATA_LINK_DYNAMIC_ID_VALUE                            0xFEFF

   /* The following defines the Data Link ID value that represents all  */
   /* data links.  This ID is to be used with the Delete Data Link      */
   /* request command only.                                             */
#define DATA_LINK_ID_ALL_DATA_LINKS                               0xFFFF

   /* The following MACRO is used to test a supplied Data Link ID to    */
   /* ensure that it is within the valid range.                         */
#define VALID_DATA_LINK_DYNAMIC_ID_ID_VALUE(_x)                   ((_x) && ((_x) <= MAX_DATA_LINK_DYNAMIC_ID_VALUE))

   /* The following defines the Maximum value of a Data Endpoint ID.    */
#define MAX_DATA_ENDPOINT_ID_VALUE                                  0x7F

#define VALID_MDEP_ID(_x)                                         (((unsigned)(_x) <= 0x7F))

   /* The following define the Request/Response OpCodes that are defined*/
   /* for this protocol.                                                */
#define MCAP_OPCODE_ERROR_RESPONSE                              (0x00)
#define MCAP_OPCODE_CREATE_DATA_LINK_REQUEST                    (0x01)
#define MCAP_OPCODE_CREATE_DATA_LINK_RESPONSE                   (0x02)
#define MCAP_OPCODE_RECONNECT_DATA_LINK_REQUEST                 (0x03)
#define MCAP_OPCODE_RECONNECT_DATA_LINK_RESPONSE                (0x04)
#define MCAP_OPCODE_ABORT_DATA_LINK_REQUEST                     (0x05)
#define MCAP_OPCODE_ABORT_DATA_LINK_RESPONSE                    (0x06)
#define MCAP_OPCODE_DELETE_DATA_LINK_REQUEST                    (0x07)
#define MCAP_OPCODE_DELETE_DATA_LINK_RESPONSE                   (0x08)
#define MCAP_OPCODE_CLOCK_SYNC_CAPABILITIES_REQUEST             (0x11)
#define MCAP_OPCODE_CLOCK_SYNC_CAPABILITIES_RESPONSE            (0x12)
#define MCAP_OPCODE_CLOCK_SYNC_SET_REQUEST                      (0x13)
#define MCAP_OPCODE_CLOCK_SYNC_SET_RESPONSE                     (0x14)
#define MCAP_OPCODE_CLOCK_SYNC_INFO_INDICATION                  (0x15)

   /* The following defines the response codes that are defined for this*/
   /* protocol.                                                         */
#define MCAP_RESPONSE_CODE_SUCCESS                              (0x00)
#define MCAP_RESPONSE_CODE_INVALID_OPCODE                       (0x01)
#define MCAP_RESPONSE_CODE_INVALID_PARAMETER_VALUE              (0x02)
#define MCAP_RESPONSE_CODE_INVALID_DATA_ENDPOINT                (0x03)
#define MCAP_RESPONSE_CODE_DATA_ENDPOINT_BUSY                   (0x04)
#define MCAP_RESPONSE_CODE_INVALID_DATA_LINK_ID                 (0x05)
#define MCAP_RESPONSE_CODE_DATA_LINK_BUSY                       (0x06)
#define MCAP_RESPONSE_CODE_INVALID_OPERATION                    (0x07)
#define MCAP_RESPONSE_CODE_RESOURCE_UNAVAILABLE                 (0x08)
#define MCAP_RESPONSE_CODE_UNSPECIFIED_ERROR                    (0x09)
#define MCAP_RESPONSE_CODE_REQUEST_NOT_SUPPORTED                (0x0A)
#define MCAP_RESPONSE_CODE_CONFIGURATION_REJECTED               (0x0B)

   /* The following defines the structure of an MCAP Error Response     */
   /* Packet.                                                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Error_Response_t
{
  NonAlignedByte_t OpCode;
  NonAlignedByte_t Response_Code;
  NonAlignedWord_t MDLID;
} __PACKED_STRUCT_END__ MCAP_Error_Response_t;

#define MCAP_ERROR_RESPONSE_DATA_SIZE                   (sizeof(MCAP_Error_Response_t))

   /* The following defines the structure of an MCAP Create Data Link   */
   /* Request Packet.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Create_Data_Link_Request_t
{
  NonAlignedByte_t OpCode;
  NonAlignedWord_t MDLID;
  NonAlignedByte_t MDEPID;
  NonAlignedByte_t Configuration;
} __PACKED_STRUCT_END__ MCAP_Create_Data_Link_Request_t;

#define MCAP_CREATE_DATA_LINK_REQUEST_DATA_SIZE         (sizeof(MCAP_Create_Data_Link_Request_t))

   /* The following defines the structure of an MCAP Create Data Link   */
   /* Response Packet.                                                  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Create_Data_Link_Response_t
{
  NonAlignedByte_t OpCode;
  NonAlignedByte_t Response_Code;
  NonAlignedWord_t MDLID;
  NonAlignedByte_t Configuration;
} __PACKED_STRUCT_END__ MCAP_Create_Data_Link_Response_t;

#define MCAP_CREATE_DATA_LINK_RESPONSE_DATA_SIZE        (sizeof(MCAP_Create_Data_Link_Response_t))

   /* The following defines the structure of an MCAP Reconnect Data Link*/
   /* Request Packet.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Reconnect_Data_Link_Request_t
{
  NonAlignedByte_t OpCode;
  NonAlignedWord_t MDLID;
} __PACKED_STRUCT_END__ MCAP_Reconnect_Data_Link_Request_t;

#define MCAP_RECONNECT_DATA_LINK_REQUEST_DATA_SIZE      (sizeof(MCAP_Reconnect_Data_Link_Request_t))

   /* The following defines the structure of an MCAP Reconnect Data Link*/
   /* Response Packet.                                                  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Reconnect_Data_Link_Response_t
{
  NonAlignedByte_t OpCode;
  NonAlignedByte_t Response_Code;
  NonAlignedWord_t MDLID;
} __PACKED_STRUCT_END__ MCAP_Reconnect_Data_Link_Response_t;

#define MCAP_RECONNECT_DATA_LINK_RESPONSE_DATA_SIZE     (sizeof(MCAP_Reconnect_Data_Link_Response_t))

   /* The following defines the structure of an MCAP Abort Data Link    */
   /* Request Packet.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Abort_Data_Link_Request_t
{
  NonAlignedByte_t OpCode;
  NonAlignedWord_t MDLID;
} __PACKED_STRUCT_END__ MCAP_Abort_Data_Link_Request_t;

#define MCAP_ABORT_DATA_LINK_REQUEST_DATA_SIZE          (sizeof(MCAP_Abort_Data_Link_Request_t))

   /* The following defines the structure of an MCAP Abort Data Link    */
   /* Response Packet.                                                  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Abort_Data_Link_Response_t
{
  NonAlignedByte_t OpCode;
  NonAlignedByte_t Response_Code;
  NonAlignedWord_t MDLID;
} __PACKED_STRUCT_END__ MCAP_Abort_Data_Link_Response_t;

#define MCAP_ABORT_DATA_LINK_RESPONSE_DATA_SIZE         (sizeof(MCAP_Abort_Data_Link_Response_t))

   /* The following defines the structure of an MCAP Delete Data Link   */
   /* Request Packet.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Delete_Data_Link_Request_t
{
  NonAlignedByte_t OpCode;
  NonAlignedWord_t MDLID;
} __PACKED_STRUCT_END__ MCAP_Delete_Data_Link_Request_t;

#define MCAP_DELETE_DATA_LINK_REQUEST_DATA_SIZE         (sizeof(MCAP_Delete_Data_Link_Request_t))

   /* The following defines the structure of an MCAP Delete Data Link   */
   /* Response Packet.                                                  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Delete_Data_Link_Response_t
{
  NonAlignedByte_t OpCode;
  NonAlignedByte_t Response_Code;
  NonAlignedWord_t MDLID;
} __PACKED_STRUCT_END__ MCAP_Delete_Data_Link_Response_t;

#define MCAP_DELETE_DATA_LINK_RESPONSE_DATA_SIZE        (sizeof(MCAP_Delete_Data_Link_Response_t))

   /* The following defines the structure of an MCAP Clock Sync         */
   /* Capabilities Request Packet.                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Clock_Sync_Capabilities_Request_t
{
  NonAlignedByte_t OpCode;
  NonAlignedWord_t Timestamp_Required_Accuracy;
} __PACKED_STRUCT_END__ MCAP_Clock_Sync_Capabilities_Request_t;

#define MCAP_CLOCK_SYNC_CAPABILITIES_REQUEST_DATA_SIZE  (sizeof(MCAP_Clock_Sync_Capabilities_Request_t))

   /* The following defines the structure of an MCAP Clock Sync         */
   /* Capabilities Response Packet.                                     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Clock_Sync_Capabilities_Response_t
{
  NonAlignedByte_t OpCode;
  NonAlignedByte_t Response_Code;
  NonAlignedByte_t Bluetooth_Clock_Access_Resolution;
  NonAlignedWord_t Sync_Lead_Time;
  NonAlignedWord_t Timestamp_Native_Resolution;
  NonAlignedWord_t Timestamp_Native_Accuracy;
} __PACKED_STRUCT_END__ MCAP_Clock_Sync_Capabilities_Response_t;

#define MCAP_CLOCK_SYNC_CAPABILITIES_RESPONSE_DATA_SIZE (sizeof(MCAP_Clock_Sync_Capabilities_Response_t))

   /* The following defines the structure of an MCAP Clock Sync Set     */
   /* Request Packet.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Clock_Sync_Set_Request_t
{
  NonAlignedByte_t  OpCode;
  NonAlignedByte_t  Timestamp_Update_Information;
  NonAlignedDWord_t Bluetooth_Clock_Sync_Time;
  NonAlignedQWord_t Timestamp_Sync_Time;
} __PACKED_STRUCT_END__ MCAP_Clock_Sync_Set_Request_t;

#define MCAP_CLOCK_SYNC_SET_REQUEST_DATA_SIZE           (sizeof(MCAP_Clock_Sync_Set_Request_t))

   /* The following defines the structure of an MCAP Clock Sync Set     */
   /* Response Packet.                                                  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Clock_Sync_Set_Response_t
{
  NonAlignedByte_t  OpCode;
  NonAlignedByte_t  Response_Code;
  NonAlignedDWord_t Bluetooth_Clock_Sync_Time;
  NonAlignedQWord_t Timestamp_Sync_Time;
  NonAlignedWord_t  Timestamp_Sample_Accuracy;
} __PACKED_STRUCT_END__ MCAP_Clock_Sync_Set_Response_t;

#define MCAP_CLOCK_SYNC_SET_RESPONSE_DATA_SIZE          (sizeof(MCAP_Clock_Sync_Set_Response_t))

   /* The following defines the structure of an MCAP Clock Sync Info    */
   /* Indication Packet.                                                */
typedef __PACKED_STRUCT_BEGIN__ struct _tagMCAP_Clock_Sync_Info_Indication_t
{
  NonAlignedByte_t  OpCode;
  NonAlignedByte_t  Response_Code;
  NonAlignedDWord_t Bluetooth_Clock_Sync_Time;
  NonAlignedQWord_t Timestamp_Sync_Time;
  NonAlignedWord_t  Timestamp_Sample_Accuracy;
} __PACKED_STRUCT_END__ MCAP_Clock_Sync_Info_Indication_t;

#define MCAP_CLOCK_SYNC_INFO_INDICATION_DATA_SIZE       (sizeof(MCAP_Clock_Sync_Info_Indication_t))

#endif
