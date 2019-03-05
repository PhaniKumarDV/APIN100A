/*****< panmmsg.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PANMMSG - Defined Interprocess Communication Messages for the Personal    */
/*            Area Network (PAN) Manager for Stonestreet One Bluetopia        */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author: Matt Seabold                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/28/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __PANMMSGH__
#define __PANMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTPAN.h"            /* PAN Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Personal Area   */
   /* Network (PAN) Manager.                                            */
#define BTPM_MESSAGE_GROUP_PAN_MANAGER                         0x00001004

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Personal Area    */
   /* Network (PAN) Manager.                                            */

   /* Personal Area Network (PAN) Manager Commands.                     */
#define PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE      0x00001001
#define PANM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE            0x00001002
#define PANM_MESSAGE_FUNCTION_CLOSE_CONNECTION                 0x00001003
#define PANM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES          0x00001004

#define PANM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION      0x00001101
#define PANM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS 0x00001102

#define PANM_MESSAGE_FUNCTION_REGISTER_EVENTS                  0x00002001
#define PANM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS               0x00002002

   /* Personal Area Network (PAN) Asynchronous Events.                  */
#define PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST               0x00010001
#define PANM_MESSAGE_FUNCTION_DEVICE_CONNECTED                 0x00010002
#define PANM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS         0x00010003
#define PANM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED              0x00010004

   /* Personal Area Networking (PAN) Manager Manager Command/Response   */
   /* Message Formats.                                                  */

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager message to respond to an incoming*/
   /* Connection/Authorization (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              Accept;
} PANM_Connection_Request_Response_Request_t;

#define PANM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(PANM_Connection_Request_Response_Request_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager message to respond to an incoming*/
   /* Connection/Authorization (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PANM_Connection_Request_Response_Response_t;

#define PANM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(PANM_Connection_Request_Response_Response_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager message to connect to a remote   */
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Connect_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   PAN_Service_Type_t    LocalServiceType;
   PAN_Service_Type_t    RemoteServiceType;
   unsigned long         ConnectionFlags;
} PANM_Connect_Remote_Device_Request_t;

#define PANM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(PANM_Connect_Remote_Device_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the PANM_Connect_Remote_Device_Request_t message to control    */
   /* various connection options.                                       */
#define PANM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION   0x00000001
#define PANM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION       0x00000002

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager message to connect to a remote   */
   /* device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Connect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PANM_Connect_Remote_Device_Response_t;

#define PANM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(PANM_Connect_Remote_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager message to disconnect a currently*/
   /* connected device (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CLOSE_CONNECTION                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Close_Connection_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
} PANM_Close_Connection_Request_t;

#define PANM_CLOSE_CONNECTION_REQUEST_SIZE                     (sizeof(PANM_Close_Connection_Request_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager message to disconnect a currently*/
   /* connected device (Response).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CLOSE_CONNECTION                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Close_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PANM_Close_Connection_Response_t;

#define PANM_CLOSE_CONNECTION_RESPONSE_SIZE                    (sizeof(PANM_Close_Connection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager Message to query the currently   */
   /* connected devices (Request).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Query_Connected_Devices_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} PANM_Query_Connected_Devices_Request_t;

#define PANM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE              (sizeof(PANM_Query_Connected_Devices_Request_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager Message to query the currently   */
   /* connected devices (Response).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Query_Connected_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberDevicesConnected;
   BD_ADDR_t             DeviceConnectedList[1];
} PANM_Query_Connected_Devices_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire query connected devices        */
   /* request message given the number of connected devices.  This MACRO*/
   /* accepts as it's input the total number of connected devices (NOT  */
   /* bytes) that are present starting from the DeviceConnectedList     */
   /* member of the PANM_Query_Connected_Devices_Response_t structure   */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define PANM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(_x)         (STRUCTURE_OFFSET(PANM_Query_Connected_Devices_Response_t, DeviceConnectedList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager Message to query the current     */
   /* configuration for the specified connection type (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Query_Current_Configuration_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} PANM_Query_Current_Configuration_Request_t;

#define PANM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE          (sizeof(PANM_Query_Current_Configuration_Request_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager Message to query the current     */
   /* configuration for the specified connection type (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Query_Current_Configuration_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned long         ServiceTypeFlags;
   unsigned long         IncomingConnectionFlags;
} PANM_Query_Current_Configuration_Response_t;

#define PANM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE         (sizeof(PANM_Query_Current_Configuration_Response_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking message to change the current incoming   */
   /* connection flags of a connection (Request).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Change_Incoming_Connection_Flags_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ConnectionFlags;
} PANM_Change_Incoming_Connection_Flags_Request_t;

#define PANM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE     (sizeof(PANM_Change_Incoming_Connection_Flags_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the PANM_Change_Incoming_Connection_Flags_Request_t structure  */
   /* to specify the various flags to apply to incoming Connections.    */
#define PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION   0x00000001
#define PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION  0x00000002
#define PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION      0x00000004

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking message to change the current incoming   */
   /* connection flags of a connection (Response).                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Change_Incoming_Connection_Flags_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PANM_Change_Incoming_Connection_Flags_Response_t;

#define PANM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE    (sizeof(PANM_Change_Incoming_Connection_Flags_Response_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking (PAN) Manager message to register for PAN*/
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_REGISTER_EVENTS                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Register_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} PANM_Register_Events_Request_t;

#define PANM_REGISTER_EVENTS_REQUEST_SIZE                      (sizeof(PANM_Register_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking (PAN) Manager message to register for PAN*/
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_REGISTER_EVENTS                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Register_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          EventsHandlerID;
} PANM_Register_Events_Response_t;

#define PANM_REGISTER_EVENTS_RESPONSE_SIZE                     (sizeof(PANM_Register_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking (PAN) Manager message to un-register for */
   /* PAN events (Request).                                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Un_Register_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventsHandlerID;
} PANM_Un_Register_Events_Request_t;

#define PANM_UN_REGISTER_EVENTS_REQUEST_SIZE                   (sizeof(PANM_Un_Register_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking (PAN) Manager message to un-register for */
   /* PAN events (Response).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Un_Register_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PANM_Un_Register_Events_Response_t;

#define PANM_UN_REGISTER_EVENTS_RESPONSE_SIZE                  (sizeof(PANM_Un_Register_Events_Response_t))

   /* Personal Area Networking (PAN) Manager Asynchronous Message       */
   /* Formats.                                                          */

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking message that informs the client that a   */
   /* remote device connection request has been received                */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Connection_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} PANM_Connection_Request_Message_t;

#define PANM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(PANM_Connection_Request_Message_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking Manager message that informs the client  */
   /* that a device is currently connected (asynchronously).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_DEVICE_CONNECTED                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Device_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   PAN_Service_Type_t    ServiceType;
} PANM_Device_Connected_Message_t;

#define PANM_DEVICE_CONNECTED_MESSAGE_SIZE                     (sizeof(PANM_Device_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking message that informs the client of the   */
   /* specified connection status (asynchronously).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Device_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   PAN_Service_Type_t    ServiceType;
   unsigned int          ConnectionStatus;
} PANM_Device_Connection_Status_Message_t;

#define PANM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE             (sizeof(PANM_Device_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the PANM_Device_Connection_Status_Message_t message to describe*/
   /* the actual connection result status.                              */
#define PANM_DEVICE_CONNECTION_STATUS_SUCCESS                  0x00000000
#define PANM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT          0x00000001
#define PANM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED          0x00000002
#define PANM_DEVICE_CONNECTION_STATUS_FAILURE_SECURITY         0x00000003
#define PANM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF 0x00000004
#define PANM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN          0x00000005

   /* The following structure represents the message definition for a   */
   /* Personal Area Networking message that informs the client that a   */
   /* current Personal Area Networking connection is now disconnected   */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PANM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPANM_Device_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   PAN_Service_Type_t    ServiceType;
} PANM_Device_Disconnected_Message_t;

#define PANM_DEVICE_DISCONNECTED_MESSAGE_SIZE                  (sizeof(PANM_Device_Disconnected_Message_t))

#endif
