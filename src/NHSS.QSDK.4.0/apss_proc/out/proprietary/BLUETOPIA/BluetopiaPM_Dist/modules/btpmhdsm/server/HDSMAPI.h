/*****< hdsmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDSMAPI - Local Headset Profile API for Stonestreet One Bluetooth         */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/17/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HDSMAPIH__
#define __HDSMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HDSMMSG.h"             /* BTPM Headset Profile Message Formats.     */

   /* The following structure holds the information needed to initialize*/
   /* the Audio Gateway and Headset server components of the Headset.   */
   /* Manager.  The ServerPort member should specify a valid RFCOMM port*/
   /* number on which to host the server.                               */
typedef struct _tagHDSM_Initialization_Data_t
{
   char          *ServiceName;
   unsigned int   ServerPort;
   unsigned long  IncomingConnectionFlags;
   unsigned long  SupportedFeaturesMask;
   unsigned int   MaximumNumberServers;
} HDSM_Initialization_Data_t;

#define HDSM_INITIALIZATION_DATA_SIZE                          (sizeof(HDSM_Initialization_Data_t))

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Headset Profile Module is     */
   /* initialized.                                                      */
typedef struct _tagHDSM_Initialization_Info_t
{
   HDSM_Initialization_Data_t *AudioGatewayInitializationInfo;
   HDSM_Initialization_Data_t *HeadsetInitializationInfo;
} HDSM_Initialization_Info_t;

#define HDSM_INITIALIZATION_INFO_SIZE                          (sizeof(HDSM_Initialization_Info_t))

   /* The following enumerated type represents the Headset Manager Event*/
   /* Types that are dispatched by this module.                         */
typedef enum
{
   /* Common Headset/Audio Gateway events.                              */
   hetHDSIncomingConnectionRequest,
   hetHDSConnected,
   hetHDSDisconnected,
   hetHDSConnectionStatus,
   hetHDSAudioConnected,
   hetHDSAudioDisconnected,
   hetHDSAudioConnectionStatus,
   hetHDSAudioData,
   hetHDSSpeakerGainIndication,
   hetHDSMicrophoneGainIndication,

   /* Headset specific events.                                          */
   hetHDSRingIndication,

   /* Audio Gateway specific events.                                    */
   hetHDSButtonPressedIndication,
} HDSM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHDSIncomingConnectionRequest */
   /* event.                                                            */
   /* * NOTE * The ConnectionType member specifies the type of of       */
   /*          connection of the local server (not the remote device    */
   /*          type).                                                   */
typedef struct _tagHDSM_Incoming_Connection_Request_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Incoming_Connection_Request_Event_Data_t;

#define HDSM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE       (sizeof(HDSM_Incoming_Connection_Request_Event_Data_t))

   /* The following event is dispatched when a Headset connection       */
   /* occurs.  The ConnectionType member specifies which local Headset  */
   /* role type has been connected to and the RemoteDeviceAddress member*/
   /* specifies the remote Bluetooth device that has connected to the   */
   /* specified Headset Role.                                           */
typedef struct _tagHDSM_Connected_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Connected_Event_Data_t;

#define HDSM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(HDSM_Connected_Event_Data_t))

   /* The following event is dispatched when a remote device disconnects*/
   /* from the local device (for the specified Headset role).  The      */
   /* ConnectionType member identifies the local Headset role type being*/
   /* disconnected and the RemoteDeviceAddress member specifies the     */
   /* Bluetooth device address of the device that disconnected from the */
   /* profile.                                                          */
typedef struct _tagHDSM_Disconnected_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           DisconnectReason;
} HDSM_Disconnected_Event_Data_t;

#define HDSM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(HDSM_Disconnected_Event_Data_t))

   /* The following event is dispatched when a client receives the      */
   /* connection response from a remote server which was previously     */
   /* attempted to be connected to.  The ConnectionType member specifies*/
   /* the local client that has requested the connection, the           */
   /* RemoteDeviceAddress member specifies the remote device that was   */
   /* attempted to be connected to, and the ConnectionStatus member     */
   /* represents the connection status of the request.                  */
typedef struct _tagHDSM_Connection_Status_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ConnectionStatus;
} HDSM_Connection_Status_Event_Data_t;

#define HDSM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(HDSM_Connection_Status_Event_Data_t))

   /* The following event is dispatched to the local device when an     */
   /* audio connection is established.  The ConnectionType member       */
   /* identifies the connection that is receiving this indication.  The */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the remote device that the audio connection is established     */
   /* with.                                                             */
typedef struct _tagHDSM_Audio_Connected_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Audio_Connected_Event_Data_t;

#define HDSM_AUDIO_CONNECTED_EVENT_DATA_SIZE                   (sizeof(HDSM_Audio_Connected_Event_Data_t))

   /* The following event is dispatched to the local device when an     */
   /* audio connection is disconnected.  The ConnectionType member      */
   /* identifies the connection that is receiving this indication.  The */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the remote device that the audio connection is no longer       */
   /* established with.                                                 */
typedef struct _tagHDSM_Audio_Disconnected_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} HDSM_Audio_Disconnected_Event_Data_t;

#define HDSM_AUDIO_DISCONNECTED_EVENT_DATA_SIZE                (sizeof(HDSM_Audio_Disconnected_Event_Data_t))

   /* The following event is dispatched when the originator of the audio*/
   /* connection (Audio Gateway only) receives the audio connection     */
   /* response from a remote device from which an audio connection      */
   /* request was previously sent.  The ConnectionType member identifies*/
   /* the connection that was attempting the audio connection.  The     */
   /* RemoteDeviceAddress member specifies the remote device address of */
   /* the remote device that the audio connection was requested.  The   */
   /* Status member specifies the result of the audio connection event. */
typedef struct _tagHDSM_Audio_Connection_Status_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              Successful;
} HDSM_Audio_Connection_Status_Event_Data_t;

#define HDSM_AUDIO_CONNECTION_STATUS_EVENT_DATA_SIZE           (sizeof(HDSM_Audio_Connection_Status_Event_Data_t))

   /* The following event is dispatched to the local device upon the    */
   /* reception of SCO audio data.  The ConnectionType member identifies*/
   /* the connection that has received the audio data.  The             */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the Bluetooth device that the specified audio data was received*/
   /* from.  The AudioDataLength member represents the size of the audio*/
   /* data pointed to the buffer that is specified by the AudioData     */
   /* member.                                                           */
typedef struct _tagHDSM_Audio_Data_Event_Data_t
{
   HDSM_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            DataEventsHandlerID;
   unsigned long           AudioDataFlags;
   unsigned int            AudioDataLength;
   unsigned char          *AudioData;
} HDSM_Audio_Data_Event_Data_t;

#define HDSM_AUDIO_DATA_EVENT_DATA_SIZE                        (sizeof(HDSM_Audio_Data_Event_Data_t))

   /* The following event may be dispatched to either a local Headset   */
   /* unit or a local Audio Gateway.  When this event is received by a  */
   /* local Headset unit it is to set the local speaker gain.  When this*/
   /* event is received by a local audio gateway it is used in volume   */
   /* level synchronization to inform the local Audio Gateway of the    */
   /* current speaker gain on the remote Headset unit.  The             */
   /* ConnectionType member identifies the local connection that has    */
   /* received this indication.  The RemoteDeviceAddress member         */
   /* specifies the remote Bluetooth device address of the remote       */
   /* Bluetooth device that is informing the local device of the new    */
   /* speaker gain.  The SpeakerGain member is used to set or inform the*/
   /* device of the speaker gain.                                       */
typedef struct _tagHDSM_Speaker_Gain_Indication_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SpeakerGain;
} HDSM_Speaker_Gain_Indication_Event_Data_t;

#define HDSM_SPEAKER_GAIN_INDICATION_EVENT_DATA_SIZE           (sizeof(HDSM_Speaker_Gain_Indication_Event_Data_t))

   /* The following event may be dispatched to either a local Headset   */
   /* unit or a local Audio Gateway.  When this event is received by a  */
   /* local Headset unit it is to set the local microphone gain.  When  */
   /* this event is received by a local audio gateway it is used in     */
   /* volume level synchronization to inform the local Audio Gateway of */
   /* the current microphone gain on the remote Headset unit.  The      */
   /* ConnectionType member identifies the local connection that has    */
   /* received this indication.  The RemoteDeviceAddress member         */
   /* specifies the remote Bluetooth device address of the remote       */
   /* Bluetooth device that is informing the local device of the new    */
   /* microphone gain.  The MicrophoneGain member is used to set or     */
   /* inform the device of the microphone gain.                         */
typedef struct _tagHDSM_Microphone_Gain_Indication_Event_Data_t
{
   HDSM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           MicrophoneGain;
} HDSM_Microphone_Gain_Indication_Event_Data_t;

#define HDSM_MICROPHONE_GAIN_INDICATION_EVENT_DATA_SIZE        (sizeof(HDSM_Microphone_Gain_Indication_Event_Data_t))

   /* Headset specific events.                                          */

   /* The following event is dispatched to a local Headset unit when the*/
   /* remote Audio Gateway sends a RING indication to the local device. */
   /* The RemoteDeviceAddress member specifies the local connection     */
   /* which is receiving this indication.                               */
typedef struct _tagHDSM_Ring_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HDSM_Ring_Indication_Event_Data_t;

#define HDSM_RING_INDICATION_EVENT_DATA_SIZE                   (sizeof(HDSM_Ring_Indication_Event_Data_t))

   /* Audio Gateway specific events.                                    */

   /* The following event is dispatched to a local Audio Gateway when   */
   /* the remote Headset device issues the command to answer an incoming*/
   /* call.  The RemoteDeviceAddress member identifies the local        */
   /* connection receiving this indication.                             */
typedef struct _tagHDSM_Button_Pressed_Indication_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HDSM_Button_Pressed_Indication_Event_Data_t;

#define HDSM_BUTTON_PRESSED_INDICATION_EVENT_DATA_SIZE         (sizeof(HDSM_Button_Pressed_Indication_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Headset Manager Event (and Event Data) of a Headset Manager Event.*/
typedef struct _tagHDSM_Event_Data_t
{
   HDSM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      HDSM_Incoming_Connection_Request_Event_Data_t IncomingConnectionRequestEventData;
      HDSM_Connected_Event_Data_t                   ConnectedEventData;
      HDSM_Disconnected_Event_Data_t                DisconnectedEventData;
      HDSM_Connection_Status_Event_Data_t           ConnectionStatusEventData;
      HDSM_Audio_Connected_Event_Data_t             AudioConnectedEventData;
      HDSM_Audio_Disconnected_Event_Data_t          AudioDisconnectedEventData;
      HDSM_Audio_Connection_Status_Event_Data_t     AudioConnectionStatusEventData;
      HDSM_Audio_Data_Event_Data_t                  AudioDataEventData;
      HDSM_Speaker_Gain_Indication_Event_Data_t     SpeakerGainIndicationEventData;
      HDSM_Microphone_Gain_Indication_Event_Data_t  MicrophoneGainIndicationEventData;

      /* Headset specific events.                                       */
      HDSM_Ring_Indication_Event_Data_t             RingIndicationEventData;

      /* Audio Gateway specific events.                                 */
      HDSM_Button_Pressed_Indication_Event_Data_t   ButtonPressIndicationEventData;
   } EventData;
} HDSM_Event_Data_t;

#define HDSM_EVENT_DATA_SIZE                                   (sizeof(HDSM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Headset Manager dispatches an event (and the client has registered*/
   /* for events).  This function passes to the caller the Headset      */
   /* Manager Event and the Callback Parameter that was specified when  */
   /* this Callback was installed.  The caller is free to use the       */
   /* contents of the Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Message will not be   */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT block and wait for events that can*/
   /*          only be satisfied by Receiving other Events.  A deadlock */
   /*          WILL occur because NO Event Callbacks will be issued     */
   /*          while this function is currently outstanding.            */
typedef void (BTPSAPI *HDSM_Event_Callback_t)(HDSM_Event_Data_t *EventData, void *CallbackParameter);

   /* Headset/Audio Gateway Module Installation/Support Functions.      */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Headset Manager module.  This      */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI HDSM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDSM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* Headset Manager Connection Management Functions.                  */

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened.  A  */
   /*          hetHDSConnected event will notify if the connection is   */
   /*          successful.                                              */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Connection_Request_Response(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Connection_Request_Response_t)(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Headset/Audio Gateway device.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function accepts the connection */
   /* type to make as the first parameter.  This parameter specifies the*/
   /* LOCAL connection type (i.e.  if the caller would like to connect  */
   /* the local Headset service to a remote Audio Gateway device, the   */
   /* Headset connection type would be specified for this parameter).   */
   /* This function also accepts the connection information for the     */
   /* remote device (address and server port).  This function accepts   */
   /* the connection flags to apply to control how the connection is    */
   /* made regarding encryption and/or authentication.                  */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Headset Manager Connection Status Event (if specified).  */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHDSConnectionStatus event will be dispatched to denote*/
   /*          the status of the connection.  This is the ONLY way to   */
   /*          receive this event, as an event callack registered with  */
   /*          the HDSM_Register_Event_Callback() will NOT receive      */
   /*          connection status events.                                */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Connect_Remote_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Connect_Remote_Device_t)(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function exists to close an active Headset or Audio */
   /* Gateway connection that was previously opened by any of the       */
   /* following mechanisms:                                             */
   /*   - Successful call to HDSM_Connect_Remote_Device() function.     */
   /*   - Incoming open request (Headset or Audio Gateway) which was    */
   /*     accepted either automatically or by a call to                 */
   /*     HDSM_Connection_Request_Response().                           */
   /* This function accepts as input the type of the local connection   */
   /* which should close its active connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.  This function does NOT un-register any Headset or Audio   */
   /* Gateway services from the system, it ONLY disconnects any         */
   /* connection that is currently active on the specified service.     */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Disconnect_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Disconnect_Device_t)(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Headset */
   /* or Audio Gateway Devices (specified by the first parameter).  This*/
   /* function accepts a the local service type to query, followed by   */
   /* buffer information to receive any currently connected device      */
   /* addresses of the specified connection type.  The first parameter  */
   /* specifies the local service type to query the connection          */
   /* information for.  The second parameter specifies the maximum      */
   /* number of BD_ADDR entries that the buffer will support (i.e. can  */
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
BTPSAPI_DECLARATION int BTPSAPI HDSM_Query_Connected_Devices(HDSM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Query_Connected_Devices_t)(HDSM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Headset or Audio   */
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Query_Current_Configuration(HDSM_Connection_Type_t ConnectionType, HDSM_Current_Configuration_t *CurrentConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Query_Current_Configuration_t)(HDSM_Connection_Type_t ConnectionType, HDSM_Current_Configuration_t *CurrentConfiguration);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Headset and   */
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI HDSM_Change_Incoming_Connection_Flags(HDSM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Change_Incoming_Connection_Flags_t)(HDSM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags);
#endif

   /* Shared Headset/Audio Gateway Functions.                           */

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  When called by a     */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current speaker gain value.  When     */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the speaker gain of the remote Headset   */
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the speaker gain to be sent to the remote device. */
   /* The speaker gain Parameter *MUST* be between the values:          */
   /*                                                                   */
   /*    HDSET_SPEAKER_GAIN_MINIMUM                                     */
   /*    HDSET_SPEAKER_GAIN_MAXIMUM                                     */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Set_Remote_Speaker_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Set_Remote_Speaker_Gain_t)(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain);
#endif

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  When called by a  */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current microphone gain value.  When  */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the microphone gain of the remote Headset*/
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the microphone gain to be sent to the remote      */
   /* device.  The microphone gain Parameter *MUST* be between the      */
   /* values:                                                           */
   /*                                                                   */
   /*    HDSET_MICROPHONE_GAIN_MINIMUM                                  */
   /*    HDSET_MICROPHONE_GAIN_MAXIMUM                                  */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Set_Remote_Microphone_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Set_Remote_Microphone_Gain_t)(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain);
#endif

   /* Headset Functions.                                                */

   /* This function is responsible for sending the command to a remote  */
   /* Audi Gateway to answer an incoming call.  This function may only  */
   /* be performed by Headset devices.  This function return zero if    */
   /* successful or a negative return error code if there was an error. */

   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Send_Button_Press(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Send_Button_Press_t)(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* Audio Gateway Functions.                                          */

   /* This function is responsible for sending a ring indication to a   */
   /* remote Headset unit.  This function may only be performed by Audio*/
   /* Gateways.  This function returns zero if successful or a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Ring_Indication(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Ring_Indication_t)(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* Headset Manager Audio Connection Management Functions.            */

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Headset devices.  This function      */
   /* accepts as its input parameter the connection type indicating     */
   /* which connection will process the command.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
   /* * NOTE * The InBandRinging parameter specifies if the purpose of  */
   /*          setting up the Audio Connection is for In-Band Ring      */
   /*          (TRUE) or not (FALSE).                                   */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Setup_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t InBandRinging);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Setup_Audio_Connection_t)(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t InBandRinging);
#endif

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HDSM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Headset */
   /* device.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Release_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Release_Audio_Connection_t)(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Headset Manager Data Handler ID (registered  */
   /* via call to the HDSM_Register_Data_Event_Callback() function),    */
   /* followed by the the connection type indicating which connection   */
   /* will transmit the audio data, the length (in Bytes) of the audio  */
   /* data to send, and a pointer to the audio data to send to the      */
   /* remote entity.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function is only applicable for Bluetooth devices   */
   /*          that are configured to support packetized SCO audio.     */
   /*          This function will have no effect on Bluetooth devices   */
   /*          that are configured to process SCO audio via hardare     */
   /*          codec.                                                   */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to process the audio data themselves (as */
   /*          opposed to having the hardware process the audio data via*/
   /*          a hardware codec.                                        */
   /* * NOTE * The data that is sent *MUST* be formatted in the correct */
   /*          SCO format that is expected by the device.               */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Send_Audio_Data(unsigned int HeadsetManagerDataEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Send_Audio_Data_t)(unsigned int HeadsetManagerDataEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData);
#endif

   /* Headset Manager Event Callback Registration Functions.            */

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Headset Profile  */
   /* Manager Service.  This Callback will be dispatched by the Headset */
   /* Manager when various Headset Manager events occur.  This function */
   /* accepts the callback function and callback parameter              */
   /* (respectively) to call when a Headset Manager event needs to be   */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDSM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Register_Event_Callback(HDSM_Connection_Type_t ConnectionType, Boolean_t ControlCallback, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Register_Event_Callback_t)(HDSM_Connection_Type_t ConnectionType, Boolean_t ControlCallback, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager event Callback*/
   /* (registered via a successful call to the                          */
   /* HDSM_Register_Event_Callback() function.  This function accepts as*/
   /* input the Headset Manager event callback ID (return value from the*/
   /* HDSM_Register_Event_Callback() function).                         */
BTPSAPI_DECLARATION void BTPSAPI HDSM_Un_Register_Event_Callback(unsigned int HeadsetManagerEventCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HDSM_Un_Register_Event_Callback_t)(unsigned int HeadsetManagerEventCallbackID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Headset   */
   /* Profile Manager service to explicitly process SCO audio data.     */
   /* This callback will be dispatched by the Headset Manager when      */
   /* various Headset Manager events occur.  This function accepts the  */
   /* connection type which indicates the connection type the data      */
   /* registration callback to register for, and the callback function  */
   /* and callback parameter (respectively) to call when a Headset      */
   /* Manager event needs to be dispatched.  This function returns a    */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HDSM_Send_Audio_Data() function to send SCO audio data.  */
   /* * NOTE * There can only be a single data event handler registered */
   /*          for each type of Headset Manager connection type.        */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDSM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Register_Data_Event_Callback(HDSM_Connection_Type_t ConnectionType, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Register_Data_Event_Callback_t)(HDSM_Connection_Type_t ConnectionType, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager data event    */
   /* callback (registered via a successful call to the                 */
   /* HDSM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Headset Manager data event callback ID       */
   /* (return value from HDSM_Register_Data_Event_Callback() function). */
BTPSAPI_DECLARATION void BTPSAPI HDSM_Un_Register_Data_Event_Callback(unsigned int HeadsetManagerDataCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HDSM_Un_Register_Data_Event_Callback_t)(unsigned int HeadsetManagerDataCallbackID);
#endif

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Callback ID that is returned from a        */
   /* successful call to HDSM_Register_Event_Callback().  The second    */
   /* parameter is the local connection type of the SCO connection.  The*/
   /* third parameter is the address of the remote device of the SCO    */
   /* connection.  The fourth parameter is a pointer to the location to */
   /* store the SCO Handle. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Query_SCO_Connection_Handle(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Query_SCO_Connection_Handle_t)(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* query the HDSET SDP information from a remote device. The         */
   /* RemoteDeviceAddress parameter is the BD_ADDR of the remote        */
   /* device. The ServiceInformation parameter is a pointer to a        */
   /* structure for storing the service information. This function      */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI HDSM_Query_Remote_Device_Service_Information(BD_ADDR_t RemoteDeviceAddress, HDSM_Service_Information_t *ServiceInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDSM_Query_Remote_Device_Service_Information_t)(BD_ADDR_t RemoteDeviceAddress, HDSM_Service_Information_t *ServiceInformation);
#endif
   
#endif
