/*****< hidmmsg.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDMMSG - Defined Interprocess Communication Messages for the Human       */
/*            Interface Device (HID) Manager for Stonestreet One Bluetopia    */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/15/11  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __HIDMMSGH__
#define __HIDMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHIDH.h"           /* HID Host Framework Prototypes/Constants.  */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HIDMType.h"            /* BTPM HID Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Human Interface */
   /* Device (HID) Host Manager.                                        */
#define BTPM_MESSAGE_GROUP_HID_MANAGER                          0x00001001

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Human Interface  */
   /* Device (HID) Host Manager.                                        */

   /* Human Interface Device (HID) Host Manager Commands.               */
#define HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE      0x00001001
#define HIDM_MESSAGE_FUNCTION_CONNECT_HID_DEVICE               0x00001002
#define HIDM_MESSAGE_FUNCTION_DISCONNECT_HID_DEVICE            0x00001003
#define HIDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HID_DEVICES      0x00001004

#define HIDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS 0x00001101
#define HIDM_MESSAGE_FUNCTION_SET_KEYBOARD_REPEAT_RATE         0x00001102

#define HIDM_MESSAGE_FUNCTION_SEND_REPORT_DATA                 0x00001201
#define HIDM_MESSAGE_FUNCTION_SEND_GET_REPORT_REQUEST          0x00001202
#define HIDM_MESSAGE_FUNCTION_SEND_SET_REPORT_REQUEST          0x00001203
#define HIDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_REQUEST        0x00001204
#define HIDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_REQUEST        0x00001205
#define HIDM_MESSAGE_FUNCTION_SEND_GET_IDLE_REQUEST            0x00001206
#define HIDM_MESSAGE_FUNCTION_SEND_SET_IDLE_REQUEST            0x00001207

#define HIDM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS              0x00002001
#define HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS           0x00002002

#define HIDM_MESSAGE_FUNCTION_REGISTER_HID_DATA                0x00002101
#define HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA             0x00002102

   /* Human Interface Device (HID) Host Manager Asynchronous Events.    */
#define HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST               0x00010001
#define HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED             0x00010002
#define HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTION_STATUS     0x00010003
#define HIDM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED          0x00010004

#define HIDM_MESSAGE_FUNCTION_BOOT_KEYBOARD_KEY_PRESS_EVENT    0x00011001
#define HIDM_MESSAGE_FUNCTION_BOOT_KEYBOARD_KEY_REPEAT_EVENT   0x00011002
#define HIDM_MESSAGE_FUNCTION_BOOT_MOUSE_MOUSE_EVENT           0x00011003
#define HIDM_MESSAGE_FUNCTION_HID_REPORT_DATA_RECEIVED         0x00011004
#define HIDM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT        0x00011005
#define HIDM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT        0x00011006
#define HIDM_MESSAGE_FUNCTION_GET_PROTOCOL_RESPONSE_EVENT      0x00011007
#define HIDM_MESSAGE_FUNCTION_SET_PROTOCOL_RESPONSE_EVENT      0x00011008
#define HIDM_MESSAGE_FUNCTION_GET_IDLE_RESPONSE_EVENT          0x00011009
#define HIDM_MESSAGE_FUNCTION_SET_IDLE_RESPONSE_EVENT          0x0001100a

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Human Interface Device  */
   /* Host (HID) Manager.                                               */

   /* Human Interface Device Host (HID) Manager Manager Command/Response*/
   /* Message Formats.                                                  */

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to Respond to an incoming HID Device     */
   /* Connection/Authorization (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Accept;
   unsigned long         ConnectionFlags;
} HIDM_Connection_Request_Response_Request_t;

#define HIDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(HIDM_Connection_Request_Response_Request_t))

   /* The following bit-masks are used with the ConnectionFlags member  */
   /* of the HIDM_Connection_Request_Response_Request_t structure to    */
   /* denote special processing that might be required for the HID      */
   /* device.                                                           */
#define HIDM_CONNECTION_REQUEST_CONNECTION_FLAGS_REPORT_MODE   0x00000001
#define HIDM_CONNECTION_REQUEST_CONNECTION_FLAGS_PARSE_BOOT    0x00000002

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to Respond to an incoming HID Device     */
   /* Connection/Authorization (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Connection_Request_Response_Response_t;

#define HIDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(HIDM_Connection_Request_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to Connect to a remote HID device        */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_CONNECT_HID_DEVICE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Connect_HID_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         ConnectionFlags;
} HIDM_Connect_HID_Device_Request_t;

#define HIDM_CONNECT_HID_DEVICE_REQUEST_SIZE                   (sizeof(HIDM_Connect_HID_Device_Request_t))

   /* The following constants are used with the ConnectFlags member of  */
   /* the HIDM_Connect_HID_Device_Request_t message to control various  */
   /* connection options.                                               */
#define HIDM_CONNECT_HID_DEVICE_FLAGS_REQUIRE_AUTHENTICATION   0x00000001
#define HIDM_CONNECT_HID_DEVICE_FLAGS_REQUIRE_ENCRYPTION       0x00000002
#define HIDM_CONNECT_HID_DEVICE_FLAGS_REPORT_MODE              0x00000004
#define HIDM_CONNECT_HID_DEVICE_FLAGS_PARSE_BOOT               0x00000008

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to Connect to a remote HID device        */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_CONNECT_HID_DEVICE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Connect_HID_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Connect_HID_Device_Response_t;

#define HIDM_CONNECT_HID_DEVICE_RESPONSE_SIZE                  (sizeof(HIDM_Connect_HID_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to Disconnect a currently connected HID  */
   /* Device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_DISCONNECT_HID_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Disconnect_HID_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             SendVirtualCableDisconnect;
} HIDM_Disconnect_HID_Device_Request_t;

#define HIDM_DISCONNECT_HID_DEVICE_REQUEST_SIZE                (sizeof(HIDM_Disconnect_HID_Device_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to Disconnect a currently connected HID  */
   /* Device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_DISCONNECT_HID_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Disconnect_HID_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Disconnect_HID_Device_Response_t;

#define HIDM_DISCONNECT_HID_DEVICE_RESPONSE_SIZE               (sizeof(HIDM_Disconnect_HID_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to query the currently connected HID     */
   /* Devices (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HID_DEVICES     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Query_Connected_HID_Devices_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HIDM_Query_Connected_HID_Devices_Request_t;

#define HIDM_QUERY_CONNECTED_HID_DEVICES_REQUEST_SIZE          (sizeof(HIDM_Query_Connected_HID_Devices_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to query the currently connected HID     */
   /* Devices (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HID_DEVICES     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Query_Connected_HID_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberDevicesConnected;
   BD_ADDR_t             DeviceConnectedList[1];
} HIDM_Query_Connected_HID_Devices_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Query Connected HID Devices  */
   /* Request Message given the number of connected HID devices.  This  */
   /* function accepts as it's input the total number of connected HID  */
   /* Devices (NOT bytes) that are present starting from the            */
   /* DeviceConnectedList member of the                                 */
   /* HIDM_Query_Connected_HID_Devices_Response_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define HIDM_QUERY_CONNECTED_HID_DEVICES_RESPONSE_SIZE(_x)     (STRUCTURE_OFFSET(HIDM_Query_Connected_HID_Devices_Response_t, DeviceConnectedList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to change the current incoming Connection*/
   /* Flags of a HID Connection (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Change_Incoming_Connection_Flags_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ConnectionFlags;
} HIDM_Change_Incoming_Connection_Flags_Request_t;

#define HIDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE     (sizeof(HIDM_Change_Incoming_Connection_Flags_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the HIDM_Change_Incoming_Connection_Flags_Request_t structure  */
   /* to specify the various flags to apply to incoming Connections.    */
#define HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION   0x00000001
#define HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION  0x00000002
#define HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION      0x00000004
#define HIDM_INCOMING_CONNECTION_FLAGS_REPORT_MODE             0x40000000
#define HIDM_INCOMING_CONNECTION_FLAGS_PARSE_BOOT              0x80000000

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to change the current incoming Connection*/
   /* Flags of a HID Connection (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Change_Incoming_Connection_Flags_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Change_Incoming_Connection_Flags_Response_t;

#define HIDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE    (sizeof(HIDM_Change_Incoming_Connection_Flags_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to set the current Keyboard Repeat Rate  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SET_KEYBOARD_REPEAT_RATE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Set_Keyboard_Repeat_Rate_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RepeatDelay;
   unsigned int          RepeatRate;
} HIDM_Set_Keyboard_Repeat_Rate_Request_t;

#define HIDM_SET_KEYBOARD_REPEAT_RATE_REQUEST_SIZE             (sizeof(HIDM_Set_Keyboard_Repeat_Rate_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to set the current Keyboard Repeat Rate  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SET_KEYBOARD_REPEAT_RATE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Set_Keyboard_Repeat_Rate_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Set_Keyboard_Repeat_Rate_Response_t;

#define HIDM_SET_KEYBOARD_REPEAT_RATE_RESPONSE_SIZE            (sizeof(HIDM_Set_Keyboard_Repeat_Rate_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send Raw HID Report Data.             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_REPORT_DATA                */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * There is NO Response message for this message.           */
typedef struct _tagHIDM_Send_Report_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ReportLength;
   Byte_t                ReportData[1];
} HIDM_Send_Report_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Send Report Data Request     */
   /* Message given the number of actual report data bytes.  This       */
   /* function accepts as it's input the total number individual report */
   /* data bytes are present starting from the ReportData member of the */
   /* HIDM_Send_Report_Data_Request_t structure and returns the total   */
   /* number of bytes required to hold the entire message.              */
#define HIDM_SEND_REPORT_DATA_REQUEST_SIZE(_x)                 (STRUCTURE_OFFSET(HIDM_Send_Report_Data_Request_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Get Report request (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_GET_REPORT_REQUEST         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Get_Report_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Report_Type_t    ReportType;
   Byte_t                ReportID;
} HIDM_Send_Get_Report_Request_Request_t;

#define HIDM_SEND_GET_REPORT_REQUEST_REQUEST_SIZE              (sizeof(HIDM_Send_Get_Report_Request_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Get Report request (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_GET_REPORT_REQUEST         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Get_Report_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Send_Get_Report_Request_Response_t;

#define HIDM_SEND_GET_REPORT_REQUEST_RESPONSE_SIZE             (sizeof(HIDM_Send_Get_Report_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Set Report request (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_SET_REPORT_REQUEST         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Set_Report_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Report_Type_t    ReportType;
   Word_t                ReportDataLength;
   Byte_t                ReportData[1];
} HIDM_Send_Set_Report_Request_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold a an entire Send Set Report Request */
   /* Request Message given the number of actual report data bytes.     */
   /* This function accepts as it's input the total number of individual*/
   /* report data bytes that are present starting from the ReportData   */
   /* member of the HIDM_Send_Report_Data_Request_t structure and       */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HIDM_SEND_SET_REPORT_REQUEST_REQUEST_SIZE(_x)          (STRUCTURE_OFFSET(HIDM_Send_Set_Report_Request_Request_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Set Report request (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_SET_REPORT_REQUEST         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Set_Report_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Send_Set_Report_Request_Response_t;

#define HIDM_SEND_SET_REPORT_REQUEST_RESPONSE_SIZE             (sizeof(HIDM_Send_Set_Report_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Get Protocol request (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_REQUEST       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Get_Protocol_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HIDM_Send_Get_Protocol_Request_Request_t;

#define HIDM_SEND_GET_PROTOCOL_REQUEST_REQUEST_SIZE            (sizeof(HIDM_Send_Get_Protocol_Request_Request_t))

   /* The following structure represents the Message definition for     */
   /* a HID Host Manager Message to send a Get Protocol request         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_REQUEST       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Get_Protocol_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Send_Get_Protocol_Request_Response_t;

#define HIDM_SEND_GET_PROTOCOL_REQUEST_RESPONSE_SIZE           (sizeof(HIDM_Send_Get_Protocol_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Set Protocol request (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_REQUEST       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Set_Protocol_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Protocol_t       Protocol;
} HIDM_Send_Set_Protocol_Request_Request_t;

#define HIDM_SEND_SET_PROTOCOL_REQUEST_REQUEST_SIZE            (sizeof(HIDM_Send_Set_Protocol_Request_Request_t))

   /* The following structure represents the Message definition for     */
   /* a HID Host Manager Message to send a Set Protocol request         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_REQUEST       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Set_Protocol_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Send_Set_Protocol_Request_Response_t;

#define HIDM_SEND_SET_PROTOCOL_REQUEST_RESPONSE_SIZE           (sizeof(HIDM_Send_Set_Protocol_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Get Idle request (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_GET_IDLE_REQUEST           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Get_Idle_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HIDM_Send_Get_Idle_Request_Request_t;

#define HIDM_SEND_GET_IDLE_REQUEST_REQUEST_SIZE                (sizeof(HIDM_Send_Get_Idle_Request_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Get Idle request (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_GET_IDLE_REQUEST           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Get_Idle_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Send_Get_Idle_Request_Response_t;

#define HIDM_SEND_GET_IDLE_REQUEST_RESPONSE_SIZE               (sizeof(HIDM_Send_Get_Idle_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Set Idle request (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_SET_IDLE_REQUEST           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Set_Idle_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Byte_t                IdleRate;
} HIDM_Send_Set_Idle_Request_Request_t;

#define HIDM_SEND_SET_IDLE_REQUEST_REQUEST_SIZE                (sizeof(HIDM_Send_Set_Idle_Request_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to send a Set Idle request (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SEND_SET_IDLE_REQUEST           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Send_Set_Idle_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Send_Set_Idle_Request_Response_t;

#define HIDM_SEND_SET_IDLE_REQUEST_RESPONSE_SIZE               (sizeof(HIDM_Send_Set_Idle_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to register for HID Manager events       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Register_HID_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HIDM_Register_HID_Events_Request_t;

#define HIDM_REGISTER_HID_EVENTS_REQUEST_SIZE                  (sizeof(HIDM_Register_HID_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to register for HID Manager events       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Register_HID_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          HIDEventsHandlerID;
} HIDM_Register_HID_Events_Response_t;

#define HIDM_REGISTER_HID_EVENTS_RESPONSE_SIZE                 (sizeof(HIDM_Register_HID_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to un-register for HID Manager events    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Un_Register_HID_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDEventsHandlerID;
} HIDM_Un_Register_HID_Events_Request_t;

#define HIDM_UN_REGISTER_HID_EVENTS_REQUEST_SIZE               (sizeof(HIDM_Un_Register_HID_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to un-register for HID Manager events    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Un_Register_HID_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Un_Register_HID_Events_Response_t;

#define HIDM_UN_REGISTER_HID_EVENTS_RESPONSE_SIZE              (sizeof(HIDM_Un_Register_HID_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to register for HID Manager Data events  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_REGISTER_HID_DATA               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Register_HID_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HIDM_Register_HID_Data_Events_Request_t;

#define HIDM_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE             (sizeof(HIDM_Register_HID_Data_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to register for HID Manager Data events  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_REGISTER_HID_DATA               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Register_HID_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          HIDDataEventsHandlerID;
} HIDM_Register_HID_Data_Events_Response_t;

#define HIDM_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE            (sizeof(HIDM_Register_HID_Data_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to un-register for HID Manager Data      */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Un_Register_HID_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
} HIDM_Un_Register_HID_Data_Events_Request_t;

#define HIDM_UN_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE          (sizeof(HIDM_Un_Register_HID_Data_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message to un-register for HID Manager Data      */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Un_Register_HID_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HIDM_Un_Register_HID_Data_Events_Response_t;

#define HIDM_UN_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE         (sizeof(HIDM_Un_Register_HID_Data_Events_Response_t))

   /* HID Host Manager Asynchronous Message Formats.                    */

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that informs the client that a remote HID*/
   /* Device Connection Request has been received (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_Connection_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HIDM_Connection_Request_Message_t;

#define HIDM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(HIDM_Connection_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that informs the client that a HID device*/
   /* is currently connected (asynchronously).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Device_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HIDM_HID_Device_Connected_Message_t;

#define HIDM_HID_DEVICE_CONNECTED_MESSAGE_SIZE                 (sizeof(HIDM_HID_Device_Connected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that informs the client of the specified */
   /* connection status (asynchronously).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTION_STATUS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Device_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionStatus;
   BD_ADDR_t             RemoteDeviceAddress;
} HIDM_HID_Device_Connection_Status_Message_t;

#define HIDM_HID_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE         (sizeof(HIDM_HID_Device_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the HIDM_HID_Device_Connection_Status_Message_t message to     */
   /* describe the actual Connection Result Status.                     */
#define HIDM_HID_DEVICE_CONNECTION_STATUS_SUCCESS                    0x00000000
#define HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT            0x00000001
#define HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED            0x00000002
#define HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_SECURITY           0x00000003
#define HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF   0x00000004
#define HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN            0x00000005

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that informs the client that a HID       */
   /* connection is now disconnected (asynchronously).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Device_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HIDM_HID_Device_Disconnected_Message_t;

#define HIDM_HID_DEVICE_DISCONNECTED_MESSAGE_SIZE              (sizeof(HIDM_HID_Device_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies that Boot Mode Keyboard   */
   /* Event has occurred (asynchronously).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_BOOT_KEYBOARD_KEY_PRESS_EVENT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             KeyDown;
   Byte_t                KeyModifiers;
   Byte_t                Key;
} HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t;

#define HIDM_HID_BOOT_KEYBOARD_KEY_PRESS_EVENT_MESSAGE_SIZE    (sizeof(HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies that Boot Mode Keyboard   */
   /* Event has occurred (asynchronously).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_HID_BOOT_KEYBOARD_KEY_REPEAT    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Byte_t                KeyModifiers;
   Byte_t                Key;
} HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t;

#define HIDM_HID_BOOT_KEYBOARD_KEY_REPEAT_EVENT_MESSAGE_SIZE   (sizeof(HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies that Boot Mode Mouse      */
   /* Event has occurred (asynchronously).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_BOOT_MOUSE_MOUSE_EVENT          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Boot_Mouse_Mouse_Event_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   SByte_t               CX;
   SByte_t               CY;
   Byte_t                ButtonState;
   SByte_t               CZ;
} HIDM_HID_Boot_Mouse_Mouse_Event_Message_t;

#define HIDM_HID_BOOT_MOUSE_MOUSE_EVENT_MESSAGE_SIZE           (sizeof(HIDM_HID_Boot_Mouse_Mouse_Event_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies actual received HID       */
   /* Report data (asynchronously).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_HID_REPORT_DATA_RECEIVED        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Report_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ReportLength;
   Byte_t                ReportData[1];
} HIDM_HID_Report_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire HID Report Data received     */
   /* Message given the number of actual report data bytes.  This       */
   /* function accepts as it's input the total number individual report */
   /* bytes are present starting from the ReportData member of the      */
   /* HIDM_HID_Report_Data_Received_Message_t structure and returns the */
   /* total number of bytes required to hold the entire message.        */
#define HIDM_HID_REPORT_DATA_RECEIVED_MESSAGE_SIZE(_x)         (STRUCTURE_OFFSET(HIDM_HID_Report_Data_Received_Message_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies actual received HID       */
   /* GET_REPORT request response (asynchronously).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Get_Report_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Result_t         Status;
   HIDM_Report_Type_t    ReportType;
   Word_t                ReportLength;
   Byte_t                ReportData[1];
} HIDM_HID_Get_Report_Response_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold a an entire HID Get Report Response */
   /* Message given the number of actual report data bytes.  This       */
   /* function accepts as it's input the total number individual report */
   /* bytes are present starting from the ReportData member of the      */
   /* HIDM_HID_Get_Report_Response_Message_t structure and returns the  */
   /* total number of bytes required to hold the entire message.        */
#define HIDM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(_x)          (STRUCTURE_OFFSET(HIDM_HID_Get_Report_Response_Message_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies actual received HID       */
   /* SET_REPORT request response (asynchronously).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Set_Report_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Result_t         Status;
} HIDM_HID_Set_Report_Response_Message_t;

#define HIDM_HID_SET_REPORT_RESPONSE_MESSAGE_SIZE              (sizeof(HIDM_HID_Set_Report_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies actual received HID       */
   /* GET_PROTOCOL request response (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_GET_PROTOCOL_RESPONSE_EVENT     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Get_Protocol_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Result_t         Status;
   HIDM_Protocol_t       Protocol;
} HIDM_HID_Get_Protocol_Response_Message_t;

#define HIDM_HID_GET_PROTOCOL_RESPONSE_MESSAGE_SIZE            (sizeof(HIDM_HID_Get_Protocol_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies actual received HID       */
   /* SET_PROTOCOL request response (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SET_PROTOCOL_RESPONSE_EVENT     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Set_Protocol_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Result_t         Status;
} HIDM_HID_Set_Protocol_Response_Message_t;

#define HIDM_HID_SET_PROTOCOL_RESPONSE_MESSAGE_SIZE            (sizeof(HIDM_HID_Set_Protocol_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies actual received HID       */
   /* GET_IDLE request response (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_GET_IDLE_RESPONSE_EVENT         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Get_Idle_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Result_t         Status;
   Byte_t                IdleRate;
} HIDM_HID_Get_Idle_Response_Message_t;

#define HIDM_HID_GET_IDLE_RESPONSE_MESSAGE_SIZE                (sizeof(HIDM_HID_Get_Idle_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HID Host Manager Message that specifies actual received HID       */
   /* SET_IDLE request response (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HIDM_MESSAGE_FUNCTION_SET_IDLE_RESPONSE_EVENT         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHIDM_HID_Set_Idle_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HIDDataEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HIDM_Result_t         Status;
} HIDM_HID_Set_Idle_Response_Message_t;

#define HIDM_HID_SET_IDLE_RESPONSE_MESSAGE_SIZE                (sizeof(HIDM_HID_Set_Idle_Response_Message_t))

#endif

