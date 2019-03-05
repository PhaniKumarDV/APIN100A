/*****< mapmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAPMMSG - Defined Interprocess Communication Messages for the Message     */
/*            Access Profile Manager (MAPM) for Stonestreet One Bluetopia     */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/25/12  M. Seabold     Initial creation                                */
/******************************************************************************/
#ifndef __MAPMMSGH__
#define __MAPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions            */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants  */

#include "SS1BTMAP.h"            /* Message Access Prototypes/Constants       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants   */

#include "MAPMType.h"            /* BTPM MAP Manager Type Definitions         */

   /* The following message group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Message Access  */
   /* (MAP) Manager.                                                    */
#define BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER                  0x00001005

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message functions that are valid for the Message Access   */
   /* (MAP) Manager.                                                    */

   /* Message Access Profile Manager (MAPM) Common/Connection Commands. */
#define MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE          0x00001001
#define MAPM_MESSAGE_FUNCTION_REGISTER_SERVER                      0x00001002
#define MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER                   0x00001003
#define MAPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD              0x00001004
#define MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVICE_RECORD           0x00001005
#define MAPM_MESSAGE_FUNCTION_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES 0x00001006
#define MAPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE                0x00001007
#define MAPM_MESSAGE_FUNCTION_DISCONNECT                           0x00001008
#define MAPM_MESSAGE_FUNCTION_ABORT                                0x00001009

   /* Message Access Profile Manager (MAPM) Message Client Equipment    */
   /* (MCE) Commands.                                                   */
#define MAPM_MESSAGE_FUNCTION_QUERY_CURRENT_FOLDER                 0x00001101
#define MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS                 0x00001102
#define MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING                   0x00001103
#define MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE              0x00001104
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING                  0x00001105
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE             0x00001106
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE                          0x00001107
#define MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS                   0x00001108
#define MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE                         0x00001109
#define MAPM_MESSAGE_FUNCTION_UPDATE_INBOX                         0x0000110A
#define MAPM_MESSAGE_FUNCTION_SET_FOLDER                           0x0000110B
#define MAPM_MESSAGE_FUNCTION_SET_FOLDER_ABSOLUTE                  0x0000110C

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Commands.                                                   */
#define MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_CONFIRMATION    0x00001201
#define MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING                  0x00001202
#define MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING_SIZE             0x00001203
#define MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING                 0x00001204
#define MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING_SIZE            0x00001205
#define MAPM_MESSAGE_FUNCTION_SEND_MESSAGE                         0x00001206
#define MAPM_MESSAGE_FUNCTION_MESSAGE_STATUS_CONFIRMATION          0x00001207
#define MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_CONFIRMATION            0x00001208
#define MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_CONFIRMATION            0x00001209
#define MAPM_MESSAGE_FUNCTION_SET_FOLDER_CONFIRMATION              0x0000120A

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Notification Commands.                                      */
#define MAPM_MESSAGE_FUNCTION_SEND_NOTIFICATION                    0x00001301

   /* Message Access Profile Manager (MAPM) Asynchronous Events         */
   /* Common/Connection Asynchronous Events.                            */
#define MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST                   0x00010001
#define MAPM_MESSAGE_FUNCTION_DEVICE_CONNECTED                     0x00010002
#define MAPM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED                  0x00010003
#define MAPM_MESSAGE_FUNCTION_CONNECTION_STATUS                    0x00010004

   /* Message Access Profile Manager (MAPM) Message Client Equipment    */
   /* (MCE) Asynchronous Events.                                        */
#define MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_RESPONSE        0x00011001
#define MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_RESPONSE          0x00011002
#define MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_RESPONSE     0x00011003
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_RESPONSE         0x00011004
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE_RESPONSE    0x00011005
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_RESPONSE                 0x00011006
#define MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_RESPONSE          0x00011007
#define MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_RESPONSE                0x00011008
#define MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_RESPONSE                0x00011009
#define MAPM_MESSAGE_FUNCTION_SET_FOLDER_RESPONSE                  0x0001100A

   /* Message Access Profile Manager (MAPM) Message Client Equipment    */
   /* (MCE) Notification Asynchronous Events.                           */
#define MAPM_MESSAGE_FUNCTION_NOTIFICATION_INDICATION              0x00012001

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Asynchronous Events.                                        */
#define MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_REQUEST         0x00013001
#define MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_REQUEST           0x00013002
#define MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_REQUEST      0x00013003
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_REQUEST          0x00013004
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE_REQUEST     0x00013005
#define MAPM_MESSAGE_FUNCTION_GET_MESSAGE_REQUEST                  0x00013006
#define MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_REQUEST           0x00013007
#define MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_REQUEST                 0x00013008
#define MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_REQUEST                 0x00013009
#define MAPM_MESSAGE_FUNCTION_SET_FOLDER_REQUEST                   0x0001300A

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Notification Asynchronous Events.                           */
#define MAPM_MESSAGE_FUNCTION_NOTIFICATION_CONFIRMATION            0x00014001

   /* Message Access Profile Manager (MAPM) Common/Connection Commands. */

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Respond to an incoming          */
   /* Connection/Authorization (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   Boolean_t              Accept;
} MAPM_Connection_Request_Response_Request_t;

#define MAPM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(MAPM_Connection_Request_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Respond to an incoming          */
   /* Connection/Authorization (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Connection_Request_Response_Response_t;

#define MAPM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(MAPM_Connection_Request_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Register a local MAP Server     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_REGISTER_SERVER                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * This message only registeres MAP Servers (MSE).  It does */
   /*          allow the registration of Notification servers.          */
typedef struct _tagMAPM_Register_Server_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerPort;
   unsigned long         ServerFlags;
   unsigned int          InstanceID;
   unsigned long         SupportedMessageTypes;
} MAPM_Register_Server_Request_t;

#define MAPM_REGISTER_SERVER_REQUEST_SIZE                      (sizeof(MAPM_Register_Server_Request_t))

   /* The following constants are used with the PortFlags member of the */
   /* MAPM_Register_Server_Request_t message to control various options */
   /* for the registered port.                                          */
#define MAPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHORIZATION       0x00000001
#define MAPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION      0x00000002
#define MAPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION          0x00000004

   /* The following bit mask constants are used with the                */
   /* SupportedMessageTypes member of the MAPM_Register_Server_Request_t*/
   /* message.                                                          */
#define MAPM_REGISTER_SERVER_SUPPORTED_MESSAGE_TYPE_EMAIL      0x00000001
#define MAPM_REGISTER_SERVER_SUPPORTED_MESSAGE_TYPE_SMS_GSM    0x00000002
#define MAPM_REGISTER_SERVER_SUPPORTED_MESSAGE_TYPE_SMS_CDMA   0x00000004
#define MAPM_REGISTER_SERVER_SUPPORTED_MESSAGE_TYPE_MMS        0x00000008

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Register a local MAP Server     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_REGISTER_SERVER                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Register_Server_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          InstanceID;
} MAPM_Register_Server_Response_t;

#define MAPM_REGISTER_SERVER_RESPONSE_SIZE                     (sizeof(MAPM_Register_Server_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Un-Register a Local server that */
   /* was previously registered (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * This message only allows Message Access Servers (MSE)    */
   /*          that were registered locally.  It does not apply to      */
   /*          notification servers.                                    */
typedef struct _tagMAPM_Un_Register_Server_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          InstanceID;
} MAPM_Un_Register_Server_Request_t;

#define MAPM_UN_REGISTER_SERVER_REQUEST_SIZE                   (sizeof(MAPM_Un_Register_Server_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Un-Register a Local server that */
   /* was previously registered (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Un_Register_Server_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Un_Register_Server_Response_t;

#define MAPM_UN_REGISTER_SERVER_RESPONSE_SIZE                  (sizeof(MAPM_Un_Register_Server_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Register an SDP Record for an   */
   /* already registered Message Access Server (Request).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD         */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * This message is only applicable to servers that were     */
   /*          registered as Message Access Servers (MSE).  It is not   */
   /*          applicable to notification servers.                      */
typedef struct _tagMAPM_Register_Service_Record_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          InstanceID;
   unsigned int          ServiceNameLength;
   char                  ServiceName[1];
} MAPM_Register_Service_Record_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager         */
   /* message to register a MAP Service Record given the length (in     */
   /* bytes) of the Message Access Service name.  This MACRO accepts the*/
   /* total number of individual UTF-8 characters (NULL terminated -    */
   /* including the NULL terminator) and returns the total number of    */
   /* bytes required to hold the entire message.                        */
#define MAPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(_x)          (STRUCTURE_OFFSET(MAPM_Register_Service_Record_Request_t, ServiceName) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Register an SDP Record for an   */
   /* already registered Message Access Server (Response).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Register_Service_Record_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned long         ServiceRecordHandle;
} MAPM_Register_Service_Record_Response_t;

#define MAPM_REGISTER_SERVICE_RECORD_RESPONSE_SIZE             (sizeof(MAPM_Register_Service_Record_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Un-Register an SDP Record that  */
   /* was previously registered (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVICE_RECORD      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Un_Register_Service_Record_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          InstanceID;
} MAPM_Un_Register_Service_Record_Request_t;

#define MAPM_UN_REGISTER_SERVICE_RECORD_REQUEST_SIZE           (sizeof(MAPM_Un_Register_Service_Record_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Un-Register an SDP Record that  */
   /* was previously registered (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVICE_RECORD      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Un_Register_Service_Record_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Un_Register_Service_Record_Response_t;

#define MAPM_UN_REGISTER_SERVICE_RECORD_RESPONSE_SIZE          (sizeof(MAPM_Un_Register_Service_Record_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Parse the Service Records of a  */
   /* remote Message Access Server device. (Request).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*        MAPM_MESSAGE_FUNCTION_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Parse_Remote_Message_Access_Services_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} MAPM_Parse_Remote_Message_Access_Services_Request_t;

#define MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_REQUEST_SIZE (sizeof(MAPM_Parse_Remote_Message_Access_Services_Request_t))

   /* The following structure represents the profile-specific data      */
   /* advertised for a single Message Access Server, as used in the     */
   /* MAPM_Parse_Remote_Message_Access_Services_Response_t message.     */
typedef struct _tagMAPM_Parse_Response_Service_Details_t
{
   unsigned int   ServerPort;
   unsigned int   InstanceID;
   unsigned long  SupportedMessageTypes;
   unsigned long  ServiceNameBytes;
} MAPM_Parse_Response_Service_Details_t;

   /* The following structure represents the Message definition for a   */
   /* Message Access Manager Message to Parse the Service Records of a  */
   /* remote Message Access Server device. (Response).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*        MAPM_MESSAGE_FUNCTION_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Parse_Remote_Message_Access_Services_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberServices;
   unsigned int          ReservedBufferLength;
   Byte_t                VariableData[1];
} MAPM_Parse_Remote_Message_Access_Services_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold an entire Message Access Manager    */
   /* message response to parse remote device service records given the */
   /* number of Service Detail structures and the length (in bytes)     */
   /* of the Reserved buffer. This MACRO accepts the total number of    */
   /* Service Detail structures and the length of the Reserved buffer in*/
   /* bytes (as used in the MAPM_Parsed_Message_Access_Service_Info_t   */
   /* structure) and returns the total number of bytes required to hold */
   /* the entire message.                                               */
#define MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_RESPONSE_SIZE(_x, _y) (STRUCTURE_OFFSET(MAPM_Parse_Remote_Message_Access_Services_Response_t, VariableData) + (sizeof(MAPM_Parse_Response_Service_Details_t) * (unsigned int)(_x)) + ((unsigned int)(_y)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to connect to a remote device      */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Connect_Remote_Device_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServerPort;
   unsigned int           InstanceID;
   unsigned long          ConnectionFlags;
} MAPM_Connect_Remote_Device_Request_t;

#define MAPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(MAPM_Connect_Remote_Device_Request_t))

   /* The following constants are used with the ConnectionFlags         */
   /* parameter of the MAPM_Connect_Remote_Device_Request_t message.    */
#define MAPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION    0x00000001
#define MAPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION        0x00000002

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to connect to a remote device    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Connect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          InstanceID;
} MAPM_Connect_Remote_Device_Response_t;

#define MAPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(MAPM_Connect_Remote_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to disconnect a remote device      */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_DISCONNECT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Disconnect_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
} MAPM_Disconnect_Request_t;

#define MAPM_DISCONNECT_REQUEST_SIZE                           (sizeof(MAPM_Disconnect_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to disconnect a remote device      */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_DISCONNECT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Disconnect_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Disconnect_Response_t;

#define MAPM_DISCONNECT_RESPONSE_SIZE                          (sizeof(MAPM_Disconnect_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to abort a current operation       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_ABORT                           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Abort_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
} MAPM_Abort_Request_t;

#define MAPM_ABORT_REQUEST_SIZE                                (sizeof(MAPM_Abort_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to abort a current operation       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_ABORT                           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Abort_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Abort_Response_t;

#define MAPM_ABORT_RESPONSE_SIZE                               (sizeof(MAPM_Abort_Response_t))

   /* Message Access Profile Manager (MAPM) Message Client Equipment    */
   /* (MCE) Command Messages.                                           */

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to query the current/working folder*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_QUERY_CURRENT_FOLDER            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Query_Current_Folder_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
} MAPM_Query_Current_Folder_Request_t;

#define MAPM_QUERY_CURRENT_FOLDER_REQUEST_SIZE                 (sizeof(MAPM_Query_Current_Folder_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to query the current/working folder*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_QUERY_CURRENT_FOLDER            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Query_Current_Folder_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          FolderNameLength;
   char                  FolderName[1];
} MAPM_Query_Current_Folder_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager message */
   /* response to query the current/working folder given the length (in */
   /* bytes) of the Message Access Folder name.  This MACRO accepts the */
   /* total number of individual UTF-8 characters (NULL terminated -    */
   /* including the NULL terminator) and returns the total number of    */
   /* bytes required to hold the entire message.                        */
#define MAPM_QUERY_CURRENT_FOLDER_RESPONSE_SIZE(_x)            (STRUCTURE_OFFSET(MAPM_Query_Current_Folder_Response_t, FolderName) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to connect to enable             */
   /* notifications (Request).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Enable_Notifications_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Enable;
} MAPM_Enable_Notifications_Request_t;

#define MAPM_ENABLE_NOTIFICATIONS_REQUEST_SIZE                 (sizeof(MAPM_Enable_Notifications_Request_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to connect to enable             */
   /* notifications (Response).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Enable_Notifications_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Enable_Notifications_Response_t;

#define MAPM_ENABLE_NOTIFICATIONS_RESPONSE_SIZE                (sizeof(MAPM_Enable_Notifications_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a folder listing.           */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Word_t                MaxListCount;
   Word_t                ListStartOffset;
} MAPM_Get_Folder_Listing_Request_t;

#define MAPM_GET_FOLDER_LISTING_REQUEST_SIZE                   (sizeof(MAPM_Get_Folder_Listing_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a folder listing.           */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Get_Folder_Listing_Response_t;

#define MAPM_GET_FOLDER_LISTING_RESPONSE_SIZE                  (sizeof(MAPM_Get_Folder_Listing_Response_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to get a folder listing size.    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Size_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
} MAPM_Get_Folder_Listing_Size_Request_t;

#define MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_SIZE              (sizeof(MAPM_Get_Folder_Listing_Size_Request_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to get a folder listing size.    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Get_Folder_Listing_Size_Response_t;

#define MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_SIZE             (sizeof(MAPM_Get_Folder_Listing_Size_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a message listing.          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Request_t
{
   BTPM_Message_Header_t      MessageHeader;
   BD_ADDR_t                  RemoteDeviceAddress;
   unsigned int               InstanceID;
   Word_t                     MaxListCount;
   Word_t                     ListStartOffset;
   Boolean_t                  ListingInfoPresent;
   MAP_Message_Listing_Info_t ListingInfo;
   unsigned int               FilterRecipientLength;
   unsigned int               FilterOriginatorLength;
   unsigned int               FolderNameLength;
   Byte_t                     VariableData[1];
} MAPM_Get_Message_Listing_Request_t;

   /* The following MACRO is used to calculate the size (in bytes)      */
   /* required to hold an entire MAPM_Get_Message_Listing_Request_t     */
   /* message given:                                                    */
   /*    - Number of bytes of the Filter Recipient (including NULL      */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Filter Originator (including NULL     */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Folder Name (including NULL           */
   /*      terminator).                                                 */
   /* In each of the cases, the variable data will contain the above    */
   /* three items packed in the VariableData member.  Each of the above */
   /* are ASCII UTF-8 character strings that are NULL terminated.       */
   /* This MACRO accepts as input the the length of each of the above   */
   /* mentioned members (including NULL terminator) and returns the     */
   /* total number of bytes required to hold the entire message.        */
#define MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(_x, _y, _z)      (STRUCTURE_OFFSET(MAPM_Get_Message_Listing_Request_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)) + ((unsigned int)(_z)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a message listing.          */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Get_Message_Listing_Response_t;

#define MAPM_GET_MESSAGE_LISTING_RESPONSE_SIZE                 (sizeof(MAPM_Get_Message_Listing_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a message listing size.     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Size_Request_t
{
   BTPM_Message_Header_t      MessageHeader;
   BD_ADDR_t                  RemoteDeviceAddress;
   unsigned int               InstanceID;
   Boolean_t                  ListingInfoPresent;
   MAP_Message_Listing_Info_t ListingInfo;
   unsigned int               FilterRecipientLength;
   unsigned int               FilterOriginatorLength;
   unsigned int               FolderNameLength;
   Byte_t                     VariableData[1];
} MAPM_Get_Message_Listing_Size_Request_t;

   /* The following MACRO is used to calculate the size (in bytes)      */
   /* required to hold an entire MAPM_Get_Message_Listing_Size_Request_t*/
   /* message given:                                                    */
   /*    - Number of bytes of the Filter Recipient (including NULL      */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Filter Originator (including NULL     */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Folder Name (including NULL           */
   /*      terminator).                                                 */
   /* In each of the cases, the variable data will contain the above    */
   /* three items packed in the VariableData member.  Each of the above */
   /* are ASCII UTF-8 character strings that are NULL terminated.       */
   /* This MACRO accepts as input the the length of each of the above   */
   /* mentioned members (including NULL terminator) and returns the     */
   /* total number of bytes required to hold the entire message.        */
#define MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(_x, _y, _z)    (STRUCTURE_OFFSET(MAPM_Get_Message_Listing_Size_Request_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)) + ((unsigned int)(_z)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a message listing size.     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Get_Message_Listing_Size_Response_t;

#define MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_SIZE            (sizeof(MAPM_Get_Message_Listing_Size_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a message from the server.  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Attachment;
   MAP_CharSet_t         CharSet;
   MAP_Fractional_Type_t FractionalType;
   char                  MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
} MAPM_Get_Message_Request_t;

#define MAPM_GET_MESSAGE_REQUEST_SIZE                          (sizeof(MAPM_Get_Message_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to get a message from the server.  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Get_Message_Response_t;

#define MAPM_GET_MESSAGE_RESPONSE_SIZE                         (sizeof(MAPM_Get_Message_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to set a message status.           */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Message_Status_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   MAP_Status_Indicator_t StatusIndicator;
   Boolean_t              StatusValue;
   char                   MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
} MAPM_Set_Message_Status_Request_t;

#define MAPM_SET_MESSAGE_STATUS_REQUEST_SIZE                   (sizeof(MAPM_Set_Message_Status_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to set a message status.           */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Message_Status_Response_t
{
   BTPM_Message_Header_t   MessageHeader;
   int                     Status;
} MAPM_Set_Message_Status_Response_t;

#define MAPM_SET_MESSAGE_STATUS_RESPONSE_SIZE                  (sizeof(MAPM_Set_Message_Status_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to push a message (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Push_Message_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Transparent;
   Boolean_t             Retry;
   MAP_CharSet_t         CharSet;
   unsigned int          FolderNameLength;
   unsigned int          MessageLength;
   Boolean_t             Final;
   Byte_t                VariableData[1];
} MAPM_Push_Message_Request_t;

   /* The following MACRO is used to calculate the size (in bytes)      */
   /* required to hold an entire MAPM_Push_Message_Request_t message    */
   /* given:                                                            */
   /*    - Number of bytes of the Folder Name (including NULL           */
   /*      terminator).                                                 */
   /*    - Number of bytes of raw message data (non-NULL binary data).  */
   /* In each of the cases, the variable data will contain the above    */
   /* two items packed in the VariableData member.  The Folder Name is  */
   /* an ASCII UTF-8 character strings that is NULL terminated.  The    */
   /* Message Data is treated as binary data (i.e. non-NULL terminated  */
   /* and can have embedded NULL's in it).  This MACRO accepts as input */
   /* the the length of each of the above mentioned members (including  */
   /* NULL terminator for the Folder Name) and returns the total number */
   /* of bytes required to hold the entire message.                     */
#define MAPM_PUSH_MESSAGE_REQUEST_SIZE(_x, _y)                 (STRUCTURE_OFFSET(MAPM_Push_Message_Request_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to push a message (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Push_Message_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Push_Message_Response_t;

#define MAPM_PUSH_MESSAGE_RESPONSE_SIZE                        (sizeof(MAPM_Push_Message_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to request an Inbox update.        */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UPDATE_INBOX                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Update_Inbox_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
} MAPM_Update_Inbox_Request_t;

#define MAPM_UPDATE_INBOX_REQUEST_SIZE                         (sizeof(MAPM_Update_Inbox_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to request an Inbox update.        */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UPDATE_INBOX                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Update_Inbox_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Update_Inbox_Response_t;

#define MAPM_UPDATE_INBOX_RESPONSE_SIZE                        (sizeof(MAPM_Update_Inbox_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to set the current folder.         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Request_t
{
   BTPM_Message_Header_t   MessageHeader;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            InstanceID;
   MAP_Set_Folder_Option_t PathOption;
   unsigned int            FolderNameLength;
   char                    FolderName[1];
} MAPM_Set_Folder_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager         */
   /* message to set the current relative folder given the length (in   */
   /* bytes) of the Message Access Folder name.  This MACRO accepts the */
   /* total number of individual UTF-8 characters (NULL terminated -    */
   /* including the NULL terminator) and returns the total number of    */
   /* bytes required to hold the entire message.                        */
#define MAPM_SET_FOLDER_REQUEST_SIZE(_x)                       (STRUCTURE_OFFSET(MAPM_Set_Folder_Request_t, FolderName) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to set the current folder.         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Set_Folder_Response_t;

#define MAPM_SET_FOLDER_RESPONSE_SIZE                          (sizeof(MAPM_Set_Folder_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to set the folder to an absolute   */
   /* path (Request).                                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER_ABSOLUTE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Absolute_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          FolderNameLength;
   char                  FolderName[1];
} MAPM_Set_Folder_Absolute_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager         */
   /* message to set the current absolute folder given the length (in   */
   /* bytes) of the Message Access Folder name.  This MACRO accepts the */
   /* total number of individual UTF-8 characters (NULL terminated -    */
   /* including the NULL terminator) and returns the total number of    */
   /* bytes required to hold the entire message.                        */
#define MAPM_SET_FOLDER_ABSOLUTE_REQUEST_SIZE(_x)              (STRUCTURE_OFFSET(MAPM_Set_Folder_Absolute_Request_t, FolderName) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to set the folder to an absolute   */
   /* path (Response).                                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER_ABSOLUTE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Absolute_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Set_Folder_Absolute_Response_t;

#define MAPM_SET_FOLDER_ABSOLUTE_RESPONSE_SIZE                 (sizeof(MAPM_Set_Folder_Absolute_Response_t))

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Command Messages.                                           */

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a request to enable  */
   /* notifications (Request).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_CONFIRMATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Enable_Notifications_Confirmation_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Enable_Notifications_Confirmation_Request_t;

#define MAPM_ENABLE_NOTIFICATIONS_CONFIRMATION_REQUEST_SIZE    (sizeof(MAPM_Enable_Notifications_Confirmation_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a request to enable  */
   /* notifications (Response).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_CONFIRMATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Enable_Notifications_Confirmation_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Enable_Notifications_Confirmation_Response_t;

#define MAPM_ENABLE_NOTIFICATIONS_CONFIRMATION_RESPONSE_SIZE   (sizeof(MAPM_Enable_Notifications_Confirmation_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a folder listing     */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Folder_Listing_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Boolean_t             Final;
   unsigned int          FolderListingLength;
   Byte_t                FolderListing[1];
} MAPM_Send_Folder_Listing_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager         */
   /* message to send a folder listing, given the length (in bytes) of  */
   /* the Message Access folder listing data.  This MACRO accepts the   */
   /* total number of individual bytes of folder listing bytes that are */
   /* to be included in the message and returns the total number of     */
   /* bytes required to hold the entire message.                        */
#define MAPM_SEND_FOLDER_LISTING_REQUEST_SIZE(_x)              (STRUCTURE_OFFSET(MAPM_Send_Folder_Listing_Request_t, FolderListing) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a folder listing     */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Folder_Listing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Send_Folder_Listing_Response_t;

#define MAPM_SEND_FOLDER_LISTING_RESPONSE_SIZE                 (sizeof(MAPM_Send_Folder_Listing_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a folder listing size*/
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING_SIZE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Folder_Listing_Size_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Word_t                FolderCount;
} MAPM_Send_Folder_Listing_Size_Request_t;

#define MAPM_SEND_FOLDER_LISTING_SIZE_REQUEST_SIZE             (sizeof(MAPM_Send_Folder_Listing_Size_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a folder listing size*/
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING_SIZE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Folder_Listing_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Send_Folder_Listing_Size_Response_t;

#define MAPM_SEND_FOLDER_LISTING_SIZE_RESPONSE_SIZE            (sizeof(MAPM_Send_Folder_Listing_Size_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a message listing    */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Message_Listing_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Word_t                MessageCount;
   Boolean_t             NewMessage;
   MAP_TimeDate_t        CurrentTime;
   Boolean_t             Final;
   unsigned int          MessageListingLength;
   Byte_t                MessageListing[1];
} MAPM_Send_Message_Listing_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager         */
   /* message to send a message listing, given the length (in bytes) of */
   /* the Message Access message listing data.  This MACRO accepts the  */
   /* total number of individual bytes of folder listing bytes that are */
   /* to be included in the message and returns the total number of     */
   /* bytes required to hold the entire message.                        */
#define MAPM_SEND_MESSAGE_LISTING_REQUEST_SIZE(_x)             (STRUCTURE_OFFSET(MAPM_Send_Message_Listing_Request_t, MessageListing) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a message listing    */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Message_Listing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Send_Message_Listing_Response_t;

#define MAPM_SEND_MESSAGE_LISTING_RESPONSE_SIZE                (sizeof(MAPM_Send_Message_Listing_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a message listing    */
   /* size request (Request).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING_SIZE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Message_Listing_Size_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Word_t                MessageCount;
   Boolean_t             NewMessage;
   MAP_TimeDate_t        CurrentTime;
} MAPM_Send_Message_Listing_Size_Request_t;

#define MAPM_SEND_MESSAGE_LISTING_SIZE_REQUEST_SIZE            (sizeof(MAPM_Send_Message_Listing_Size_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a message listing    */
   /* size request (Response).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING_SIZE       */
   /*                                                                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Message_Listing_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Send_Message_Listing_Size_Response_t;

#define MAPM_SEND_MESSAGE_LISTING_SIZE_RESPONSE_SIZE           (sizeof(MAPM_Send_Message_Listing_Size_Response_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to respond to a get message      */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_MESSAGE                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Message_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   MAP_Fractional_Type_t FractionalType;
   Boolean_t             Final;
   unsigned int          MessageDataLength;
   Byte_t                MessageData[1];
} MAPM_Send_Message_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager message */
   /* to send an individual message, given the length (in bytes) of the */
   /* Message Access message data.  This MACRO accepts the total number */
   /* of individual bytes of message data that are to be included in the*/
   /* message and returns the total number of bytes required to hold the*/
   /* entire message.                                                   */
#define MAPM_SEND_MESSAGE_REQUEST_SIZE(_x)                     (STRUCTURE_OFFSET(MAPM_Send_Message_Request_t, MessageData) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to respond to a get message      */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_MESSAGE                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Message_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Send_Message_Response_t;

#define MAPM_SEND_MESSAGE_RESPONSE_SIZE                        (sizeof(MAPM_Send_Message_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a set message status */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_MESSAGE_STATUS_CONFIRMATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Message_Status_Confirmation_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Message_Status_Confirmation_Request_t;

#define MAPM_MESSAGE_STATUS_CONFIRMATION_REQUEST_SIZE          (sizeof(MAPM_Message_Status_Confirmation_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a set message status */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_MESSAGE_STATUS_CONFIRMATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Message_Status_Confirmation_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Message_Status_Confirmation_Response_t;

#define MAPM_MESSAGE_STATUS_CONFIRMATION_RESPONSE_SIZE         (sizeof(MAPM_Message_Status_Confirmation_Response_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to respond to a push message     */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_CONFIRMATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Push_Message_Confirmation_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   char                  MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
} MAPM_Push_Message_Confirmation_Request_t;

#define MAPM_PUSH_MESSAGE_CONFIRMATION_REQUEST_SIZE            (sizeof(MAPM_Push_Message_Confirmation_Request_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to respond to a push message     */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_CONFIRMATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Push_Message_Confirmation_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Push_Message_Confirmation_Response_t;

#define MAPM_PUSH_MESSAGE_CONFIRMATION_RESPONSE_SIZE           (sizeof(MAPM_Push_Message_Confirmation_Response_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to respond to an update inbox    */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_CONFIRMATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Update_Inbox_Confirmation_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Update_Inbox_Confirmation_Request_t;

#define MAPM_UPDATE_INBOX_CONFIRMATION_REQUEST_SIZE            (sizeof(MAPM_Update_Inbox_Confirmation_Request_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message to respond to an update inbox    */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_CONFIRMATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Update_Inbox_Confirmation_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Update_Inbox_Confirmation_Response_t;

#define MAPM_UPDATE_INBOX_CONFIRMATION_RESPONSE_SIZE           (sizeof(MAPM_Update_Inbox_Confirmation_Response_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a set folder request */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER_CONFIRMATION         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Confirmation_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Set_Folder_Confirmation_Request_t;

#define MAPM_SET_FOLDER_CONFIRMATION_REQUEST_SIZE              (sizeof(MAPM_Set_Folder_Confirmation_Request_t))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to respond to a set folder request */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER_CONFIRMATION         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Confirmation_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Set_Folder_Confirmation_Response_t;

#define MAPM_SET_FOLDER_CONFIRMATION_RESPONSE_SIZE             (sizeof(MAPM_Set_Folder_Confirmation_Response_t))

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Notification Commands.                                      */

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to send an event request (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_NOTIFICATION               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Notification_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Final;
   unsigned int          EventDataLength;
   Byte_t                EventData[1];
} MAPM_Send_Notification_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager         */
   /* message to send a notificiation event given the length (in bytes) */
   /* of the Message Access Event Data.  This MACRO accepts the total   */
   /* number of individual event data bytes that are to be included in  */
   /* the message and returns the total number of bytes required to hold*/
   /* the entire message.                                               */
#define MAPM_SEND_NOTIFICATION_REQUEST_SIZE(_x)                (STRUCTURE_OFFSET(MAPM_Send_Notification_Request_t, EventData) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Message Access Manager message to send an event request           */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SEND_NOTIFICATION               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Send_Notification_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} MAPM_Send_Notification_Response_t;

#define MAPM_SEND_NOTIFICATION_RESPONSE_SIZE                   (sizeof(MAPM_Send_Notification_Response_t))

   /* Message Access Manager Asynchronous Events.                       */

   /* Message Access Profile Manager (MAPM) Asynchronous Events         */
   /* Common/Connection Asynchronous Events.                            */

   /* The following structure holds the information delivered to the    */
   /* PM client when a connection request has been received.            */
   /* The RemoteDeviceAddress member specifies the Bluetooth            */
   /* device address of the remote device.                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Connection_Request_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
} MAPM_Connection_Request_Message_t;

#define MAPM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(MAPM_Connection_Request_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of a     */
   /* connected device (asynchronously).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_DEVICE_CONNECTED                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Device_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
} MAPM_Device_Connected_Message_t;

#define MAPM_DEVICE_CONNECTED_MESSAGE_SIZE                     (sizeof(MAPM_Device_Connected_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of a     */
   /* disconnection (asynchronously).                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Device_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
} MAPM_Device_Disconnected_Message_t;

#define MAPM_DEVICE_DISCONNECTED_MESSAGE_SIZE                  (sizeof(MAPM_Device_Disconnected_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of a     */
   /* connection status (asynchronously).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_CONNECTION_STATUS               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Connection_Status_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   unsigned int           ConnectionStatus;
} MAPM_Connection_Status_Message_t;

#define MAPM_CONNECTION_STATUS_MESSAGE_SIZE                    (sizeof(MAPM_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the MAPM_Connection_Status_Message_t message to describe the   */
   /* actual Connection Result Status.                                  */
#define MAPM_CONNECTION_STATUS_SUCCESS                         0x00000000
#define MAPM_CONNECTION_STATUS_FAILURE_TIMEOUT                 0x00000001
#define MAPM_CONNECTION_STATUS_FAILURE_REFUSED                 0x00000002
#define MAPM_CONNECTION_STATUS_FAILURE_SECURITY                0x00000003
#define MAPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF        0x00000004
#define MAPM_CONNECTION_STATUS_FAILURE_UNKNOWN                 0x00000005

   /* Message Access Profile Manager (MAPM) Message Client Equipment    */
   /* (MCE) Asynchronous Events.                                        */

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of the   */
   /* status of a request to enable notifications (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_RESPONSE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Enable_Notifications_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Enable_Notifications_Response_Message_t;

#define MAPM_ENABLE_NOTIFICATIONS_RESPONSE_MESSAGE_SIZE        (sizeof(MAPM_Enable_Notifications_Response_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of an    */
   /* folder listing response (asynchronously).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Boolean_t             Final;
   unsigned int          FolderListingLength;
   Byte_t                FolderListing[1];
} MAPM_Get_Folder_Listing_Response_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager response*/
   /* message for a folder listing, given the length (in bytes) of the  */
   /* Message Access folder listing data.  This MACRO accepts the total */
   /* number of individual bytes of folder listing bytes that are to be */
   /* included in the message and returns the total number of bytes     */
   /* required to hold the entire message.                              */
#define MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(_x)      (STRUCTURE_OFFSET(MAPM_Get_Folder_Listing_Response_Message_t, FolderListing) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of an    */
   /* get folder listing size response (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_RESPONSE*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Size_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Word_t                NumberOfFolders;
} MAPM_Get_Folder_Listing_Size_Response_Message_t;

#define MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_MESSAGE_SIZE     (sizeof(MAPM_Get_Folder_Listing_Size_Response_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of an    */
   /* message listing response (asynchronously).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_MESSAGE_LISTING_RESPONSE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Word_t                MessageCount;
   Boolean_t             NewMessage;
   MAP_TimeDate_t        MSETime;
   Boolean_t             Final;
   unsigned int          MessageListingLength;
   Byte_t                MessageListing[1];
} MAPM_Get_Message_Listing_Response_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager response*/
   /* message to send a message listing, given the length (in bytes) of */
   /* the Message Access message listing data.  This MACRO accepts the  */
   /* total number of individual bytes of folder listing bytes that are */
   /* to be included in the message and returns the total number of     */
   /* bytes required to hold the entire message.                        */
#define MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(_x)     (STRUCTURE_OFFSET(MAPM_Get_Message_Listing_Response_Message_t, MessageListing) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of an    */
   /* message listing size response (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_MESSAGE_LISTING_SIZE_RESPONSE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Size_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   Word_t                MessageCount;
   Boolean_t             NewMessage;
   MAP_TimeDate_t        CurrentTime;
} MAPM_Get_Message_Listing_Size_Response_Message_t;

#define MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_MESSAGE_SIZE    (sizeof(MAPM_Get_Message_Listing_Size_Response_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of an    */
   /* message response (asynchronously).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_RESPONSE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* Boolean_t             Final; not in bluetopia                     */
typedef struct _tagMAPM_Get_Message_Response_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   unsigned int           ResponseStatusCode;
   MAP_Fractional_Type_t  FractionalType;
   Boolean_t              Final;
   unsigned int           MessageDataLength;
   Byte_t                 MessageData[1];
} MAPM_Get_Message_Response_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager response*/
   /* message to query an individual message, given the length (in      */
   /* bytes) of the Message Access message data.  This MACRO accepts the*/
   /* total number of individual bytes of message data that are to be   */
   /* included in the message and returns the total number of bytes     */
   /* required to hold the entire message.                              */
#define MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(_x)             (STRUCTURE_OFFSET(MAPM_Get_Message_Response_Message_t, MessageData) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of a     */
   /* set message status response. (asynchronously).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Message_Status_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Set_Message_Status_Response_Message_t;

#define MAPM_SET_MESSAGE_STATUS_RESPONSE_MESSAGE_SIZE          (sizeof(MAPM_Set_Message_Status_Response_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of a     */
   /* push message response (asynchronously).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Push_Message_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   char                  MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
} MAPM_Push_Message_Response_Message_t;

#define MAPM_PUSH_MESSAGE_RESPONSE_MESSAGE_SIZE                (sizeof(MAPM_Push_Message_Response_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of an    */
   /* update inbox response (asynchronously).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Update_Inbox_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Update_Inbox_Response_Message_t;

#define MAPM_UPDATE_INBOX_RESPONSE_MESSAGE_SIZE                (sizeof(MAPM_Update_Inbox_Response_Message_t))

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of a     */
   /* set folder response (asynchronously).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER_RESPONSE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
   unsigned int          CurrentPathLength;
   char                  CurrentPath[1];
} MAPM_Set_Folder_Response_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager response*/
   /* message for a Set Folder Response given the length (in bytes) of  */
   /* the current path.  This MACRO accepts the total number of         */
   /* individual UTF-8 characters (NULL terminated - including the NULL */
   /* terminator) and returns the total number of bytes required to hold*/
   /* the entire message.                                               */
#define MAPM_SET_FOLDER_RESPONSE_MESSAGE_SIZE(_x)              (STRUCTURE_OFFSET(MAPM_Set_Folder_Response_Message_t, CurrentPath) + (unsigned int)(_x))

   /* Message Access Profile Manager (MAPM) Message Client Equipment    */
   /* (MCE) Notification Asynchronous Events.                           */

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the client of an    */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_NOTIFICATION_INDICATION         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Notification_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Final;
   unsigned int          EventDataLength;
   Byte_t                EventData[1];
} MAPM_Notification_Indication_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Message Access Manager event   */
   /* notification message given the length (in bytes) of the Message   */
   /* Access Event Data.  This MACRO accepts the total number of        */
   /* individual event data bytes that are to be included in the message*/
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define MAPM_NOTIFICATION_INDICATION_MESSAGE_SIZE(_x)          (STRUCTURE_OFFSET(MAPM_Notification_Indication_Message_t, EventData) + ((unsigned int)(_x)))

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Asynchronous Events.                                        */

   /* The following structure holds the information delivered to the PM */
   /* client of the MAS server when an Enable Notifications Request     */
   /* Indication has been received.  The RemoteDeviceAddress member     */
   /* equals the Bluetooth device address of the MCE device.  The       */
   /* InstanceID member is equal to the MAS instance ID.  The Enabled   */
   /* member indicates whether message notifications should be enabled  */
   /* or disabled.                                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_REQUEST    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Enable_Notifications_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Enable;
} MAPM_Enable_Notifications_Request_Message_t;

#define MAPM_ENABLE_NOTIFICATIONS_REQUEST_MESSAGE_SIZE         (sizeof(MAPM_Enable_Notifications_Request_Message_t))

   /* The following structure holds the information delivered to the    */
   /* PM client of the MAS server when a Folder Listing request has     */
   /* been received. The RemoteDeviceAddress member equals the          */
   /* Bluetooth device address of the MCE device. The InstanceID        */
   /* member is equal to the MAS instance ID. MaxlistCount determines   */
   /* the maximum number of listings returned. ListStartOffset          */
   /* indicates where a partial listing should begin in the whole       */
   /* listing.                                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_REQUEST      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Word_t                MaxListCount;
   Word_t                ListStartOffset;
} MAPM_Get_Folder_Listing_Request_Message_t;

#define MAPM_GET_FOLDER_LISTING_REQUEST_MESSAGE_SIZE           (sizeof(MAPM_Get_Folder_Listing_Request_Message_t))

   /* The following structure holds the information delivered to the PM */
   /* client of the MAS server when a Folder Listing Size request has   */
   /* been received.  The RemoteDeviceAddress member equals the         */
   /* Bluetooth device address of the MCE device.  The InstanceID member*/
   /* is equal to the MAS instance ID.                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_REQUEST */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Size_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
} MAPM_Get_Folder_Listing_Size_Request_Message_t;

#define MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_MESSAGE_SIZE      (sizeof(MAPM_Get_Folder_Listing_Size_Request_Message_t))

   /* The following structure holds the information delivered to the    */
   /* PM client of the MAS server when a Message Listing request has    */
   /* been received.                                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_REQUEST     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Request_Message_t
{
   BTPM_Message_Header_t      MessageHeader;
   BD_ADDR_t                  RemoteDeviceAddress;
   unsigned int               InstanceID;
   Word_t                     MaxListCount;
   Word_t                     ListStartOffset;
   MAP_Message_Listing_Info_t ListingInfo;
   unsigned int               FilterRecipientLength;
   unsigned int               FilterOriginatorLength;
   unsigned int               FolderNameLength;
   Byte_t                     VariableData[1];
} MAPM_Get_Message_Listing_Request_Message_t;

   /* The following MACRO is used to calculate the size (in bytes)      */
   /* required to hold an entire                                        */
   /* MAPM_Get_Message_Listing_Request_Message_t message given:         */
   /*    - Number of bytes of the Filter Recipient (including NULL      */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Filter Originator (including NULL     */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Folder Name (including NULL           */
   /*      terminator).                                                 */
   /* In each of the cases, the variable data will contain the above    */
   /* three items packed in the VariableData member.  Each of the above */
   /* are ASCII UTF-8 character strings that are NULL terminated.       */
   /* This MACRO accepts as input the the length of each of the above   */
   /* mentioned members (including NULL terminator) and returns the     */
   /* total number of bytes required to hold the entire message.        */
#define MAPM_GET_MESSAGE_LISTING_REQUEST_MESSAGE_SIZE(_x, _y, _z) (STRUCTURE_OFFSET(MAPM_Get_Message_Listing_Request_Message_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)) + ((unsigned int)(_z)))

   /* The following structure holds the information delivered to the PM */
   /* client of the MAS server when a Message Listing Size request has  */
   /* been received.                                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_REQUEST     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Listing_Size_Request_Message_t
{
   BTPM_Message_Header_t      MessageHeader;
   BD_ADDR_t                  RemoteDeviceAddress;
   unsigned int               InstanceID;
   MAP_Message_Listing_Info_t ListingInfo;
   unsigned int               FilterRecipientLength;
   unsigned int               FilterOriginatorLength;
   unsigned int               FolderNameLength;
   Byte_t                     VariableData[1];
} MAPM_Get_Message_Listing_Size_Request_Message_t;

   /* The following MACRO is used to calculate the size (in bytes)      */
   /* required to hold an entire                                        */
   /* MAPM_Get_Message_Listing_Size_Request_Message_t message given:    */
   /*    - Number of bytes of the Filter Recipient (including NULL      */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Filter Originator (including NULL     */
   /*      terminator).                                                 */
   /*    - Number of bytes of the Folder Name (including NULL           */
   /*      terminator).                                                 */
   /* In each of the cases, the variable data will contain the above    */
   /* three items packed in the VariableData member.  Each of the above */
   /* are ASCII UTF-8 character strings that are NULL terminated.       */
   /* This MACRO accepts as input the the length of each of the above   */
   /* mentioned members (including NULL terminator) and returns the     */
   /* total number of bytes required to hold the entire message.        */
#define MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_MESSAGE_SIZE(_x, _y, _z)  (STRUCTURE_OFFSET(MAPM_Get_Message_Listing_Size_Request_Message_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)) + ((unsigned int)(_z)))

   /* The following structure holds the information delivered to the    */
   /* PM client of the MAS server when a Get Message request            */
   /* has been received. The RemoteDeviceAddress member equals the      */
   /* Bluetooth device address of the MCE device. The InstanceID        */
   /* member is equal to the MAS instance ID. The Attachment member     */
   /* indicates whether attachments should be included in the response. */
   /* CharSet indicates how the text portion of the message shall be    */
   /* coded. The FractionalType member is for use when the message      */
   /* requested is a fractioned email. This member is used to request   */
   /* the first or the next fraction. The MessageHandle member is the   */
   /* message identifier.                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_GET_MESSAGE_REQUEST             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Get_Message_Request_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   Boolean_t              Attachment;
   MAP_CharSet_t          CharSet;
   MAP_Fractional_Type_t  FractionalType;
   char                   MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
} MAPM_Get_Message_Request_Message_t;

#define MAPM_GET_MESSAGE_REQUEST_MESSAGE_SIZE                  (sizeof(MAPM_Get_Message_Request_Message_t))

   /* The following structure holds the information delivered to the    */
   /* PM client of the MAS server when a Set Message Status request     */
   /* has been received. The RemoteDeviceAddress member equals the      */
   /* Bluetooth device address of the MCE device. The InstanceID        */
   /* member is equal to the MAS instance ID. The MessageHandle member  */
   /* is the message identifier. The StatusIndicator dictates whether   */
   /* the read or deleted status will be set. The StatusValue           */
   /* whether the status type in the indicator member will be set or    */
   /* unset.                                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_REQUEST      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Message_Status_Request_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   char                   MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
   MAP_Status_Indicator_t StatusIndicator;
   Boolean_t              StatusValue;
} MAPM_Set_Message_Status_Request_Message_t;

#define MAPM_SET_MESSAGE_STATUS_REQUEST_MESSAGE_SIZE           (sizeof(MAPM_Set_Message_Status_Request_Message_t))

   /* The following structure holds the information delivered to the PM */
   /* client of the MAS server when a Push Message request has been     */
   /* received.                                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_MESSAGE_PUSH_MESSAGE_REQUEST    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Push_Message_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Transparent;
   Boolean_t             Retry;
   MAP_CharSet_t         CharSet;
   unsigned int          FolderNameLength;
   unsigned int          MessageLength;
   Boolean_t             Final;
   Byte_t                VariableData[1];
} MAPM_Push_Message_Request_Message_t;

   /* The following MACRO is used to calculate the size (in bytes)      */
   /* required to hold an entire MAPM_Push_Message_Request_Message_t    */
   /* message given:                                                    */
   /*    - Number of bytes of the Folder Name (including NULL           */
   /*      terminator).                                                 */
   /*    - Number of bytes of raw message data (non-NULL binary data).  */
   /* In each of the cases, the variable data will contain the above    */
   /* two items packed in the VariableData member.  The Folder Name is  */
   /* an ASCII UTF-8 character strings that is NULL terminated.  The    */
   /* Message Data is treated as binary data (i.e. non-NULL terminated  */
   /* and can have embedded NULL's in it).  This MACRO accepts as input */
   /* the the length of each of the above mentioned members (including  */
   /* NULL terminator for the Folder Name) and returns the total number */
   /* of bytes required to hold the entire message.                     */
#define MAPM_PUSH_MESSAGE_REQUEST_MESSAGE_SIZE(_x, _y)         (STRUCTURE_OFFSET(MAPM_Push_Message_Request_Message_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)))

   /* The following structure holds the information delivered to the    */
   /* PM client of the MAS server when a Update Inbox request           */
   /* has been received. The RemoteDeviceAddress member equals the      */
   /* Bluetooth device address of the MCE device. The InstanceID        */
   /* member is equal to the MAS instance ID.                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_REQUEST            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Update_Inbox_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
} MAPM_Update_Inbox_Request_Message_t;

#define MAPM_UPDATE_INBOX_REQUEST_MESSAGE_SIZE                 (sizeof(MAPM_Update_Inbox_Request_Message_t))

   /* The following structure holds the information delivered to the    */
   /* PM client of the MAS server when a Set Folder request             */
   /* has been received. The RemoteDeviceAddress member equals the      */
   /* Bluetooth device address of the MCE device. The InstanceID        */
   /* member is equal to the MAS instance ID. PathOption determines     */
   /* how the directory tree will be navigated. The FolderName member   */
   /* contains the name of the child directory of the current directory */
   /* if PathOption equals down or the name of another child directory  */
   /* of the current parent directory if PathOption equals up.          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_SET_FOLDER_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Set_Folder_Request_Message_t
{
   BTPM_Message_Header_t   MessageHeader;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            InstanceID;
   MAP_Set_Folder_Option_t PathOption;
   unsigned int            FolderNameLength;
   unsigned int            NewPathLength;
   char                    VariableData[1];
} MAPM_Set_Folder_Request_Message_t;

   /* The following MACRO is used to calculate the size (in bytes)      */
   /* required to hold an entire MAPM_Get_Message_Listing_Request_t     */
   /* message given:                                                    */
   /*    - Number of bytes of the Folder Name (including NULL           */
   /*      terminator).                                                 */
   /*    - Number of bytes of the New Path (including the NULL          */
   /*      terminator).                                                 */
   /* In each of the cases, the variable data will contain the above    */
   /* two items packed in the VariableData member.  Each of the above   */
   /* are ASCII UTF-8 character strings that are NULL terminated.       */
   /* This MACRO accepts as input the the length of each of the above   */
   /* mentioned members (including NULL terminator) and returns the     */
   /* total number of bytes required to hold the entire message.        */
#define MAPM_SET_FOLDER_REQUEST_MESSAGE_SIZE(_x, _y)           (STRUCTURE_OFFSET(MAPM_Set_Folder_Request_Message_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)))

   /* Message Access Profile Manager (MAPM) Message Server Equipment    */
   /* (MSE) Notification Asynchronous Events.                           */

   /* The following structure represents the message definition for     */
   /* a Message Access Manager message that informs the server of a     */
   /* notification confirmation response. (asynchronously).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             MAPM_MESSAGE_FUNCTION_NOTIFICATION_CONFIRMATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagMAPM_Notification_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          ResponseStatusCode;
} MAPM_Notification_Confirmation_Message_t;

#define MAPM_NOTIFICATION_CONFIRMATION_MESSAGE_SIZE            (sizeof(MAPM_Notification_Confirmation_Message_t))

#endif
