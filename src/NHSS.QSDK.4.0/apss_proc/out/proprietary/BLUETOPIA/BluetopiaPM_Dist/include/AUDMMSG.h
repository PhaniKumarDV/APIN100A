/*****< audmmsg.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDMMSG - Defined Interprocess Communication Messages for the Audio       */
/*            (AUD) Manager for Stonestreet One Bluetopia Protocol Stack      */
/*            Platform Manager.                                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/23/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __AUDMMSGH__
#define __AUDMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTAUD.h"            /* Audio Framework Prototypes/Constants.     */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Audio (AUD)     */
   /* Manager.                                                          */
#define BTPM_MESSAGE_GROUP_AUDIO_MANAGER                       0x00000130

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Audio (AUD)      */
   /* Manager.                                                          */

   /* Audio (AUD) Manager Commands.                                     */
#define AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE            0x00001001
#define AUDM_MESSAGE_FUNCTION_CONNECT_AUDIO_STREAM                   0x00001002
#define AUDM_MESSAGE_FUNCTION_DISCONNECT_AUDIO_STREAM                0x00001003
#define AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_STATE               0x00001005
#define AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_FORMAT              0x00001006
#define AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE              0x00001007
#define AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT             0x00001008
#define AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_CONFIGURATION       0x00001009
#define AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_CONNECTED_DEVICES          0x00001010

#define AUDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS       0x00001101

#define AUDM_MESSAGE_FUNCTION_SEND_ENCODED_AUDIO_DATA                0x00001201
#define AUDM_MESSAGE_FUNCTION_SEND_RTP_ENCODED_AUDIO_DATA            0x00001202

#define AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_COMMAND            0x00001301
#define AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_RESPONSE           0x00001302
#define AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL                 0x00001303
#define AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL              0x00001304
#define AUDM_MESSAGE_FUNCTION_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES 0X00001305
#define AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL_BROWSING        0x00001306
#define AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL_BROWSING     0x00001307

#define AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_EVENTS           0x00002001
#define AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_EVENTS        0x00002002

#define AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_DATA             0x00002101
#define AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_DATA          0x00002102

#define AUDM_MESSAGE_FUNCTION_REGISTER_REMOTE_CONTROL_DATA           0x00002201
#define AUDM_MESSAGE_FUNCTION_UN_REGISTER_REMOTE_CONTROL_DATA        0x00002202

   /* Audio (AUD) Manager Asynchronous Events.                          */
#define AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST                     0x00010001
#define AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTED                 0x00010002
#define AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTION_STATUS         0x00010003
#define AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_DISCONNECTED              0x00010004
#define AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_STATE_CHANGED             0x00010005
#define AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE_STATUS       0x00010006
#define AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_FORMAT_CHANGED            0x00010007
#define AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT_STAT        0x00010008

#define AUDM_MESSAGE_FUNCTION_ENCODED_AUDIO_DATA_RECEIVED            0x00011001
#define AUDM_MESSAGE_FUNCTION_RTP_ENCODED_AUDIO_DATA_RECEIVED        0x00011002

#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_STATUS             0x00012001
#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_RECEIVED           0x00012002
#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTED                  0x00012003
#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTION_STATUS          0x00012004
#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_DISCONNECTED               0x00012005
#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTED         0x00012006
#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS 0x00012007
#define AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_DISCONNECTED      0x00012008

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Audio (AUD) Manager.    */

   /* Audio (AUD) Manager Manager Command/Response Message Formats.     */

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to Respond to an incoming Audio Stream*/
   /* or Remote Control Connection/Authorization (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t         MessageHeader;
   AUD_Connection_Request_Type_t RequestType;
   BD_ADDR_t                     RemoteDeviceAddress;
   Boolean_t                     Accept;
} AUDM_Connection_Request_Response_Request_t;

#define AUDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(AUDM_Connection_Request_Response_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to Respond to an incoming Audio Stream*/
   /* or Remote Control Connection/Authorization (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Connection_Request_Response_Response_t;

#define AUDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(AUDM_Connection_Request_Response_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to Connect to an Audio Stream         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECT_AUDIO_STREAM            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connect_Audio_Stream_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
   unsigned long         StreamFlags;
} AUDM_Connect_Audio_Stream_Request_t;

#define AUDM_CONNECT_AUDIO_STREAM_REQUEST_SIZE                 (sizeof(AUDM_Connect_Audio_Stream_Request_t))

   /* The following constants are used with the StreamFlags member of   */
   /* the AUDM_Connect_Audio_Stream_Request_t message to control various*/
   /* options for the stream.                                           */
#define AUDM_CONNECT_AUDIO_STREAM_FLAGS_REQUIRE_AUTHENTICATION 0x00000001
#define AUDM_CONNECT_AUDIO_STREAM_FLAGS_REQUIRE_ENCRYPTION     0x00000002

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to Connect to an Audio Stream         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECT_AUDIO_STREAM            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connect_Audio_Stream_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Connect_Audio_Stream_Response_t;

#define AUDM_CONNECT_AUDIO_STREAM_RESPONSE_SIZE                (sizeof(AUDM_Connect_Audio_Stream_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to Disconnect an Audio Stream         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_DISCONNECT_AUDIO_STREAM         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Disconnect_Audio_Stream_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
} AUDM_Disconnect_Audio_Stream_Request_t;

#define AUDM_DISCONNECT_AUDIO_STREAM_REQUEST_SIZE              (sizeof(AUDM_Disconnect_Audio_Stream_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to Disconnect an Audio Stream         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_DISCONNECT_AUDIO_STREAM         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Disconnect_Audio_Stream_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Disconnect_Audio_Stream_Response_t;

#define AUDM_DISCONNECT_AUDIO_STREAM_RESPONSE_SIZE             (sizeof(AUDM_Disconnect_Audio_Stream_Response_t))

   /* The following structure represents the message definition for an  */
   /* Audio (AUD) Manager Message to query the currently connected Audio*/
   /* devices (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*       AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Connected_Devices_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   AUD_Stream_Type_t     StreamType;
} AUDM_Query_Audio_Connected_Devices_Request_t;

#define AUDM_QUERY_AUDIO_CONNECTED_DEVICES_REQUEST_SIZE        (sizeof(AUDM_Query_Audio_Connected_Devices_Request_t))

   /* The following structure represents the message definition for an  */
   /* Audio (AUD) Manager Message to query the currently connected Audio*/
   /* devices (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*       AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Connected_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberDevicesConnected;
   BD_ADDR_t             DeviceConnectedList[1];
} AUDM_Query_Audio_Connected_Devices_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire Query Audio Connected Devices  */
   /* Response message given the number of connected devices. This MACRO*/
   /* accepts as it's input the total number of connected devices (NOT  */
   /* bytes) that are present starting from the DeviceConnectedList     */
   /* member of the AUDM_Query_Audio_Connected_Devices_Response_t       */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define AUDM_QUERY_AUDIO_CONNECTED_DEVICES_RESPONSE_SIZE(_x)   (STRUCTURE_OFFSET(AUDM_Query_Audio_Connected_Devices_Response_t, DeviceConnectedList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to query the current streaming state  */
   /* of an Audio Stream (Request).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_STATE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Stream_State_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
} AUDM_Query_Audio_Stream_State_Request_t;

#define AUDM_QUERY_AUDIO_STREAM_STATE_REQUEST_SIZE             (sizeof(AUDM_Query_Audio_Stream_State_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to query the current streaming state  */
   /* of an Audio Stream (Response).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_STATE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Stream_State_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   AUD_Stream_State_t    StreamState;
} AUDM_Query_Audio_Stream_State_Response_t;

#define AUDM_QUERY_AUDIO_STREAM_STATE_RESPONSE_SIZE            (sizeof(AUDM_Query_Audio_Stream_State_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to query the current stream format of */
   /* of an Audio Stream (Request).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_FORMAT       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Stream_Format_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
} AUDM_Query_Audio_Stream_Format_Request_t;

#define AUDM_QUERY_AUDIO_STREAM_FORMAT_REQUEST_SIZE            (sizeof(AUDM_Query_Audio_Stream_Format_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to query the current stream format of */
   /* of an Audio Stream (Response).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_FORMAT       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Stream_Format_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   AUD_Stream_Format_t   StreamFormat;
} AUDM_Query_Audio_Stream_Format_Response_t;

#define AUDM_QUERY_AUDIO_STREAM_FORMAT_RESPONSE_SIZE           (sizeof(AUDM_Query_Audio_Stream_Format_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to change the current stream state of */
   /* of an Audio Stream to Streaming/Suspended (Request).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Audio_Stream_State_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_State_t    StreamState;
} AUDM_Change_Audio_Stream_State_Request_t;

#define AUDM_CHANGE_AUDIO_STREAM_STATE_REQUEST_SIZE            (sizeof(AUDM_Change_Audio_Stream_State_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to change the current stream state of */
   /* of an Audio Stream to Streaming/Suspended (Response).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Audio_Stream_State_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Change_Audio_Stream_State_Response_t;

#define AUDM_CHANGE_AUDIO_STREAM_STATE_RESPONSE_SIZE           (sizeof(AUDM_Change_Audio_Stream_State_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to change the current stream format of*/
   /* of an Audio Stream (Request).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Audio_Stream_Format_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_Format_t   StreamFormat;
} AUDM_Change_Audio_Stream_Format_Request_t;

#define AUDM_CHANGE_AUDIO_STREAM_FORMAT_REQUEST_SIZE           (sizeof(AUDM_Change_Audio_Stream_Format_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to change the current stream format of*/
   /* of an Audio Stream (Response).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Audio_Stream_Format_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Change_Audio_Stream_Format_Response_t;

#define AUDM_CHANGE_AUDIO_STREAM_FORMAT_RESPONSE_SIZE          (sizeof(AUDM_Change_Audio_Stream_Format_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to query the current stream           */
   /* configuration of an Audio Stream (Streaming or Suspended)         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_CONFIGURATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Stream_Configuration_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
} AUDM_Query_Audio_Stream_Configuration_Request_t;

#define AUDM_QUERY_AUDIO_STREAM_CONFIGURATION_REQUEST_SIZE     (sizeof(AUDM_Query_Audio_Stream_Configuration_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to query the current stream           */
   /* configuration of an Audio Stream (Streaming or Suspended)         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_CONFIGURATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Audio_Stream_Configuration_Response_t
{
   BTPM_Message_Header_t      MessageHeader;
   int                        Status;
   AUD_Stream_Configuration_t StreamConfiguration;
} AUDM_Query_Audio_Stream_Configuration_Response_t;

#define AUDM_QUERY_AUDIO_STREAM_CONFIGURATION_RESPONSE_SIZE    (sizeof(AUDM_Query_Audio_Stream_Configuration_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to establish a Remote Control         */
   /* connection (Request).                                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connect_Remote_Control_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         ConnectionFlags;
} AUDM_Connect_Remote_Control_Request_t;

#define AUDM_CONNECT_REMOTE_CONTROL_REQUEST_SIZE               (sizeof(AUDM_Connect_Remote_Control_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the AUDM_Connect_Remote_Control_Request_t message to control   */
   /* various options for the stream.                                   */
#define AUDM_CONNECT_REMOTE_CONTROL_FLAGS_REQUIRE_AUTHENTICATION 0x00000001
#define AUDM_CONNECT_REMOTE_CONTROL_FLAGS_REQUIRE_ENCRYPTION     0x00000002

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to establish a Remote Control         */
   /* connection (Response).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connect_Remote_Control_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Connect_Remote_Control_Response_t;

#define AUDM_CONNECT_REMOTE_CONTROL_RESPONSE_SIZE              (sizeof(AUDM_Connect_Remote_Control_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to disconnect a Remote Control        */
   /* session (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Disconnect_Remote_Control_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} AUDM_Disconnect_Remote_Control_Request_t;

#define AUDM_DISCONNECT_REMOTE_CONTROL_REQUEST_SIZE            (sizeof(AUDM_Disconnect_Remote_Control_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to disconnect a Remote Control        */
   /* session (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Disconnect_Remote_Control_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Disconnect_Remote_Control_Response_t;

#define AUDM_DISCONNECT_REMOTE_CONTROL_RESPONSE_SIZE           (sizeof(AUDM_Disconnect_Remote_Control_Response_t))

   /* The following structure represents the message definition for an  */
   /* Audio (AUD) Manager Message to query the currently connected      */
   /* Remote Control devices (Request).                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*       AUDM_MESSAGE_FUNCTION_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Remote_Control_Connected_Devices_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} AUDM_Query_Remote_Control_Connected_Devices_Request_t;

#define AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_REQUEST_SIZE (sizeof(AUDM_Query_Remote_Control_Connected_Devices_Request_t))

   /* The following structure represents the message definition for     */
   /* an Audio (AUD) Manager Message to query the currently connected   */
   /* Remote Control devices (Response).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*       AUDM_MESSAGE_FUNCTION_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Query_Remote_Control_Connected_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberDevicesConnected;
   BD_ADDR_t             DeviceConnectedList[1];
} AUDM_Query_Remote_Control_Connected_Devices_Response_t;

   /* The following MACRO is provided to allow the programmer a         */
   /* very simple means of quickly determining the total number of      */
   /* bytes that will be required to hold an entire Query Remote        */
   /* Control Connected Devices Response message given the number       */
   /* of connected devices. This MACRO accepts as it's input the        */
   /* total number of connected devices (NOT bytes) that are            */
   /* present starting from the DeviceConnectedList member of the       */
   /* AUDM_Query_Remote_Control_Connected_Devices_Response_t structure  */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_RESPONSE_SIZE(_x) (STRUCTURE_OFFSET(AUDM_Query_Remote_Control_Connected_Devices_Response_t, DeviceConnectedList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to change the current incoming Flags  */
   /* of an Audio Stream or Remote Control Connection (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Incoming_Connection_Flags_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ConnectionFlags;
} AUDM_Change_Incoming_Connection_Flags_Request_t;

#define AUDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE     (sizeof(AUDM_Change_Incoming_Connection_Flags_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the AUDM_Change_Incoming_Connection_Flags_Request_t structure  */
   /* to specify the various flags to apply to incoming Connections.    */
#define AUDM_INCOMING_CONNECTION_FLAGS_BIT_MASK                0x000000FF
#define AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION   0x00000001
#define AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION  0x00000002
#define AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION      0x00000004

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to change the current incoming Flags  */
   /* of an Audio Stream or Remote Control Connection (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Incoming_Connection_Flags_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Change_Incoming_Connection_Flags_Response_t;

#define AUDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE    (sizeof(AUDM_Change_Incoming_Connection_Flags_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to send Raw, encoded, Audio Data out  */
   /* of an Audio Stream Source.                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_SEND_ENCODED_AUDIO_DATA         */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * There is NO Response message for this message.           */
typedef struct _tagAUDM_Send_Encoded_Audio_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          StreamEventsHandlerID;
   unsigned int          RawAudioDataFrameLength;
   unsigned char         RawAudioDataFrame[1];
} AUDM_Send_Encoded_Audio_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Encoded Audio Data Request   */
   /* Message given the number of actual Raw Audio Data Frame bytes.    */
   /* This function accepts as it's input the total number individual   */
   /* Audio Data Frame bytes are present starting from the              */
   /* RawAudioDataFrame member of the                                   */
   /* AUDM_Send_Encoded_Audio_Data_Request_t structure and returns the  */
   /* total number of bytes required to hold the entire message.        */
#define AUDM_SEND_ENCODED_AUDIO_DATA_REQUEST_SIZE(_x)          (STRUCTURE_OFFSET(AUDM_Send_Encoded_Audio_Data_Request_t, RawAudioDataFrame) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to send Raw, encoded, Audio Data out  */
   /* of an Audio Stream Source with RTP Header Information included.   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            AUDM_MESSAGE_FUNCTION_SEND_RTP_ENCODED_AUDIO_DATA      */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * There is NO Response message for this message.           */
typedef struct _tagAUDM_Send_RTP_Encoded_Audio_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          StreamEventsHandlerID;
   unsigned long         Flags;
   AUD_RTP_Header_Info_t RTPHeaderInfo;
   unsigned int          RawAudioDataFrameLength;
   unsigned char         RawAudioDataFrame[1];
} AUDM_Send_RTP_Encoded_Audio_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Encoded Audio Data Request   */
   /* Message given the number of actual Raw Audio Data Frame bytes.    */
   /* This function accepts as it's input the total number individual   */
   /* Audio Data Frame bytes are present starting from the              */
   /* RawAudioDataFrame member of the                                   */
   /* AUDM_Send_RTP_Encoded_Audio_Data_Request_t structure and returns  */
   /* the total number of bytes required to hold the entire message.    */
#define AUDM_SEND_RTP_ENCODED_AUDIO_DATA_REQUEST_SIZE(_x)          (STRUCTURE_OFFSET(AUDM_Send_RTP_Encoded_Audio_Data_Request_t, RawAudioDataFrame) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to send a Remote Control Command to   */
   /* a Remote Device (Request).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_COMMAND     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Send_Remote_Control_Command_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RemoteControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         ResponseTimeout;
   AVRCP_Message_Type_t  MessageType;
   unsigned int          MessageDataLength;
   unsigned char         MessageData[1];
} AUDM_Send_Remote_Control_Command_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Remote Control Command       */
   /* Request Message given the number of actual Remote Control Command */
   /* Data bytes.  This function accepts as it's input the total number */
   /* individual Remote Control Command Data bytes present starting from*/
   /* the MessageData member of the                                     */
   /* AUDM_Send_Remote_Control_Command_Request_t structure and returns  */
   /* the total number of bytes required to hold the entire message.    */
#define AUDM_SEND_REMOTE_CONTROL_COMMAND_REQUEST_SIZE(_x)      (STRUCTURE_OFFSET(AUDM_Send_Remote_Control_Command_Request_t, MessageData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to send a Remote Control Command to   */
   /* a Remote Device (Response).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_COMMAND     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Send_Remote_Control_Command_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} AUDM_Send_Remote_Control_Command_Response_t;

#define AUDM_SEND_REMOTE_CONTROL_COMMAND_RESPONSE_SIZE         (sizeof(AUDM_Send_Remote_Control_Command_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to send a Remote Control Response to  */
   /* a Remote Device (Request).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_RESPONSE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Send_Remote_Control_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RemoteControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          TransactionID;
   AVRCP_Message_Type_t  MessageType;
   unsigned int          MessageDataLength;
   unsigned char         MessageData[1];
} AUDM_Send_Remote_Control_Response_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Remote Control Response      */
   /* Request Message given the number of actual Remote Control Response*/
   /* Data bytes.  This function accepts as it's input the total number */
   /* individual Remote Control Response Data bytes present starting    */
   /* from the MessageData member of the                                */
   /* AUDM_Send_Remote_Control_Response_Request_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define AUDM_SEND_REMOTE_CONTROL_RESPONSE_REQUEST_SIZE(_x)     (STRUCTURE_OFFSET(AUDM_Send_Remote_Control_Response_Request_t, MessageData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to send a Remote Control Response to  */
   /* a Remote Device (Response).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_RESPONSE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Send_Remote_Control_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Send_Remote_Control_Response_Response_t;

#define AUDM_SEND_REMOTE_CONTROL_RESPONSE_RESPONSE_SIZE        (sizeof(AUDM_Send_Remote_Control_Response_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to register for Audio Manager events  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Register_Audio_Stream_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} AUDM_Register_Audio_Stream_Events_Request_t;

#define AUDM_REGISTER_AUDIO_STREAM_EVENTS_REQUEST_SIZE         (sizeof(AUDM_Register_Audio_Stream_Events_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to register for Audio Manager events  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Register_Audio_Stream_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          StreamEventsHandlerID;
} AUDM_Register_Audio_Stream_Events_Response_t;

#define AUDM_REGISTER_AUDIO_STREAM_EVENTS_RESPONSE_SIZE        (sizeof(AUDM_Register_Audio_Stream_Events_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to un-register for Audio Manager      */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_EVENTS */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Un_Register_Audio_Stream_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          StreamEventsHandlerID;
} AUDM_Un_Register_Audio_Stream_Events_Request_t;

#define AUDM_UN_REGISTER_AUDIO_STREAM_EVENTS_REQUEST_SIZE      (sizeof(AUDM_Un_Register_Audio_Stream_Events_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to un-register for Audio Manager      */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_EVENTS */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Un_Register_Audio_Stream_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Un_Register_Audio_Stream_Events_Response_t;

#define AUDM_UN_REGISTER_AUDIO_STREAM_EVENTS_RESPONSE_SIZE     (sizeof(AUDM_Un_Register_Audio_Stream_Events_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to register for Audio Manager Data    */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_DATA      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Register_Audio_Stream_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   AUD_Stream_Type_t     StreamType;
} AUDM_Register_Audio_Stream_Data_Events_Request_t;

#define AUDM_REGISTER_AUDIO_STREAM_DATA_EVENTS_REQUEST_SIZE    (sizeof(AUDM_Register_Audio_Stream_Data_Events_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to register for Audio Manager Data    */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_DATA      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Register_Audio_Stream_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          StreamDataEventsHandlerID;
} AUDM_Register_Audio_Stream_Data_Events_Response_t;

#define AUDM_REGISTER_AUDIO_STREAM_DATA_EVENTS_RESPONSE_SIZE   (sizeof(AUDM_Register_Audio_Stream_Data_Events_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to un-register for Audio Manager      */
   /* Data events (Request).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_DATA   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Un_Register_Audio_Stream_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          StreamDataEventsHandlerID;
} AUDM_Un_Register_Audio_Stream_Data_Events_Request_t;

#define AUDM_UN_REGISTER_AUDIO_STREAM_DATA_EVENTS_REQUEST_SIZE (sizeof(AUDM_Un_Register_Audio_Stream_Data_Events_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to un-register for Audio Manager      */
   /* Data events (Response).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_DATA   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Un_Register_Audio_Stream_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Un_Register_Audio_Stream_Data_Events_Response_t;

#define AUDM_UN_REGISTER_AUDIO_STREAM_DATA_EVENTS_RESPONSE_SIZE (sizeof(AUDM_Un_Register_Audio_Stream_Data_Events_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to register for Audio Manager Remote  */
   /* Control events (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REGISTER_REMOTE_CONTROL_DATA    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Register_Remote_Control_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceType;
} AUDM_Register_Remote_Control_Data_Events_Request_t;

#define AUDM_REGISTER_REMOTE_CONTROL_DATA_EVENTS_REQUEST_SIZE  (sizeof(AUDM_Register_Remote_Control_Data_Events_Request_t))

   /* The following constants represent the Bit-Masks of the allowable  */
   /* values of the ServiceType member of the                           */
   /* AUDM_Register_Remote_Control_Data_Events_Request_t message to     */
   /* specify the type of Remote Control Data Events that are being     */
   /* registered for.                                                   */
#define AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER 0x00000001
#define AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET     0x00000002

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to register for Audio Manager Remote  */
   /* Control events (Response).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REGISTER_REMOTE_CONTROL_DATA    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Register_Remote_Control_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          RemoteControlEventsHandlerID;
} AUDM_Register_Remote_Control_Data_Events_Response_t;

#define AUDM_REGISTER_REMOTE_CONTROL_DATA_EVENTS_RESPONSE_SIZE (sizeof(AUDM_Register_Remote_Control_Data_Events_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to un-register for Audio Manager      */
   /* Remote Control events (Request).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_UN_REGISTER_REMOTE_CONTROL_DATA */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Un_Register_Remote_Control_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RemoteControlEventsHandlerID;
} AUDM_Un_Register_Remote_Control_Data_Events_Request_t;

#define AUDM_UN_REGISTER_REMOTE_CONTROL_DATA_EVENTS_REQUEST_SIZE (sizeof(AUDM_Un_Register_Remote_Control_Data_Events_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to un-register for Audio Manager      */
   /* Remote Control events (Response).                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_UN_REGISTER_REMOTE_CONTROL_DATA */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Un_Register_Remote_Control_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Un_Register_Remote_Control_Data_Events_Response_t;

#define AUDM_UN_REGISTER_REMOTE_CONTROL_DATA_EVENTS_RESPONSE_SIZE (sizeof(AUDM_Un_Register_Remote_Control_Data_Events_Response_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to establish a Remote Control Browsing*/
   /* Channel connection (Request).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL_BROWSING */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connect_Remote_Control_Browsing_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         ConnectionFlags;
} AUDM_Connect_Remote_Control_Browsing_Request_t;

#define AUDM_CONNECT_REMOTE_CONTROL_BROWSING_REQUEST_SIZE      (sizeof(AUDM_Connect_Remote_Control_Browsing_Request_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message to establish a Remote Control Browsing*/
   /* Channel connection (Response).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL_BROWSING */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connect_Remote_Control_Browsing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Connect_Remote_Control_Browsing_Response_t;

#define AUDM_CONNECT_REMOTE_CONTROL_BROWSING_RESPONSE_SIZE     (sizeof(AUDM_Connect_Remote_Control_Browsing_Response_t))

   /* The following structure represents the Message definition for     */
   /* an Audio (AUD) Manager Message to disconnect a Remote Control     */
   /* Browsing session (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL       */
   /*                 _BROWSING                                         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Disconnect_Remote_Control_Browsing_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} AUDM_Disconnect_Remote_Control_Browsing_Request_t;

#define AUDM_DISCONNECT_REMOTE_CONTROL_BROWSING_REQUEST_SIZE   (sizeof(AUDM_Disconnect_Remote_Control_Browsing_Request_t))

   /* The following structure represents the Message definition for     */
   /* an Audio (AUD) Manager Message to disconnect a Remote Control     */
   /* Browsing session (Response).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL       */
   /*                 _BROWSING                                         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Disconnect_Remote_Control_Browsing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} AUDM_Disconnect_Remote_Control_Browsing_Response_t;

#define AUDM_DISCONNECT_REMOTE_CONTROL_BROWSING_RESPONSE_SIZE  (sizeof(AUDM_Disconnect_Remote_Control_Browsing_Response_t))

   /* Audio (AUD) Manager Asynchronous Message Formats.                 */

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that an Audio */
   /* Stream or Remote Control Connection Request has been              */
   /* received (asynchronously).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Connection_Request_Message_t
{
   BTPM_Message_Header_t         MessageHeader;
   BD_ADDR_t                     RemoteDeviceAddress;
   AUD_Connection_Request_Type_t RequestType;
} AUDM_Connection_Request_Message_t;

#define AUDM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(AUDM_Connection_Request_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that a Stream */
   /* is currently connected (asynchronously).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTED          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Audio_Stream_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          MediaMTU;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_Format_t   StreamFormat;
} AUDM_Audio_Stream_Connected_Message_t;

#define AUDM_AUDIO_STREAM_CONNECTED_MESSAGE_SIZE               (sizeof(AUDM_Audio_Stream_Connected_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that an Audio */
   /* Stream is currently connected (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTION_STATUS  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Audio_Stream_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ConnectionStatus;
   unsigned int          MediaMTU;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_Format_t   StreamFormat;
} AUDM_Audio_Stream_Connection_Status_Message_t;

#define AUDM_AUDIO_STREAM_CONNECTION_STATUS_MESSAGE_SIZE       (sizeof(AUDM_Audio_Stream_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionResult member */
   /* of the AUDM_Audio_Stream_Connection_Status_Message_t message to   */
   /* describe the actual Stream Connection Result Status.              */
#define AUDM_STREAM_CONNECTION_STATUS_SUCCESS                  0x00000000
#define AUDM_STREAM_CONNECTION_STATUS_FAILURE_TIMEOUT          0x00000001
#define AUDM_STREAM_CONNECTION_STATUS_FAILURE_REFUSED          0x00000002
#define AUDM_STREAM_CONNECTION_STATUS_FAILURE_SECURITY         0x00000003
#define AUDM_STREAM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF 0x00000004
#define AUDM_STREAM_CONNECTION_STATUS_FAILURE_UNKNOWN          0x00000005

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that an Audio */
   /* Stream now disconnected (asynchronously).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_DISCONNECTED       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Audio_Stream_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
} AUDM_Audio_Stream_Disconnected_Message_t;

#define AUDM_AUDIO_STREAM_DISCONNECTED_MESSAGE_SIZE            (sizeof(AUDM_Audio_Stream_Disconnected_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that the      */
   /* Stream State of a connected Audio Stream has changed              */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_STATE_CHANGED      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Audio_Stream_State_Changed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_State_t    StreamState;
} AUDM_Audio_Stream_State_Changed_Message_t;

#define AUDM_AUDIO_STREAM_STATE_CHANGED_MESSAGE_SIZE           (sizeof(AUDM_Audio_Stream_State_Changed_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that the      */
   /* Stream State of a connected Audio Stream has been changed         */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE_STATUS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Audio_Stream_State_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Successful;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_State_t    StreamState;
} AUDM_Change_Audio_Stream_State_Status_Message_t;

#define AUDM_CHANGE_AUDIO_STREAM_STATE_STATUS_MESSAGE_SIZE     (sizeof(AUDM_Change_Audio_Stream_State_Status_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that the      */
   /* Stream Format of a connected Audio Stream has changed             */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_FORMAT_CHANGED     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Audio_Stream_Format_Changed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_Format_t   StreamFormat;
} AUDM_Audio_Stream_Format_Changed_Message_t;

#define AUDM_AUDIO_STREAM_FORMAT_CHANGED_MESSAGE_SIZE          (sizeof(AUDM_Audio_Stream_Format_Changed_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that the      */
   /* Stream Format of a connected Audio Stream has been changed        */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT_STAT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Change_Audio_Stream_Format_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Successful;
   AUD_Stream_Type_t     StreamType;
   AUD_Stream_Format_t   StreamFormat;
} AUDM_Change_Audio_Stream_Format_Status_Message_t;

#define AUDM_CHANGE_AUDIO_STREAM_FORMAT_STATUS_MESSAGE_SIZE    (sizeof(AUDM_Change_Audio_Stream_Format_Status_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that specifies received, Raw, encoded,*/
   /* Audio Data into an Audio Stream Sink.                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_ENCODED_AUDIO_DATA_RECEIVED     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Encoded_Audio_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          StreamDataEventsHandlerID;
   unsigned int          RawAudioDataFrameLength;
   unsigned char         RawAudioDataFrame[1];
} AUDM_Encoded_Audio_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Encoded Audio Data Request   */
   /* Message given the number of actual Raw Audio Data Frame bytes.    */
   /* This function accepts as it's input the total number individual   */
   /* Audio Data Frame bytes are present starting from the              */
   /* RawAudioDataFrame member of the                                   */
   /* AUDM_Encoded_Audio_Data_Received_Message_t structure and returns  */
   /* the total number of bytes required to hold the entire message.    */
#define AUDM_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(_x)      (STRUCTURE_OFFSET(AUDM_Encoded_Audio_Data_Received_Message_t, RawAudioDataFrame) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that specifies received, Raw, encoded,*/
   /* Audio Data into an Audio Stream Sink.                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            AUDM_MESSAGE_FUNCTION_RTP_ENCODED_AUDIO_DATA_RECEIVED  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_RTP_Encoded_Audio_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          StreamDataEventsHandlerID;
   AUD_RTP_Header_Info_t RTPHeaderInfo;
   unsigned int          DataLength;
   unsigned char         Data[1];
} AUDM_RTP_Encoded_Audio_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Encoded Audio Data Request   */
   /* Message given the number of actual Raw Audio Data Frame bytes.    */
   /* This function accepts as it's input the total number individual   */
   /* Audio Data Frame bytes are present starting from the Data member  */
   /* of the AUDM_RTP_Encoded_Audio_Data_Received_Message_t structure   */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define AUDM_RTP_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(_x)      (STRUCTURE_OFFSET(AUDM_RTP_Encoded_Audio_Data_Received_Message_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client of the status */
   /* of a previously submitted Remote Control Command (asynchronously).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_STATUS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Command_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          TransactionID;
   int                   Status;
   AVRCP_Message_Type_t  MessageType;
   unsigned int          MessageDataLength;
   unsigned char         MessageData[1];
} AUDM_Remote_Control_Command_Status_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Remote Control Command Status*/
   /* Message given the number of actual Remote Control Response Data   */
   /* bytes.  This function accepts as it's input the total number      */
   /* individual Remote Control Response Data bytes present starting    */
   /* from the MessageData member of the                                */
   /* AUDM_Remote_Control_Command_Status_Message_t structure and returns*/
   /* the total number of bytes required to hold the entire message.    */
#define AUDM_REMOTE_CONTROL_COMMAND_STATUS_MESSAGE_SIZE(_x)    (STRUCTURE_OFFSET(AUDM_Remote_Control_Command_Status_Message_t, MessageData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client of the status */
   /* of a previously submitted Remote Control Command (asynchronously).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_RECEIVED */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Command_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          TransactionID;
   AVRCP_Message_Type_t  MessageType;
   unsigned int          MessageDataLength;
   unsigned char         MessageData[1];
} AUDM_Remote_Control_Command_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Remote Control Command       */
   /* Received Message given the number of actual Remote Control Command*/
   /* Data bytes.  This function accepts as it's input the total number */
   /* individual Remote Control Command Data bytes present starting from*/
   /* the MessageData member of the                                     */
   /* AUDM_Remote_Control_Command_Received_Message_t structure and      */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define AUDM_REMOTE_CONTROL_COMMAND_RECEIVED_MESSAGE_SIZE(_x)  (STRUCTURE_OFFSET(AUDM_Remote_Control_Command_Received_Message_t, MessageData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that a Remote */
   /* Control Connection is established (asynchronously).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTED        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} AUDM_Remote_Control_Connected_Message_t;

#define AUDM_REMOTE_CONTROL_CONNECTED_MESSAGE_SIZE             (sizeof(AUDM_Remote_Control_Connected_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client of the result */
   /* of a Remote Control Connection attempt requested by the client.   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTION_STATUS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ConnectionStatus;
} AUDM_Remote_Control_Connection_Status_Message_t;

#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_MESSAGE_SIZE     (sizeof(AUDM_Remote_Control_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the AUDM_Remote_Control_Connection_Status_Message_t message to */
   /* describe the actual Connection Result Status.                     */
#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_SUCCESS                  0x00000000
#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_TIMEOUT          0x00000001
#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_REFUSED          0x00000002
#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_SECURITY         0x00000003
#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF 0x00000004
#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_UNKNOWN          0x00000005

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that a Remote */
   /* Control Connection was disconnected (asynchronously).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_DISCONNECTED     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Disconnected_Message_t
{
   BTPM_Message_Header_t   MessageHeader;
   BD_ADDR_t               RemoteDeviceAddress;
   AUD_Disconnect_Reason_t DisconnectReason;
} AUDM_Remote_Control_Disconnected_Message_t;

#define AUDM_REMOTE_CONTROL_DISCONNECTED_MESSAGE_SIZE          (sizeof(AUDM_Remote_Control_Disconnected_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that a Remote */
   /* Control Connection is established (asynchronously).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTE*/
   /*             D                                                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Browsing_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} AUDM_Remote_Control_Browsing_Connected_Message_t;

#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_MESSAGE_SIZE             (sizeof(AUDM_Remote_Control_Browsing_Connected_Message_t))

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client of the result */
   /* of a Remote Control Connection attempt requested by the client.   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_        */
   /*             CONNECTION_STATUS                                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Browsing_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ConnectionStatus;
} AUDM_Remote_Control_Browsing_Connection_Status_Message_t;

#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_MESSAGE_SIZE     (sizeof(AUDM_Remote_Control_Browsing_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the AUDM_Remote_Control_Browsing_Connection_Status_Message_t message to */
   /* describe the actual Connection Result Status.                     */
#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_SUCCESS                  0x00000000
#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_FAILURE_TIMEOUT          0x00000001
#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_FAILURE_REFUSED          0x00000002
#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_FAILURE_SECURITY         0x00000003
#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF 0x00000004
#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_FAILURE_UNKNOWN          0x00000005

   /* The following structure represents the Message definition for an  */
   /* Audio (AUD) Manager Message that informs the client that a Remote */
   /* Control Connection was disconnected (asynchronously).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_        */
   /*                 DISCONNECTED                                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagAUDM_Remote_Control_Browsing_Disconnected_Message_t
{
   BTPM_Message_Header_t   MessageHeader;
   BD_ADDR_t               RemoteDeviceAddress;
} AUDM_Remote_Control_Browsing_Disconnected_Message_t;

#define AUDM_REMOTE_CONTROL_BROWSING_DISCONNECTED_MESSAGE_SIZE          (sizeof(AUDM_Remote_Control_Browsing_Disconnected_Message_t))

#endif
