/*****< hfrmmsg.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HFRMMSG - Defined Interprocess Communication Messages for the Hands       */
/*            Free (HFR) Manager for Stonestreet One Bluetopia Protocol       */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HFRMMSGH__
#define __HFRMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHFR.h"            /* Hands Free Framework Prototypes/Constants.*/

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HFRMType.h"            /* BTPM Hands Free Manager Type Definitions. */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Hands Free (HFR)*/
   /* Manager.                                                          */
#define BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER                     0x00001002

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Hands Free (HFR) */
   /* Manager.                                                          */

   /* Hands Free (HFR) Manager Commands.                                */
#define HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE         0x00001001
#define HFRM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE               0x00001002
#define HFRM_MESSAGE_FUNCTION_DISCONNECT_DEVICE                   0x00001003
#define HFRM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES             0x00001004

#define HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION         0x00001101
#define HFRM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS    0x00001102
                                                                  
#define HFRM_MESSAGE_FUNCTION_DISABLE_ECHO_NOISE_CANCELLATION     0x00001201
#define HFRM_MESSAGE_FUNCTION_SET_VOICE_RECOGNITION_ACTIVATION    0x00001202
#define HFRM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN                    0x00001203
#define HFRM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN                 0x00001204
                                                                  
#define HFRM_MESSAGE_FUNCTION_QUERY_CONTROL_INDICATOR_STATUS      0x00001301
#define HFRM_MESSAGE_FUNCTION_ENABLE_INDICATOR_NOTIFICATION       0x00001302
#define HFRM_MESSAGE_FUNCTION_QUERY_CALL_HOLD_MULTI_SUPPORT       0x00001303
#define HFRM_MESSAGE_FUNCTION_SEND_CALL_HOLD_MULTI_SELECTION      0x00001304
#define HFRM_MESSAGE_FUNCTION_ENABLE_CALL_WAIT_NOTIFICATION       0x00001305
#define HFRM_MESSAGE_FUNCTION_ENABLE_CALL_LINE_ID_NOTIFICATION    0x00001306
#define HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER                   0x00001307
#define HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEMORY       0x00001308
#define HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER           0x00001309
#define HFRM_MESSAGE_FUNCTION_ANSWER_INCOMING_CALL                0x0000130A
#define HFRM_MESSAGE_FUNCTION_TRANSMIT_DTMF_CODE                  0x0000130B
#define HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST                   0x0000130C
#define HFRM_MESSAGE_FUNCTION_HANG_UP_CALL                        0x0000130D
#define HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST            0x0000130E
#define HFRM_MESSAGE_FUNCTION_SET_NETWORK_OPERATOR_FORMAT         0x0000130F
#define HFRM_MESSAGE_FUNCTION_QUERY_NETWORK_OPERATOR_SELECTION    0x00001310
#define HFRM_MESSAGE_FUNCTION_ENABLE_EXTENDED_ERROR_RESULT        0x00001311
#define HFRM_MESSAGE_FUNCTION_QUERY_SUBSCRIBER_NUMBER_INFO        0x00001312
#define HFRM_MESSAGE_FUNCTION_QUERY_RESPONSE_HOLD_STATUS          0x00001313
#define HFRM_MESSAGE_FUNCTION_SET_INCOMING_CALL_STATE             0x00001314
#define HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_COMMAND              0x00001315
#define HFRM_MESSAGE_FUNCTION_SEND_AVAILABLE_CODEC_LIST           0x00001316
#define HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_NOTIFICATION_STATE 0x00001317

#define HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS             0x00001401
#define HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS_BY_NAME     0x00001402
#define HFRM_MESSAGE_FUNCTION_SEND_CALL_WAITING_NOTIFICATION      0x00001403
#define HFRM_MESSAGE_FUNCTION_SEND_CALL_LINE_ID_NOTIFICATION      0x00001404
#define HFRM_MESSAGE_FUNCTION_RING_INDICATION                     0x00001405
#define HFRM_MESSAGE_FUNCTION_ENABLE_IN_BAND_RING_TONE_SETTING    0x00001406
#define HFRM_MESSAGE_FUNCTION_VOICE_TAG_RESPONSE                  0x00001407
#define HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V1          0x00001408    /* DEPRECATED */
#define HFRM_MESSAGE_FUNCTION_SEND_NETWORK_OPERATOR_SELECTION     0x00001409
#define HFRM_MESSAGE_FUNCTION_SEND_EXTENDED_ERROR_RESULT          0x0000140A
#define HFRM_MESSAGE_FUNCTION_SEND_SUBSCRIBER_NUMBER_INFO         0x0000140B
#define HFRM_MESSAGE_FUNCTION_SEND_INCOMING_CALL_STATE            0x0000140C
#define HFRM_MESSAGE_FUNCTION_SEND_TERMINATING_RESPONSE           0x0000140D
#define HFRM_MESSAGE_FUNCTION_ENABLE_ARBITRARY_CMD_PROCESSING     0x0000140E
#define HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_RESPONSE             0x0000140F
#define HFRM_MESSAGE_FUNCTION_SEND_SELECT_CODEC                   0x00001410
#define HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V2          0x00001411

#define HFRM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION              0x00001501
#define HFRM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION            0x00001502
#define HFRM_MESSAGE_FUNCTION_SEND_AUDIO_DATA                     0x00001503
#define HFRM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE         0x00001504
                                                                  
#define HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_EVENTS          0x00002001
#define HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_EVENTS       0x00002002
                                                                  
#define HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_DATA            0x00002101
#define HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_DATA         0x00002102
                                                                  
   /* Hands Free (HFR) Manager Asynchronous Events.                     */
#define HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST                  0x00010001
#define HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTED                    0x00010002
#define HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS            0x00010003
#define HFRM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED                 0x00010004
#define HFRM_MESSAGE_FUNCTION_SERVICE_LEVEL_CONNECTION            0x00010005
#define HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTED                     0x00010006
#define HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTION_STATUS             0x00010007
#define HFRM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED                  0x00010008
#define HFRM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED                 0x00010009
#define HFRM_MESSAGE_FUNCTION_VOICE_RECOGNITION_IND               0x0001000A
#define HFRM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND                    0x0001000B
#define HFRM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND                 0x0001000C
#define HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_IND             0x0001000D
                                                                  
#define HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_CFM             0x00011001
#define HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_IND        0x00011002
#define HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_CFM        0x00011003
#define HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SUPPORT_CFM         0x00011004
#define HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOTIFICATION_IND          0x00011005
#define HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOTIFICATION_IND       0x00011006
#define HFRM_MESSAGE_FUNCTION_RING_INDICATION_IND                 0x00011007
#define HFRM_MESSAGE_FUNCTION_IN_BAND_RING_TONE_SETTING_IND       0x00011008
#define HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_CFM               0x00011009
#define HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V1     0x0001100A    /* DEPRECATED */
#define HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_CFM      0x0001100B
#define HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INFO_CFM          0x0001100C
#define HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_CFM            0x0001100D
#define HFRM_MESSAGE_FUNCTION_COMMAND_RESULT                      0x0001100E
#define HFRM_MESSAGE_FUNCTION_ARBITRARY_RESPONSE                  0x0001100F
#define HFRM_MESSAGE_FUNCTION_SELECT_CODEC_IND                    0x00011010
#define HFRM_MESSAGE_FUNCTION_INDICATOR_NOTIFICATION_STATE_IND    0x00011011
#define HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V2     0x00011012

#define HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SELECTION_IND       0x00012001
#define HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOT_ACTIVATION_IND        0x00012002
#define HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOT_ACTIVATION_IND     0x00012003
#define HFRM_MESSAGE_FUNCTION_DISABLE_SOUND_ENHANCEMENT_IND       0x00012004
#define HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_IND               0x00012005
#define HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEM_IND      0x00012006
#define HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER_IND       0x00012007
#define HFRM_MESSAGE_FUNCTION_GENERATE_DTMF_CODE_IND              0x00012008
#define HFRM_MESSAGE_FUNCTION_ANSWER_CALL_IND                     0x00012009
#define HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_IND               0x0001200A
#define HFRM_MESSAGE_FUNCTION_HANG_UP_IND                         0x0001200B
#define HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_IND        0x0001200C
#define HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_FORMAT_IND         0x0001200D
#define HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_IND      0x0001200E
#define HFRM_MESSAGE_FUNCTION_EXTENDED_ERROR_RESULT_ACT_IND       0x0001200F
#define HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INF_IND           0x00012010
#define HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_IND            0x00012011
#define HFRM_MESSAGE_FUNCTION_ARBITRARY_COMMAND_IND               0x00012012
#define HFRM_MESSAGE_FUNCTION_AVAILABLE_CODEC_LIST_IND            0x00012013
#define HFRM_MESSAGE_FUNCTION_SELECT_CODEC_CFM                    0x00012014
#define HFRM_MESSAGE_FUNCTION_CONNECT_CODEC_IND                   0x00012015

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Hands Free (HFR)        */
   /* Manager.                                                          */

   /* Hands Free (HFR) Manager Manager Command/Response Message Formats.*/

   /* Hands Free/Audio Gateway Message Definitions.                     */

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to respond to an incoming              */
   /* Connection/Authorization (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              Accept;
} HFRM_Connection_Request_Response_Request_t;

#define HFRM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(HFRM_Connection_Request_Response_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to respond to an incoming              */
   /* Connection/Authorization (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Connection_Request_Response_Response_t;

#define HFRM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(HFRM_Connection_Request_Response_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to connect to a remote device          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Connect_Remote_Device_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   unsigned int           RemoteServerPort;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned long          ConnectionFlags;
} HFRM_Connect_Remote_Device_Request_t;

#define HFRM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(HFRM_Connect_Remote_Device_Request_t))

   /* The following constants are used with the ConnectFlags member of  */
   /* the HFRM_Connect_Remote_Device_Request_t message to control       */
   /* various connection options.                                       */
#define HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION   0x00000001
#define HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION       0x00000002

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to connect to a remote device          */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Connect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Connect_Remote_Device_Response_t;

#define HFRM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(HFRM_Connect_Remote_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to disconnect a currently connected    */
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DISCONNECT_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Disconnect_Device_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Disconnect_Device_Request_t;

#define HFRM_DISCONNECT_DEVICE_REQUEST_SIZE                    (sizeof(HFRM_Disconnect_Device_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to disconnect a currently connected    */
   /* device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DISCONNECT_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Disconnect_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Disconnect_Device_Response_t;

#define HFRM_DISCONNECT_DEVICE_RESPONSE_SIZE                   (sizeof(HFRM_Disconnect_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager Message to query the currently connected       */
   /* devices (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Connected_Devices_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
} HFRM_Query_Connected_Devices_Request_t;

#define HFRM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE              (sizeof(HFRM_Query_Connected_Devices_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager Message to query the currently connected       */
   /* devices (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Connected_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberDevicesConnected;
   BD_ADDR_t             DeviceConnectedList[1];
} HFRM_Query_Connected_Devices_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire query connected devices        */
   /* request message given the number of connected devices.  This MACRO*/
   /* accepts as it's input the total number of connected devices (NOT  */
   /* bytes) that are present starting from the DeviceConnectedList     */
   /* member of the HFRM_Query_Connected_Devices_Response_t structure   */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define HFRM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(_x)         (STRUCTURE_OFFSET(HFRM_Query_Connected_Devices_Response_t, DeviceConnectedList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager Message to query the current configuration for */
   /* the specified connection type (Request).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Current_Configuration_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
} HFRM_Query_Current_Configuration_Request_t;

#define HFRM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE          (sizeof(HFRM_Query_Current_Configuration_Request_t))

   /* The following structure is a container structure that is used to  */
   /* hold an additional indicator information entry (when querying the */
   /* current configuration).                                           */
   /* * NOTE * The IndicatorDescription member *MUST* be NULL           */
   /*          terminated.                                              */
typedef struct _tagHFRM_Configuration_Indicator_Entry_t
{
   char                                     IndicatorDescription[HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM + 1];
   HFRE_Control_Indicator_Type_t            ControlIndicatorType;
   union
   {
      HFRE_Control_Indicator_Range_Type_t   ControlIndicatorRangeType;
      HFRE_Control_Indicator_Boolean_Type_t ControlIndicatorBooleanType;
   } Control_Indicator_Data;
} HFRM_Configuration_Indicator_Entry_t;

#define HFRM_CONFIGURATION_INDICATOR_ENTRY_SIZE                (sizeof(HFRM_Configuration_Indicator_Entry_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager Message to query the current configuration for */
   /* the specified connection type (Response).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Current_Configuration_Response_t
{
   BTPM_Message_Header_t                MessageHeader;
   int                                  Status;
   unsigned long                        IncomingConnectionFlags;
   unsigned long                        SupportedFeaturesMask;
   unsigned long                        CallHoldingSupportMask;
   unsigned long                        NetworkType;
   unsigned int                         TotalNumberAdditionalIndicators;
   HFRM_Configuration_Indicator_Entry_t AdditionalIndicatorList[1];
} HFRM_Query_Current_Configuration_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire query current configuration    */
   /* request message given the number of additional indicators.  This  */
   /* MACRO accepts as it's input the total number of additional        */
   /* indicators (NOT bytes) that are present starting from the         */
   /* AdditionalIndicatorList member of the                             */
   /* HFRM_Query_Current_Configuration_Response_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(_x)     (STRUCTURE_OFFSET(HFRM_Query_Current_Configuration_Response_t, AdditionalIndicatorList) + (unsigned int)((sizeof(BD_ADDR_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to change the current incoming         */
   /* connection flags of a connection (Request).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Change_Incoming_Connection_Flags_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   unsigned long          ConnectionFlags;
} HFRM_Change_Incoming_Connection_Flags_Request_t;

#define HFRM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE     (sizeof(HFRM_Change_Incoming_Connection_Flags_Request_t))

   /* The following constants are used with the ConnectionFlags member  */
   /* of the HFRM_Change_Incoming_Connection_Flags_Request_t structure  */
   /* to specify the various flags to apply to incoming Connections.    */
#define HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION   0x00000001
#define HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION  0x00000002
#define HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION      0x00000004

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to change the current incoming         */
   /* connection flags of a connection (Response).                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Change_Incoming_Connection_Flags_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Change_Incoming_Connection_Flags_Response_t;

#define HFRM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE    (sizeof(HFRM_Change_Incoming_Connection_Flags_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the disabling Echo/Noise    */
   /* Cancellation processing on the remote device (Request).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DISABLE_ECHO_NOISE_CANCELLATION */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Disable_Echo_Noise_Cancellation_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Disable_Echo_Noise_Cancellation_Request_t;

#define HFRM_DISABLE_ECHO_NOISE_CANCELLATION_REQUEST_SIZE      (sizeof(HFRM_Disable_Echo_Noise_Cancellation_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the disabling Echo/Noise    */
   /* Cancellation processing on the remote device (Response).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DISABLE_ECHO_NOISE_CANCELLATION */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Disable_Echo_Noise_Cancellation_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Disable_Echo_Noise_Cancellation_Response_t;

#define HFRM_DISABLE_ECHO_NOISE_CANCELLATION_RESPONSE_SIZE     (sizeof(HFRM_Disable_Echo_Noise_Cancellation_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the activation/de-activation*/
   /* of voice recognition on the remote device (Request).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_VOICE_RECOGNITION_ACTIVATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Voice_Recognition_Activation_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              VoiceRecognitionActive;
} HFRM_Set_Voice_Recognition_Activation_Request_t;

#define HFRM_SET_VOICE_RECOGNITION_ACTIVATION_REQUEST_SIZE     (sizeof(HFRM_Set_Voice_Recognition_Activation_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the activation/de-activation*/
   /* of voice recognition on the remote device (Response).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_VOICE_RECOGNITION_ACTIVATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Voice_Recognition_Activation_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Set_Voice_Recognition_Activation_Response_t;

#define HFRM_SET_VOICE_RECOGNITION_ACTIVATION_RESPONSE_SIZE    (sizeof(HFRM_Set_Voice_Recognition_Activation_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the setting of the speaker  */
   /* gain on the remote device (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Speaker_Gain_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SpeakerGain;
} HFRM_Set_Speaker_Gain_Request_t;

#define HFRM_SET_SPEAKER_GAIN_REQUEST_SIZE                     (sizeof(HFRM_Set_Speaker_Gain_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the setting of the speaker  */
   /* gain on the remote device (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Speaker_Gain_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Set_Speaker_Gain_Response_t;

#define HFRM_SET_SPEAKER_GAIN_RESPONSE_SIZE                    (sizeof(HFRM_Set_Speaker_Gain_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the setting of the          */
   /* microphone gain on the remote device (Request).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Microphone_Gain_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           MicrophoneGain;
} HFRM_Set_Microphone_Gain_Request_t;

#define HFRM_SET_MICROPHONE_GAIN_REQUEST_SIZE                  (sizeof(HFRM_Set_Microphone_Gain_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the setting of the          */
   /* microphone gain on the remote device (Response).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Microphone_Gain_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Set_Microphone_Gain_Response_t;

#define HFRM_SET_MICROPHONE_GAIN_RESPONSE_SIZE                 (sizeof(HFRM_Set_Microphone_Gain_Response_t))

   /* Hands Free Message Definitions.                                   */

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the control indicator status  */
   /* of the remote Audio Gateway device (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CONTROL_INDICATOR_STATUS  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Control_Indicator_Status_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Query_Control_Indicator_Status_Request_t;

#define HFRM_QUERY_CONTROL_INDICATOR_STATUS_REQUEST_SIZE       (sizeof(HFRM_Query_Control_Indicator_Status_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the control indicator status  */
   /* of the remote Audio Gateway device (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CONTROL_INDICATOR_STATUS  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Control_Indicator_Status_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Query_Control_Indicator_Status_Response_t;

#define HFRM_QUERY_CONTROL_INDICATOR_STATUS_RESPONSE_SIZE      (sizeof(HFRM_Query_Control_Indicator_Status_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable indicator event notification */
   /* on the remote Audio Gateway device (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_INDICATOR_NOTIFICATION   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Indicator_Event_Notification_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             EnableEventNotification;
} HFRM_Enable_Indicator_Event_Notification_Request_t;

#define HFRM_ENABLE_INDICATOR_EVENT_NOTIFICATION_REQUEST_SIZE  (sizeof(HFRM_Enable_Indicator_Event_Notification_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable indicator event notification */
   /* on the remote Audio Gateway device (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_INDICATOR_NOTIFICATION   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Indicator_Event_Notification_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Enable_Indicator_Event_Notification_Response_t;

#define HFRM_ENABLE_INDICATOR_EVENT_NOTIFICATION_RESPONSE_SIZE (sizeof(HFRM_Enable_Indicator_Event_Notification_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the call holding/multi-party  */
   /* support of the remote Audio Gateway device (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CALL_HOLD_MULTI_SUPPORT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Call_Holding_Multiparty_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Query_Call_Holding_Multiparty_Request_t;

#define HFRM_QUERY_CALL_HOLDING_MULTIPARTY_REQUEST_SIZE        (sizeof(HFRM_Query_Call_Holding_Multiparty_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the call holding/multi-party  */
   /* support of the remote Audio Gateway device (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CALL_HOLD_MULTI_SUPPORT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Call_Holding_Multiparty_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Query_Call_Holding_Multiparty_Response_t;

#define HFRM_QUERY_CALL_HOLDING_MULTIPARTY_RESPONSE_SIZE       (sizeof(HFRM_Query_Call_Holding_Multiparty_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the call holding/multi-party   */
   /* selection on the remote Audio Gateway device (Request).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CALL_HOLD_MULTI_SELECTION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Call_Holding_Multiparty_Selection_Request_t
{
   BTPM_Message_Header_t                     MessageHeader;
   unsigned int                              ControlEventsHandlerID;
   BD_ADDR_t                                 RemoteDeviceAddress;
   HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling;
   unsigned int                              Index;
} HFRM_Send_Call_Holding_Multiparty_Selection_Request_t;

#define HFRM_SEND_CALL_HOLDING_MULTIPARTY_SELECTION_REQUEST_SIZE  (sizeof(HFRM_Send_Call_Holding_Multiparty_Selection_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the call holding/multi-party   */
   /* selection on the remote Audio Gateway device (Response).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CALL_HOLD_MULTI_SELECTION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Call_Holding_Multiparty_Selection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Call_Holding_Multiparty_Selection_Response_t;

#define HFRM_SEND_CALL_HOLDING_MULTIPARTY_SELECTION_RESPONSE_SIZE (sizeof(HFRM_Send_Call_Holding_Multiparty_Selection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable call waiting notification    */
   /* on the remote Audio Gateway device (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_CALL_WAIT_NOTIFICATION   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Call_Waiting_Notification_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             EnableNotification;
} HFRM_Enable_Call_Waiting_Notification_Request_t;

#define HFRM_ENABLE_CALL_WAITING_NOTIFICATION_REQUEST_SIZE  (sizeof(HFRM_Enable_Call_Waiting_Notification_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable call waiting notification    */
   /* on the remote Audio Gateway device (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_CALL_WAIT_NOTIFICATION   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Call_Waiting_Notification_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Enable_Call_Waiting_Notification_Response_t;

#define HFRM_ENABLE_CALL_WAITING_NOTIFICATION_RESPONSE_SIZE (sizeof(HFRM_Enable_Call_Waiting_Notification_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable call line ID notification    */
   /* on the remote Audio Gateway device (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_CALL_LINE_ID_NOTIFICATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Call_Line_Identification_Notification_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             EnableNotification;
} HFRM_Enable_Call_Line_Identification_Notification_Request_t;

#define HFRM_ENABLE_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE  (sizeof(HFRM_Enable_Call_Line_Identification_Notification_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable call line ID notification    */
   /* on the remote Audio Gateway device (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_CALL_LINE_ID_NOTIFICATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Call_Line_Identification_Notification_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Enable_Call_Line_Identification_Notification_Response_t;

#define HFRM_ENABLE_CALL_LINE_IDENTIFICATION_NOTIFICATION_RESPONSE_SIZE (sizeof(HFRM_Enable_Call_Line_Identification_Notification_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to dial a phone number on the remote   */
   /* Audio Gateway device (Request).                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER               */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Dial_Phone_Number_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Dial_Phone_Number_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire dial phone number request      */
   /* message given the length (in characters) of the dial string/phone */
   /* number.  This MACRO accepts as it's input the total number of     */
   /* characters (including the NULL terminator) that are present       */
   /* starting from the PhoneNumber member of the                       */
   /* HFRM_Dial_Phone_Number_Request_t structure and returns the total  */
   /* number of bytes required to hold the entire message.              */
#define HFRM_DIAL_PHONE_NUMBER_REQUEST_SIZE(_x)                (STRUCTURE_OFFSET(HFRM_Dial_Phone_Number_Request_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to dial a phone number on the remote   */
   /* Audio Gateway device (Response).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Dial_Phone_Number_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Dial_Phone_Number_Response_t;

#define HFRM_DIAL_PHONE_NUMBER_RESPONSE_SIZE                   (sizeof(HFRM_Dial_Phone_Number_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to dial a phone number (from memory) on*/
   /* the remote Audio Gateway device (Request).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEMORY   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Dial_Phone_Number_From_Memory_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          MemoryLocation;
} HFRM_Dial_Phone_Number_From_Memory_Request_t;

#define HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_REQUEST_SIZE        (sizeof(HFRM_Dial_Phone_Number_From_Memory_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to dial a phone number (from memory) on*/
   /* the remote Audio Gateway device (Response).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEMORY   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Dial_Phone_Number_From_Memory_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Dial_Phone_Number_From_Memory_Response_t;

#define HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_RESPONSE_SIZE       (sizeof(HFRM_Dial_Phone_Number_From_Memory_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to re-dial the last called phone number*/
   /* on the remote Audio Gateway device (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Re_Dial_Last_Phone_Number_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Re_Dial_Last_Phone_Number_Request_t;

#define HFRM_RE_DIAL_LAST_PHONE_NUMBER_REQUEST_SIZE            (sizeof(HFRM_Re_Dial_Last_Phone_Number_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to re-dial the last called phone number*/
   /* on the remote Audio Gateway device (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Re_Dial_Last_Phone_Number_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Re_Dial_Last_Phone_Number_Response_t;

#define HFRM_RE_DIAL_LAST_PHONE_NUMBER_RESPONSE_SIZE           (sizeof(HFRM_Re_Dial_Last_Phone_Number_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to answer an incoming phone call on the*/
   /* remote Audio Gateway device (Request).                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ANSWER_INCOMING_CALL            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Answer_Incoming_Call_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Answer_Incoming_Call_Request_t;

#define HFRM_ANSWER_INCOMING_CALL_REQUEST_SIZE                 (sizeof(HFRM_Answer_Incoming_Call_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to answer an incoming phone call on the*/
   /* remote Audio Gateway device (Response).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ANSWER_INCOMING_CALL            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Answer_Incoming_Call_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Answer_Incoming_Call_Response_t;

#define HFRM_ANSWER_INCOMING_CALL_RESPONSE_SIZE                (sizeof(HFRM_Answer_Incoming_Call_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to instruct the remote Audio Gateway to*/
   /* transmit a DTMF code (Request).                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_TRANSMIT_DTMF_CODE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Transmit_DTMF_Code_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   char                  DTMFCode;
} HFRM_Transmit_DTMF_Code_Request_t;

#define HFRM_TRANSMIT_DTMF_CODE_REQUEST_SIZE                   (sizeof(HFRM_Transmit_DTMF_Code_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to instruct the remote Audio Gateway to*/
   /* transmit a DTMF code (Response).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_TRANSMIT_DTMF_CODE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Transmit_DTMF_Code_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Transmit_DTMF_Code_Response_t;

#define HFRM_TRANSMIT_DTMF_CODE_RESPONSE_SIZE                  (sizeof(HFRM_Transmit_DTMF_Code_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the phone number from the   */
   /* remote Audio Gateway to be associated with a voice tag (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Voice_Tag_Request_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Voice_Tag_Request_Request_t;

#define HFRM_VOICE_TAG_REQUEST_REQUEST_SIZE                    (sizeof(HFRM_Voice_Tag_Request_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to request the phone number from the   */
   /* remote Audio Gateway to be associated with a voice tag (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Voice_Tag_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Voice_Tag_Request_Response_t;

#define HFRM_VOICE_TAG_REQUEST_RESPONSE_SIZE                   (sizeof(HFRM_Voice_Tag_Request_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to hang up a phone call on the remote  */
   /* Audio Gateway device (Request).                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_HANG_UP_CALL                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Hang_Up_Call_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Hang_Up_Call_Request_t;

#define HFRM_HANG_UP_CALL_REQUEST_SIZE                         (sizeof(HFRM_Hang_Up_Call_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to hang up a phone call on the remote  */
   /* Audio Gateway device (Response).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_HANG_UP_CALL                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Hang_Up_Call_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Hang_Up_Call_Response_t;

#define HFRM_HANG_UP_CALL_RESPONSE_SIZE                        (sizeof(HFRM_Hang_Up_Call_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the remote Audio Gateway for  */
   /* the current calls list (Request).                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Current_Calls_List_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Query_Current_Calls_List_Request_t;

#define HFRM_QUERY_CURRENT_CALLS_LIST_REQUEST_SIZE             (sizeof(HFRM_Query_Current_Calls_List_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the remote Audio Gateway for  */
   /* the current calls list (Response).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Current_Calls_List_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Query_Current_Calls_List_Response_t;

#define HFRM_QUERY_CURRENT_CALLS_LIST_RESPONSE_SIZE            (sizeof(HFRM_Query_Current_Calls_List_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to set the current network operator    */
   /* format on the remote Audio Gateway device (Request).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_NETWORK_OPERATOR_FORMAT     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Network_Operator_Selection_Format_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Set_Network_Operator_Selection_Format_Request_t;

#define HFRM_SET_NETWORK_OPERATOR_SELECTION_FORMAT_REQUEST_SIZE   (sizeof(HFRM_Set_Network_Operator_Selection_Format_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to set the current network operator    */
   /* format on the remote Audio Gateway device (Response).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_NETWORK_OPERATOR_FORMAT     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Network_Operator_Selection_Format_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Set_Network_Operator_Selection_Format_Response_t;

#define HFRM_SET_NETWORK_OPERATOR_SELECTION_FORMAT_RESPONSE_SIZE  (sizeof(HFRM_Set_Network_Operator_Selection_Format_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the current network operator  */
   /* format on the remote Audio Gateway device (Request).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_NETWORK_OPERATOR_SELECTION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Network_Operator_Selection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Query_Network_Operator_Selection_Request_t;

#define HFRM_QUERY_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE     (sizeof(HFRM_Query_Network_Operator_Selection_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the current network operator  */
   /* format on the remote Audio Gateway device (Response).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_NETWORK_OPERATOR_SELECTION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Network_Operator_Selection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Query_Network_Operator_Selection_Response_t;

#define HFRM_QUERY_NETWORK_OPERATOR_SELECTION_RESPONSE_SIZE   (sizeof(HFRM_Query_Network_Operator_Selection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable extended error results on the*/
   /* remote Audio Gateway device (Request).                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_EXTENDED_ERROR_RESULT    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Extended_Error_Result_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             EnableExtendedErrorResults;
} HFRM_Enable_Extended_Error_Result_Request_t;

#define HFRM_ENABLE_EXTENDED_ERROR_RESULT_REQUEST_SIZE         (sizeof(HFRM_Enable_Extended_Error_Result_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable extended error results on the*/
   /* remote Audio Gateway device (Response).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_EXTENDED_ERROR_RESULT    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Extended_Error_Result_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Enable_Extended_Error_Result_Response_t;

#define HFRM_ENABLE_EXTENDED_ERROR_RESULT_RESPONSE_SIZE        (sizeof(HFRM_Enable_Extended_Error_Result_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the subscriber number         */
   /* information from the remote Audio Gateway device (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_SUBSCRIBER_NUMBER_INFO    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Subscriber_Number_Information_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Query_Subscriber_Number_Information_Request_t;

#define HFRM_QUERY_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE  (sizeof(HFRM_Query_Subscriber_Number_Information_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the subscriber number         */
   /* information from the remote Audio Gateway device (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_SUBSCRIBER_NUMBER_INFO    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Subscriber_Number_Information_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Query_Subscriber_Number_Information_Response_t;

#define HFRM_QUERY_SUBSCRIBER_NUMBER_INFORMATION_RESPONSE_SIZE (sizeof(HFRM_Query_Subscriber_Number_Information_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the response/hold status      */
   /* from the remote Audio Gateway device (Request).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_RESPONSE_HOLD_STATUS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Response_Hold_Status_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Query_Response_Hold_Status_Request_t;

#define HFRM_QUERY_RESPONSE_HOLD_STATUS_REQUEST_SIZE           (sizeof(HFRM_Query_Response_Hold_Status_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the response/hold status      */
   /* from the remote Audio Gateway device (Response).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_RESPONSE_HOLD_STATUS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Response_Hold_Status_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Query_Response_Hold_Status_Response_t;

#define HFRM_QUERY_RESPONSE_HOLD_STATUS_RESPONSE_SIZE          (sizeof(HFRM_Query_Response_Hold_Status_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to set the incoming call state on the  */
   /* remote Audio Gateway device (Request).                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_INCOMING_CALL_STATE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Incoming_Call_State_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HFRE_Call_State_t     CallState;
} HFRM_Set_Incoming_Call_State_Request_t;

#define HFRM_SET_INCOMING_CALL_STATE_REQUEST_SIZE              (sizeof(HFRM_Set_Incoming_Call_State_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to set the incoming call state on the  */
   /* remote Audio Gateway device (Response).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SET_INCOMING_CALL_STATE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Set_Incoming_Call_State_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Set_Incoming_Call_State_Response_t;

#define HFRM_SET_INCOMING_CALL_STATE_RESPONSE_SIZE             (sizeof(HFRM_Set_Incoming_Call_State_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send an arbitrary command to the    */
   /* remote Audio Gateway device (Request).                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_COMMAND          */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The ArbitraryCommand member *MUST* be NULL               */
   /*          terminated and the ArbitraryCommandLength member *MUST*  */
   /*          include the NULL terminator.                             */
typedef struct _tagHFRM_Send_Arbitrary_Command_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ArbitraryCommandLength;
   char                  ArbitraryCommand[1];
} HFRM_Send_Arbitrary_Command_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send arbitrary command request */
   /* message given the length (in characters) of the arbitrary command.*/
   /* This MACRO accepts as it's input the total number of characters   */
   /* (including the NULL terminator) that are present starting from the*/
   /* ArbitraryCommand member of the                                    */
   /* HFRM_Send_Arbitrary_Command_Request_t structure and returns the   */
   /* total number of bytes required to hold the entire message.        */
#define HFRM_SEND_ARBITRARY_COMMAND_REQUEST_SIZE(_x)           (STRUCTURE_OFFSET(HFRM_Send_Arbitrary_Command_Request_t, ArbitraryCommand) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send an arbitrary command to the    */
   /* remote Audio Gateway device (Response).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_COMMAND          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Arbitrary_Command_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Arbitrary_Command_Response_t;

#define HFRM_SEND_ARBITRARY_COMMAND_RESPONSE_SIZE              (sizeof(HFRM_Send_Arbitrary_Command_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the supported codecs to the    */
   /* remote Audio Gateway device (Request).                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_AVAILABLE_CODEC_LIST       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Available_Codec_List_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          NumberSupportedCodecs;
   unsigned char         AvailableCodecList[HFRE_MAX_SUPPORTED_CODECS];
} HFRM_Send_Available_Codec_List_Request_t;

#define HFRM_SEND_AVAILABLE_CODEC_LIST_REQUEST_SIZE            (sizeof(HFRM_Send_Available_Codec_List_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the supported codecs to the    */
   /* remote Audio Gateway device (Response).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_AVAILABLE_CODEC_LIST       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Available_Codec_List_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Available_Codec_List_Response_t;

#define HFRM_SEND_AVAILABLE_CODEC_LIST_RESPONSE_SIZE           (sizeof(HFRM_Send_Available_Codec_List_Response_t))

   /* The following structure is a container structure that is used to  */
   /* hold the Notification Update information entry.                   */
   /* * NOTE * The IndicatorDescription member *MUST* be NULL           */
   /*          terminated.                                              */
typedef struct _tagHFRM_Notification_Update_Entry_t
{
   HFRE_Notification_Update_t NotificationUpdate;
   unsigned int               IndicatorDescriptionLength;
   char                       IndicatorDescription[HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM + 1];
} HFRM_Notification_Update_Entry_t;

#define HFRM_NOTIFICATION_UPDATE_ENTRY_SIZE                (sizeof(HFRM_Notification_Update_Entry_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable individual indicator event   */
   /* notification on the remote Audio Gateway device (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*          HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_NOTIFICATION_STATE*/
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Update_Indicator_Notification_State_Request_t
{
   BTPM_Message_Header_t            MessageHeader;
   unsigned int                     ControlEventsHandlerID;
   BD_ADDR_t                        RemoteDeviceAddress;
   unsigned int                     NumberUpdateIndicators;
   HFRM_Notification_Update_Entry_t NotificationUpdateList[1];
} HFRM_Update_Indicator_Notification_State_Request_t;

#define HFRM_UPDATE_INDICATOR_NOTIFICATION_STATE_REQUEST_SIZE(_x)  (STRUCTURE_OFFSET(HFRM_Update_Indicator_Notification_State_Request_t, NotificationUpdateList) + (unsigned int)((sizeof(HFRM_Notification_Update_Entry_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable individual indicator event   */
   /* notification on the remote Audio Gateway device (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*          HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_NOTIFICATION_STATE*/
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Update_Indicator_Notification_State_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Update_Indicator_Notification_State_Response_t;

#define HFRM_UPDATE_INDICATOR_NOTIFICATION_STATE_RESPONSE_SIZE  (sizeof(HFRM_Update_Indicator_Notification_State_Response_t))

   /* Audio Gateway Message Definitions.                                */

   /* The following structure is used to hold all information related to*/
   /* the update of control indicators.                                 */
   /* * NOTE * This structure is used with the                          */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS         */
   /*                                                                   */
   /*          Request message                                          */
   /*          (HFRM_Update_Control_Indicator_Status_Request_t          */
   /*          structure) to specify an individual indicator update     */
   /*          entry.  This structure is required because memory needs  */
   /*          to be allocated for the IndicatorDescription that is     */
   /*          part of the Indicator.  The actual IndicatorDescription  */
   /*          of the HFRE_Indicator_Update_t structure that is part of */
   /*          this structure (in the IndicatorUpdate member) should be */
   /*          set to NULL in each entry.  The indicator description    */
   /*          should be specified in the IndicatorDescriptionLength    */
   /*          and IndicatorDescription members.                        */
   /* * NOTE * The IndicatorDescription member *MUST* be NULL           */
   /*          terminated.                                              */
typedef struct _tagHFRM_Indicator_Update_List_Entry_t
{
   HFRE_Indicator_Update_t IndicatorUpdate;
   unsigned int            IndicatorDescriptionLength;
   char                    IndicatorDescription[HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM + 1];
} HFRM_Indicator_Update_List_Entry_t;

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send control indicator status       */
   /* updates to a remote Hands Free device (Request).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Update_Control_Indicator_Status_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ControlEventsHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
   unsigned int                       NumberUpdateIndicators;
   HFRM_Indicator_Update_List_Entry_t UpdateIndicatorsList[1];
} HFRM_Update_Control_Indicator_Status_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire update control indicator status*/
   /* request message given the length (in individual indicator list    */
   /* entries - NOT bytes) of the update message.  This MACRO accepts as*/
   /* it's input the total number of indicator list entries (NOT bytes) */
   /* that are present starting from the UpdateIndicatorsList member of */
   /* the HFRM_Update_Control_Indicator_Status_Request_t structure and  */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HFRM_UPDATE_CONTROL_INDICATOR_STATUS_REQUEST_SIZE(_x)  (STRUCTURE_OFFSET(HFRM_Update_Control_Indicator_Status_Request_t, UpdateIndicatorsList) + (unsigned int)((sizeof(HFRM_Indicator_Update_List_Entry_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send control indicator status       */
   /* updates to a remote Hands Free device (Response).                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Update_Control_Indicator_Status_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Update_Control_Indicator_Status_Response_t;

#define HFRM_UPDATE_CONTROL_INDICATOR_STATUS_RESPONSE_SIZE     (sizeof(HFRM_Update_Control_Indicator_Status_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send control indicator status       */
   /* update (by name) to a remote Hands Free device (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS_BY_NAME */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Update_Control_Indicator_Status_By_Name_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          IndicatorValue;
   unsigned int          IndicatorNameLength;
   char                  IndicatorName[1];
} HFRM_Update_Control_Indicator_Status_By_Name_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire update control indicator status*/
   /* by name request message given the length (in characters) of the   */
   /* indicator name.  This MACRO accepts as it's input the total number*/
   /* of characters (non NULL terminated) that are present starting from*/
   /* the IndicatorName member of the                                   */
   /* HFRM_Update_Control_Indicator_Status_By_Name_Request_t structure  */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_REQUEST_SIZE(_x)   (STRUCTURE_OFFSET(HFRM_Update_Control_Indicator_Status_By_Name_Request_t, IndicatorName) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send control indicator status       */
   /* update (by name) to a remote Hands Free device (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS_BY_NAME */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Update_Control_Indicator_Status_By_Name_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Update_Control_Indicator_Status_By_Name_Response_t;

#define HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_RESPONSE_SIZE      (sizeof(HFRM_Update_Control_Indicator_Status_By_Name_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a call waiting notification    */
   /* event to a remote Hands Free device (Request).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CALL_WAITING_NOTIFICATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Send_Call_Waiting_Notification_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Send_Call_Waiting_Notification_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send call waiting notification */
   /* request message given the length (in characters) of the phone     */
   /* number.  This MACRO accepts as it's input the total number of     */
   /* characters (including the NULL terminator) that are present       */
   /* starting from the PhoneNumber member of the                       */
   /* HFRM_Send_Call_Waiting_Notification_Request_t structure and       */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HFRM_SEND_CALL_WAITING_NOTIFICATION_REQUEST_SIZE(_x)   (STRUCTURE_OFFSET(HFRM_Send_Call_Waiting_Notification_Request_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a call waiting notification    */
   /* event to a remote Hands Free device (Response).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CALL_WAITING_NOTIFICATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Call_Waiting_Notification_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Call_Waiting_Notification_Response_t;

#define HFRM_SEND_CALL_WAITING_NOTIFICATION_RESPONSE_SIZE      (sizeof(HFRM_Send_Call_Waiting_Notification_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a call line identification     */
   /* notification event to a remote Hands Free device (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CALL_LINE_ID_NOTIFICATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Send_Call_Line_Identification_Notification_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Send_Call_Line_Identification_Notification_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send call line identification  */
   /* notification request message given the length (in characters) of  */
   /* the phone number.  This MACRO accepts as it's input the total     */
   /* number of characters (including the NULL terminator) that are     */
   /* present starting from the PhoneNumber member of the               */
   /* HFRM_Send_Call_Line_Identification_Notification_Request_t         */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE(_x)   (STRUCTURE_OFFSET(HFRM_Send_Call_Line_Identification_Notification_Request_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a call line identification     */
   /* notification event to a remote Hands Free device (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CALL_LINE_ID_NOTIFICATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Call_Line_Identification_Notification_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Call_Line_Identification_Notification_Response_t;

#define HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_RESPONSE_SIZE      (sizeof(HFRM_Send_Call_Line_Identification_Notification_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a ring indication to the remote*/
   /* Hands Free device (Request).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RING_INDICATION                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Ring_Indication_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Ring_Indication_Request_t;

#define HFRM_RING_INDICATION_REQUEST_SIZE                      (sizeof(HFRM_Ring_Indication_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a ring indication to the remote*/
   /* Hands Free device (Response).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RING_INDICATION                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Ring_Indication_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Ring_Indication_Response_t;

#define HFRM_RING_INDICATION_RESPONSE_SIZE                     (sizeof(HFRM_Ring_Indication_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable/disable the in-band ring tone*/
   /* on a remote Hands Free device (Request).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_IN_BAND_RING_TONE_SETTING*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_In_Band_Ring_Tone_Setting_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             EnableInBandRing;
} HFRM_Enable_In_Band_Ring_Tone_Setting_Request_t;

#define HFRM_ENABLE_IN_BAND_RING_TONE_SETTING_REQUEST_SIZE     (sizeof(HFRM_Enable_In_Band_Ring_Tone_Setting_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable/disable the in-band ring tone*/
   /* on a remote Hands Free device (Response).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_IN_BAND_RING_TONE_SETTING*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_In_Band_Ring_Tone_Setting_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Enable_In_Band_Ring_Tone_Setting_Response_t;

#define HFRM_ENABLE_IN_BAND_RING_TONE_SETTING_RESPONSE_SIZE    (sizeof(HFRM_Enable_In_Band_Ring_Tone_Setting_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a phone number for a voice     */
   /* tag response message to a remote Hands Free device (Request).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_VOICE_TAG_RESPONSE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Voice_Tag_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Voice_Tag_Response_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire voice tag response message     */
   /* given the length (in characters) of the phone number.  This MACRO */
   /* accepts as it's input the total number of characters (including   */
   /* the NULL terminator) that are present starting from the           */
   /* PhoneNumber member of the HFRM_Voice_Tag_Response_Request_t       */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define HFRM_VOICE_TAG_RESPONSE_REQUEST_SIZE(_x)               (STRUCTURE_OFFSET(HFRM_Voice_Tag_Response_Request_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a phone number for a voice     */
   /* tag response message to a remote Hands Free device (Response).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_VOICE_TAG_RESPONSE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Voice_Tag_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Voice_Tag_Response_Response_t;

#define HFRM_VOICE_TAG_RESPONSE_RESPONSE_SIZE                  (sizeof(HFRM_Voice_Tag_Response_Response_t))

   /* The following structure is used to hold all information related to*/
   /* the a call list entry (when sending the current calls list). This */
   /* structure mirrors the HFRE_Current_Call_List_Entry_t structure,   */
   /* but with static storage for variable-length strings.              */
   /* * NOTE * This structure is used with the                          */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V2      */
   /*                                                                   */
   /*          Request message                                          */
   /*          (HFRM_Send_Current_Calls_List_Request_v2_t structure) to */
   /*          specify an individual call list entry.                   */
   /* * NOTE * This structure is used with the                          */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V2 */
   /*                                                                   */
   /*          Request message                                          */
   /*          (HFRM_Current_Calls_List_Confirmation_Message_t          */
   /*          structure) to specify an individual call list entry.     */
typedef struct _tagHFRM_Call_List_List_Entry_v2_t
{
   Word_t                Flags;
   unsigned int          Index;
   HFRE_Call_Direction_t CallDirection;
   HFRE_Call_Status_t    CallStatus;
   HFRE_Call_Mode_t      CallMode;
   Boolean_t             Multiparty;
   char                  PhoneNumber[HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1];
   unsigned int          NumberFormat;
   char                  PhonebookName[HFRE_PHONEBOOK_NAME_LENGTH_MAXIMUM + 1];
} HFRM_Call_List_List_Entry_v2_t;

#define HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONE_NUMBER_VALID    0x0001
#define HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONEBOOK_NAME_VALID  0x0002

   /* --- DEPRECATED ---                                                */
   /* Replaced by HFRM_Call_List_List_Entry_v2_t                        */
   /*                                                                   */
   /* The following structure is used to hold all information related to*/
   /* the a call list entry (when sending the current calls list).      */
   /* * NOTE * This structure is used with the                          */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V1      */
   /*                                                                   */
   /*          Request message                                          */
   /*          (HFRM_Send_Current_Calls_List_Request_v1_t structure) to */
   /*          specify an individual call list entry.  This structure   */
   /*          is required because memory needs to be allocated for the */
   /*          phone number that is part of the call list entry.  The   */
   /*          actual phone number of the HFRE_Current_Call_List_Entry_t*/
   /*          structure that is part of this structure (in the         */
   /*          CallListEntry member) should be set to NULL in each      */
   /*          entry.  The phone number should be specified in the      */
   /*          PhoneNumberLength and Phone Number members.              */
   /* * NOTE * This structure is used with the                          */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V1 */
   /*                                                                   */
   /*          Request message                                          */
   /*          (HFRM_Current_Calls_List_Confirmation_Message_t          */
   /*          structure) to specify an individual call list entry.     */
   /*          This structure is required because memory needs to be    */
   /*          allocated for the phone number that is part of the call  */
   /*          list entry.  The actual phone number of the              */
   /*          HFRE_Current_Call_List_Entry_t structure that is part of */
   /*          this structure (in the CallListEntry member) should be   */
   /*          set to NULL in each entry.  The phone number should be   */
   /*          specified in the PhoneNumberLength and Phone Number      */
   /*          members.                                                 */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
   /* --- DEPRECATED ---                                                */
typedef struct _tagHFRM_Call_List_List_Entry_v1_t
{
   HFRE_Current_Call_List_Entry_t CallListEntry;
   unsigned int                   PhoneNumberLength;
   char                           PhoneNumber[HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1];
} HFRM_Call_List_List_Entry_v1_t;

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the current calls list to a    */
   /* remote Hands Free device (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V2      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Current_Calls_List_Request_v2_t
{
   BTPM_Message_Header_t          MessageHeader;
   unsigned int                   ControlEventsHandlerID;
   BD_ADDR_t                      RemoteDeviceAddress;
   unsigned int                   NumberCallListEntries;
   HFRM_Call_List_List_Entry_v2_t CallListEntryList[1];
} HFRM_Send_Current_Calls_List_Request_v2_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send current calls list message*/
   /* given the length (in individual call entry list entries - NOT     */
   /* bytes) of the message.  This MACRO accepts as it's input the total*/
   /* number of call list entries (NOT bytes) that are present starting */
   /* from the CallListEntryList member of the                          */
   /* HFRM_Send_Current_Calls_List_Request_v2_t structure and returns   */
   /* the total number of bytes required to hold the entire message.    */
#define HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V2_SIZE(_x)       (STRUCTURE_OFFSET(HFRM_Send_Current_Calls_List_Request_v2_t, CallListEntryList) + (unsigned int)((sizeof(HFRM_Call_List_List_Entry_v2_t)*((unsigned int)(_x)))))

   /* --- DEPRECATED ---                                                */
   /* Replaced by HFRM_Send_Current_Calls_List_Request_v2_t             */
   /*                                                                   */
   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the current calls list to a    */
   /* remote Hands Free device (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V1      */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* --- DEPRECATED ---                                                */
typedef struct _tagHFRM_Send_Current_Calls_List_Request_v1_t
{
   BTPM_Message_Header_t          MessageHeader;
   unsigned int                   ControlEventsHandlerID;
   BD_ADDR_t                      RemoteDeviceAddress;
   unsigned int                   NumberCallListEntries;
   HFRM_Call_List_List_Entry_v1_t CallListEntryList[1];
} HFRM_Send_Current_Calls_List_Request_v1_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send current calls list message*/
   /* given the length (in individual call entry list entries - NOT     */
   /* bytes) of the message.  This MACRO accepts as it's input the total*/
   /* number of call list entries (NOT bytes) that are present starting */
   /* from the CallListEntryList member of the                          */
   /* HFRM_Send_Current_Calls_List_Request_v1_t structure and returns   */
   /* the total number of bytes required to hold the entire message.    */
#define HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V1_SIZE(_x)       (STRUCTURE_OFFSET(HFRM_Send_Current_Calls_List_Request_v1_t, CallListEntryList) + (unsigned int)((sizeof(HFRM_Call_List_List_Entry_v1_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the current calls list to a    */
   /* remote Hands Free device (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V1      */
   /*             HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V2      */
   /*                                                                   */
   /*          Message Function IDs.                                    */
typedef struct _tagHFRM_Send_Current_Calls_List_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Current_Calls_List_Response_t;

#define HFRM_SEND_CURRENT_CALLS_LIST_RESPONSE_SIZE             (sizeof(HFRM_Send_Current_Calls_List_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a network operator selection   */
   /* message to a remote Hands Free device (Request).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_NETWORK_OPERATOR_SELECTION */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The NetworkOperator member *MUST* be NULL terminated.    */
typedef struct _tagHFRM_Send_Network_Operator_Selection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          NetworkMode;
   unsigned int          NetworkOperatorLength;
   char                  NetworkOperator[1];
} HFRM_Send_Network_Operator_Selection_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send network operator selection*/
   /* message given the length (in characters) of the network operator  */
   /* string.  This MACRO accepts as it's input the total number of     */
   /* characters (including the NULL terminator) that are present       */
   /* starting from the NetworkOperator member of the                   */
   /* HFRM_Send_Network_Operator_Selection_Request_t structure and      */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HFRM_SEND_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE(_x)  (STRUCTURE_OFFSET(HFRM_Send_Network_Operator_Selection_Request_t, NetworkOperator) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a network operator selection   */
   /* message to a remote Hands Free device (Response).                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_NETWORK_OPERATOR_SELECTION */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Network_Operator_Selection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Network_Operator_Selection_Response_t;

#define HFRM_SEND_NETWORK_OPERATOR_SELECTION_RESPONSE_SIZE     (sizeof(HFRM_Send_Network_Operator_Selection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send an extended error result to a  */
   /* remote Hands Free device (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_EXTENDED_ERROR_RESULT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Extended_Error_Result_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ResultCode;
} HFRM_Send_Extended_Error_Result_Request_t;

#define HFRM_SEND_EXTENDED_ERROR_RESULT_REQUEST_SIZE           (sizeof(HFRM_Send_Extended_Error_Result_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send an extended error result to a  */
   /* remote Hands Free device (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_EXTENDED_ERROR_RESULT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Extended_Error_Result_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Extended_Error_Result_Response_t;

#define HFRM_SEND_EXTENDED_ERROR_RESULT_RESPONSE_SIZE          (sizeof(HFRM_Send_Extended_Error_Result_Response_t))

   /* The following structure is used to hold all information related to*/
   /* the a subscriber number information list entry (when sending the  */
   /* subscriber information list).                                     */
   /* * NOTE * This structure is used with the                          */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_SUBSCRIBER_NUMBER_INFO     */
   /*                                                                   */
   /*          Request message                                          */
   /*          (HFRM_Send_Subscriber_Number_Information_Request_t       */
   /*          structure) to specify an individual subscriber number    */
   /*          information entry.  This structure is required because   */
   /*          memory needs to be allocated for the phone number that is*/
   /*          part of the subscriber information entry.  The actual    */
   /*          phone number of the HFRM_Subscriber_Number_Information_t */
   /*          structure that is part of this structure (in the         */
   /*          SubscriberNumberInformationEntry member) should be set to*/
   /*          NULL in each entry.  The phone number should be specified*/
   /*          in the PhoneNumberLength and Phone Number members.       */
   /* * NOTE * This structure is used with the                          */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INFO_CFM      */
   /*                                                                   */
   /*          Request message                                          */
   /*          (HFRM_Subscriber_Number_Information...                   */
   /*          ..._Confirmation_Message_t structure) to specify the     */
   /*          subscriber number information entry.  This structure is  */
   /*          required because memory needs to be allocated for the    */
   /*          phone number that is part of the subscriber information  */
   /*          entry.  The actual phone number of the                   */
   /*          HFRM_Subscriber_Number_Information_Confirmation_Message_t*/
   /*          structure that is part of this structure (in the         */
   /*          SubscriberNumberInformationEntry member) should be set to*/
   /*          NULL in each entry.  The phone number should be specified*/
   /*          in the PhoneNumberLength and Phone Number members.       */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Subscriber_Information_List_Entry_t
{
   HFRM_Subscriber_Number_Information_t SubscriberNumberInformationEntry;
   unsigned int                         PhoneNumberLength;
   char                                 PhoneNumber[HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1];
} HFRM_Subscriber_Information_List_Entry_t;

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the subscriber number          */
   /* information list to a remote Hands Free device (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_SUBSCRIBER_NUMBER_INFO     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Subscriber_Number_Information_Request_t
{
   BTPM_Message_Header_t                    MessageHeader;
   unsigned int                             ControlEventsHandlerID;
   BD_ADDR_t                                RemoteDeviceAddress;
   unsigned int                             NumberSubscriberInformationEntries;
   HFRM_Subscriber_Information_List_Entry_t SubscriberInformationList[1];
} HFRM_Send_Subscriber_Number_Information_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send subscriber information    */
   /* list message given the length (in individual subscriber           */
   /* information list entries - NOT bytes) of the message.  This MACRO */
   /* accepts as it's input the total number of subscriber information  */
   /* list entries (NOT bytes) that are present starting from the       */
   /* SubscriberInformationList member of the                           */
   /* HFRM_Send_Subscriber_Number_Information_Request_t structure and   */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE(_x)  (STRUCTURE_OFFSET(HFRM_Send_Subscriber_Number_Information_Request_t, SubscriberInformationList) + (unsigned int)((sizeof(HFRM_Subscriber_Information_List_Entry_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the subscriber number          */
   /* information list to a remote Hands Free device (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_SUBSCRIBER_NUMBER_INFO     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Subscriber_Number_Information_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Subscriber_Number_Information_Response_t;

#define HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_RESPONSE_SIZE     (sizeof(HFRM_Send_Subscriber_Number_Information_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the incoming call state to a   */
   /* remote Hands Free device (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_INCOMING_CALL_STATE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Incoming_Call_State_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   HFRE_Call_State_t     CallState;
} HFRM_Send_Incoming_Call_State_Request_t;

#define HFRM_SEND_INCOMING_CALL_STATE_REQUEST_SIZE             (sizeof(HFRM_Send_Incoming_Call_State_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the incoming call state to a   */
   /* remote Hands Free device (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_INCOMING_CALL_STATE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Incoming_Call_State_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Incoming_Call_State_Response_t;

#define HFRM_SEND_INCOMING_CALL_STATE_RESPONSE_SIZE            (sizeof(HFRM_Send_Incoming_Call_State_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a terminating response to a    */
   /* remote Hands Free device (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_TERMINATING_RESPONSE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Terminating_Response_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   BD_ADDR_t              RemoteDeviceAddress;
   HFRE_Extended_Result_t ResultType;
   unsigned int           ResultValue;
} HFRM_Send_Terminating_Response_Request_t;

#define HFRM_SEND_TERMINATING_RESPONSE_REQUEST_SIZE            (sizeof(HFRM_Send_Terminating_Response_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send a terminating response to a    */
   /* remote Hands Free device (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_TERMINATING_RESPONSE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Terminating_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Terminating_Response_Response_t;

#define HFRM_SEND_TERMINATING_RESPONSE_RESPONSE_SIZE           (sizeof(HFRM_Send_Terminating_Response_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable arbitrary command processing */
   /* on the local Audio Gateway service (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_ARBITRARY_CMD_PROCESSING */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Arbitrary_Command_Processing_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
} HFRM_Enable_Arbitrary_Command_Processing_Request_t;

#define HFRM_ENABLE_ARBITRARY_COMMAND_PROCESSING_REQUEST_SIZE  (sizeof(HFRM_Enable_Arbitrary_Command_Processing_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to enable arbitrary command processing */
   /* on the local Audio Gateway service (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ENABLE_ARBITRARY_CMD_PROCESSING */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Enable_Arbitrary_Command_Processing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Enable_Arbitrary_Command_Processing_Response_t;

#define HFRM_ENABLE_ARBITRARY_COMMAND_PROCESSING_RESPONSE_SIZE (sizeof(HFRM_Enable_Arbitrary_Command_Processing_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send an arbitrary response to the   */
   /* remote Hands Free device (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_RESPONSE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The ArbitraryResponse member *MUST* be NULL              */
   /*          terminated and the ArbitraryResponseLength member *MUST* */
   /*          include the NULL terminator.                             */
typedef struct _tagHFRM_Send_Arbitrary_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ControlEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ArbitraryResponseLength;
   char                  ArbitraryResponse[1];
} HFRM_Send_Arbitrary_Response_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send arbitrary response request*/
   /* message given the length (in characters) of the arbitrary         */
   /* response.  This MACRO accepts as it's input the total number of   */
   /* characters (including the NULL terminator) that are present       */
   /* starting from the ArbitraryResponse member of the                 */
   /* HFRM_Send_Arbitrary_Response_Request_t structure and returns the  */
   /* total number of bytes required to hold the entire message.        */
#define HFRM_SEND_ARBITRARY_RESPONSE_REQUEST_SIZE(_x)          (STRUCTURE_OFFSET(HFRM_Send_Arbitrary_Response_Request_t, ArbitraryResponse) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send an arbitrary response to the   */
   /* remote Hands Free device (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_RESPONSE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Arbitrary_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Arbitrary_Response_Response_t;

#define HFRM_SEND_ARBITRARY_RESPONSE_RESPONSE_SIZE             (sizeof(HFRM_Send_Arbitrary_Response_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the selected Codec to a remote */
   /* Hands Free device (Request).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_SELECT_CODEC               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Select_Codec_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   BD_ADDR_t              RemoteDeviceAddress;
   HFRM_Connection_Type_t ConnectionType;
   unsigned char          CodecID;
} HFRM_Send_Select_Codec_Request_t;

#define HFRM_SEND_SELECT_CODEC_REQUEST_SIZE                    (sizeof(HFRM_Send_Select_Codec_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send the selected Codec to a remote */
   /* Hands Free device (Response).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_SELECT_CODEC               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Send_Select_Codec_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Send_Select_Codec_Response_t;

#define HFRM_SEND_SELECT_CODEC_RESPONSE_SIZE                   (sizeof(HFRM_Send_Select_Codec_Response_t))

   /* Hands Free Manager Audio Connection Management Messages.          */

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to setup a SCO audio connection on the */
   /* local Hands Free or Audio Gateway service (Request).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Setup_Audio_Connection_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Setup_Audio_Connection_Request_t;

#define HFRM_SETUP_AUDIO_CONNECTION_REQUEST_SIZE               (sizeof(HFRM_Setup_Audio_Connection_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to setup a SCO audio connection on the */
   /* local Hands Free or Audio Gateway service (Response).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Setup_Audio_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Setup_Audio_Connection_Response_t;

#define HFRM_SETUP_AUDIO_CONNECTION_RESPONSE_SIZE              (sizeof(HFRM_Setup_Audio_Connection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to release a currently on-going SCO    */
   /* audio connection on the local Hands Free or Audio Gateway service */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Release_Audio_Connection_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ControlEventsHandlerID;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Release_Audio_Connection_Request_t;

#define HFRM_RELEASE_AUDIO_CONNECTION_REQUEST_SIZE             (sizeof(HFRM_Release_Audio_Connection_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to release a currently on-going SCO    */
   /* audio connection on the local Hands Free or Audio Gateway service */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Release_Audio_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Release_Audio_Connection_Response_t;

#define HFRM_RELEASE_AUDIO_CONNECTION_RESPONSE_SIZE            (sizeof(HFRM_Release_Audio_Connection_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to send SCO audio data over a currently*/
   /* on-going SCO audio connection to a remote Hands Free or Audio     */
   /* Gateway device (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SEND_AUDIO_DATA                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * There is NO Response message for this message.           */
typedef struct _tagHFRM_Send_Audio_Data_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           DataEventsHandlerID;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           AudioDataLength;
   unsigned char          AudioData[1];
} HFRM_Send_Audio_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold a an entire SCO audio data request       */
   /* message given the number of actual raw audio data frame bytes.    */
   /* This function accepts as it's input the total number individual   */
   /* SCO audio data frame bytes are present starting from the AudioData*/
   /* member of the HFRM_Send_Audio_Data_Request_t structure and returns*/
   /* the total number of bytes required to hold the entire message.    */
#define HFRM_SEND_AUDIO_DATA_REQUEST_SIZE(_x)                  (STRUCTURE_OFFSET(HFRM_Send_Audio_Data_Request_t, AudioData) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to register for Hands Free Manager     */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Register_Hands_Free_Events_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   Boolean_t              ControlHandler;
   HFRM_Connection_Type_t ConnectionType;
} HFRM_Register_Hands_Free_Events_Request_t;

#define HFRM_REGISTER_HANDS_FREE_EVENTS_REQUEST_SIZE           (sizeof(HFRM_Register_Hands_Free_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to register for Hands Free Manager     */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Register_Hands_Free_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          EventsHandlerID;
} HFRM_Register_Hands_Free_Events_Response_t;

#define HFRM_REGISTER_HANDS_FREE_EVENTS_RESPONSE_SIZE          (sizeof(HFRM_Register_Hands_Free_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to un-register for Hands Free Manager  */
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_EVENTS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Un_Register_Hands_Free_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventsHandlerID;
} HFRM_Un_Register_Hands_Free_Events_Request_t;

#define HFRM_UN_REGISTER_HANDS_FREE_EVENTS_REQUEST_SIZE        (sizeof(HFRM_Un_Register_Hands_Free_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to un-register for Hands Free Manager  */
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_EVENTS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Un_Register_Hands_Free_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Un_Register_Hands_Free_Events_Response_t;

#define HFRM_UN_REGISTER_HANDS_FREE_EVENTS_RESPONSE_SIZE       (sizeof(HFRM_Un_Register_Hands_Free_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to register for Hands Free Manager Data*/
   /* events (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_DATA        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Register_Hands_Free_Data_Events_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
} HFRM_Register_Hands_Free_Data_Events_Request_t;

#define HFRM_REGISTER_HANDS_FREE_DATA_EVENTS_REQUEST_SIZE      (sizeof(HFRM_Register_Hands_Free_Data_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to register for Hands Free Manager Data*/
   /* events (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_DATA        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Register_Hands_Free_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          DataEventsHandlerID;
} HFRM_Register_Hands_Free_Data_Events_Response_t;

#define HFRM_REGISTER_HANDS_FREE_DATA_EVENTS_RESPONSE_SIZE     (sizeof(HFRM_Register_Hands_Free_Data_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to un-register for Hands Free Manager  */
   /* Data events (Request).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_DATA     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Un_Register_Hands_Free_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataEventsHandlerID;
} HFRM_Un_Register_Hands_Free_Data_Events_Request_t;

#define HFRM_UN_REGISTER_HANDS_FREE_DATA_EVENTS_REQUEST_SIZE   (sizeof(HFRM_Un_Register_Hands_Free_Data_Events_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to un-register for Hands Free Manager  */
   /* Data events (Response).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_DATA     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Un_Register_Hands_Free_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HFRM_Un_Register_Hands_Free_Data_Events_Response_t;

#define HFRM_UN_REGISTER_HANDS_FREE_DATA_EVENTS_RESPONSE_SIZE     (sizeof(HFRM_Un_Register_Hands_Free_Data_Events_Response_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the SCO Handle for and active */
   /* connection (Request).                                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_SCO_Connection_Handle_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventsHandlerID;
   BD_ADDR_t              RemoteDeviceAddress;
   HFRM_Connection_Type_t ConnectionType;
} HFRM_Query_SCO_Connection_Handle_Request_t;

#define HFRM_QUERY_SCO_CONNECTION_HANDLE_REQUEST_SIZE      (sizeof(HFRM_Query_SCO_Connection_Handle_Request_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message to query the SCO Handle for and active */
   /* connection (Response).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_SCO_Connection_Handle_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   Word_t                SCOHandle;
} HFRM_Query_SCO_Connection_Handle_Response_t;

#define HFRM_QUERY_SCO_CONNECTION_HANDLE_RESPONSE_SIZE     (sizeof(HFRM_Query_SCO_Connection_Handle_Response_t))

   /* Hands Free Manager Asynchronous Message Formats.                  */

   /* Hands Free Manager Connection Management Asynchronous Message     */
   /* Formats.                                                          */

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client that a remote  */
   /* Hands Free/Audio Gateway device connection request has been       */
   /* received (asynchronously).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Connection_Request_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Connection_Request_Message_t;

#define HFRM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(HFRM_Connection_Request_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client that a Hands   */
   /* Free/Audio Gateway device is currently connected (asynchronously).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTED                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Device_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Device_Connected_Message_t;

#define HFRM_DEVICE_CONNECTED_MESSAGE_SIZE                     (sizeof(HFRM_Device_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client of the         */
   /* specified connection status (asynchronously).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Device_Connection_Status_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ConnectionStatus;
} HFRM_Device_Connection_Status_Message_t;

#define HFRM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE             (sizeof(HFRM_Device_Connection_Status_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the HFRM_Device_Connection_Status_Message_t message to describe*/
   /* the actual connection result status.                              */
#define HFRM_DEVICE_CONNECTION_STATUS_SUCCESS                  0x00000000
#define HFRM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT          0x00000001
#define HFRM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED          0x00000002
#define HFRM_DEVICE_CONNECTION_STATUS_FAILURE_SECURITY         0x00000003
#define HFRM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF 0x00000004
#define HFRM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN          0x00000005

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client that a Hands   */
   /* Free/Audio Gateway connection is now disconnected                 */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Device_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           DisconnectReason;
} HFRM_Device_Disconnected_Message_t;

#define HFRM_DEVICE_DISCONNECTED_MESSAGE_SIZE                  (sizeof(HFRM_Device_Disconnected_Message_t))

   /* The following constants are used with the DisconnectReason member */
   /* of the HFRM_Device_Disconnected_Message_t message to describe the */
   /* actual disconnect reason.                                         */
#define HFRM_DEVICE_DISCONNECT_REASON_NORMAL_DISCONNECT        0x00000000
#define HFRM_DEVICE_DISCONNECT_REASON_SERVICE_LEVEL_ERROR      0x00000001

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client that a Hands   */
   /* Free/Audio Gateway service level connection is now                */
   /* connected/present (asynchronously).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SERVICE_LEVEL_CONNECTION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Service_Level_Connection_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              RemoteSupportedFeaturesValid;
   unsigned long          RemoteSupportedFeatures;
   unsigned long          RemoteCallHoldMultipartySupport;
} HFRM_Service_Level_Connection_Message_t;

#define HFRM_SERVICE_LEVEL_CONNECTION_MESSAGE_SIZE             (sizeof(HFRM_Service_Level_Connection_Message_t))

   /* Hands Free Manager Audio Connection Management Asynchronous       */
   /* Message Formats.                                                  */

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client that a SCO     */
   /* Audio Hands Free/Audio Gateway connection is now connected/present*/
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTED                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Audio_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Audio_Connected_Message_t;

#define HFRM_AUDIO_CONNECTED_MESSAGE_SIZE                      (sizeof(HFRM_Audio_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client of the result  */
   /* of an attempted SCO Audio Hands Free/Audio Gateway audio          */
   /* connection (asynchronously).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTION_STATUS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Audio_Connection_Status_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              Successful;
} HFRM_Audio_Connection_Status_Message_t;

#define HFRM_AUDIO_CONNECTION_STATUS_MESSAGE_SIZE              (sizeof(HFRM_Audio_Connection_Status_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client that a SCO     */
   /* Audio Hands Free/Audio Gateway connection is no longer            */
   /* connected/present (asynchronously).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Audio_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Audio_Disconnected_Message_t;

#define HFRM_AUDIO_DISCONNECTED_MESSAGE_SIZE                   (sizeof(HFRM_Audio_Disconnected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that specifies received, raw SCO Audio */
   /* data (asynchronously).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Audio_Data_Received_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           DataEventsHandlerID;
   unsigned long          AudioDataFlags;
   unsigned int           AudioDataLength;
   unsigned char          AudioData[1];
} HFRM_Audio_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold a an entire SCO audio data message given */
   /* the number of actual raw audio data bytes.  This function accepts */
   /* as it's input the total number individual audio data bytes are    */
   /* present starting from the AudioData member of the                 */
   /* HFRM_Audio_Data_Received_Message_t structure and returns the total*/
   /* number of bytes required to hold the entire message.              */
#define HFRM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(_x)              (STRUCTURE_OFFSET(HFRM_Audio_Data_Received_Message_t, AudioData) + ((unsigned int)(_x)))

   /* Hands Free Manager Hands Free/Audio Gateway Asynchronous Message  */
   /* Definitions.                                                      */

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client of a received  */
   /* voice recognition indication event (asynchronously).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_VOICE_RECOGNITION_IND           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Voice_Recognition_Indication_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              VoiceRecognitionActive;
} HFRM_Voice_Recognition_Indication_Message_t;

#define HFRM_VOICE_RECOGNITION_INDICATION_MESSAGE_SIZE         (sizeof(HFRM_Voice_Recognition_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client of a received  */
   /* speaker gain indication event (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Speaker_Gain_Indication_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SpeakerGain;
} HFRM_Speaker_Gain_Indication_Message_t;

#define HFRM_SPEAKER_GAIN_INDICATION_MESSAGE_SIZE              (sizeof(HFRM_Speaker_Gain_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client of a received  */
   /* microphone gain indication event (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Microphone_Gain_Indication_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           MicrophoneGain;
} HFRM_Microphone_Gain_Indication_Message_t;

#define HFRM_MICROPHONE_GAIN_INDICATION_MESSAGE_SIZE           (sizeof(HFRM_Microphone_Gain_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the client of a received  */
   /* incoming call state indication event (asynchronously).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_IND         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Incoming_Call_State_Indication_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   HFRE_Call_State_t      CallState;
} HFRM_Incoming_Call_State_Indication_Message_t;

#define HFRM_INCOMING_CALL_STATE_INDICATION_MESSAGE_SIZE          (sizeof(HFRM_Incoming_Call_State_Indication_Message_t))

   /* Hands Free Manager Hands Free Asynchronous Message Definitions.   */

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received incoming call state confirmation event       */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_CFM         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Incoming_Call_State_Confirmation_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   HFRE_Call_State_t      CallState;
} HFRM_Incoming_Call_State_Confirmation_Message_t;

#define HFRM_INCOMING_CALL_STATE_CONFIRMATION_MESSAGE_SIZE        (sizeof(HFRM_Incoming_Call_State_Confirmation_Message_t))

   /* The following structure is a container structure that is used to  */
   /* hold a control indicator information entry (used with the         */
   /* HFRM_Control_Indicator_Status_Indication_Message_t and            */
   /* HFRM_Control_Indicator_Status_Confirmation_Message_t messages).   */
   /* * NOTE * The IndicatorDescription member *MUST* be NULL           */
   /*          terminated.                                              */
typedef struct _tagHFRM_Control_Indicator_Entry_t
{
   char                                     IndicatorDescription[HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM + 1];
   HFRE_Control_Indicator_Type_t            ControlIndicatorType;
   union
   {
      HFRE_Control_Indicator_Range_Type_t   ControlIndicatorRangeType;
      HFRE_Control_Indicator_Boolean_Type_t ControlIndicatorBooleanType;
   } Control_Indicator_Data;
} HFRM_Control_Indicator_Entry_t;

#define HFRM_CONTROL_INDICATOR_ENTRY_SIZE                      (sizeof(HFRM_Control_Indicator_Entry_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received control indicator status indication event    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_IND    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Control_Indicator_Status_Indication_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   HFRM_Control_Indicator_Entry_t ControlIndicatorEntry;
} HFRM_Control_Indicator_Status_Indication_Message_t;

#define HFRM_CONTROL_INDICATOR_STATUS_INDICATION_MESSAGE_SIZE  (sizeof(HFRM_Control_Indicator_Status_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received control indicator status confirmation event  */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_CFM    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Control_Indicator_Status_Confirmation_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   HFRM_Control_Indicator_Entry_t ControlIndicatorEntry;
} HFRM_Control_Indicator_Status_Confirmation_Message_t;

#define HFRM_CONTROL_INDICATOR_STATUS_CONFIRMATION_MESSAGE_SIZE   (sizeof(HFRM_Control_Indicator_Status_Confirmation_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received call hold/multi-party support confirmation   */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SUPPORT_CFM     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Call_Hold_Multiparty_Support_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             CallHoldSupportMaskValid;
   unsigned long         CallHoldSupportMask;
} HFRM_Call_Hold_Multiparty_Support_Confirmation_Message_t;

#define HFRM_CALL_HOLD_MULTIPARTY_SUPPORT_CONFIRMATION_MESSAGE_SIZE  (sizeof(HFRM_Call_Hold_Multiparty_Support_Confirmation_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received call call waiting notification indication    */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOTIFICATION_IND      */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Call_Waiting_Notification_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Call_Waiting_Notification_Indication_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire call waiting notification      */
   /* indication asynchronous event message given the length (in        */
   /* characters) of the phone number.  This MACRO accepts as it's input*/
   /* the total number of characters (including the NULL terminator)    */
   /* that are present starting from the PhoneNumber member of the      */
   /* HFRM_Call_Waiting_Notification_Indication_Message_t structure and */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HFRM_CALL_WAITING_NOTIFICATION_INDICATION_MESSAGE_SIZE(_x)   (STRUCTURE_OFFSET(HFRM_Call_Waiting_Notification_Indication_Message_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received call call line identification notification   */
   /* indication event (asynchronously).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOTIFICATION_IND   */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Call_Line_Identification_Notification_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Call_Line_Identification_Notification_Indication_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire call line identification       */
   /* notification indication asynchronous event message given the      */
   /* length (in characters) of the phone number.  This MACRO accepts as*/
   /* it's input the total number of characters (including the NULL     */
   /* terminator) that are present starting from the PhoneNumber member */
   /* of the                                                            */
   /* HFRM_Call_Line_Identifcation_Notification_Indication_Message_t    */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_MESSAGE_SIZE(_x)   (STRUCTURE_OFFSET(HFRM_Call_Line_Identification_Notification_Indication_Message_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received ring indication event (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RING_INDICATION_IND             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Ring_Indication_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Ring_Indication_Indication_Message_t;

#define HFRM_RING_INDICATION_INDICATION_MESSAGE_SIZE           (sizeof(HFRM_Ring_Indication_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received in-band ring tone setting indication event   */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_IN_BAND_RING_TONE_SETTING_IND   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_In_Band_Ring_Tone_Setting_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Enabled;
} HFRM_In_Band_Ring_Tone_Setting_Indication_Message_t;

#define HFRM_IN_BAND_RING_TONE_SETTING_INDICATION_MESSAGE_SIZE (sizeof(HFRM_In_Band_Ring_Tone_Setting_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received voice tag request confirmation event         */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_CFM           */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Voice_Tag_Request_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Voice_Tag_Request_Confirmation_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire voice tag request confirmation */
   /* asynchronous event message given the length (in characters) of the*/
   /* phone number.  This MACRO accepts as it's input the total number  */
   /* of characters (including the NULL terminator) that are present    */
   /* starting from the PhoneNumber member of the                       */
   /* HFRM_Voice_Tag_Request_Confirmation_Message_t structure and       */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HFRM_VOICE_TAG_REQUEST_CONFIRMATION_MESSAGE_SIZE(_x)   (STRUCTURE_OFFSET(HFRM_Voice_Tag_Request_Confirmation_Message_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received current calls list confirmation event        */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V2 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Current_Calls_List_Confirmation_Message_v2_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   HFRM_Call_List_List_Entry_v2_t CallListEntry;
} HFRM_Query_Current_Calls_List_Confirmation_Message_v2_t;

#define HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V2_SIZE (sizeof(HFRM_Query_Current_Calls_List_Confirmation_Message_v2_t))

   /* --- DEPRECATED ---                                                */
   /* Replaced by                                                       */
   /* HFRM_Query_Current_Calls_List_Confirmation_Message_v2_t           */
   /*                                                                   */
   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received current calls list confirmation event        */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V2 */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* --- DEPRECATED ---                                                */
typedef struct _tagHFRM_Query_Current_Calls_List_Confirmation_Message_v1_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   HFRM_Call_List_List_Entry_v1_t CallListEntry;
} HFRM_Query_Current_Calls_List_Confirmation_Message_v1_t;

#define HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V1_SIZE (sizeof(HFRM_Query_Current_Calls_List_Confirmation_Message_v1_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received network operator selection confirmation event*/
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_CFM  */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The NetworkOperator member *MUST* be NULL terminated.    */
typedef struct _tagHFRM_Network_Operator_Selection_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          NetworkMode;
   unsigned int          NetworkOperatorLength;
   char                  NetworkOperator[1];
} HFRM_Network_Operator_Selection_Confirmation_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire network operator selection     */
   /* confirmation asynchronous event message given the length (in      */
   /* characters) of the phone number.  This MACRO accepts as it's input*/
   /* the total number of characters (including the NULL terminator)    */
   /* that are present starting from the NetworkOperator member of the  */
   /* HFRM_Network_Operator_Selection_Confirmation_Message_t structure  */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_MESSAGE_SIZE(_x)   (STRUCTURE_OFFSET(HFRM_Network_Operator_Selection_Confirmation_Message_t, NetworkOperator) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received current calls list confirmation event        */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INFO_CFM      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Subscriber_Number_Information_Confirmation_Message_t
{
   BTPM_Message_Header_t                    MessageHeader;
   BD_ADDR_t                                RemoteDeviceAddress;
   HFRM_Subscriber_Information_List_Entry_t SubscriberInformationEntry;
} HFRM_Subscriber_Number_Information_Confirmation_Message_t;

#define HFRM_SUBSCRIBER_NUMBER_INFORMATION_CONFIRMATION_MESSAGE_SIZE (sizeof(HFRM_Subscriber_Number_Information_Confirmation_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received response hold status confirmation event      */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_CFM        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Response_Hold_Status_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   HFRE_Call_State_t     CallState;
} HFRM_Response_Hold_Status_Confirmation_Message_t;

#define HFRM_RESPONSE_HOLD_STATUS_CONFIRMATION_MESSAGE_SIZE    (sizeof(HFRM_Response_Hold_Status_Confirmation_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received command/result response event                */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_COMMAND_RESULT                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Command_Result_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   HFRE_Extended_Result_t ResultType;
   unsigned int           ResultValue;
} HFRM_Command_Result_Message_t;

#define HFRM_COMMAND_RESULT_MESSAGE_SIZE                       (sizeof(HFRM_Command_Result_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a received arbitrary response event (asynchronously).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ARBITRARY_RESPONSE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The ArbitraryResponse member *MUST* be NULL              */
   /*          terminated and the ArbitraryResponseLength member *MUST* */
   /*          include the NULL terminator.                             */
typedef struct _tagHFRM_Arbitrary_Response_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ArbitraryResponseLength;
   char                  ArbitraryResponse[1];
} HFRM_Arbitrary_Response_Indication_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire arbitrary response indication  */
   /* asynchronous event message given the length (in characters) of    */
   /* the arbitrary response data.  This MACRO accepts as it's input    */
   /* the total number of characters (including the NULL terminator)    */
   /* that are present starting from the ArbitraryResponse member of the*/
   /* HFRM_Arbitrary_Response_Indication_Message_t structure and returns*/
   /* the total number of bytes required to hold the entire message.    */
#define HFRM_ARBITRARY_RESPONSE_INDICATION_MESSAGE_SIZE(_x)    (STRUCTURE_OFFSET(HFRM_Arbitrary_Response_Indication_Message_t, ArbitraryResponse) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Hands Free      */
   /* client of a codec select event (asynchronously).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SELECT_CODEC_IND                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Codec_Select_Indication_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned char          CodecID;
} HFRM_Codec_Select_Indication_Message_t;

#define HFRM_CODEC_SELECT_INDICATION_MESSAGE_SIZE                       (sizeof(HFRM_Codec_Select_Indication_Message_t))

   /* Hands Free Manager Audio Gateway Asynchronous Message Definitions.*/

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received call hold/multi-party selection indication   */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SELECTION_IND   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Call_Hold_Multiparty_Selection_Indication_Message_t
{
   BTPM_Message_Header_t                     MessageHeader;
   BD_ADDR_t                                 RemoteDeviceAddress;
   HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling;
   unsigned int                              Index;
} HFRM_Call_Hold_Multiparty_Selection_Indication_Message_t;

#define HFRM_CALL_HOLD_MULTIPARTY_SELECTION_INDICATION_MESSAGE_SIZE  (sizeof(HFRM_Call_Hold_Multiparty_Selection_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received call waiting notification activation         */
   /* indication event (asynchronously).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOT_ACTIVATION_IND    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Call_Waiting_Notification_Activation_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Enabled;
} HFRM_Call_Waiting_Notification_Activation_Indication_Message_t;

#define HFRM_CALL_WAITING_NOTIFICATION_ACTIVATION_INDICATION_MESSAGE_SIZE  (sizeof(HFRM_Call_Waiting_Notification_Activation_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received call line identification notification        */
   /* activation indication event (asynchronously).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOT_ACTIVATION_IND */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Call_Line_Identification_Notification_Activation_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Enabled;
} HFRM_Call_Line_Identification_Notification_Activation_Indication_Message_t;

#define HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_ACTIVATION_INDICATION_MESSAGE_SIZE  (sizeof(HFRM_Call_Line_Identification_Notification_Activation_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received disable sound enhancement indication event   */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DISABLE_SOUND_ENHANCEMENT_IND   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Disable_Sound_Enhancement_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Disable_Sound_Enhancement_Indication_Message_t;

#define HFRM_DISABLE_SOUND_ENHANCEMENT_INDICATION_MESSAGE_SIZE (sizeof(HFRM_Disable_Sound_Enhancement_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received dial number indication event                 */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_IND           */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The PhoneNumber member *MUST* be NULL terminated (and    */
   /*          PhoneNumberLength member *MUST* include the NULL         */
   /*          terminator.                                              */
typedef struct _tagHFRM_Dial_Phone_Number_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PhoneNumberLength;
   char                  PhoneNumber[1];
} HFRM_Dial_Phone_Number_Indication_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire dial number indication         */
   /* asynchronous event message given the length (in characters) of the*/
   /* phone number.  This MACRO accepts as it's input the total number  */
   /* of characters (including the NULL terminator) that are present    */
   /* starting from the PhoneNumber member of the                       */
   /* HFRM_Dial_Phone_Number_Indication_Message_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define HFRM_DIAL_PHONE_NUMBER_INDICATION_MESSAGE_SIZE(_x)     (STRUCTURE_OFFSET(HFRM_Dial_Phone_Number_Indication_Message_t, PhoneNumber) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received dial number from memory (index) indication   */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEM_IND  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Dial_Phone_Number_From_Memory_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          MemoryLocation;
} HFRM_Dial_Phone_Number_From_Memory_Indication_Message_t;

#define HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_INDICATION_MESSAGE_SIZE   (sizeof(HFRM_Dial_Phone_Number_From_Memory_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received re-dial last phone number indication event   */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER_IND   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Re_Dial_Last_Phone_Number_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Re_Dial_Last_Phone_Number_Indication_Message_t;

#define HFRM_RE_DIAL_LAST_PHONE_NUMBER_INDICATION_MESSAGE_SIZE (sizeof(HFRM_Re_Dial_Last_Phone_Number_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received generate DTMF code indication event          */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_GENERATE_DTMF_CODE_IND          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Generate_DTMF_Code_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   char                  DTMFCode;
} HFRM_Generate_DTMF_Code_Indication_Message_t;

#define HFRM_GENERATE_DTMF_CODE_INDICATION_MESSAGE_SIZE        (sizeof(HFRM_Generate_DTMF_Code_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received answer call indication event                 */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ANSWER_CALL_IND                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Answer_Call_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Answer_Call_Indication_Message_t;

#define HFRM_ANSWER_CALL_INDICATION_MESSAGE_SIZE               (sizeof(HFRM_Answer_Call_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received voice tag request indication event           */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_IND           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Voice_Tag_Request_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Voice_Tag_Request_Indication_Message_t;

#define HFRM_VOICE_TAG_REQUEST_INDICATION_MESSAGE_SIZE         (sizeof(HFRM_Voice_Tag_Request_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received hang up indication event (asynchronously).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_HANG_UP_IND                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Hang_Up_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Hang_Up_Indication_Message_t;

#define HFRM_HANG_UP_INDICATION_MESSAGE_SIZE                   (sizeof(HFRM_Hang_Up_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received query current calls list indication event    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_IND    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Query_Current_Calls_List_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Query_Current_Calls_List_Indication_Message_t;

#define HFRM_QUERY_CURRENT_CALLS_LIST_INDICATION_MESSAGE_SIZE  (sizeof(HFRM_Query_Current_Calls_List_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received network operator selection format indication */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_FORMAT_IND     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Network_Operator_Selection_Format_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          Format;
} HFRM_Network_Operator_Selection_Format_Indication_Message_t;

#define HFRM_NETWORK_OPERATOR_SELECTION_FORMAT_INDICATION_MESSAGE_SIZE  (sizeof(HFRM_Network_Operator_Selection_Format_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received network operator selection indication event  */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_IND  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Network_Operator_Selection_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Network_Operator_Selection_Indication_Message_t;

#define HFRM_NETWORK_OPERATOR_SELECTION_INDICATION_MESSAGE_SIZE   (sizeof(HFRM_Network_Operator_Selection_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received extended error result activation indication  */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_EXTENDED_ERROR_RESULT_ACT_IND   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Extended_Error_Result_Activation_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Enabled;
} HFRM_Extended_Error_Result_Activation_Indication_Message_t;

#define HFRM_EXTENDED_ERROR_RESULT_ACTIVATION_INDICATION_MESSAGE_SIZE   (sizeof(HFRM_Extended_Error_Result_Activation_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received subscriber number information indication     */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INF_IND       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Subscriber_Number_Information_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Subscriber_Number_Information_Indication_Message_t;

#define HFRM_SUBSCRIBER_NUMBER_INFORMATION_INDICATION_MESSAGE_SIZE   (sizeof(HFRM_Subscriber_Number_Information_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received response hold status indication event        */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_IND        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Response_Hold_Status_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Response_Hold_Status_Indication_Message_t;

#define HFRM_RESPONSE_HOLD_STATUS_INDICATION_MESSAGE_SIZE      (sizeof(HFRM_Response_Hold_Status_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received arbitrary command indication event           */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_ARBITRARY_COMMAND_IND           */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The ArbitraryCommand member *MUST* be NULL               */
   /*          terminated and the ArbitraryCommandLength member *MUST*  */
   /*          include the NULL terminator.                             */
typedef struct _tagHFRM_Arbitrary_Command_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ArbitraryCommandLength;
   char                  ArbitraryCommand[1];
} HFRM_Arbitrary_Command_Indication_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire send arbitrary command         */
   /* indication message given the length (in characters) of the        */
   /* arbitrary command.  This MACRO accepts as it's input the total    */
   /* number of characters (including the NULL terminator) that are     */
   /* present starting from the ArbitraryCommand member of the          */
   /* HFRM_Arbitrary_Command_Indication_Message_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define HFRM_ARBITRARY_COMMAND_INDICATION_MESSAGE_SIZE(_x)     (STRUCTURE_OFFSET(HFRM_Arbitrary_Command_Indication_Message_t, ArbitraryCommand) + (unsigned int)((sizeof(char)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received available codec list indication event        */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_AVAILABLE_CODEC_LIST_IND        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Available_Codec_List_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          NumberSupportedCodecs;
   unsigned char         AvailableCodecList[HFRE_MAX_SUPPORTED_CODECS];
} HFRM_Available_Codec_List_Indication_Message_t;

#define HFRM_AVAILABLE_CODEC_LIST_INDICATION_MESSAGE_SIZE      (sizeof(HFRM_Available_Codec_List_Indication_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received codec select confirmation event              */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_SELECT_CODEC_CFM                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Codec_Select_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned char         CodecID;
} HFRM_Codec_Select_Confirmation_Message_t;

#define HFRM_CODEC_SELECT_CONFIRMATION_MESSAGE_SIZE            (sizeof(HFRM_Codec_Select_Confirmation_Message_t))

   /* The following structure represents the message definition for a   */
   /* Hands Free Manager message that informs the local Audio Gateway   */
   /* client of a received codec connect indication event               */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HFRM_MESSAGE_FUNCTION_CONNECT_CODEC_IND               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHFRM_Codec_Connection_Setup_Indication_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HFRM_Codec_Connection_Setup_Indication_Message_t;

#define HFRM_CODEC_CONNECTION_SETUP_INDICATION_MESSAGE_SIZE     (sizeof(HFRM_Codec_Connection_Setup_Indication_Message_t))

#endif

