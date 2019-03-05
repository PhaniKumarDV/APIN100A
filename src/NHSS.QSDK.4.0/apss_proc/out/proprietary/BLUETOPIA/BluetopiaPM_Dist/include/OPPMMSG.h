/*****< oppmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OPPMMSG - Defined Interprocess Communication Messages for the Object      */
/*            Push Profile Manager (OPPM) for Stonestreet One Bluetopia       */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/09/13  M. Seabold     Initial creation                                */
/******************************************************************************/
#ifndef __OPPMMSGH__
#define __OPPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions            */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants  */

#include "SS1BTOPP.h"            /* Object Push Prototypes/Constants          */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants   */

#include "OPPMType.h"            /* BTPM OPP Manager Type Definitions         */

   /* The following message group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Object Push     */
   /* (OPP) Manager.                                                    */
#define BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER                    0x0000100A

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message functions that are valid for the Object Push (OPP)*/
   /* Manager.                                                          */

   /* Object Push Profile Manager (OPPM) Commands.                      */
#define OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE         0x00001001
#define OPPM_MESSAGE_FUNCTION_REGISTER_SERVER                     0x00001002
#define OPPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER                  0x00001003
#define OPPM_MESSAGE_FUNCTION_PARSE_REMOTE_OBJECT_PUSH_SERVICES   0x00001004
#define OPPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE               0x00001005
#define OPPM_MESSAGE_FUNCTION_DISCONNECT                          0x00001006
#define OPPM_MESSAGE_FUNCTION_ABORT                               0x00001007
#define OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_REQUEST            0x00001008
#define OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_RESPONSE           0x00001009
#define OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_REQUEST     0x0000100A
#define OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_RESPONSE    0x0000100B

   /* Object Push Profile Manager (OPPM) Asynchronous Events.           */
#define OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST                  0x00010000
#define OPPM_MESSAGE_FUNCTION_CONNECTED                           0x00010001
#define OPPM_MESSAGE_FUNCTION_DISCONNECTED                        0x00010002
#define OPPM_MESSAGE_FUNCTION_CONNECTION_STATUS                   0x00010003
#define OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_REQUEST                 0x00010004
#define OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_RESPONSE                0x00010005
#define OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_REQUEST          0x00010006
#define OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_RESPONSE         0x00010007

   /* Object Push Profile Manager (OPPM) Commands.                      */

   /* The following structure represents the Message definition         */
   /* for a Object Push Manager Message to Respond to an incoming       */
   /* Connection/Authorization (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Accept;
} OPPM_Connection_Request_Response_Request_t;

#define OPPM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(OPPM_Connection_Request_Response_Request_t))

   /* The following structure represents the Message definition         */
   /* for a Object Push Manager Message to Respond to an incoming       */
   /* Connection/Authorization (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Connection_Request_Response_Response_t;

#define OPPM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(OPPM_Connection_Request_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to Register an Object Push Server     */
   /* port.  (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_REGISTER_SERVER                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Register_Server_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerPort;
   unsigned long         SupportedObjectTypes;
   unsigned long         IncomingConnectionFlags;
   unsigned int          ServiceNameLength;
   char                  ServiceName[1];
} OPPM_Register_Server_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Object Push Manager message    */
   /* to register a OPP Server given the length (in bytes) of the       */
   /* Object Push Service name.  This MACRO accepts the total number of */
   /* individual UTF-8 characters (NULL terminated - including the NULL */
   /* terminator) and returns the total number of bytes required to hold*/
   /* the entire message.                                               */
#define OPPM_REGISTER_SERVER_REQUEST_SIZE(_x)                  (STRUCTURE_OFFSET(OPPM_Register_Server_Request_t, ServiceName) + ((unsigned int)(_x)))

   /* The following constants are used with the IncomingConnectionFlags */
   /* member of the OPPM_Register_Server_Request_t message to control   */
   /* incoming connection request options for the registered port.      */
#define OPPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHORIZATION       0x00000001
#define OPPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION      0x00000002
#define OPPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION          0x00000004

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to Register an Object Push Server     */
   /* port.  (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_REGISTER_SERVER                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Register_Server_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Register_Server_Response_t;

#define OPPM_REGISTER_SERVER_RESPONSE_SIZE                     (sizeof(OPPM_Register_Server_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to Un-Register an Object Push Server  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Un_Register_Server_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
} OPPM_Un_Register_Server_Request_t;

#define OPPM_UN_REGISTER_SERVER_REQUEST_SIZE                   (sizeof(OPPM_Un_Register_Server_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to Un-Register an Object Push Server  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Un_Register_Server_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Un_Register_Server_Response_t;

#define OPPM_UN_REGISTER_SERVER_RESPONSE_SIZE                  (sizeof(OPPM_Un_Register_Server_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to connect to a remote Object Push    */
   /* Server (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Connect_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          RemoteServerPort;
   unsigned long         ConnectionFlags;
} OPPM_Connect_Remote_Device_Request_t;

#define OPPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(OPPM_Connect_Remote_Device_Request_t))

   /* The following constants are used with the ConnectionFlags         */
   /* parameter of the OPPM_Connect_Remote_Device_Request_t message.    */
#define OPPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION    0x00000001
#define OPPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION        0x00000002

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to connect to a remote Object Push    */
   /* Server (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Connect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Connect_Remote_Device_Response_t;

#define OPPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(OPPM_Connect_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to disconnect from a remote Object    */
   /* Push Server.  (Request).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_DISCONNECT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Disconnect_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortID;
} OPPM_Disconnect_Request_t;

#define OPPM_DISCONNECT_REQUEST_SIZE                           (sizeof(OPPM_Disconnect_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to disconnect from a remote Object    */
   /* Push Server.  (Response).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_DISCONNECT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Disconnect_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Disconnect_Response_t;

#define OPPM_DISCONNECT_RESPONSE_SIZE                          (sizeof(OPPM_Disconnect_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to abort an outstanding OPP request   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_ABORT                           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Abort_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientID;
} OPPM_Abort_Request_t;

#define OPPM_ABORT_REQUEST_SIZE                                (sizeof(OPPM_Abort_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to abort an outstanding OPP request   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_ABORT                           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Abort_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Abort_Response_t;

#define OPPM_ABORT_RESPONSE_SIZE                               (sizeof(OPPM_Abort_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Push Object Request to an   */
   /* Object Push Server (Request).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_REQUEST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Push_Object_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientID;
   OPPM_Object_Type_t    ObjectType;
   unsigned long         ObjectTotalLength;
   Boolean_t             Final;
   unsigned int          ObjectNameLength;
   unsigned int          DataLength;
   Byte_t                VariableData[1];
} OPPM_Push_Object_Request_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Object Push Manager message to */
   /* send an OPP Push Object Request given the length (in bytes) of the*/
   /* Object name and the object data.  This MACRO accepts the total    */
   /* number of individual UTF-8 characters (NULL terminated - including*/
   /* the NULL terminator) as well as the total length of the object    */
   /* data and returns the total number of bytes required to hold the   */
   /* entire message.                                                   */
#define OPPM_PUSH_OBJECT_REQUEST_REQUEST_SIZE(_x, _y)          (STRUCTURE_OFFSET(OPPM_Push_Object_Request_Request_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Push Object Request to an   */
   /* Object Push Server (Response).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_REQUEST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Push_Object_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Push_Object_Request_Response_t;

#define OPPM_PUSH_OBJECT_REQUEST_RESPONSE_SIZE                 (sizeof(OPPM_Push_Object_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Push Object Response to a   */
   /* remote Object Push Client (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_RESPONSE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Push_Object_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   unsigned int          ResponseCode;
} OPPM_Push_Object_Response_Request_t;

#define OPPM_PUSH_OBJECT_RESPONSE_REQUEST_SIZE                 (sizeof(OPPM_Push_Object_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Push Object Response to a   */
   /* remote Object Push Client (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_RESPONSE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Push_Object_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Push_Object_Response_Response_t;

#define OPPM_PUSH_OBJECT_RESPONSE_RESPONSE_SIZE                (sizeof(OPPM_Push_Object_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Pull Business Card Request  */
   /* to a remote Object Push Server (Request).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_REQUEST */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Pull_Business_Card_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientID;
} OPPM_Pull_Business_Card_Request_Request_t;

#define OPPM_PULL_BUSINESS_CARD_REQUEST_REQUEST_SIZE           (sizeof(OPPM_Pull_Business_Card_Request_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Pull Business Card Request  */
   /* to a remote Object Push Server (Response).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_REQUEST */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Pull_Business_Card_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Pull_Business_Card_Request_Response_t;

#define OPPM_PULL_BUSINESS_CARD_REQUEST_RESPONSE_SIZE          (sizeof(OPPM_Pull_Business_Card_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Pull Business Card Response */
   /* to a remote Object Push Client (Request).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_RESPONSE*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Pull_Business_Card_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   unsigned int          ResponseCode;
   unsigned long         ObjectTotalLength;
   Boolean_t             Final;
   unsigned int          DataLength;
   Byte_t                DataBuffer[1];
} OPPM_Pull_Business_Card_Response_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Object Push Manager message    */
   /* to send an OPP Pull Business Card Response given the length (in   */
   /* bytes) of the object data.  This MACRO accepts the total number of*/
   /* bytes contained in the object data and returns the total number of*/
   /* bytes required to hold the entire message.                        */
#define OPPM_PULL_BUSINESS_CARD_RESPONSE_REQUEST_SIZE(_x)      (STRUCTURE_OFFSET(OPPM_Pull_Business_Card_Response_Request_t, DataBuffer) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* Object Push Manager Message to send a Pull Business Card Response */
   /* to a remote Object Push Client (Response).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_RESPONSE*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Pull_Business_Card_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} OPPM_Pull_Business_Card_Response_Response_t;

#define OPPM_PULL_BUSINESS_CARD_RESPONSE_RESPONSE_SIZE         (sizeof(OPPM_Pull_Business_Card_Response_Response_t))

   /* Object Push Manager Asynchronous Events.                          */

   /* The following structure represents the message definition for a   */
   /* Object Push Manager message that informs the client of a incoming */
   /* connection request (asynchronously).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Connection_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   BD_ADDR_t             RemoteDeviceAddress;
} OPPM_Connection_Request_Message_t;

#define OPPM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(OPPM_Connection_Request_Message_t))

   /* The following structure represents the message definition for a   */
   /* Object Push Manager message that informs the client of a device   */
   /* connection (asynchronously).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   BD_ADDR_t             RemoteDeviceAddress;
} OPPM_Connected_Message_t;

#define OPPM_CONNECTED_MESSAGE_SIZE                            (sizeof(OPPM_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Object Push Manager message that informs the client of a device   */
   /* disconnection (asynchronously).                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Server;
} OPPM_Disconnected_Message_t;

#define OPPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(OPPM_Disconnected_Message_t))

   /* The following structure represents the message definition for     */
   /* a Object Push Manager message that informs the client of a        */
   /* connection status (asynchronously).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_CONNECTION_STATUS               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          Status;
} OPPM_Connection_Status_Message_t;

#define OPPM_CONNECTION_STATUS_MESSAGE_SIZE                    (sizeof(OPPM_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the OPPM_Connection_Status_Message_t message to describe the   */
   /* actual Connection Result Status.                                  */
#define OPPM_CONNECTION_STATUS_SUCCESS                         0x00000000
#define OPPM_CONNECTION_STATUS_FAILURE_TIMEOUT                 0x00000001
#define OPPM_CONNECTION_STATUS_FAILURE_REFUSED                 0x00000002
#define OPPM_CONNECTION_STATUS_FAILURE_SECURITY                0x00000003
#define OPPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF        0x00000004
#define OPPM_CONNECTION_STATUS_FAILURE_UNKNOWN                 0x00000005

   /* The following structure represents the message definition for a   */
   /* Object Push Manager message that informs the client of a Push     */
   /* Object Request (asynchronously).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_REQUEST             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Push_Object_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   BD_ADDR_t             RemoteDeviceAddress;
   OPPM_Object_Type_t    ObjectType;
   unsigned long         ObjectTotalLength;
   Boolean_t             Final;
   unsigned int          ObjectNameLength;
   unsigned int          DataLength;
   Byte_t                VariableData[1];
} OPPM_Push_Object_Request_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold an entire Object Push Manager Push Object*/
   /* Request message to send an OPP Pull Business Card Response given  */
   /* the length (in bytes) of the object name and the object data.     */
   /* This MACRO accepts the total number of bytes contained in the     */
   /* object data as well as the total length of the UTF-8 string Object*/
   /* Name (including the null-terminator) and returns the total number */
   /* of bytes required to hold the entire message.                     */
#define OPPM_PUSH_OBJECT_REQUEST_MESSAGE_SIZE(_x,_y)           (STRUCTURE_OFFSET(OPPM_Push_Object_Request_Message_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)))

   /* The following structure represents the message definition for a   */
   /* Object Push Manager message that informs the client of a Push     */
   /* Object Response (asynchronously).                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_RESPONSE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Push_Object_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ResponseCode;
} OPPM_Push_Object_Response_Message_t;

#define OPPM_PUSH_OBJECT_RESPONSE_MESSAGE_SIZE                 (sizeof(OPPM_Push_Object_Response_Message_t))

   /* The following structure represents the message definition for a   */
   /* Object Push Manager message that informs the client of a Pull     */
   /* Business Card Request (asynchronously).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_REQUEST      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Pull_Business_Card_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   BD_ADDR_t             RemoteDeviceAddress;
} OPPM_Pull_Business_Card_Request_Message_t;

#define OPPM_PULL_BUSINESS_CARD_REQUEST_MESSAGE_SIZE           (sizeof(OPPM_Pull_Business_Card_Request_Message_t))

   /* The following structure represents the message definition for a   */
   /* Object Push Manager message that informs the client of a Pull     */
   /* Business Card Response (asynchronously).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagOPPM_Pull_Business_Card_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ResponseCode;
   unsigned long         ObjectTotalLength;
   Boolean_t             Final;
   unsigned int          DataLength;
   Byte_t                DataBuffer[1];
} OPPM_Pull_Business_Card_Response_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold an entire Object Push Manager Pull  */
   /* Business Card Response message given the length (in bytes) of     */
   /* the object data.  This MACRO accepts the total number of bytes    */
   /* contained in the object data and returns the total number of bytes*/
   /* required to hold the entire message.                              */
#define OPPM_PULL_BUSINESS_CARD_RESPONSE_MESSAGE_SIZE(_x)      (STRUCTURE_OFFSET(OPPM_Pull_Business_Card_Response_Message_t, DataBuffer) + ((unsigned int)(_x)))

#endif
