/*****< hfrmapi.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HFRMAPI - Local Hands Free Profile API for Stonestreet One Bluetooth      */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HFRMAPIH__
#define __HFRMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HFRMMSG.h"             /* BTPM Hands Free Profile Message Formats.  */

   /* The following structure holds the information needed to initialize*/
   /* the Audio Gateway and Hands Free server components of the Hands   */
   /* Free Manager.  The ServerPort member should specify a valid RFCOMM*/
   /* port number on which to host the server.                          */
   /* * NOTE * The Mandatory Hands Free Indicators (call, service, and  */
   /*          call_setup) are automatically added to the list and need */
   /*          not be specified as additional indicators.               */
   /* * NOTE * The NetworkType member is *ONLY* valid when specifying   */
   /*          initialization information for the Audio Gateway.        */
typedef struct _tagHFRM_Initialization_Data_t
{
   char                           *ServiceName;
   unsigned int                    ServerPort;
   unsigned long                   IncomingConnectionFlags;
   unsigned long                   SupportedFeaturesMask;
   unsigned long                   CallHoldingSupportMask;
   unsigned long                   NetworkType;
   unsigned int                    NumberAdditionalIndicators;
   HFRE_Control_Indicator_Entry_t *AdditionalSupportedIndicators;
   unsigned int                    MaximumNumberServers;
} HFRM_Initialization_Data_t;

#define HFRM_INITIALIZATION_DATA_SIZE                          (sizeof(HFRM_Initialization_Data_t))

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Hands Free Profile Module is  */
   /* initialized.                                                      */
typedef struct _tagHFRM_Initialization_Info_t
{
   HFRM_Initialization_Data_t *AudioGatewayInitializationInfo;
   HFRM_Initialization_Data_t *HandsFreeInitializationInfo;
} HFRM_Initialization_Info_t;

#define HFRM_INITIALIZATION_INFO_SIZE                          (sizeof(HFRM_Initialization_Info_t))

   /* The following structure is used with the                          */
   /* HFRM_Query_Current_Configuration() function as a container to hold*/
   /* the currently configured configuration.  See the                  */
   /* HFRM_Query_Current_Configuration() function for more information. */
typedef struct _tagHFRM_Current_Configuration_t
{
   unsigned long                         IncomingConnectionFlags;
   unsigned long                         SupportedFeaturesMask;
   unsigned long                         CallHoldingSupportMask;
   unsigned long                         NetworkType;
   unsigned int                          TotalNumberAdditionalIndicators;
   unsigned int                          NumberAdditionalIndicators;
   HFRM_Configuration_Indicator_Entry_t *AdditionalIndicatorList;
} HFRM_Current_Configuration_t;

#define HFRM_CURRENT_CONFIGURATION_SIZE                        (sizeof(HFRM_Current_Configuration_t))

   /* The following enumerated type represents the Hands Free Manager   */
   /* Event Types that are dispatched by this module.                   */
typedef enum
{
   /* Common Hands Free/Audio Gateway events.                           */
   hetHFRIncomingConnectionRequest,
   hetHFRConnected,
   hetHFRDisconnected,
   hetHFRConnectionStatus,
   hetHFRServiceLevelConnectionEstablished,
   hetHFRAudioConnected,
   hetHFRAudioDisconnected,
   hetHFRAudioConnectionStatus,
   hetHFRAudioData,
   hetHFRVoiceRecognitionIndication,
   hetHFRSpeakerGainIndication,
   hetHFRMicrophoneGainIndication,
   hetHFRIncomingCallStateIndication,

   /* Hands Free specific events.                                       */
   hetHFRIncomingCallStateConfirmation,
   hetHFRControlIndicatorStatusIndication,
   hetHFRControlIndicatorStatusConfirmation,
   hetHFRCallHoldMultipartySupportConfirmation,
   hetHFRCallWaitingNotificationIndication,
   hetHFRCallLineIdentificationNotificationIndication,
   hetHFRRingIndication,
   hetHFRInBandRingToneSettingIndication,
   hetHFRVoiceTagRequestConfirmation,
   hetHFRCurrentCallsListConfirmation,
   hetHFRNetworkOperatorSelectionConfirmation,
   hetHFRSubscriberNumberInformationConfirmation,
   hetHFRResponseHoldStatusConfirmation,
   hetHFRCommandResult,
   hetHFRArbitraryResponseIndication,
   hetHFRCodecSelectIndication,

   /* Audio Gateway specific events.                                    */
   hetHFRCallHoldMultipartySelectionIndication,
   hetHFRCallWaitingNotificationActivationIndication,
   hetHFRCallLineIdentificationNotificationActivationIndication,
   hetHFRDisableSoundEnhancementIndication,
   hetHFRDialPhoneNumberIndication,
   hetHFRDialPhoneNumberFromMemoryIndication,
   hetHFRReDialLastPhoneNumberIndication,
   hetHFRGenerateDTMFCodeIndication,
   hetHFRAnswerCallIndication,
   hetHFRVoiceTagRequestIndication,
   hetHFRHangUpIndication,
   hetHFRCurrentCallsListIndication,
   hetHFRNetworkOperatorSelectionFormatIndication,
   hetHFRNetworkOperatorSelectionIndication,
   hetHFRExtendedErrorResultActivationIndication,
   hetHFRSubscriberNumberInformationIndication,
   hetHFRResponseHoldStatusIndication,
   hetHFRArbitraryCommandIndication,
   hetHFRAvailableCodecListIndication,
   hetHFRCodecSelectConfirmation,
   hetHFRCodecConnectionSetupIndication
} HFRM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetIncomingConnectionRequest    */
   /* event.                                                            */
   /* * NOTE * The ConnectionType member specifies the type of          */
   /*          of connection of the local server (not the remote        */
   /*          device type).                                            */
typedef struct _tagHFRM_Incoming_Connection_Request_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Incoming_Connection_Request_Event_Data_t;

#define HFRM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE       (sizeof(HFRM_Incoming_Connection_Request_Event_Data_t))

   /* The following event is dispatched when a Hands Free connection    */
   /* occurs.  The ConnectionType member specifies which local Hands    */
   /* Free role type has been connected to and the RemoteDeviceAddress  */
   /* member specifies the remote Bluetooth device that has connected to*/
   /* the specified Hands Free Role.                                    */
typedef struct _tagHFRM_Connected_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Connected_Event_Data_t;

#define HFRM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(HFRM_Connected_Event_Data_t))

   /* The following event is dispatched when a remote device disconnects*/
   /* from the local device (for the specified Hands Free role).  The   */
   /* ConnectionType member identifies the local Hands Free role type   */
   /* being disconnected and the RemoteDeviceAddress member specifies   */
   /* the Bluetooth device address of the device that disconnected from */
   /* the profile.  The DisconnectReason member specifies whether or not*/
   /* the connection was disconnected via normal means or there was a   */
   /* service level connection establishment error.                     */
typedef struct _tagHFRM_Disconnected_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           DisconnectReason;
} HFRM_Disconnected_Event_Data_t;

#define HFRM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(HFRM_Disconnected_Event_Data_t))

   /* The following event is dispatched when a client receives the      */
   /* connection response from a remote server which was previously     */
   /* attempted to be connected to.  The ConnectionType member specifies*/
   /* the local client that has requested the connection, the           */
   /* RemoteDeviceAddress member specifies the remote device that was   */
   /* attempted to be connected to, and the ConnectionStatus member     */
   /* represents the connection status of the request.                  */
typedef struct _tagHFRM_Connection_Status_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ConnectionStatus;
} HFRM_Connection_Status_Event_Data_t;

#define HFRM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(HFRM_Connection_Status_Event_Data_t))

   /* The following event acts as an indication to inform the current   */
   /* state of the service level connection.  The ConnectionType member */
   /* identifies the connection type to which this event applies.  The  */
   /* RemoteSupportedFeaturesValid member specifies whether or not the  */
   /* Remote Support Features member is valid.  The                     */
   /* RemoteSupportedFeatures member specifies supported features which */
   /* are supported by the remote device.  The                          */
   /* RemoteCallHoldMultipartySupported member specifies the support of */
   /* this feature by the remote device.  This indication must be       */
   /* received before performing any other action on this port.  If an  */
   /* error occurs during the establishment of the service level        */
   /* connection the connection will be closed and local device will    */
   /* receive a disconnection event.                                    */
   /* * NOTE * The RemoteCallHoldMultipartySupport member will only be  */
   /*          valid if the local and remote device both have the       */
   /*          "Three-way Calling Support" bit set in their supported   */
   /*          features.                                                */
   /* * NOTE * The RemoteCallHoldMultiparySupport member will always    */
   /*          be set to                                                */
   /*          HFRE_CALL_HOLD_MULTIPARTY_SUPPORTED_FEATURES_ERROR in the*/
   /*          case when this indication is received by an audio gateway*/
   /*          as Hands Free units have no call hold multi-party        */
   /*          supported features to query.                             */
typedef struct _tagHFRM_Service_Level_Connection_Established_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              RemoteSupportedFeaturesValid;
   unsigned long          RemoteSupportedFeatures;
   unsigned long          RemoteCallHoldMultipartySupport;
} HFRM_Service_Level_Connection_Established_Event_Data_t;

#define HFRM_SERVICE_LEVEL_CONNECTION_ESTABLISHED_EVENT_DATA_SIZE (sizeof(HFRM_Service_Level_Connection_Established_Event_Data_t))

   /* The following event is dispatched to the local device when an     */
   /* audio connection is established.  The ConnectionType member       */
   /* identifies the connection that is receiving this indication.  The */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the remote device that the audio connection is established     */
   /* with.                                                             */
typedef struct _tagHFRM_Audio_Connected_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Audio_Connected_Event_Data_t;

#define HFRM_AUDIO_CONNECTED_EVENT_DATA_SIZE                   (sizeof(HFRM_Audio_Connected_Event_Data_t))

   /* The following event is dispatched to the local device when an     */
   /* audio connection is disconnected.  The ConnectionType member      */
   /* identifies the connection that is receiving this indication.  The */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the remote device that the audio connection is no longer       */
   /* established with.                                                 */
typedef struct _tagHFRM_Audio_Disconnected_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HFRM_Audio_Disconnected_Event_Data_t;

#define HFRM_AUDIO_DISCONNECTED_EVENT_DATA_SIZE                (sizeof(HFRM_Audio_Disconnected_Event_Data_t))

   /* The following event is dispatched when the originator of the audio*/
   /* connection (Audio Gateway only) receives the audio connection     */
   /* response from a remote device from which an audio connection      */
   /* request was previously sent.  The ConnectionType member identifies*/
   /* the connection that was attempting the audio connection.  The     */
   /* RemoteDeviceAddress member specifies the remote device address of */
   /* the remote device that the audio connection was requested.  The   */
   /* Status member specifies the result of the audio connection event. */
typedef struct _tagHFRM_Audio_Connection_Status_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              Successful;
} HFRM_Audio_Connection_Status_Event_Data_t;

#define HFRM_AUDIO_CONNECTION_STATUS_EVENT_DATA_SIZE           (sizeof(HFRM_Audio_Connection_Status_Event_Data_t))

   /* The following event is dispatched to the local device upon the    */
   /* reception of SCO audio data.  The ConnectionType member identifies*/
   /* the connection that has received the audio data.  The             */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the Bluetooth device that the specified audio data was received*/
   /* from.  The AudioDataLength member represents the size of the audio*/
   /* data pointed to the buffer that is specified by the AudioData     */
   /* member.                                                           */
typedef struct _tagHFRM_Audio_Data_Event_Data_t
{
   HFRM_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            DataEventsHandlerID;
   unsigned long           AudioDataFlags;
   unsigned int            AudioDataLength;
   unsigned char          *AudioData;
} HFRM_Audio_Data_Event_Data_t;

#define HFRM_AUDIO_DATA_EVENT_DATA_SIZE                        (sizeof(HFRM_Audio_Data_Event_Data_t))

   /* The following event may be dispatched to either a local Hands Free*/
   /* unit or a local Audio Gateway.  When this event is received by a  */
   /* local Hands Free unit it is to inform the local Hands Free unit of*/
   /* the remote Audio Gateways current voice recognition activation    */
   /* state.  When this event is received by a local Audio Gateway it is*/
   /* responsible for activating or deactivating the voice recognition  */
   /* functions which reside locally.  The ConnectionType member        */
   /* identifies the connection receiving this indication.  On a Hands  */
   /* Free unit the VoiceRecognitionActive member is used to inform the */
   /* local device of the remote Audio Gateway device's voice           */
   /* recognition activation state.  On an Audio Gateway the            */
   /* VoiceRecognitionActive member indicates whether to activate or    */
   /* deactivate the local voice recognition functions.                 */
typedef struct _tagHFRM_Voice_Recognition_Indication_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              VoiceRecognitionActive;
} HFRM_Voice_Recognition_Indication_Event_Data_t;

#define HFRM_VOICE_RECOGNITION_INDICATION_EVENT_DATA_SIZE      (sizeof(HFRM_Voice_Recognition_Indication_Event_Data_t))

   /* The following event may be dispatched to either a local Hands Free*/
   /* unit or a local Audio Gateway.  When this event is received by a  */
   /* local Hands Free unit it is to set the local speaker gain.  When  */
   /* this event is received by a local audio gateway it is used in     */
   /* volume level synchronization to inform the local Audio Gateway of */
   /* the current speaker gain on the remote Hands Free unit.  The      */
   /* ConnectionType member identifies the local connection that has    */
   /* received this indication.  The RemoteDeviceAddress member         */
   /* specifies the remote Bluetooth device address of the remote       */
   /* Bluetooth device that is informing the local device of the new    */
   /* speaker gain.  The SpeakerGain member is used to set or inform the*/
   /* device of the speaker gain.                                       */
typedef struct _tagHFRM_Speaker_Gain_Indication_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SpeakerGain;
} HFRM_Speaker_Gain_Indication_Event_Data_t;

#define HFRM_SPEAKER_GAIN_INDICATION_EVENT_DATA_SIZE           (sizeof(HFRM_Speaker_Gain_Indication_Event_Data_t))

   /* The following event may be dispatched to either a local Hands Free*/
   /* unit or a local Audio Gateway.  When this event is received by a  */
   /* local Hands Free unit it is to set the local microphone gain.     */
   /* When this event is received by a local audio gateway it is used in*/
   /* volume level synchronization to inform the local Audio Gateway of */
   /* the current microphone gain on the remote Hands Free unit.  The   */
   /* ConnectionType member identifies the local connection that has    */
   /* received this indication.  The RemoteDeviceAddress member         */
   /* specifies the remote Bluetooth device address of the remote       */
   /* Bluetooth device that is informing the local device of the new    */
   /* microphone gain.  The MicrophoneGain member is used to set or     */
   /* inform the device of the microphone gain.                         */
typedef struct _tagHFRM_Microphone_Gain_Indication_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           MicrophoneGain;
} HFRM_Microphone_Gain_Indication_Event_Data_t;

#define HFRM_MICROPHONE_GAIN_INDICATION_EVENT_DATA_SIZE        (sizeof(HFRM_Microphone_Gain_Indication_Event_Data_t))

   /* The following event may be dispatched to either a local Hands Free*/
   /* unit or a local Audio Gateway.  The local Audio Gateway service   */
   /* will receive this event when a remote Hands Free device sends a   */
   /* command to set the current call state.  The local Hands Free      */
   /* device will receive this event when the remote Audio Gateway sends*/
   /* a notification on a change in the current Response/Hold Status.   */
   /* The ConnectionType member identifies the local connection that is */
   /* receiving the indication.  The RemoteDeviceAddress member         */
   /* specifies the remote Bluetooth device address of the remote       */
   /* Bluetooth device of the connection.  The CallState member contains*/
   /* the call state requested by the remote device.                    */
typedef struct _tagHFRM_Incoming_Call_State_Indication_Event_Data_t
{
   HFRM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   HFRE_Call_State_t      CallState;
} HFRM_Incoming_Call_State_Indication_Event_Data_t;

#define HFRM_INCOMING_CALL_STATE_INDICATION_EVENT_DATA_SIZE    (sizeof(HFRM_Incoming_Call_State_Indication_Event_Data_t))

   /* Hands Free specific events.                                       */

   /* This event is dispatched to a local Hands Free service when a     */
   /* remote Audio Gateway device responds to a request for the current */
   /* call state information.  The CallState member contains the call   */
   /* state returned by the remote device.  The RemoteDeviceAddress     */
   /* member specifies the remote Bluetooth device address of the remote*/
   /* Bluetooth device of the connection.  The CallState member contains*/
   /* the call state of the remote device.                              */
typedef struct _tagHFRM_Incoming_Call_State_Confirmation_Event_Data_t
{
   BD_ADDR_t         RemoteDeviceAddress;
   HFRE_Call_State_t CallState;
} HFRM_Incoming_Call_State_Confirmation_Event_Data_t;

#define HFRM_INCOMING_CALL_STATE_CONFIRMATION_EVENT_DATA_SIZE  (sizeof(HFRM_Incoming_Call_State_Confirmation_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when a control indicator changes on the remote Audio Gateway and  */
   /* control indicator change notification is enabled.  The            */
   /* RemoteDeviceAddress member specifies the remote Bluetooth device  */
   /* address of the remote Bluetooth device of the connection.  The    */
   /* ControlIndicatorEntry member contains the Indicator that has      */
   /* changed.                                                          */
typedef struct _tagHFRM_Control_Indicator_Status_Indication_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   HFRE_Control_Indicator_Entry_t ControlIndicatorEntry;
} HFRM_Control_Indicator_Status_Indication_Event_Data_t;

#define HFRM_CONTROL_INDICATOR_STATUS_INDICATION_EVENT_DATA_SIZE  (sizeof(HFRM_Control_Indicator_Status_Indication_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service in*/
   /* response to a query for control indicators on the remote Audio    */
   /* Gateway.  The RemoteDeviceAddress member specifies the remote     */
   /* Bluetooth device address of the remote Bluetooth device of the    */
   /* connection.  The ControlIndicatorEntry member contains the        */
   /* Indicator value.                                                  */
typedef struct _tagHFRM_Control_Indicator_Status_Confirmation_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   HFRE_Control_Indicator_Entry_t ControlIndicatorEntry;
} HFRM_Control_Indicator_Status_Confirmation_Event_Data_t;

#define HFRM_CONTROL_INDICATOR_STATUS_CONFIRMATION_EVENT_DATA_SIZE   (sizeof(HFRM_Control_Indicator_Status_Confirmation_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when the remote Audio Gateway responds to a request for the remote*/
   /* call hold and multiparty supported features.  The                 */
   /* RemoteDeviceAddress member specifies the local connection for     */
   /* which the event is valid.  The CallHoldSupportMaskValid member is */
   /* flag which specifies whether or not the CallHoldSupportMask member*/
   /* is valid.                                                         */
   /* * NOTE * If the remote Audio Gateway does not have call hold and  */
   /*          multiparty support, the CallHoldSupportMaskValid member  */
   /*          will be FALSE.                                           */
typedef struct _tagHFRM_Call_Hold_Multiparty_Support_Confirmation_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   Boolean_t     CallHoldSupportMaskValid;
   unsigned long CallHoldSupportMask;
} HFRM_Call_Hold_Multiparty_Support_Confirmation_Event_Data_t;

#define HFRM_CALL_HOLD_MULTIPARTY_SUPPORT_CONFIRMATION_EVENT_DATA_SIZE  (sizeof(HFRM_Call_Hold_Multiparty_Support_Confirmation_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when the remote Audio Gateway receives a call while there is an   */
   /* on-going call in progress.  Note that call waiting notification   */
   /* must be active in order to receive this event.  The               */
   /* RemoteDeviceAddress member specifies the local connection for     */
   /* which the event is valid.  The PhoneNumber member is a NULL       */
   /* terminated ASCII string representing the phone number of the      */
   /* waiting call.                                                     */
typedef struct _tagHFRM_Call_Waiting_Notification_Indication_Event_Data_t
{
   BD_ADDR_t  RemoteDeviceAddress;
   char      *PhoneNumber;
} HFRM_Call_Waiting_Notification_Indication_Event_Data_t;

#define HFRM_CALL_WAITING_NOTIFICATION_INDICATION_EVENT_DATA_SIZE (sizeof(HFRM_Call_Waiting_Notification_Indication_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when the remote Audio Gateway receives a call line notification.  */
   /* Note that call line identification notification must be active in */
   /* order to receive this event.  The RemoteDeviceAddress member      */
   /* specifies the local connection for which the event is valid.  The */
   /* PhoneNumber member is a NULL terminated ASCII string representing */
   /* the phone number of the incoming call.                            */
typedef struct _tagHFRM_Call_Line_Identification_Notification_Indication_Event_Data_t
{
   BD_ADDR_t  RemoteDeviceAddress;
   char      *PhoneNumber;
} HFRM_Call_Line_Identification_Notification_Indication_Event_Data_t;

#define HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_EVENT_DATA_SIZE (sizeof(HFRM_Call_Line_Identification_Notification_Indication_Event_Data_t))

   /* The following event is dispatched to a local Hands Free unit when */
   /* the remote Audio Gateway sends a RING indication to the local     */
   /* device.  The RemoteDeviceAddress member specifies the local       */
   /* connection which is receiving this indication.                    */
typedef struct _tagHFRM_Ring_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Ring_Indication_Event_Data_t;

#define HFRM_RING_INDICATION_EVENT_DATA_SIZE                   (sizeof(HFRM_Ring_Indication_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when the remote Audio Gateway wants to change the in-band ring    */
   /* tone setting during an ongoing service level connection.  The     */
   /* RemoteDeviceAddress member specifies the local connection which is*/
   /* receiving this indication.  The Enabled member specifies whether  */
   /* this is an indication that in-band ringing is enabled (TRUE) or   */
   /* disabled (FALSE).                                                 */
typedef struct _tagHFRM_In_Band_Ring_Tone_Setting_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   Boolean_t Enabled;
} HFRM_In_Band_Ring_Tone_Setting_Indication_Event_Data_t;

#define HFRM_IN_BAND_RING_TONE_SETTING_INDICATION_EVENT_DATA_SIZE (sizeof(HFRM_In_Band_Ring_Tone_Setting_Indication_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when the remote Audio Gateway responds to a request for a phone   */
   /* number to attach to a voice tag.  The RemoteDeviceAddress member  */
   /* identifies the connection receiving this confirmation.  The       */
   /* PhoneNumber member is a NULL terminated ASCII string representing */
   /* the phone number that was attached to a voice tag.                */
typedef struct _tagHFRM_Voice_Tag_Request_Confirmation_Event_Data_t
{
   BD_ADDR_t  RemoteDeviceAddress;
   char      *PhoneNumber;
} HFRM_Voice_Tag_Request_Confirmation_Event_Data_t;

#define HFRM_VOICE_TAG_REQUEST_CONFIRMATION_EVENT_DATA_SIZE    (sizeof(HFRM_Voice_Tag_Request_Confirmation_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when a remote Audio gateway responds to a request for the list of */
   /* current calls.  The RemoteDeviceAddress member identifies the     */
   /* connection receiving the confirmation.  The CurrentCallListEntry  */
   /* member contains the information pertaining to a the call list     */
   /* entry.                                                            */
typedef struct _tagHFRM_Current_Calls_List_Confirmation_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   HFRE_Current_Call_List_Entry_t CurrentCallListEntry;
} HFRM_Current_Calls_List_Confirmation_Event_Data_t;

#define HFRM_CURRENT_CALLS_LIST_CONFIRMATION_EVENT_DATA_SIZE   (sizeof(HFRM_Current_Calls_List_Confirmation_Event_Data_t))

   /* This event is dispatched to a local Hands Free service when a     */
   /* remote Audio Gateway responds to the request for current operator */
   /* selection.  The RemoteDeviceAddress member identifies the         */
   /* connection receiving the event.  The NetworkMode member contains  */
   /* the mode returned in the response.  The NetworkOperator member is */
   /* a pointer to a NULL terminated ASCII string that contains the     */
   /* returned operator name.                                           */
typedef struct _tagHFRM_Network_Operator_Selection_Confirmation_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  NetworkMode;
   char         *NetworkOperator;
} HFRM_Network_Operator_Selection_Confirmation_Event_Data_t;

#define HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_EVENT_DATA_SIZE (sizeof(HFRM_Network_Operator_Selection_Confirmation_Event_Data_t))

   /* This event is dispatched to a local Hands Free service when a     */
   /* remote Audio Gateway responds to a request for the current        */
   /* subscriber number.  The RemoteDeviceAddress member identifies the */
   /* connection receiving the confirmation.  The                       */
   /* SubscriberNumberInformation entry contains the subscriber number  */
   /* information.                                                      */
typedef struct _tagHFRM_Subscriber_Number_Information_Confirmation_Event_Data_t
{
   BD_ADDR_t                            RemoteDeviceAddress;
   HFRM_Subscriber_Number_Information_t SubscriberNumberInformation;
} HFRM_Subscriber_Number_Information_Confirmation_Event_Data_t;

#define HFRM_SUBSCRIBER_NUMBER_INFORMATION_CONFIRMATION_EVENT_DATA_SIZE (sizeof(HFRM_Subscriber_Number_Information_Confirmation_Event_Data_t))

   /* This event is dispatched to a local Hands Free service when a     */
   /* remote Audio Gateway device responds to a request for the current */
   /* Response/Hold status.  The RemoteDeviceAddress member identifies  */
   /* the connection receiving the event.  The CallState member contains*/
   /* the call state sent in the response.                              */
typedef struct _tagHFRM_Response_Hold_Status_Confirmation_Event_Data_t
{
   BD_ADDR_t         RemoteDeviceAddress;
   HFRE_Call_State_t CallState;
} HFRM_Response_Hold_Status_Confirmation_Event_Data_t;

#define HFRM_RESPONSE_HOLD_STATUS_CONFIRMATION_EVENT_DATA_SIZE (sizeof(HFRM_Response_Hold_Status_Confirmation_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when the remote Audio Gateway responds to a commmand sent by the  */
   /* Hands Free unit or generates an unsolicited result code.  The     */
   /* RemoteDeviceAddress member identifies the connection receiving    */
   /* this indication.  The ResultValue member contains the actual      */
   /* result code value if the ResultType parameter indicates that a    */
   /* result code is expected (erResultCode).                           */
typedef struct _tagHFRM_Command_Result_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   HFRE_Extended_Result_t ResultType;
   unsigned int           ResultValue;
} HFRM_Command_Result_Event_Data_t;

#define HFRM_COMMAND_RESULT_EVENT_DATA_SIZE                    (sizeof(HFRM_Command_Result_Event_Data_t))

   /* The following event is dispatched to a local Hands Free unit when */
   /* the remote Audio Gateway issues either a non-solicited arbitrary  */
   /* response OR a solicited arbitrary response that is not recognized */
   /* by the local Hands Free unit (i.e.  an arbitrary AT Response).    */
   /* The RemoteDeviceAddress member identifies the connection receiving*/
   /* this evnet.  The ResponseData member is a pointer to a NULL       */
   /* terminated ASCII string that represents the actual response data  */
   /* that was received.                                                */
typedef struct _tagHFRM_Arbitrary_Response_Indication_Event_Data_t
{
   BD_ADDR_t  RemoteDeviceAddress;
   char      *ResponseData;
} HFRM_Arbitrary_Response_Indication_Event_Data_t;

#define HFRM_ARBITRARY_RESPONSE_INDICATION_EVENT_DATA_SIZE     (sizeof(HFRM_Arbitrary_Response_Indication_Event_Data_t))

   /* The following event is dispatched to a local Hands Free service   */
   /* when the remote Audio Gateway device indicates a codec to select. */
   /* The RemoteDeviceAddress member identifies the local connection    */
   /* receiving this indication. The CodecID member is the selected     */
   /* codec.                                                            */
typedef struct _tagHFRM_Codec_Select_Indication_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned char CodecID;
} HFRM_Codec_Select_Indication_Event_Data_t;

#define HFRM_CODEC_SELECT_INDICATION_EVENT_DATA_SIZE           (sizeof(HFRM_Codec_Select_Indication_Event_Data_t))

   /* Audio Gateway specific events.                                    */

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free service responds to a call waiting          */
   /* notification by selecting how to handle the waiting call.  The    */
   /* RemoteDeviceAddress member identifies the local service receiving */
   /* this indication.  The CallHoldMultipartyHandling member specifies */
   /* the requested action to take regarding the waiting call.  If the  */
   /* CallHoldMultipartyHandling member indicates an extended type then */
   /* the Index member will contain the call index to which the         */
   /* operation refers.                                                 */
typedef struct _tagHFRM_Call_Hold_Multiparty_Selection_Indication_Event_Data_t
{
   BD_ADDR_t                                 RemoteDeviceAddress;
   HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling;
   unsigned int                              Index;
} HFRM_Call_Hold_Multiparty_Selection_Indication_Event_Data_t;

#define HFRM_CALL_HOLD_MULTIPARTY_SELECTION_INDICATION_EVENT_DATA_SIZE  (sizeof(HFRM_Call_Hold_Multiparty_Selection_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command to enable or      */
   /* disable call waiting notification.  The RemoteDeviceAddress member*/
   /* identifies the local service receiving this indication.  The      */
   /* Enabled member specifies whether this is an indication to enable  */
   /* (TRUE) or disable (FALSE) call waiting notification.              */
typedef struct _tagHFRM_Call_Waiting_Notification_Activation_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   Boolean_t Enabled;
} HFRM_Call_Waiting_Notification_Activation_Indication_Event_Data_t;

#define HFRM_CALL_WAITING_NOTIFICATION_ACTIVATION_INDICATION_EVENT_DATA_SIZE (sizeof(HFRM_Call_Waiting_Notification_Activation_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command to enable or      */
   /* disable call line identification notification.  The               */
   /* RemoteDeviceAddress member identifies the local service receiving */
   /* this indication.  The Enable member specifies whether this is an  */
   /* indication to enable (TRUE) or disable (FALSE) call line          */
   /* identification notification.                                      */
typedef struct _tagHFRM_Call_Line_Identification_Notification_Activation_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   Boolean_t Enabled;
} HFRM_Call_Line_Identification_Notification_Activation_Indication_Event_Data_t;

#define HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_ACTIVATION_INDICATION_EVENT_DATA_SIZE  (sizeof(HFRM_Call_Line_Identification_Notification_Activation_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device requests turning off the Audio       */
   /* Gateways echo cancelling and noise reduction functions.  The      */
   /* RemoteDeviceAddress member identifies the local connection        */
   /* receiving this indication.                                        */
typedef struct _tagHFRM_Disable_Sound_Enhancement_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Disable_Sound_Enhancement_Indication_Event_Data_t;

#define HFRM_DISABLE_SOUND_ENHANCEMENT_INDICATION_EVENT_DATA_SIZE (sizeof(HFRM_Disable_Sound_Enhancement_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command to place a call to*/
   /* a specific phone number.  The RemoteDeviceAddress member          */
   /* identifies the local connection receiving this indication.  The   */
   /* PhoneNumber member is a NULL terminated ASCII string representing */
   /* the phone number in which to place the call.                      */
typedef struct _tagHFRM_Dial_Phone_Number_Indication_Event_Data_t
{
   BD_ADDR_t  RemoteDeviceAddress;
   char      *PhoneNumber;
} HFRM_Dial_Phone_Number_Indication_Event_Data_t;

#define HFRM_DIAL_PHONE_NUMBER_INDICATION_EVENT_DATA_SIZE      (sizeof(HFRM_Dial_Phone_Number_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command for memory        */
   /* dialing.  The RemoteDeviceAddress member identifies the local     */
   /* connection receiving this indication.  The MemoryLocation member  */
   /* specifies the memory location in which the phone number to dial   */
   /* exists.                                                           */
typedef struct _tagHFRM_Dial_Phone_Number_From_Memory_Indication_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int MemoryLocation;
} HFRM_Dial_Phone_Number_From_Memory_Indication_Event_Data_t;

#define HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_INDICATION_EVENT_DATA_SIZE   (sizeof(HFRM_Dial_Phone_Number_From_Memory_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command to re-dial the    */
   /* last number dialed.  The RemoteDeviceAddress member identifies the*/
   /* local connection receiving this indication.                       */
typedef struct _tagHFRM_Re_Dial_Last_Phone_Number_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Re_Dial_Last_Phone_Number_Indication_Event_Data_t;

#define HFRM_RE_DIAL_LAST_PHONE_NUMBER_INDICATION_EVENT_DATA_SIZE (sizeof(HFRM_Re_Dial_Last_Phone_Number_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command to transmit a DTMF*/
   /* code.  The RemoteDeviceAddress member identifies the local        */
   /* connection receiving this indication.  The DTMFCode member        */
   /* specifies the DTMF code to generate.                              */
typedef struct _tagHFRM_Generate_DTMF_Code_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   char      DTMFCode;
} HFRM_Generate_DTMF_Code_Indication_Event_Data_t;

#define HFRM_GENERATE_DTMF_CODE_INDICATION_EVENT_DATA_SIZE     (sizeof(HFRM_Generate_DTMF_Code_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command to answer an      */
   /* incoming call.  The RemoteDeviceAddress member identifies the     */
   /* local connection receiving this indication.                       */
typedef struct _tagHFRM_Answer_Call_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Answer_Call_Indication_Event_Data_t;

#define HFRM_ANSWER_CALL_INDICATION_EVENT_DATA_SIZE            (sizeof(HFRM_Answer_Call_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device makes a request for a phone number to*/
   /* attach to a voice tag.  The RemoteDeviceAddress member identifies */
   /* the local connection receiving this indication.                   */
typedef struct _tagHFRM_Voice_Tag_Request_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Voice_Tag_Request_Indication_Event_Data_t;

#define HFRM_VOICE_TAG_REQUEST_INDICATION_EVENT_DATA_SIZE      (sizeof(HFRM_Voice_Tag_Request_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Hands Free device issues the command to hang-up an     */
   /* on-going call or to reject and incoming call request.  The        */
   /* RemoteDeviceAddress member identifies the local connection        */
   /* receiving this indication.                                        */
typedef struct _tagHFRM_Hang_Up_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Hang_Up_Indication_Event_Data_t;

#define HFRM_HANG_UP_INDICATION_EVENT_DATA_SIZE                (sizeof(HFRM_Hang_Up_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio gateway when it*/
   /* receives a request for the current call list.  The                */
   /* RemoteDeviceAddress member identifies the local connection        */
   /* receiving the indication.                                         */
typedef struct _tagHFRM_Current_Calls_List_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Current_Calls_List_Indication_Event_Data_t;

#define HFRM_CURRENT_CALLS_LIST_INDICATION_EVENT_DATA_SIZE     (sizeof(HFRM_Current_Calls_List_Indication_Event_Data_t))

   /* This event is dispatched to a local Audio Gateway service when the*/
   /* remote Hands Free device issues a set operator selection format   */
   /* command.  The RemoteDeviceAddress member identifies the local     */
   /* connection receiving the indication.  The Format member contains  */
   /* the format value provided in this operation.  The Bluetooth Hands */
   /* Free specification requires that the remote device choose format  */
   /* 0.  As a result, this event can generally be ignored.             */
typedef struct _tagHFRM_Network_Operator_Selection_Format_Indication_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Format;
} HFRM_Network_Operator_Selection_Format_Indication_Event_Data_t;

#define HFRM_NETWORK_OPERATOR_SELECTION_FORMAT_INDICATION_EVENT_DATA_SIZE  (sizeof(HFRM_Network_Operator_Selection_Format_Indication_Event_Data_t))

   /* This event is dispatched to a local Audio Gateway service when the*/
   /* remote Hands Free device has requested the current operator       */
   /* selection.  The RemoteDeviceAddress member identifies the local   */
   /* connection receiving the indication.                              */
typedef struct _tagHFRM_Network_Operator_Selection_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Network_Operator_Selection_Indication_Event_Data_t;

#define HFRM_NETWORK_OPERATOR_SELECTION_INDICATION_EVENT_DATA_SIZE (sizeof(HFRM_Network_Operator_Selection_Indication_Event_Data_t))

   /* This event is dispatched to a local Audio Gateway service when a  */
   /* remote Hands Free device sends a command activate extended error  */
   /* reporting.  The RemoteDeviceAddress member identifies the local   */
   /* connection receiving the indication.  The Enabled member contains */
   /* a BOOLEAN value which indicates the current state of extended     */
   /* error reporting.                                                  */
typedef struct _tagHFRM_Extended_Error_Result_Activation_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   Boolean_t Enabled;
} HFRM_Extended_Error_Result_Activation_Indication_Event_Data_t;

#define HFRM_EXTENDED_ERROR_RESULT_ACTIVATION_INDICATION_EVENT_DATA_SIZE   (sizeof(HFRM_Extended_Error_Result_Activation_Indication_Event_Data_t))

   /* This event is dispatched to a local Audio Gateway service when a  */
   /* remote Hands Free device requests the current subscriber number.  */
   /* The RemoteDeviceAddress member identifies the local connection    */
   /* receiving the indication.                                         */
typedef struct _tagHFRM_Subscriber_Number_Information_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Subscriber_Number_Information_Indication_Event_Data_t;

#define HFRM_SUBSCRIBER_NUMBER_INFORMATION_INDICATION_EVENT_DATA_SIZE   (sizeof(HFRM_Subscriber_Number_Information_Indication_Event_Data_t))

   /* This event is dispatched to a local Audio Gateway service when a  */
   /* remote Hands Free device requests the current response/hold       */
   /* status.  The RemoteDeviceAddress member identifies the local      */
   /* connection receiving the indication.                              */
typedef struct _tagHFRM_Response_Hold_Status_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Response_Hold_Status_Indication_Event_Data_t;

#define HFRM_RESPONSE_HOLD_STATUS_INDICATION_EVENT_DATA_SIZE   (sizeof(HFRM_Response_Hold_Status_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway service*/
   /* when the remote Hands Free device issues a commmand that is not   */
   /* recognized by the local Audio Gateway (i.e.  an arbitrary AT      */
   /* Command).  The RemoteDeviceAddress member identifies the local    */
   /* connection receiving this indication.  The CommandData member is a*/
   /* pointer to a NULL terminated ASCII string that represents the     */
   /* actual command data that was received.                            */
typedef struct _tagHFRM_Arbitrary_Command_Indication_Event_Data_t
{
   BD_ADDR_t  RemoteDeviceAddress;
   char      *CommandData;
} HFRM_Arbitrary_Command_Indication_Event_Data_t;

#define HFRM_ARBITRARY_COMMAND_INDICATION_EVENT_DATA_SIZE      (sizeof(HFRM_Arbitrary_Command_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway when   */
   /* when the remote Hands Free device sends a list of supported       */
   /* codecs. An audio gateway responds to an event initiated by an AT  */
   /* command from the hands free device with 'OK'. Because this list   */
   /* is typically sent as part of a connection process this Bluetopia  */
   /* implementation handles the response automatically. This is        */
   /* usually the responsibility of the application. The                */ 
   /* RemoteDeviceAddress member identifies the remote sender of the    */
   /* list. The NumberSupportedCodecs member indicates the number of    */
   /* codecs in the list. The AvailableCodecList member is the list of  */
   /* supported Codecs.                                                 */
typedef struct _tagHFRM_Available_Codec_List_Indication_Event_Data_t
{
   BD_ADDR_t      RemoteDeviceAddress;
   unsigned int   NumberSupportedCodecs;
   unsigned char *AvailableCodecList;
} HFRM_Available_Codec_List_Indication_Event_Data_t;

#define HFRM_AVAILABLE_CODEC_LIST_INDICATION_EVENT_DATA_SIZE   (sizeof(HFRM_Available_Codec_List_Indication_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway service*/
   /* when the remote Hands Free device accepts a codec selection.      */
   /* The RemoteDeviceAddress member identifies the local connection    */
   /* receiving this indication. The AcceptedCodec member is the Codec  */
   /* ID returned by the remote Hands Free device.                      */
typedef struct _tagHFRM_Codec_Select_Confirmation_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned char AcceptedCodec;
} HFRM_Codec_Select_Confirmation_Event_Data_t;

#define HFRM_CODEC_SELECT_CONFIRMATION_EVENT_DATA_SIZE         (sizeof(HFRM_Codec_Select_Confirmation_Event_Data_t))

   /* The following event is dispatched to a local Audio Gateway service*/
   /* when the remote Hands Free device requests to setup a codec audio */
   /* connection.  The RemoteDeviceAddress member identifies the local  */
   /* connection receiving this indication.                             */
typedef struct _tagHFRM_Codec_Connection_Setup_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HFRM_Codec_Connection_Setup_Indication_Event_Data_t;

#define HFRM_CODEC_CONNECTION_SETUP_INDICATION_EVENT_DATA_SIZE          (sizeof(HFRM_Codec_Connection_Setup_Indication_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Hands Free Manager Event (and Event Data) of a Hands Free Manager */
   /* Event.                                                            */
typedef struct _tagHFRM_Event_Data_t
{
   HFRM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      HFRM_Incoming_Connection_Request_Event_Data_t                                 IncomingConnectionRequestEventData;
      HFRM_Connected_Event_Data_t                                                   ConnectedEventData;
      HFRM_Disconnected_Event_Data_t                                                DisconnectedEventData;
      HFRM_Connection_Status_Event_Data_t                                           ConnectionStatusEventData;
      HFRM_Service_Level_Connection_Established_Event_Data_t                        ServiceLevelConnectionEstablishedEventData;
      HFRM_Audio_Connected_Event_Data_t                                             AudioConnectedEventData;
      HFRM_Audio_Disconnected_Event_Data_t                                          AudioDisconnectedEventData;
      HFRM_Audio_Connection_Status_Event_Data_t                                     AudioConnectionStatusEventData;
      HFRM_Audio_Data_Event_Data_t                                                  AudioDataEventData;
      HFRM_Voice_Recognition_Indication_Event_Data_t                                VoiceRecognitionIndicationEventData;
      HFRM_Speaker_Gain_Indication_Event_Data_t                                     SpeakerGainIndicationEventData;
      HFRM_Microphone_Gain_Indication_Event_Data_t                                  MicrophoneGainIndicationEventData;
      HFRM_Incoming_Call_State_Indication_Event_Data_t                              IncomingCallStateIndicationEventData;

      /* Hands Free specific events.                                    */
      HFRM_Incoming_Call_State_Confirmation_Event_Data_t                            IncomingCallStateConfirmationEventData;
      HFRM_Control_Indicator_Status_Indication_Event_Data_t                         ControlIndicatorStatusIndicationEventData;
      HFRM_Control_Indicator_Status_Confirmation_Event_Data_t                       ControlIndicatorStatusConfirmationEventData;
      HFRM_Call_Hold_Multiparty_Support_Confirmation_Event_Data_t                   CallHoldMultipartySupportConfirmationEventData;
      HFRM_Call_Waiting_Notification_Indication_Event_Data_t                        CallWaitingNotificationIndicationEventData;
      HFRM_Call_Line_Identification_Notification_Indication_Event_Data_t            CallLineIdentificationNotificationIndicationEventData;
      HFRM_Ring_Indication_Event_Data_t                                             RingIndicationEventData;
      HFRM_In_Band_Ring_Tone_Setting_Indication_Event_Data_t                        InBandRingToneSettingIndicationEventData;
      HFRM_Voice_Tag_Request_Confirmation_Event_Data_t                              VoiceTagRequestConfirmationEventData;
      HFRM_Current_Calls_List_Confirmation_Event_Data_t                             CurrentCallsListConfirmationEventData;
      HFRM_Network_Operator_Selection_Confirmation_Event_Data_t                     NetworkOperatorSelectionConfirmationEventData;
      HFRM_Subscriber_Number_Information_Confirmation_Event_Data_t                  SubscriberNumberInformationConfirmationEventData;
      HFRM_Response_Hold_Status_Confirmation_Event_Data_t                           ResponseHoldStatusConfirmationEventData;
      HFRM_Command_Result_Event_Data_t                                              CommandResultEventData;
      HFRM_Arbitrary_Response_Indication_Event_Data_t                               ArbitraryResponseIndicationEventData;
      HFRM_Codec_Select_Indication_Event_Data_t                                     CodecSelectIndicationEventData;

      /* Audio Gateway specific events.                                 */
      HFRM_Call_Hold_Multiparty_Selection_Indication_Event_Data_t                   CallHoldMultipartySelectionIndicationEventData;
      HFRM_Call_Waiting_Notification_Activation_Indication_Event_Data_t             CallWaitingNotificationActivationIndicationEventData;
      HFRM_Call_Line_Identification_Notification_Activation_Indication_Event_Data_t CallLineIdentificationNotificationActivationIndicationEventData;
      HFRM_Disable_Sound_Enhancement_Indication_Event_Data_t                        DisableSoundEnhancementIndicationEventData;
      HFRM_Dial_Phone_Number_Indication_Event_Data_t                                DialPhoneNumberIndicationEventData;
      HFRM_Dial_Phone_Number_From_Memory_Indication_Event_Data_t                    DialPhoneNumberFromMemoryIndicationEventData;
      HFRM_Re_Dial_Last_Phone_Number_Indication_Event_Data_t                        ReDialLastPhoneNumberIndicationEventData;
      HFRM_Answer_Call_Indication_Event_Data_t                                      AnswerCallIndicationEventData;
      HFRM_Generate_DTMF_Code_Indication_Event_Data_t                               GenerateDTMFCodeIndicationEventData;
      HFRM_Voice_Tag_Request_Indication_Event_Data_t                                VoiceTagRequestIndicationEventData;
      HFRM_Hang_Up_Indication_Event_Data_t                                          HangUpIndicationEventData;
      HFRM_Current_Calls_List_Indication_Event_Data_t                               CurrentCallsListIndicationEventData;
      HFRM_Network_Operator_Selection_Format_Indication_Event_Data_t                NetworkOperatorSelectionFormatIndicationEventData;
      HFRM_Network_Operator_Selection_Indication_Event_Data_t                       NetworkOperatorSelectionIndicationEventData;
      HFRM_Extended_Error_Result_Activation_Indication_Event_Data_t                 ExtendedErrorResultActivationIndicationEventData;
      HFRM_Subscriber_Number_Information_Indication_Event_Data_t                    SubscriberNumberInformationIndicationEventData;
      HFRM_Response_Hold_Status_Indication_Event_Data_t                             ResponseHoldStatusIndicationEventData;
      HFRM_Arbitrary_Command_Indication_Event_Data_t                                ArbitraryCommandIndicationEventData;
      HFRM_Available_Codec_List_Indication_Event_Data_t                             AvailableCodecListIndicationEventData;
      HFRM_Codec_Select_Confirmation_Event_Data_t                                   CodecSelectConfirmationEventData;
      HFRM_Codec_Connection_Setup_Indication_Event_Data_t                           CodecConnectionSetupIndicationEventData;
   } EventData;
} HFRM_Event_Data_t;

#define HFRM_EVENT_DATA_SIZE                                   (sizeof(HFRM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Hands Free Manager dispatches an event (and the client has        */
   /* registered for events).  This function passes to the caller the   */
   /* Hands Free Manager Event and the Callback Parameter that was      */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the Event Data ONLY in the context of this    */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be reentrant).  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another Message will */
   /* not be processed while this function call is outstanding).        */
   /* * NOTE * This function MUST NOT block and wait for events that can*/
   /*          only be satisfied by Receiving other Events.  A deadlock */
   /*          WILL occur because NO Event Callbacks will be issued     */
   /*          while this function is currently outstanding.            */
typedef void (BTPSAPI *HFRM_Event_Callback_t)(HFRM_Event_Data_t *EventData, void *CallbackParameter);

   /* Hands Free/Audio Gateway Module Installation/Support Functions.   */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Hands Free Manager module.  This   */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI HFRM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HFRM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* Hands Free Manager Connection Management Functions.               */

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server. This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened. A   */
   /*          hetHFRConnected event will notify if the connection is   */
   /*          successful.                                              */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Connection_Request_Response(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Connection_Request_Response_t)(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Hands Free/Audio Gateway device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.  This function accepts the      */
   /* connection type to make as the first parameter.  This parameter   */
   /* specifies the LOCAL connection type (i.e. if the caller would     */
   /* like to connect the local Hands Free service to a remote Audio    */
   /* Gateway device, the Hands Free connection type would be specified */
   /* for this parameter).  This function also accepts the connection   */
   /* information for the remote device (address and server port).      */
   /* This function accepts the connection flags to apply to control    */
   /* how the connection is made regarding encryption and/or            */
   /* authentication.                                                   */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Hands Free Manager Connection Status Event (if           */
   /*          specified).                                              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHFRConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HFRM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Connect_Remote_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Connect_Remote_Device_t)(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function exists to close an active Hands Free or    */
   /* Audio Gateway connection that was previously opened by any of the */
   /* following mechanisms:                                             */
   /*   - Successful call to HFRM_Connect_Remote_Device() function.     */
   /*   - Incoming open request (Hands Free or Audio Gateway) which was */
   /*     accepted either automatically or by a call to                 */
   /*     HFRM_Connection_Request_Response().                           */
   /* This function accepts as input the type of the local connection   */
   /* which should close its active connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.  This function does NOT un-register any Hands Free or Audio*/
   /* Gateway services from the system, it ONLY disconnects any         */
   /* connection that is currently active on the specified service.     */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Disconnect_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Disconnect_Device_t)(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Hands   */
   /* Free or Audio Gateway Devices (specified by the first parameter). */
   /* This function accepts a the local service type to query, followed */
   /* by buffer information to receive any currently connected device   */
   /* addresses of the specified connection type.  The first parameter  */
   /* specifies the local service type to query the connection          */
   /* information for.  The second parameter specifies the maximum      */
   /* number of BD_ADDR entries that the buffer will support (i.e.  can */
   /* be copied into the buffer).  The next parameter is optional and,  */
   /* if specified, will be populated with the total number of connected*/
   /* devices if the function is successful.  The final parameter can be*/
   /* used to retrieve the total number of connected devices (regardless*/
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of connected devices that were copied into  */
   /* the specified input buffer.  This function returns a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Connected_Devices(HFRM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Connected_Devices_t)(HFRM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Hands Free or Audio*/
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * On input the TotalNumberAdditionalIndicators member of   */
   /*          the structure should be set to the total number of       */
   /*          additional indicator entry structures that the           */
   /*          AdditionalIndicatorList member points to.  On return from*/
   /*          this function this structure member holds the total      */
   /*          number of additional indicator entries that are supported*/
   /*          by the connection.  The NumberAdditionalIndicators       */
   /*          member will hold (on return) the number of indicator     */
   /*          entries that are actually present in the list.           */
   /* * NOTE * It is possible to not query the additional indicators    */
   /*          by passing zero for the TotalNumberAdditionalIndicators  */
   /*          member (the AdditionalIndicatorList member will be       */
   /*          ignored in this case).  This member will still hold the  */
   /*          total supported indicators on return of this function,   */
   /*          however, no indicators will be returned.                 */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Current_Configuration(HFRM_Connection_Type_t ConnectionType, HFRM_Current_Configuration_t *CurrentConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Current_Configuration_t)(HFRM_Connection_Type_t ConnectionType, HFRM_Current_Configuration_t *CurrentConfiguration);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Hands Free and*/
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI HFRM_Change_Incoming_Connection_Flags(HFRM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Change_Incoming_Connection_Flags_t)(HFRM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags);
#endif

   /* Shared Hands Free/Audio Gateway Functions.                        */

   /* This function is responsible for disabling echo cancellation and  */
   /* noise reduction on the remote device.  This function may be       */
   /* performed by both the Hands Free and the Audio Gateway connections*/
   /* for which a valid service level connection exists but no audio    */
   /* connection exists.  This function accepts as its input parameter  */
   /* the connection type indicating which local service which will send*/
   /* this command.  This function returns zero if successful or a      */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * It is not possible to enable this feature once it has    */
   /*          been disbled because the specification provides no means */
   /*          to re-enable this feature.  This feature will remained   */
   /*          disabled until the current service level connection has  */
   /*          been dropped.                                            */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* When called by a Hands Free device, this function is responsible  */
   /* for requesting activation or deactivation of the voice recognition*/
   /* which resides on the remote Audio Gateway.  When called by an     */
   /* Audio Gateway, this function is responsible for informing the     */
   /* remote Hands Free device of the current activation state of the   */
   /* local voice recognition function.  This function may only be      */
   /* called by local devices that were opened with support for voice   */
   /* recognition.  This function accepts as its input parameters the   */
   /* connection type indicating the local connection which will process*/
   /* the command and a BOOLEAN flag specifying the type of request or  */
   /* notification to send.  When active the voice recognition function */
   /* on the Audio Gateway is turned on, when inactive the voice        */
   /* recognition function on the Audio Gateway is turned off.  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Set_Remote_Voice_Recognition_Activation(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t VoiceRecognitionActive);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Set_Remote_Voice_Recognition_Activation_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableVoiceRecognition);
#endif

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  This function may    */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current speaker   */
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the speaker gain*/
   /* of the remote Hands Free device.  This function accepts as its    */
   /* input parameters the connection type indicating the local         */
   /* connection which will process the command and the speaker gain to */
   /* be sent to the remote device.  The speaker gain Parameter *MUST*  */
   /* be between the values:                                            */
   /*                                                                   */
   /*    HFRE_SPEAKER_GAIN_MINIMUM                                      */
   /*    HFRE_SPEAKER_GAIN_MAXIMUM                                      */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Set_Remote_Speaker_Gain(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Set_Remote_Speaker_Gain_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain);
#endif

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  This function may */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current microphone*/
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the microphone  */
   /* gain of the remote Hands Free device.  This function accepts as   */
   /* its input parameters the connection type indicating the local     */
   /* connection which will process the command and the microphone gain */
   /* to be sent to the remote device.  The microphone gain Parameter   */
   /* *MUST* be between the values:                                     */
   /*                                                                   */
   /*    HFRE_MICROPHONE_GAIN_MINIMUM                                   */
   /*    HFRE_MICROPHONE_GAIN_MAXIMUM                                   */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Set_Remote_Microphone_Gain(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Set_Remote_Microphone_Gain_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain);
#endif

   /* Send a codec identifier to a remote device. The audio gateway     */
   /* uses this function to select a codec from the list of codecs      */
   /* supported by the hands free device. The hands free device uses    */
   /* the function to confirm the audio gateway's selection. The first  */
   /* parameter is the Callback ID that is returned from                */
   /* HFRM_Register_Event_Callback(). The second parameter is the       */
   /* address of the remote device. The third parameter identifies the  */
   /* codec. This function returns zero if successful and a negative    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Select_Codec(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Select_Codec_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID);
#endif

   /* Hands Free Functions.                                             */

   /* The following function is responsible for querying the remote     */
   /* control indicator status.  This function may only be performed by */
   /* a local Hands Free unit with a valid service level connection to a*/
   /* connected remote Audio Gateway.  The results to this query will be*/
   /* returned as part of the control indicator status confirmation     */
   /* event (hetHFRControlIndicatorStatus).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Remote_Control_Indicator_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Remote_Control_Indicator_Status_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This function is responsible for enabling or disabling the        */
   /* indicator event notification on a remote Audio Gateway.  This     */
   /* function may only be performed by Hands Free devices that have a  */
   /* valid service level connection to a connected remote Audio        */
   /* Gateway.  When enabled, the remote Audio Gateway device will send */
   /* unsolicited responses to update the local device of the current   */
   /* control indicator values.  This function accepts as its input     */
   /* parameter a BOOLEAN flag used to enable or disable event          */
   /* notification.  This function returns zero if successful or a      */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Enable_Remote_Indicator_Event_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableEventNotification);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Enable_Remote_Indicator_Event_Notification_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableEventNotification);
#endif

   /* This function is responsible for querying the call holding and    */
   /* multi-party services which are supported by the remote Audio      */
   /* Gateway.  This function is used by Hands Free connections which   */
   /* support three way calling and call waiting to determine the       */
   /* features supported by the remote Audio Gateway.  This function can*/
   /* only be used if a valid service level connection to a connected   */
   /* remote Audio Gateway exists.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This function is responsible for allowing the control of multiple */
   /* concurrent calls and provides a means for holding calls, releasing*/
   /* calls, switching between two calls and adding a call to a         */
   /* multi-party conference.  This function may only be performed by   */
   /* Hands Free units that support call waiting and multi-party        */
   /* services as well as have a valid service level connection to a    */
   /* connected remote Audio Gateway.  The selection which is made      */
   /* should be one that is supported by the remote Audio Gateway       */
   /* (queried via a call to the                                        */
   /* HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support()       */
   /* function).  This function accepts as its input parameter the      */
   /* selection of how to handle the currently waiting call.  If the    */
   /* selected handling type requires an index it should be provided in */
   /* the last parameter.  Otherwise the final paramter is ignored.     */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Call_Holding_Multiparty_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Call_Holding_Multiparty_Selection_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index);
#endif

   /* This function is responsible for enabling or disabling call       */
   /* waiting notification on a remote Audio Gateway.  By default the   */
   /* call waiting notification is enabled in the network but disabled  */
   /* for notification via the service level connection (between Hands  */
   /* Free and Audio Gateway).  This function may only be performed by a*/
   /* Hands Free unit for which a valid service level connection to a   */
   /* connected remote Audio Gateway exists.  This function may only be */
   /* used to enable call waiting notifications if the local Hands Free */
   /* service supports call waiting and multi-party services.  This     */
   /* function accepts as its input parameter a BOOLEAN flag specifying */
   /* if this is a call to enable or disable this functionality.  This  */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Enable_Remote_Call_Waiting_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableNotification);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Enable_Remote_Call_Waiting_Notification_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress,Boolean_t EnableNotification);
#endif

   /* This function is responsible for enabling or disabling call line  */
   /* identification notification on a remote Audio Gateway.  By        */
   /* default, the call line identification notification via the service*/
   /* level connection is disabled.  This function may only be performed*/
   /* by Hands Free units for which a valid service level connection to */
   /* a connected remote Audio Gateway exists.  This function may only  */
   /* be used to enable call line notifications if the local Hands Free */
   /* unit supports call line identification.  This function accepts as */
   /* its input parameters a BOOLEAN flag specifying if this is a call  */
   /* to enable or disable this functionality.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Enable_Remote_Call_Line_Identification_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableNotification);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Enable_Remote_Call_Line_Identification_Notification_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableNotification);
#endif

   /* This function is responsible for dialing a phone number on a      */
   /* remote Audio Gateway.  This function may only be performed by     */
   /* Hands Free units for which a valid service level connection to a  */
   /* remote Audio Gateway exists.  This function accepts as its input  */
   /* parameter the phone number to dial on the remote Audio Gateway.   */
   /* This parameter should be a pointer to a NULL terminated string and*/
   /* its length *MUST* be between the values of:                       */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Dial_Phone_Number(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Dial_Phone_Number_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);
#endif

   /* This function is responsible for dialing a phone number from a    */
   /* memory location (index) found on the remote Audio Gateway.  This  */
   /* function may only be performed by Hands Free devices for which a  */
   /* valid service level connection to a connected remote Audio Gateway*/
   /* exists.  This function accepts as its input parameter the memory  */
   /* location (index) for which the phone number to dial already exists*/
   /* on the remote Audio Gateway.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Dial_Phone_Number_From_Memory(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MemoryLocation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Dial_Phone_Number_From_Memory_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MemoryLocation);
#endif

   /* This function is responsible for re-dialing the last number dialed*/
   /* on a remote Audio Gateway.  This function may only be performed by*/
   /* Hands Free devices for which a valid service level connection to a*/
   /* connected remote Audio Gateway exists.  This function returns zero*/
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Redial_Last_Phone_Number(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Redial_Last_Phone_Number_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This function is responsible for sending the command to a remote  */
   /* Audi Gateway to answer an incoming call.  This function may only  */
   /* be performed by Hands Free devices for which a valid service level*/
   /* connection to a connected remote Audio Gateway exists.  This      */
   /* function return zero if successful or a negative return error code*/
   /* if there was an error.                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Answer_Incoming_Call(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Answer_Incoming_Call_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This function is responsible for transmitting DTMF codes to a     */
   /* remote Audio Gateway to be sent as a DTMF code over an on-going   */
   /* call.  This function may only be performed by Hands Free devices  */
   /* for which a valid service level connection to a connected remote  */
   /* Audio Gateway exists and an on-going call exists.  This function  */
   /* accepts as input the DTMF code to be transmitted.  This Code must */
   /* be one of the characters:                                         */
   /*                                                                   */
   /*   0-9, *, #, or A-D.                                              */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Transmit_DTMF_Code(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char DTMFCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Transmit_DTMF_Code_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char DTMFCode);
#endif

   /* This function is responsible for retrieving a phone number to     */
   /* associate with a unique voice tag to be stored in memory by the   */
   /* local Hands Free device.  This function may only be performed by a*/
   /* Hands Free device for which a valid service level connection to a */
   /* connected remote Audio Gateway exists.  The Hands Free unit must  */
   /* also support voice recognition to be able to use this function.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * When this function is called no other function may be    */
   /*          called until a voice tag response is received from the   */
   /*          remote Audio Gateway.                                    */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Voice_Tag_Request(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Voice_Tag_Request_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This function is responsible for sending a hang-up command to a   */
   /* remote Audio Gateway.  This function may be used to reject an     */
   /* incoming call or to terminate an on-going call.  This function may*/
   /* only be performed by Hands Free devices for which a valid service */
   /* level connection exists.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Hang_Up_Call(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Hang_Up_Call_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for querying the current    */
   /* call list of the remote Audio Gateway device.  This function may  */
   /* only be performed by a Hands Free device with a valid service     */
   /* level connection to a connected Audio Gateway.  This function     */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Remote_Current_Calls_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Remote_Current_Calls_List_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for setting the network     */
   /* operator format to long alphanumeric.  This function may only be  */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected Audio Gateway.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Set_Network_Operator_Selection_Format(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Set_Network_Operator_Selection_Format_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for reading the network     */
   /* operator.  This function may only be performed by a Hands Free    */
   /* device with a valid service level connection.  This function      */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The network operator format must be set before querying  */
   /*          the current network operator.                            */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Remote_Network_Operator_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Remote_Network_Operator_Selection_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for enabling or disabling   */
   /* extended error results reporting.  This function may only be      */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* accepts as its input parameter a BOOLEAN flag indicating whether  */
   /* the reporting should be enabled (TRUE) or disabled (FALSE).  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Enable_Remote_Extended_Error_Result(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableExtendedErrorResults);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Enable_Remote_Extended_Error_Result_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableExtendedErrorResults);
#endif

   /* The following function is responsible for retrieving the          */
   /* subscriber number information.  This function may only be         */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Subscriber_Number_Information(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Subscriber_Number_Information_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for retrieving the current  */
   /* response and hold status.  This function may only be performed by */
   /* a Hands Free device with a valid service level connection to a    */
   /* connected Audio Gateway.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_Response_Hold_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_Response_Hold_Status_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for setting the state of an */
   /* incoming call.  This function may only be performed by a Hands    */
   /* Free unit with a valid service level connection to a remote Audio */
   /* Gateway.  This function accepts as its input parameter the call   */
   /* state to set as part of this message.  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Set_Incoming_Call_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Set_Incoming_Call_State_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState);
#endif

   /* The following function is responsible for sending an arbitrary    */
   /* command to the remote Audio Gateway (i.e.  non Bluetooth Hands    */
   /* Free Profile command).  This function may only be performed by a  */
   /* Hands Free with a valid service level connection.  This function  */
   /* accepts as its input parameter a NULL terminated ASCII string that*/
   /* represents the arbitrary command to send.  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The Command string passed to this function *MUST* begin  */
   /*          with AT and *MUST* end with the a carriage return ('\r') */
   /*          if this is the first portion of an arbitrary command     */
   /*          that will span multiple writes.  Subsequent calls (until */
   /*          the actual status reponse is received) can begin with    */
   /*          any character, however, they must end with a carriage    */
   /*          return ('\r').                                           */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Arbitrary_Command(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryCommand);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Arbitrary_Command_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryCommand);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* a local Hands Free service to send the available supported        */
   /* codecs to a remote Audio Gateway. The first parameter is          */
   /* the Callback ID that is returned from a successful call to        */
   /* HFRM_Register_Event_Callback().  The second parameter is the      */
   /* address of the remote device. The third parameter is the number of*/
   /* supported codecs being set. The fourth parameter is a list of the */
   /* CodecIDs supported. This function returns zero if successful and a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function may only be called by a Hands Free Unit.   */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Available_Codec_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Available_Codec_List_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList);
#endif

   /* Audio Gateway Functions.                                          */

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway with a valid service level connection to a       */
   /* connected remote Hands Free device.  This function accepts as its */
   /* input parameters the number of indicators and list of name/value  */
   /* pairs for the indicators to be updated.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Update_Current_Control_Indicator_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberUpdateIndicators, HFRE_Indicator_Update_t *UpdateIndicatorList);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Update_Current_Control_Indicator_Status_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberUpdateIndicators, HFRE_Indicator_Update_t *UpdateIndicatorsList);
#endif

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway.  The function will initially set the specified  */
   /* indicator, then, if a valid service level connection exists and   */
   /* event reporting is activated (via the set remote event indicator  */
   /* event notification function by the remote device) an event        */
   /* notification will be sent to the remote device.  This function    */
   /* accepts as its input parameters the name of the indicator to be   */
   /* updated and the new indicator value.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Update_Current_Control_Indicator_Status_By_Name(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *IndicatorName, unsigned int IndicatorValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Update_Current_Control_Indicator_Status_By_Name_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *IndicatorName, unsigned int IndicatorValue);
#endif

   /* This function is responsible for sending a call waiting           */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call waiting       */
   /* notification enabled and have a valid service level connection to */
   /* a connected remote Hands Free device.  This function accepts as   */
   /* its input parameter the phone number of the incoming call, if a   */
   /* number is available.  This parameter should be a pointer to a NULL*/
   /* terminated ASCII string (if specified) and must have a length less*/
   /* than:                                                             */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * It is valid to either pass a NULL for the PhoneNumber    */
   /*          parameter or a blank string to specify that there is no  */
   /*          phone number present.                                    */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Call_Waiting_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Call_Waiting_Notification_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);
#endif

   /* This function is responsible for sending call line identification */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call line          */
   /* identification notification enabled and have a valid service level*/
   /* connection to a connected remote Hands Free device.  This function*/
   /* accepts as its input parameters the phone number of the incoming  */
   /* call.  This parameter should be a pointer to a NULL terminated    */
   /* string and its length *MUST* be between the values of:            */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function return zero if successful or a negative return error*/
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Call_Line_Identification_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_CallLine_Identification_Notification_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);
#endif

   /* This function is responsible for sending a ring indication to a   */
   /* remote Hands Free unit.  This function may only be performed by   */
   /* Audio Gateways for which a valid service level connection to a    */
   /* connected remote Hands Free device exists.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Ring_Indication(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Ring_Indication_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This function is responsible for enabling or disabling in-band    */
   /* ring tone capabilities for a connected Hands Free device.  This   */
   /* function may only be performed by Audio Gateways for which a valid*/
   /* service kevel connection exists.  This function may only be used  */
   /* to enable in-band ring tone capabilities if the local Audio       */
   /* Gateway supports this feature.  This function accepts as its input*/
   /* parameter a BOOLEAN flag specifying if this is a call to Enable or*/
   /* Disable this functionality.  This function returns zero if        */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableInBandRing);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Enable_Remote_In_Band_Ring_Tone_Setting_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableInBandRing);
#endif

   /* This function is responsible for responding to a request that was */
   /* received for a phone number to be associated with a unique voice  */
   /* tag by a remote Hands Free device.  This function may only be     */
   /* performed by Audio Gateways that have received a voice tag request*/
   /* Indication.  This function accepts as its input parameter the     */
   /* phone number to be associated with the voice tag.  If the request */
   /* is accepted, the phone number Parameter string length *MUST* be   */
   /* between the values:                                               */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* If the caller wishes to reject the request, the phone number      */
   /* parameter should be set to NULL to indicate this.  This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Voice_Tag_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Voice_Tag_Response_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber);
#endif

   /* The following function is responsible for sending the current     */
   /* calls list entries to a remote Hands Free device.  This function  */
   /* may only be performed by Audio Gateways that have received a      */
   /* request to query the remote current calls list.  This function    */
   /* accepts as its input parameters the list of current call entries  */
   /* to be sent and length of the list.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Current_Calls_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRE_Current_Call_List_Entry_t *CurrentCallListEntryList);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Current_Calls_List_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRE_Current_Call_List_Entry_t *CurrentCallListEntryList);
#endif

   /* The following function is responsible for sending the network     */
   /* operator.  This function may only be performed by Audio Gateways  */
   /* that have received a request to query the remote network operator */
   /* selection.  This function accepts as input the current network    */
   /* mode and the current network operator.  The network operator      */
   /* should be expressed as a NULL terminated ASCII string (if         */
   /* specified) and must have a length less than:                      */
   /*                                                                   */
   /*    HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM                           */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * It is valid to either pass a NULL for the NetworkOperator*/
   /*          parameter or a blank string to specify that there is no  */
   /*          network operator present.                                */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Network_Operator_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NetworkMode, char *NetworkOperator);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Network_Operator_Selection_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NetworkMode, char *NetworkOperator);
#endif

   /* The following function is responsible for sending extended error  */
   /* results.  This function may only be performed by an Audio Gateway */
   /* with a valid service level connection.  This function accepts as  */
   /* its input parameter the result code to send as part of the error  */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Extended_Error_Result(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ResultCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Extended_Error_Result_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ResultCode);
#endif

   /* The following function is responsible for sending subscriber      */
   /* number information.  This function may only be performed by an    */
   /* Audio Gateway that has received a request to query the subscriber */
   /* number information.  This function accepts as its input parameters*/
   /* the number of subscribers followed by a list of subscriber        */
   /* numbers.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Subscriber_Number_Information(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRM_Subscriber_Number_Information_t *SubscriberNumberList);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Subscriber_Number_Information_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRM_Subscriber_Number_Information_t *SubscriberNumberList);
#endif

   /* The following function is responsible for sending information     */
   /* about the incoming call state.  This function may only be         */
   /* performed by an Audio Gateway that has a valid service level      */
   /* connection to a remote Hands Free device.  This function accepts  */
   /* as its input parameter the call state to set as part of this      */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Incoming_Call_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Incoming_Call_State_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState);
#endif

   /* The following function is responsible for sending a terminating   */
   /* response code from an Audio Gateway to a remote Hands Free device.*/
   /* This function may only be performed by an Audio Gateway that has a*/
   /* valid service level connection to a remote Hands Free device.     */
   /* This function can be called in any context where a normal Audio   */
   /* Gateway response function is called if the intention is to        */
   /* generate an error in response to the request.  It also must be    */
   /* called after certain requests that previously automatically       */
   /* generated an OK response.  In general, either this function or an */
   /* explicit response must be called after each request to the Audio  */
   /* Gateway.  This function accepts as its input parameters the type  */
   /* of result to return in the terminating response and, if the result*/
   /* type indicates an extended error code value, the error code.  This*/
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Terminating_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Extended_Result_t ResultType, unsigned int ResultValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Terminating_Response_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Extended_Result_t ResultType, unsigned int ResultValue);
#endif

   /* The following function is responsible for enabling the processing */
   /* of arbitrary commands from a remote Hands Free device.  Once this */
   /* function is called the hetHFRArbitraryCommandIndication event will*/
   /* be dispatched when an arbitrary command is received (i.e. a non   */
   /* Hands Free profile command).  If this function is not called, the */
   /* Audio Gateway will silently respond to any arbitrary commands with*/
   /* an error response ("ERROR").  If support is enabled, then the     */
   /* caller is responsible for responding TO ALL arbitrary command     */
   /* indications (hetHFRArbitraryCommandIndication).  If the arbitrary */
   /* command is not supported, then the caller should simply respond   */
   /* with:                                                             */
   /*                                                                   */
   /*   HFRM_Send_Terminating_Response()                                */
   /*                                                                   */
   /* specifying the erError response. This function returns zero if    */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * Once arbitrary command processing is enabled for an      */
   /*          Audio Gateway it cannot be disabled.                     */
   /* * NOTE * The default value is disabled (i.e. the                  */
   /*          hetHFRArbitraryCommandIndication will NEVER be dispatched*/
   /*          and the Audio Gateway will always respond with an error  */
   /*          response ("ERROR") when an arbitrary command is received.*/
   /* * NOTE * If support is enabled, the caller is guaranteed that a   */
   /*          hetHFRArbitraryCommandIndication will NOT be dispatched  */
   /*          before a service level indication is present. If an      */
   /*          arbitrary command is received, it will be responded with */
   /*          silently with an error response ("ERROR").               */
   /* * NOTE * This function is not applicable to Hands Free devices,   */
   /*          as Hands Free devices will always receive the            */
   /*          hetHFRArbitraryResponseIndication.  No action is required*/
   /*          and the event can simply be ignored.                     */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Enable_Arbitrary_Command_Processing(unsigned int HandsFreeManagerEventCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Enable_Arbitrary_Command_Processing_t)(unsigned int HandsFreeManagerEventCallbackID);
#endif

   /* The following function is responsible for sending an arbitrary    */
   /* response to the remote Hands Free device (i.e. non Bluetooth      */
   /* Hands Free Profile response) - either solicited or non-solicited. */
   /* This function may only be performed by an Audio Gateway with a    */
   /* valid service level connection. This function accepts as its      */
   /* input parameter a NULL terminated ASCII string that represents    */
   /* the arbitrary response to send. This function returns zero if     */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The Response string passed to this function *MUST* begin */
   /*          with a carriage return/line feed ("\r\n").               */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Arbitrary_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryResponse);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Arbitrary_Response_t)(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryResponse);
#endif

   /* Hands Free Manager Audio Connection Management Functions.         */

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Hands Free device for which a valid  */
   /* service level connection Exists.  This function accepts as its    */
   /* input parameter the connection type indicating which connection   */
   /* will process the command.  This function returns zero if          */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Setup_Audio_Connection(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Setup_Audio_Connection_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HFRM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Hands   */
   /* Free device.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Release_Audio_Connection(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Release_Audio_Connection_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Hands Free Manager Data Handler ID           */
   /* (registered via call to the HFRM_Register_Data_Event_Callback()   */
   /* function), followed by the the connection type indicating which   */
   /* connection will transmit the audio data, the length (in Bytes) of */
   /* the audio data to send, and a pointer to the audio data to send to*/
   /* the remote entity.  This function returns zero if successful or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function is only applicable for Bluetooth devices   */
   /*          that are configured to support packetized SCO audio.     */
   /*          This function will have no effect on Bluetooth devices   */
   /*          that are configured to process SCO audio via hardare     */
   /*          codec.                                                   */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to process the audio data themselves (as */
   /*          opposed to having the hardware process the audio data    */
   /*          via a hardware codec.                                    */
   /* * NOTE * The data that is sent *MUST* be formatted in the correct */
   /*          SCO format that is expected by the device.               */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Send_Audio_Data(unsigned int HandsFreeManagerDataEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Send_Audio_Data_t)(unsigned int HandsFreeManagerDataEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData);
#endif

   /* Hands Free Manager Event Callback Registration Functions.         */

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Hands Free       */
   /* Profile Manager Service.  This Callback will be dispatched by the */
   /* Hands Free Manager when various Hands Free Manager events occur.  */
   /* This function accepts the callback function and callback parameter*/
   /* (respectively) to call when a Hands Free Manager event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HFRM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Register_Event_Callback(HFRM_Connection_Type_t ConnectionType, Boolean_t ControlCallback, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Register_Event_Callback_t)(HFRM_Connection_Type_t ConnectionType, Boolean_t ControlCallback, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Hands Free Manager event      */
   /* Callback (registered via a successful call to the                 */
   /* HFRM_Register_Event_Callback() function.  This function accepts as*/
   /* input the Hands Free Manager event callback ID (return value from */
   /* the HFRM_Register_Event_Callback() function).                     */
BTPSAPI_DECLARATION void BTPSAPI HFRM_Un_Register_Event_Callback(unsigned int HandsFreeManagerEventCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HFRM_Un_Register_Event_Callback_t)(unsigned int HandsFreeManagerEventCallbackID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Hands Free*/
   /* Profile Manager service to explicitly process SCO audio data.     */
   /* This callback will be dispatched by the Hands Free Manager when   */
   /* various Hands Free Manager events occur.  This function accepts   */
   /* the connection type which indicates the connection type the data  */
   /* registration callback to register for, and the callback function  */
   /* and callback parameter (respectively) to call when a Hands Free   */
   /* Manager event needs to be dispatched.  This function returns a    */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HFRM_Send_Audio_Data() function to send SCO audio data.  */
   /* * NOTE * There can only be a single data event handler registered */
   /*          for each type of Hands Free Manager connection type.     */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HFRM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Register_Data_Event_Callback(HFRM_Connection_Type_t ConnectionType, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Register_Data_Event_Callback_t)(HFRM_Connection_Type_t ConnectionType, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Hands Free Manager data event */
   /* callback (registered via a successful call to the                 */
   /* HFRM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Hands Free Manager data event callback ID    */
   /* (return value from HFRM_Register_Data_Event_Callback() function). */
BTPSAPI_DECLARATION void BTPSAPI HFRM_Un_Register_Data_Event_Callback(unsigned int HandsFreeManagerDataCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HFRM_Un_Register_Data_Event_Callback_t)(unsigned int HandsFreeManagerDataCallbackID);
#endif

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Callback ID that is returned from a        */
   /* successful call to HFRM_Register_Event_Callback().  The second    */
   /* parameter is the local connection type of the SCO connection.  The*/
   /* third parameter is the address of the remote device of the SCO    */
   /* connection.  The fourth parameter is a pointer to the location to */
   /* store the SCO Handle. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI HFRM_Query_SCO_Connection_Handle(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HFRM_Query_SCO_Connection_Handle_t)(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle);
#endif

#endif
