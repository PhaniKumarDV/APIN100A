/*****< hdsmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDSMMSG - Defined Interprocess Communication Messages for the Headset     */
/*            (HDSET) Manager for Stonestreet One Bluetopia Protocol Stack    */
/*            Platform Manager.                                               */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/17/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HDSMMSGH__
#define __HDSMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHDS.h"            /* Headset Framework Prototypes/Constants.   */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HDSMType.h"            /* BTPM Headset Manager Type Definitions.    */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Headset (HDSET) */
   /* Manager.                                                          */
#define BTPM_MESSAGE_GROUP_HEADSET_MANAGER                     0x00001007

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Headset (HDSET)  */
   /* Manager.                                                          */

   /* Headset (HDSET) Manager Commands.                                 */
#define HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE      0x00001001
#define HDSM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE            0x00001002
#define HDSM_MESSAGE_FUNCTION_DISCONNECT_DEVICE                0x00001003
#define HDSM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES          0x00001004

#define HDSM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION      0x00001101
#define HDSM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS 0x00001102

#define HDSM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN                 0x00001201
#define HDSM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN              0x00001202

#define HDSM_MESSAGE_FUNCTION_SEND_BUTTON_PRESS                0x00001301

#define HDSM_MESSAGE_FUNCTION_RING_INDICATION                  0x00001401

#define HDSM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION           0x00001501
#define HDSM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION         0x00001502
#define HDSM_MESSAGE_FUNCTION_SEND_AUDIO_DATA                  0x00001503
#define HDSM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE      0x00001504

#define HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_EVENTS          0x00002001
#define HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_EVENTS       0x00002002

#define HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_DATA            0x00002101
#define HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_DATA         0x00002102


   /* Headset (HDSET) Manager Asynchronous Events.                      */
#define HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST               0x00010001
#define HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTED                 0x00010002
#define HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS         0x00010003
#define HDSM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED              0x00010004
#define HDSM_MESSAGE_FUNCTION_AUDIO_CONNECTED                  0x00010005
#define HDSM_MESSAGE_FUNCTION_AUDIO_CONNECTION_STATUS          0x00010006
#define HDSM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED               0x00010007
#define HDSM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED              0x00010008
#define HDSM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND                 0x00010009
#define HDSM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND              0x0001000A

#define HDSM_MESSAGE_FUNCTION_RING_INDICATION_IND              0x00011001

#define HDSM_MESSAGE_FUNCTION_BUTTON_PRESSED_IND               0x00012001

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Headset (HDSET) Manager.*/

   /* Headset (HDSET) Manager Manager Command/Response Message Formats. */

   /* Headset/Audio Gateway Message Definitions.                        */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to respond to an incoming                 */
   /* Connection/Authorization (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              Accept;
} HDSM_Connection_Request_Response_Request_t;

#define HDSM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(HDSM_Connection_Request_Response_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to respond to an incoming                 */
   /* Connection/Authorization (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Connection_Request_Response_Response_t;

#define HDSM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(HDSM_Connection_Request_Response_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to connect to a remote device (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Connect_Remote_Device_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   unsigned int           RemoteServerPort;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned long          ConnectionFlags;
} HDSM_Connect_Remote_Device_Request_t;

#define HDSM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(HDSM_Connect_Remote_Device_Request_t))

   /* The following constants are used with the ConnectFlags member of  */
   /* the HDSM_Connect_Remote_Device_Request_t message to control       */
   /* various connection options.                                       */
#define HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION   0x00000001
#define HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION       0x00000002

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to connect to a remote device (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Connect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Connect_Remote_Device_Response_t;

#define HDSM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(HDSM_Connect_Remote_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to disconnect a currently connected device*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_DISCONNECT_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Disconnect_Device_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Disconnect_Device_Request_t;

#define HDSM_DISCONNECT_DEVICE_REQUEST_SIZE                    (sizeof(HDSM_Disconnect_Device_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to disconnect a currently connected device*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_DISCONNECT_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Disconnect_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Disconnect_Device_Response_t;

#define HDSM_DISCONNECT_DEVICE_RESPONSE_SIZE                   (sizeof(HDSM_Disconnect_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager Message to query the currently connected devices  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Query_Connected_Devices_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
} HDSM_Query_Connected_Devices_Request_t;

#define HDSM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE              (sizeof(HDSM_Query_Connected_Devices_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager Message to query the currently connected devices  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Query_Connected_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberDevicesConnected;
   BD_ADDR_t             DeviceConnectedList[1];
} HDSM_Query_Connected_Devices_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire query connected devices        */
   /* request message given the number of connected devices.  This MACRO*/
   /* accepts as it's input the total number of connected devices (NOT  */
   /* bytes) that are present starting from the DeviceConnectedList     */
   /* member of the HDSM_Query_Connected_Devices_Response_t structure   */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define HDSM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(_x)         (STRUCTURE_OFFSET(HDSM_Query_Connected_Devices_Response_t, DeviceConnectedList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Headset Manager Message to query the current configuration for the*/
   /* specified connection type (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Query_Current_Configuration_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
} HDSM_Query_Current_Configuration_Request_t;

#define HDSM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE          (sizeof(HDSM_Query_Current_Configuration_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager Message to query the current configuration for the*/
   /* specified connection type (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Query_Current_Configuration_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned long         IncomingConnectionFlags;
   unsigned long         SupportedFeaturesMask;
} HDSM_Query_Current_Configuration_Response_t;

#define HDSM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE         (sizeof(HDSM_Query_Current_Configuration_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to change the current incoming connection */
   /* flags of a connection (Request).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Change_Incoming_Connection_Flags_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   unsigned long          ConnectionFlags;
} HDSM_Change_Incoming_Connection_Flags_Request_t;

#define HDSM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE     (sizeof(HDSM_Change_Incoming_Connection_Flags_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the HDSM_Change_Incoming_Connection_Flags_Request_t structure  */
   /* to specify the various flags to apply to incoming Connections.    */
#define HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION   0x00000001
#define HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION  0x00000002
#define HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION      0x00000004

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to change the current incoming connection */
   /* flags of a connection (Response).                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDSM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Change_Incoming_Connection_Flags_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Change_Incoming_Connection_Flags_Response_t;

#define HDSM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE    (sizeof(HDSM_Change_Incoming_Connection_Flags_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to request the setting of the speaker gain*/
   /* on the remote device (Request).                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Set_Speaker_Gain_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SpeakerGain;
} HDSM_Set_Speaker_Gain_Request_t;

#define HDSM_SET_SPEAKER_GAIN_REQUEST_SIZE                     (sizeof(HDSM_Set_Speaker_Gain_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to request the setting of the speaker gain*/
   /* on the remote device (Response).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Set_Speaker_Gain_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Set_Speaker_Gain_Response_t;

#define HDSM_SET_SPEAKER_GAIN_RESPONSE_SIZE                    (sizeof(HDSM_Set_Speaker_Gain_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to request the setting of the microphone  */
   /* gain on the remote device (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Set_Microphone_Gain_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           MicrophoneGain;
} HDSM_Set_Microphone_Gain_Request_t;

#define HDSM_SET_MICROPHONE_GAIN_REQUEST_SIZE                  (sizeof(HDSM_Set_Microphone_Gain_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to request the setting of the microphone  */
   /* gain on the remote device (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Set_Microphone_Gain_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Set_Microphone_Gain_Response_t;

#define HDSM_SET_MICROPHONE_GAIN_RESPONSE_SIZE                 (sizeof(HDSM_Set_Microphone_Gain_Response_t))

   /* Headset Message Definitions.                                      */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to send a button press to the remote Audio*/
   /* Gateway device (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SEND_BUTTON_PRESS               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Send_Button_Press_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HDSM_Send_Button_Press_Request_t;

#define HDSM_SEND_BUTTON_PRESS_REQUEST_SIZE                    (sizeof(HDSM_Send_Button_Press_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to send a button press to the remote Audio*/
   /* Gateway device (Response).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SEND_BUTTON_PRESS               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Send_Button_Press_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Send_Button_Press_Response_t;

#define HDSM_SEND_BUTTON_PRESS_RESPONSE_SIZE                   (sizeof(HDSM_Send_Button_Press_Response_t))

   /* Audio Gateway Message Definitions.                                */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to send a ring indication to the remote   */
   /* Headset device (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_RING_INDICATION                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Ring_Indication_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HDSM_Ring_Indication_Request_t;

#define HDSM_RING_INDICATION_REQUEST_SIZE                      (sizeof(HDSM_Ring_Indication_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to send a ring indication to the remote   */
   /* Headset device (Response).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_RING_INDICATION                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Ring_Indication_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Ring_Indication_Response_t;

#define HDSM_RING_INDICATION_RESPONSE_SIZE                     (sizeof(HDSM_Ring_Indication_Response_t))

   /* Headset Manager Audio Connection Management Messages.             */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to setup a SCO audio connection on the    */
   /* local Headset or Audio Gateway service (Request).                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Setup_Audio_Connection_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              InBandRinging;
} HDSM_Setup_Audio_Connection_Request_t;

#define HDSM_SETUP_AUDIO_CONNECTION_REQUEST_SIZE               (sizeof(HDSM_Setup_Audio_Connection_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to setup a SCO audio connection on the    */
   /* local Headset or Audio Gateway service (Response).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Setup_Audio_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Setup_Audio_Connection_Response_t;

#define HDSM_SETUP_AUDIO_CONNECTION_RESPONSE_SIZE              (sizeof(HDSM_Setup_Audio_Connection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to release a currently on-going SCO audio */
   /* connection on the local Headset or Audio Gateway service          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Release_Audio_Connection_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Release_Audio_Connection_Request_t;

#define HDSM_RELEASE_AUDIO_CONNECTION_REQUEST_SIZE             (sizeof(HDSM_Release_Audio_Connection_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to release a currently on-going SCO audio */
   /* connection on the local Headset or Audio Gateway service          */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Release_Audio_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Release_Audio_Connection_Response_t;

#define HDSM_RELEASE_AUDIO_CONNECTION_RESPONSE_SIZE            (sizeof(HDSM_Release_Audio_Connection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to send SCO audio data over a currently   */
   /* on-going SCO audio connection to a remote Headset or Audio Gateway*/
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SEND_AUDIO_DATA                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * There is NO Response message for this message.           */
typedef struct _tagHDSM_Send_Audio_Data_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           DataEventsHandlerID;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           AudioDataLength;
   unsigned char          AudioData[1];
} HDSM_Send_Audio_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold a an entire SCO audio data request       */
   /* message given the number of actual raw audio data frame bytes.    */
   /* This function accepts as it's input the total number individual   */
   /* SCO audio data frame bytes are present starting from the AudioData*/
   /* member of the HDSM_Send_Audio_Data_Request_t structure and returns*/
   /* the total number of bytes required to hold the entire message.    */
#define HDSM_SEND_AUDIO_DATA_REQUEST_SIZE(_x)                  (STRUCTURE_OFFSET(HDSM_Send_Audio_Data_Request_t, AudioData) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to register for Headset Manager events    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_EVENTS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Register_Headset_Events_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   Boolean_t              ControlHandler;
   HDSM_Connection_Type_t ConnectionType;
} HDSM_Register_Headset_Events_Request_t;

#define HDSM_REGISTER_HEADSET_EVENTS_REQUEST_SIZE              (sizeof(HDSM_Register_Headset_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to register for Headset Manager events    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_EVENTS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Register_Headset_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          EventsHandlerID;
} HDSM_Register_Headset_Events_Response_t;

#define HDSM_REGISTER_HEADSET_EVENTS_RESPONSE_SIZE             (sizeof(HDSM_Register_Headset_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to un-register for Headset Manager events */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Un_Register_Headset_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventsHandlerID;
} HDSM_Un_Register_Headset_Events_Request_t;

#define HDSM_UN_REGISTER_HEADSET_EVENTS_REQUEST_SIZE           (sizeof(HDSM_Un_Register_Headset_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to un-register for Headset Manager events */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Un_Register_Headset_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Un_Register_Headset_Events_Response_t;

#define HDSM_UN_REGISTER_HEADSET_EVENTS_RESPONSE_SIZE          (sizeof(HDSM_Un_Register_Headset_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to register for Headset Manager Data      */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_DATA           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Register_Headset_Data_Events_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
} HDSM_Register_Headset_Data_Events_Request_t;

#define HDSM_REGISTER_HEADSET_DATA_EVENTS_REQUEST_SIZE         (sizeof(HDSM_Register_Headset_Data_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to register for Headset Manager Data      */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_DATA           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Register_Headset_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          DataEventsHandlerID;
} HDSM_Register_Headset_Data_Events_Response_t;

#define HDSM_REGISTER_HEADSET_DATA_EVENTS_RESPONSE_SIZE        (sizeof(HDSM_Register_Headset_Data_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to un-register for Headset Manager Data   */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_DATA        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Un_Register_Headset_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataEventsHandlerID;
} HDSM_Un_Register_Headset_Data_Events_Request_t;

#define HDSM_UN_REGISTER_HEADSET_DATA_EVENTS_REQUEST_SIZE      (sizeof(HDSM_Un_Register_Headset_Data_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to un-register for Headset Manager Data   */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_DATA        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Un_Register_Headset_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDSM_Un_Register_Headset_Data_Events_Response_t;

#define HDSM_UN_REGISTER_HEADSET_DATA_EVENTS_RESPONSE_SIZE     (sizeof(HDSM_Un_Register_Headset_Data_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to query the SCO Handle for and active    */
   /* connection (Request).                                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Query_SCO_Connection_Handle_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventsHandlerID;
   BD_ADDR_t              RemoteDeviceAddress;
   HDSM_Connection_Type_t ConnectionType;
} HDSM_Query_SCO_Connection_Handle_Request_t;

#define HDSM_QUERY_SCO_CONNECTION_HANDLE_REQUEST_SIZE      (sizeof(HDSM_Query_SCO_Connection_Handle_Request_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message to query the SCO Handle for and active    */
   /* connection (Response).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Query_SCO_Connection_Handle_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   Word_t                SCOHandle;
} HDSM_Query_SCO_Connection_Handle_Response_t;

#define HDSM_QUERY_SCO_CONNECTION_HANDLE_RESPONSE_SIZE     (sizeof(HDSM_Query_SCO_Connection_Handle_Response_t))

   /* Headset Manager Asynchronous Message Formats.                     */

   /* Headset Manager Connection Management Asynchronous Message        */
   /* Formats.                                                          */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that a remote     */
   /* Headset/Audio Gateway device connection request has been received */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Connection_Request_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Connection_Request_Message_t;

#define HDSM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(HDSM_Connection_Request_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that a Headset/   */
   /* Audio Gateway device is currently connected (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTED                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Device_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Device_Connected_Message_t;

#define HDSM_DEVICE_CONNECTED_MESSAGE_SIZE                     (sizeof(HDSM_Device_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client of the specified  */
   /* connection status (asynchronously).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Device_Connection_Status_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ConnectionStatus;
} HDSM_Device_Connection_Status_Message_t;

#define HDSM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE             (sizeof(HDSM_Device_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the HDSM_Device_Connection_Status_Message_t message to describe*/
   /* the actual connection result status.                              */
#define HDSM_DEVICE_CONNECTION_STATUS_SUCCESS                  0x00000000
#define HDSM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT          0x00000001
#define HDSM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED          0x00000002
#define HDSM_DEVICE_CONNECTION_STATUS_FAILURE_SECURITY         0x00000003
#define HDSM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF 0x00000004
#define HDSM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN          0x00000005

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that a            */
   /* Headset/Audio Gateway connection is now disconnected              */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Device_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           DisconnectReason;
} HDSM_Device_Disconnected_Message_t;

#define HDSM_DEVICE_DISCONNECTED_MESSAGE_SIZE                  (sizeof(HDSM_Device_Disconnected_Message_t))

   /* The following constants are used with the DisconnectReason member */
   /* of the HDSM_Device_Disconnected_Message_t message to describe the */
   /* actual disconnect reason.                                         */
#define HDSM_DEVICE_DISCONNECT_REASON_NORMAL_DISCONNECT        0x00000000
#define HDSM_DEVICE_DISCONNECT_REASON_SERVICE_LEVEL_ERROR      0x00000001

   /* Headset Manager Audio Connection Management Asynchronous Message  */
   /* Formats.                                                          */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that a SCO Audio  */
   /* Headset/Audio Gateway connection is now connected/present         */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_AUDIO_CONNECTED                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Audio_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Audio_Connected_Message_t;

#define HDSM_AUDIO_CONNECTED_MESSAGE_SIZE                      (sizeof(HDSM_Audio_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client of the result of  */
   /* an attempted SCO Audio Headset/Audio Gateway audio connection     */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_AUDIO_CONNECTION_STATUS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Audio_Connection_Status_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              Successful;
} HDSM_Audio_Connection_Status_Message_t;

#define HDSM_AUDIO_CONNECTION_STATUS_MESSAGE_SIZE              (sizeof(HDSM_Audio_Connection_Status_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that a SCO Audio  */
   /* Headset/Audio Gateway connection is no longer connected/present   */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Audio_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Audio_Disconnected_Message_t;

#define HDSM_AUDIO_DISCONNECTED_MESSAGE_SIZE                   (sizeof(HDSM_Audio_Disconnected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that specifies received, raw SCO Audio    */
   /* data (asynchronously).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Audio_Data_Received_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           DataEventsHandlerID;
   unsigned long          AudioDataFlags;
   unsigned int           AudioDataLength;
   unsigned char          AudioData[1];
} HDSM_Audio_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold a an entire SCO audio data message given */
   /* the number of actual raw audio data bytes.  This function accepts */
   /* as it's input the total number individual audio data bytes are    */
   /* present starting from the AudioData member of the                 */
   /* HDSM_Audio_Data_Received_Message_t structure and returns the total*/
   /* number of bytes required to hold the entire message.              */
#define HDSM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(_x)              (STRUCTURE_OFFSET(HDSM_Audio_Data_Received_Message_t, AudioData) + ((unsigned int)(_x)))

   /* Headset Manager Headset/Audio Gateway Asynchronous Message        */
   /* Definitions.                                                      */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client of a received     */
   /* speaker gain indication event (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Speaker_Gain_Indication_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SpeakerGain;
} HDSM_Speaker_Gain_Indication_Message_t;

#define HDSM_SPEAKER_GAIN_INDICATION_MESSAGE_SIZE              (sizeof(HDSM_Speaker_Gain_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client of a received     */
   /* microphone gain indication event (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Microphone_Gain_Indication_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           MicrophoneGain;
} HDSM_Microphone_Gain_Indication_Message_t;

#define HDSM_MICROPHONE_GAIN_INDICATION_MESSAGE_SIZE           (sizeof(HDSM_Microphone_Gain_Indication_Message_t))

   /* Headset Manager Headset Asynchronous Message Definitions.         */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the local Headset client of a*/
   /* received ring indication event (asynchronously).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_RING_INDICATION_IND             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Ring_Indication_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDSM_Ring_Indication_Indication_Message_t;

#define HDSM_RING_INDICATION_INDICATION_MESSAGE_SIZE           (sizeof(HDSM_Ring_Indication_Indication_Message_t))

   /* Headset Manager Audio Gateway Asynchronous Message Definitions.   */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the local Audio Gateway      */
   /* client of a received button press indication event                */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDSM_MESSAGE_FUNCTION_BUTTON_PRESSED_IND              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDSM_Button_Pressed_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDSM_Button_Pressed_Indication_Message_t;

#define HDSM_BUTTON_PRESSED_INDICATION_MESSAGE_SIZE            (sizeof(HDSM_Button_Pressed_Indication_Message_t))

#endif

