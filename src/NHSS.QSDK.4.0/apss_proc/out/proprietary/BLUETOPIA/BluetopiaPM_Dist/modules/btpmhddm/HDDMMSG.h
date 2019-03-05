/*****< hddmmsg.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDDMMSG - Defined Interprocess Communication Messages for the Human       */
/*            Interface Device (HID) Device Manager for Stonestreet One       */
/*            Bluetopia Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/29/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __HDDMMSGH__
#define __HDDMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHID.h"            /* HID Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HDDMType.h"            /* BTPM HID Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Human Interface */
   /* Device (HID) Device Manager.                                      */
#define BTPM_MESSAGE_GROUP_HDD_MANAGER                         0x0000110C

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Human Interface  */
   /* Device (HID) Device Manager.                                      */

   /* Human Interface Device (HID) Device Manager Commands.             */
#define HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE      0x00001001
#define HDDM_MESSAGE_FUNCTION_CONNECT_REMOTE_HOST              0x00001002
#define HDDM_MESSAGE_FUNCTION_DISCONNECT                       0x00001003
#define HDDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HOSTS            0x00001004

#define HDDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS 0x00001101

#define HDDM_MESSAGE_FUNCTION_SEND_REPORT_DATA                 0x00001201
#define HDDM_MESSAGE_FUNCTION_SEND_GET_REPORT_RESPONSE         0x00001202
#define HDDM_MESSAGE_FUNCTION_SEND_SET_REPORT_RESPONSE         0x00001203
#define HDDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_RESPONSE       0x00001204
#define HDDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_RESPONSE       0x00001205
#define HDDM_MESSAGE_FUNCTION_SEND_GET_IDLE_RESPONSE           0x00001206
#define HDDM_MESSAGE_FUNCTION_SEND_SET_IDLE_RESPONSE           0x00001207

#define HDDM_MESSAGE_FUNCTION_REGISTER_EVENTS                  0x00002001
#define HDDM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS               0x00002002

#define HDDM_MESSAGE_FUNCTION_REGISTER_DATA_EVENTS             0x00002101
#define HDDM_MESSAGE_FUNCTION_UN_REGISTER_DATA_EVENTS          0x00002102

   /* Human Interface Device (HID) Device Manager Asynchronous Events.  */
#define HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST               0x00010001
#define HDDM_MESSAGE_FUNCTION_CONNECTED                        0x00010002
#define HDDM_MESSAGE_FUNCTION_CONNECTION_STATUS                0x00010003
#define HDDM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010004

#define HDDM_MESSAGE_FUNCTION_CONTROL_EVENT                    0x00011001
#define HDDM_MESSAGE_FUNCTION_REPORT_DATA_RECEIVED             0x00011002
#define HDDM_MESSAGE_FUNCTION_GET_REPORT_REQUEST               0x00011003
#define HDDM_MESSAGE_FUNCTION_SET_REPORT_REQUEST               0x00011004
#define HDDM_MESSAGE_FUNCTION_GET_PROTOCOL_REQUEST             0x00011005
#define HDDM_MESSAGE_FUNCTION_SET_PROTOCOL_REQUEST             0x00011006
#define HDDM_MESSAGE_FUNCTION_GET_IDLE_REQUEST                 0x00011007
#define HDDM_MESSAGE_FUNCTION_SET_IDLE_REQUEST                 0x00011008

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Human Interface Device  */
   /* Device (HID) Manager.                                             */

   /* Human Interface Device Device (HID) Manager Manager               */
   /* Command/Response Message Formats.                                 */

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to Respond to an incoming HID Host     */
   /* Connection/Authorization (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Accept;
} HDDM_Connection_Request_Response_Request_t;

#define HDDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(HDDM_Connection_Request_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to Respond to an incoming HID Host     */
   /* Connection/Authorization (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Connection_Request_Response_Response_t;

#define HDDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(HDDM_Connection_Request_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to connect to a remote HID Host device */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONNECT_REMOTE_HOST             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Connect_Remote_Host_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ConnectionFlags;
} HDDM_Connect_Remote_Host_Request_t;

#define HDDM_CONNECT_REMOTE_HOST_REQUEST_SIZE                  (sizeof(HDDM_Connect_Remote_Host_Request_t))

   /* The following constants are used with the ConnectFlags member of  */
   /* the HDDM_Connect_Remote_Host_Request_t message to control various */
   /* connection options.                                               */
#define HDDM_CONNECT_REMOTE_HOST_FLAGS_REQUIRE_AUTHENTICATION  0x00000001
#define HDDM_CONNECT_REMOTE_HOST_FLAGS_REQUIRE_ENCRYPTION      0x00000002

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to connect to a remote HID Host device */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONNECT_REMOTE_HOST             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Connect_Remote_Host_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Connect_Remote_Host_Response_t;

#define HDDM_CONNECT_REMOTE_HOST_RESPONSE_SIZE                 (sizeof(HDDM_Connect_Remote_Host_Response_t))

   /* The following structure represents the Message definition for     */
   /* a HID Device Manager Message to disconnect a remote HID Host      */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_DISCONNECT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Disconnect_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             SendVirtualCableUnplug;
} HDDM_Disconnect_Request_t;

#define HDDM_DISCONNECT_REQUEST_SIZE                           (sizeof(HDDM_Disconnect_Request_t))

   /* The following structure represents the Message definition for     */
   /* a HID Device Manager Message to disconnect a remote HID Host      */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_DISCONNECT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Disconnect_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Disconnect_Response_t;

#define HDDM_DISCONNECT_RESPONSE_SIZE                          (sizeof(HDDM_Disconnect_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to query the currently connected HID   */
   /* Host devices (Request).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HOSTS           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Query_Connected_Hosts_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MaximumNumberDevices;
} HDDM_Query_Connected_Hosts_Request_t;

#define HDDM_QUERY_CONNECTED_HOSTS_REQUEST_SIZE                (sizeof(HDDM_Query_Connected_Hosts_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to query the currently connected HID   */
   /* Host devices (Response).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HOSTS           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Query_Connected_Hosts_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TotalNumberDevices;
   unsigned int          NumberDevicesConnected;
   BD_ADDR_t             DeviceConnectedList[1];
} HDDM_Query_Connected_Hosts_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Query Connected HID Devices  */
   /* Request Message given the number of connected HID devices.  This  */
   /* function accepts as it's input the total number of connected HID  */
   /* Devices (NOT bytes) that are present starting from the            */
   /* DeviceConnectedList member of the                                 */
   /* HDDM_Query_Connected_HID_Devices_Response_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define HDDM_QUERY_CONNECTED_HOSTS_RESPONSE_SIZE(_x)           (STRUCTURE_OFFSET(HDDM_Query_Connected_Hosts_Response_t, DeviceConnectedList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to change the incoming connection flags*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Change_Incoming_Connection_Flags_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionFlags;
} HDDM_Change_Incoming_Connection_Flags_Request_t;

#define HDDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE     (sizeof(HDDM_Change_Incoming_Connection_Flags_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the HDDM_Change_Incoming_Connection_Flags_Request_t structure  */
   /* to specify the various flags to apply to incoming Connections.    */
#define HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION   0x00000001
#define HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION  0x00000002
#define HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION      0x00000004

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to change the incoming connection flags*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Change_Incoming_Connection_Flags_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Change_Incoming_Connection_Flags_Response_t;

#define HDDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE    (sizeof(HDDM_Change_Incoming_Connection_Flags_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to send report data (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SEND_REPORT_DATA                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Send_Report_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataCallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ReportLength;
   Byte_t                ReportData[1];
} HDDM_Send_Report_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Send Report Data Request     */
   /* Message given the size of the report data.  This function accepts */
   /* as it's input the number of bytes in the report data and returns  */
   /* the total number of bytes required to hold the entire message.    */
#define HDDM_SEND_REPORT_DATA_REQUEST_SIZE(_x)                 (STRUCTURE_OFFSET(HDDM_Send_Report_Data_Request_t, ReportData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to send report data (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SEND_REPORT_DATA                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Send_Report_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Send_Report_Data_Response_t;

#define HDDM_SEND_REPORT_DATA_RESPONSE_SIZE                    (sizeof(HDDM_Send_Report_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Get Report Request     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Report_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataCallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Result_t         Result;
   HDDM_Report_Type_t    ReportType;
   unsigned int          ReportLength;
   Byte_t                ReportData[1];
} HDDM_Get_Report_Response_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Get Report Response Message  */
   /* given the size of the report data.  This function accepts as it's */
   /* input the number of bytes in the report data and returns the total*/
   /* number of bytes required to hold the entire message.              */
#define HDDM_GET_REPORT_RESPONSE_REQUEST_SIZE(_x)              (STRUCTURE_OFFSET(HDDM_Get_Report_Response_Request_t, ReportData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Get Report Request     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Report_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Get_Report_Response_Response_t;

#define HDDM_GET_REPORT_RESPONSE_RESPONSE_SIZE                 (sizeof(HDDM_Get_Report_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Set Report Request     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Report_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataCallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Result_t         Result;
} HDDM_Set_Report_Response_Request_t;

#define HDDM_SET_REPORT_RESPONSE_REQUEST_SIZE                  (sizeof(HDDM_Set_Report_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Set Report Request     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Report_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Set_Report_Response_Response_t;

#define HDDM_SET_REPORT_RESPONSE_RESPONSE_SIZE                 (sizeof(HDDM_Set_Report_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Get Protocol Request   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_PROTOCOL_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Protocol_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataCallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Result_t         Result;
   HDDM_Protocol_t       Protocol;
} HDDM_Get_Protocol_Response_Request_t;

#define HDDM_GET_PROTOCOL_RESPONSE_REQUEST_SIZE                (sizeof(HDDM_Get_Protocol_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Get Protocol Request   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_PROTOCOL_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Protocol_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Get_Protocol_Response_Response_t;

#define HDDM_GET_PROTOCOL_RESPONSE_RESPONSE_SIZE               (sizeof(HDDM_Get_Protocol_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Set Protocol Request   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_PROTOCOL_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Protocol_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataCallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Result_t Result;
} HDDM_Set_Protocol_Response_Request_t;

#define HDDM_SET_PROTOCOL_RESPONSE_REQUEST_SIZE                (sizeof(HDDM_Set_Protocol_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to respond to a Set Protocol Request   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_PROTOCOL_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Protocol_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Set_Protocol_Response_Response_t;

#define HDDM_SET_PROTOCOL_RESPONSE_RESPONSE_SIZE               (sizeof(HDDM_Set_Protocol_Response_Response_t))

   /* The following structure represents the Message definition for     */
   /* a HID Device Manager Message to respond to a Get Idle Request     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_IDLE_RESPONSE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Idle_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataCallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Result_t         Result;
   unsigned int          IdleRate;
} HDDM_Get_Idle_Response_Request_t;

#define HDDM_GET_IDLE_RESPONSE_REQUEST_SIZE                    (sizeof(HDDM_Get_Idle_Response_Request_t))

   /* The following structure represents the Message definition for     */
   /* a HID Device Manager Message to respond to a Get Idle Request     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_IDLE_RESPONSE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Idle_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Get_Idle_Response_Response_t;

#define HDDM_GET_IDLE_RESPONSE_RESPONSE_SIZE                   (sizeof(HDDM_Get_Idle_Response_Response_t))

   /* The following structure represents the Message definition for     */
   /* a HID Device Manager Message to respond to a Set Idle Request     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_IDLE_RESPONSE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Idle_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataCallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Result_t         Result;
} HDDM_Set_Idle_Response_Request_t;

#define HDDM_SET_IDLE_RESPONSE_REQUEST_SIZE                    (sizeof(HDDM_Set_Idle_Response_Request_t))

   /* The following structure represents the Message definition for     */
   /* a HID Device Manager Message to respond to a Set Idle Request     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_IDLE_RESPONSE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Idle_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Set_Idle_Response_Response_t;

#define HDDM_SET_IDLE_RESPONSE_RESPONSE_SIZE                   (sizeof(HDDM_Set_Idle_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to register for generic HID Device     */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_REGISTER_EVENTS                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Register_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HDDM_Register_Events_Request_t;

#define HDDM_REGISTER_EVENTS_REQUEST_SIZE                      (sizeof(HDDM_Register_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to register for generic HID Device     */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_REGISTER_EVENTS                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Register_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          CallbackID;
} HDDM_Register_Events_Response_t;

#define HDDM_REGISTER_EVENTS_RESPONSE_SIZE                     (sizeof(HDDM_Register_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to un register for generic HID Device  */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Un_Register_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
} HDDM_Un_Register_Events_Request_t;

#define HDDM_UN_REGISTER_EVENTS_REQUEST_SIZE                   (sizeof(HDDM_Un_Register_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to un register for generic HID Device  */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Un_Register_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Un_Register_Events_Response_t;

#define HDDM_UN_REGISTER_EVENTS_RESPONSE_SIZE                  (sizeof(HDDM_Un_Register_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to register for HID Device Data events */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_REGISTER_DATA_EVENTS            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Register_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HDDM_Register_Data_Events_Request_t;

#define HDDM_REGISTER_DATA_EVENTS_REQUEST_SIZE                 (sizeof(HDDM_Register_Data_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to register for HID Device Data events */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_REGISTER_DATA_EVENTS            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Register_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          CallbackID;
} HDDM_Register_Data_Events_Response_t;

#define HDDM_REGISTER_DATA_EVENTS_RESPONSE_SIZE                (sizeof(HDDM_Register_Data_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to un-register HID Device data events  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_UN_REGISTER_DATA_EVENTS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Un_Register_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
} HDDM_Un_Register_Data_Events_Request_t;

#define HDDM_UN_REGISTER_DATA_EVENTS_REQUEST_SIZE              (sizeof(HDDM_Un_Register_Data_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message to un-register HID Device data events  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_UN_REGISTER_DATA_EVENTS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Un_Register_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDDM_Un_Register_Data_Events_Response_t;

#define HDDM_UN_REGISTER_DATA_EVENTS_RESPONSE_SIZE             (sizeof(HDDM_Un_Register_Data_Events_Response_t))

   /* HID Device Manager Asynchronous Message Formats.                  */

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a remote  */
   /* HID Host connection request has been received (asynchronously).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Connection_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDDM_Connection_Request_Message_t;

#define HDDM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(HDDM_Connection_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a remote  */
   /* HID Host connected event has been received (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDDM_Connected_Message_t;

#define HDDM_CONNECTED_MESSAGE_SIZE                            (sizeof(HDDM_Connected_Message_t))

   /* The following structure represents the Message definition for     */
   /* a HID Device Manager Message that informs the client that a       */
   /* remote HID Host connection status event has been received         */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONNECTION_STATUS               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ConnectionStatus;
} HDDM_Connection_Status_Message_t;

#define HDDM_CONNECTION_STATUS_MESSAGE_SIZE                    (sizeof(HDDM_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the HDDM_Connection_Status_Message_t message to describe the   */
   /* actual Connection Result Status.                                  */
#define HDDM_CONNECTION_STATUS_SUCCESS                         0x00000000
#define HDDM_CONNECTION_STATUS_FAILURE_TIMEOUT                 0x00000001
#define HDDM_CONNECTION_STATUS_FAILURE_REFUSED                 0x00000002
#define HDDM_CONNECTION_STATUS_FAILURE_SECURITY                0x00000003
#define HDDM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF        0x00000004
#define HDDM_CONNECTION_STATUS_FAILURE_UNKNOWN                 0x00000005

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a remote  */
   /* HID Host disconnected event has been received (asynchronously).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDDM_Disconnected_Message_t;

#define HDDM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(HDDM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a control */
   /* event has been received (asynchronously).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_CONTROL_EVENT                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Control_Event_Message_t
{
   BTPM_Message_Header_t    MessageHeader;
   BD_ADDR_t                RemoteDeviceAddress;
   HDDM_Control_Operation_t ControlOperation;
} HDDM_Control_Event_Message_t;

#define HDDM_CONTROL_EVENT_MESSAGE_SIZE                        (sizeof(HDDM_Control_Event_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a report  */
   /* data has been received (asynchronously).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_REPORT_DATA_RECEIVED            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Report_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ReportLength;
   Byte_t                ReportData[1];
} HDDM_Report_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold an entire HID Report Data received  */
   /* Message given the number of actual report data bytes.  This       */
   /* function accepts as it's input the total number individual report */
   /* bytes are present starting from the ReportData member of the      */
   /* HDDM_Report_Data_Received_Message_t structure and returns the     */
   /* total number of bytes required to hold the entire message.        */
#define HDDM_REPORT_DATA_RECEIVED_MESSAGE_SIZE(_x)             (STRUCTURE_OFFSET(HDDM_Report_Data_Received_Message_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a Get     */
   /* Report Request has been received (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_REPORT_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Report_Request_Message_t
{
   BTPM_Message_Header_t       MessageHeader;
   BD_ADDR_t                   RemoteDeviceAddress;
   HDDM_Get_Report_Size_Type_t SizeType;
   HDDM_Report_Type_t          ReportType;
   unsigned int                ReportID;
   unsigned int                BufferSize;
} HDDM_Get_Report_Request_Message_t;

#define HDDM_GET_REPORT_REQUEST_MESSAGE_SIZE                   (sizeof(HDDM_Get_Report_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a Set     */
   /* Report Request has been received (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_REPORT_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Report_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Report_Type_t    ReportType;
   unsigned int          ReportLength;
   Byte_t                ReportData[1];
} HDDM_Set_Report_Request_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold an entire Set Report Request        */
   /* Message given the number of actual report data bytes.  This       */
   /* function accepts as it's input the total number individual report */
   /* bytes are present starting from the ReportData member of the      */
   /* HDDM_Set_Report_Request_Message_t structure and returns the total */
   /* number of bytes required to hold the entire message.              */
#define HDDM_SET_REPORT_REQUEST_MESSAGE_SIZE(_x)               (STRUCTURE_OFFSET(HDDM_Set_Report_Request_Message_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a Get     */
   /* Protocol Request has been received (asynchronously).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_PROTOCOL_REQUEST            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Protocol_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDDM_Get_Protocol_Request_Message_t;

#define HDDM_GET_PROTOCOL_REQUEST_MESSAGE_SIZE                 (sizeof(HDDM_Get_Protocol_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a Set     */
   /* Protocol Request has been received (asynchronously).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_PROTOCOL_REQUEST            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Protocol_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   HDDM_Protocol_t       Protocol;
} HDDM_Set_Protocol_Request_Message_t;

#define HDDM_SET_PROTOCOL_REQUEST_MESSAGE_SIZE                 (sizeof(HDDM_Set_Protocol_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a Get Idle*/
   /* Request has been received (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_GET_IDLE_REQUEST                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Get_Idle_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDDM_Get_Idle_Request_Message_t;

#define HDDM_GET_IDLE_REQUEST_MESSAGE_SIZE                     (sizeof(HDDM_Get_Idle_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Device Manager Message that informs the client that a Set Idle*/
   /* Request has been received (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDDM_MESSAGE_FUNCTION_SET_IDLE_REQUEST                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDDM_Set_Idle_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          IdleRate;
} HDDM_Set_Idle_Request_Message_t;

#define HDDM_SET_IDLE_REQUEST_MESSAGE_SIZE                     (sizeof(HDDM_Set_Idle_Request_Message_t))

#endif
