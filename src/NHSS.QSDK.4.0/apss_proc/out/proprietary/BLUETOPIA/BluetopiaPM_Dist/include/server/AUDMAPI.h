/*****< audmapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDMAPI - Local Audio Manager API for Stonestreet One Bluetooth Protocol  */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __AUDMAPIH__
#define __AUDMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "AUDMMSG.h"             /* BTPM Audio Manager Message Formats.       */

#include "SS1BTAUD.h"            /* Audio Framework Prototypes/Constants.     */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Audio Module is initialized.  */
   /* * NOTE * This structure is used with the                          */
   /*          AUDM_InitializationHandlerFunction() function to denote  */
   /*          the initialization parameters required for the module.   */
typedef struct _tagAUDM_Initialization_Data_t
{
   unsigned long                             IncomingConnectionFlags;
   unsigned long                             InitializationFlags;
   AUD_Stream_Initialization_Info_t         *SRCInitializationInfo;
   AUD_Stream_Initialization_Info_t         *SNKInitializationInfo;
   AUD_Remote_Control_Initialization_Info_t *RemoteControlInitializationInfo;
} AUDM_Initialization_Data_t;

#define AUDM_INITIALIZATION_DATA_SIZE                          (sizeof(AUDM_Initialization_Data_t))

typedef struct _tagAUDM_Remote_Control_Role_Info_t
{
   AVRCP_Version_t Version;
   Word_t          SupportedFeaturesFlags;
} AUDM_Remote_Control_Role_Info_t;

typedef struct _tagAUDM_Remote_Control_Services_Info_t
{
   unsigned long                   ServiceFlags;
   AUDM_Remote_Control_Role_Info_t ControllerInfo;
   AUDM_Remote_Control_Role_Info_t TargetInfo;
} AUDM_Remote_Control_Services_Info_t;

#define AUDM_REMOTE_CONTROL_SERVICES_FLAGS_CONTROLLER_ROLE_SUPPORTED 0x00000001
#define AUDM_REMOTE_CONTROL_SERVICES_FLAGS_TARGET_ROLE_SUPPORTED     0x00000002

   /* The following enumerated type represents the Audio Manager Event  */
   /* Types that are dispatched by this module.                         */
typedef enum
{
   aetIncomingConnectionRequest,
   aetAudioStreamConnected,
   aetAudioStreamConnectionStatus,
   aetAudioStreamDisconnected,
   aetAudioStreamStateChanged,
   aetChangeAudioStreamStateStatus,
   aetAudioStreamFormatChanged,
   aetChangeAudioStreamFormatStatus,
   aetEncodedAudioStreamData,
   aetRemoteControlConnected,
   aetRemoteControlConnectionStatus,
   aetRemoteControlDisconnected,
   aetRemoteControlCommandIndication,
   aetRemoteControlCommandConfirmation,
   aetRemoteControlBrowsingConnected,
   aetRemoteControlBrowsingConnectionStatus,
   aetRemoteControlBrowsingDisconnected
} AUDM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetIncomingConnectionRequest    */
   /* event.                                                            */
typedef struct _tagAUDM_Incoming_Connection_Request_Event_Data_t
{
   BD_ADDR_t                     RemoteDeviceAddress;
   AUD_Connection_Request_Type_t RequestType;
} AUDM_Incoming_Connection_Request_Event_Data_t;

#define AUDM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE       (sizeof(AUDM_Incoming_Connection_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetAudioStreamConnected event.  */
typedef struct _tagAUDM_Audio_Stream_Connected_Event_Data_t
{
   AUD_Stream_Type_t   StreamType;
   BD_ADDR_t           RemoteDeviceAddress;
   unsigned int        MediaMTU;
   AUD_Stream_Format_t StreamFormat;
} AUDM_Audio_Stream_Connected_Event_Data_t;

#define AUDM_AUDIO_STREAM_CONNECTED_EVENT_DATA_SIZE            (sizeof(AUDM_Audio_Stream_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetAudioStreamConnectionStatus  */
   /* event.                                                            */
typedef struct _tagAUDM_Audio_Stream_Connection_Status_Event_Data_t
{
   BD_ADDR_t           RemoteDeviceAddress;
   unsigned int        ConnectionStatus;
   unsigned int        MediaMTU;
   AUD_Stream_Type_t   StreamType;
   AUD_Stream_Format_t StreamFormat;
} AUDM_Audio_Stream_Connection_Status_Event_Data_t;

#define AUDM_AUDIO_STREAM_CONNECTION_STATUS_EVENT_DATA_SIZE    (sizeof(AUDM_Audio_Stream_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetAudioStreamDisconnected      */
   /* event.                                                            */
typedef struct _tagAUDM_Audio_Stream_Disconnected_Event_Data_t
{
   BD_ADDR_t         RemoteDeviceAddress;
   AUD_Stream_Type_t StreamType;
} AUDM_Audio_Stream_Disconnected_Event_Data_t;

#define AUDM_AUDIO_STREAM_DISCONNECTED_EVENT_DATA_SIZE         (sizeof(AUDM_Audio_Stream_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetAudioStreamStateChanged      */
   /* event.                                                            */
typedef struct _tagAUDM_Audio_Stream_State_Changed_Event_Data_t
{
   BD_ADDR_t          RemoteDeviceAddress;
   AUD_Stream_Type_t  StreamType;
   AUD_Stream_State_t StreamState;
} AUDM_Audio_Stream_State_Changed_Event_Data_t;

#define AUDM_AUDIO_STREAM_STATE_CHANGED_EVENT_DATA_SIZE        (sizeof(AUDM_Audio_Stream_State_Changed_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetChangeAudioStreamStateStatus */
   /* event.                                                            */
typedef struct _tagAUDM_Change_Audio_Stream_State_Status_Event_Data_t
{
   BD_ADDR_t          RemoteDeviceAddress;
   Boolean_t          Successful;
   AUD_Stream_Type_t  StreamType;
   AUD_Stream_State_t StreamState;
} AUDM_Change_Audio_Stream_State_Status_Event_Data_t;

#define AUDM_CHANGE_AUDIO_STREAM_STATE_STATUS_EVENT_DATA_SIZE  (sizeof(AUDM_Change_Audio_Stream_State_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetAudioStreamFormatChanged     */
   /* event.                                                            */
typedef struct _tagAUDM_Audio_Stream_Format_Changed_Event_Data_t
{
   BD_ADDR_t           RemoteDeviceAddress;
   AUD_Stream_Type_t   StreamType;
   AUD_Stream_Format_t StreamFormat;
} AUDM_Audio_Stream_Format_Changed_Event_Data_t;

#define AUDM_AUDIO_STREAM_FORMAT_CHANGED_EVENT_DATA_SIZE       (sizeof(AUDM_Audio_Stream_Format_Changed_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetChangeAudioStreamFormatStatus*/
   /* event.                                                            */
typedef struct _tagAUDM_Change_Audio_Stream_Format_Status_Event_Data_t
{
   BD_ADDR_t           RemoteDeviceAddress;
   Boolean_t           Successful;
   AUD_Stream_Type_t   StreamType;
   AUD_Stream_Format_t StreamFormat;
} AUDM_Change_Audio_Stream_Format_Status_Event_Data_t;

#define AUDM_CHANGE_AUDIO_STREAM_FORMAT_STATUS_EVENT_DATA_SIZE (sizeof(AUDM_Change_Audio_Stream_Format_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetEncodedAudioStreamData event.*/
typedef struct _tagAUDM_Encoded_Audio_Stream_Data_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           StreamDataEventsHandlerID;
   unsigned int           RawAudioDataFrameLength;
   unsigned char         *RawAudioDataFrame;
   AUD_RTP_Header_Info_t *RTPHeaderInfo;
} AUDM_Encoded_Audio_Stream_Data_Event_Data_t;

#define AUDM_ENCODED_AUDIO_STREAM_DATA_EVENT_DATA_SIZE         (sizeof(AUDM_Encoded_Audio_Stream_Data_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetRemoteControlConnected event.*/
typedef struct _tagAUDM_Remote_Control_Connected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} AUDM_Remote_Control_Connected_Event_Data_t;

#define AUDM_REMOTE_CONTROL_CONNECTED_EVENT_DATA_SIZE          (sizeof(AUDM_Remote_Control_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetRemoteControlConnectionStatus*/
   /* event.                                                            */
typedef struct _tagAUDM_Remote_Control_Connection_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ConnectionStatus;
} AUDM_Remote_Control_Connection_Status_Event_Data_t;

#define AUDM_REMOTE_CONTROL_CONNECTION_STATUS_EVENT_DATA_SIZE  (sizeof(AUDM_Remote_Control_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetRemoteControlDisconnected    */
   /* event.                                                            */
typedef struct _tagAUDM_Remote_Control_Disconnected_Event_Data_t
{
   BD_ADDR_t               RemoteDeviceAddress;
   AUD_Disconnect_Reason_t DisconnectReason;
} AUDM_Remote_Control_Disconnected_Event_Data_t;

#define AUDM_REMOTE_CONTROL_DISCONNECTED_EVENT_DATA_SIZE          (sizeof(AUDM_Remote_Control_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* aetRemoteControlCommandIndication event.                          */
typedef struct _tagAUDM_Remote_Control_Command_Indication_Event_Data_t
{
   BD_ADDR_t                         RemoteDeviceAddress;
   unsigned int                      TransactionID;
   AUD_Remote_Control_Command_Data_t RemoteControlCommandData;
} AUDM_Remote_Control_Command_Indication_Event_Data_t;

#define AUDM_REMOTE_CONTROL_COMMAND_INDICATION_EVENT_DATA_SIZE (sizeof(AUDM_Remote_Control_Command_Indication_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* aetRemoteControlCommandConfirmation event.                        */
typedef struct _tagAUDM_Remote_Control_Command_Confirmation_Event_Data_t
{
   BD_ADDR_t                          RemoteDeviceAddress;
   unsigned int                       TransactionID;
   int                                Status;
   AUD_Remote_Control_Response_Data_t RemoteControlResponseData;
} AUDM_Remote_Control_Command_Confirmation_Event_Data_t;

#define AUDM_REMOTE_CONTROL_COMMAND_CONFIRMATION_EVENT_DATA_SIZE (sizeof(AUDM_Remote_Control_Command_Confirmation_Event_Data_t))

   /* The following structure is a container structure                  */
   /* that holds the information that is returned in a                  */
   /* aetRemoteControlBrowsingConnected event.                          */
typedef struct _tagAUDM_Remote_Control_Browsing_Connected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} AUDM_Remote_Control_Browsing_Connected_Event_Data_t;

#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_EVENT_DATA_SIZE          (sizeof(AUDM_Remote_Control_Browsing_Connected_Event_Data_t))

   /* The following structure is a container structure                  */
   /* that holds the information that is returned in a                  */
   /* aetRemoteControlBrowsingConnectionStatus event.                   */
typedef struct _tagAUDM_Remote_Control_Browsing_Connection_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ConnectionStatus;
} AUDM_Remote_Control_Browsing_Connection_Status_Event_Data_t;

#define AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_EVENT_DATA_SIZE  (sizeof(AUDM_Remote_Control_Browsing_Connection_Status_Event_Data_t))

   /* The following structure is a container structure                  */
   /* that holds the information that is returned in a                  */
   /* aetRemoteControlBrowsingDisconnected event.                       */
typedef struct _tagAUDM_Remote_Control_Browsing_Disconnected_Event_Data_t
{
   BD_ADDR_t               RemoteDeviceAddress;
} AUDM_Remote_Control_Browsing_Disconnected_Event_Data_t;

#define AUDM_REMOTE_CONTROL_BROWSING_DISCONNECTED_EVENT_DATA_SIZE       (sizeof(AUDM_Remote_Control_Browsing_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Audio Manager Event (and Event Data) of an Audio Manager Event.   */
typedef struct _tagAUDM_Event_Data_t
{
   AUDM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      AUDM_Incoming_Connection_Request_Event_Data_t               IncomingConnectionRequestEventData;
      AUDM_Audio_Stream_Connected_Event_Data_t                    AudioStreamConnectedEventData;
      AUDM_Audio_Stream_Connection_Status_Event_Data_t            AudioStreamConnectionStatusEventData;
      AUDM_Audio_Stream_Disconnected_Event_Data_t                 AudioStreamDisconnectedEventData;
      AUDM_Audio_Stream_State_Changed_Event_Data_t                AudioStreamStateChangedEventData;
      AUDM_Change_Audio_Stream_State_Status_Event_Data_t          ChangeAudioStreamStateStatusEventData;
      AUDM_Audio_Stream_Format_Changed_Event_Data_t               AudioStreamFormatChangedEventData;
      AUDM_Change_Audio_Stream_Format_Status_Event_Data_t         ChangeAudioStreamFormatStatusEventData;
      AUDM_Encoded_Audio_Stream_Data_Event_Data_t                 EncodedAudioStreamDataEventData;
      AUDM_Remote_Control_Connected_Event_Data_t                  RemoteControlConnectedEventData;
      AUDM_Remote_Control_Connection_Status_Event_Data_t          RemoteControlConnectionStatusEventData;
      AUDM_Remote_Control_Disconnected_Event_Data_t               RemoteControlDisconnectedEventData;
      AUDM_Remote_Control_Command_Indication_Event_Data_t         RemoteControlCommandIndicationEventData;
      AUDM_Remote_Control_Command_Confirmation_Event_Data_t       RemoteControlCommandConfirmationEventData;
      AUDM_Remote_Control_Browsing_Connected_Event_Data_t         RemoteControlBrowsingConnectedEventData;
      AUDM_Remote_Control_Browsing_Connection_Status_Event_Data_t RemoteControlBrowsingConnectionStatusEventData;
      AUDM_Remote_Control_Browsing_Disconnected_Event_Data_t      RemoteControlBrowsingDisconnectedEventData;
   } EventData;
} AUDM_Event_Data_t;

#define AUDM_EVENT_DATA_SIZE                                   (sizeof(AUDM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Audio Manager dispatches an event (and the client has registered  */
   /* for events).  This function passes to the caller the Audio Manager*/
   /* Event and the Callback Parameter that was specified when this     */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the Event Data ONLY in the context of this callback.  If the      */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Message will not be   */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *AUDM_Event_Callback_t)(AUDM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Audio Manager Module.  This        */
   /* function should be registered with the Bluetopia Platform Manager */
   /* Module Handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI AUDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI AUDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Audio Stream or*/
   /* Remote Control connection.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  An Audio Stream   */
   /*          Connected event will be dispatched to signify the actual */
   /*          result.                                                  */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Connection_Request_Response(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Connection_Request_Response_t)(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect an Audio Stream to a remote device.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Audio Stream Connection Status Event (if specified).     */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          aetAudioStreamConnectionStatus event will be dispatched  */
   /*          to to denote the status of the connection.  This is the  */
   /*          ONLY way to receive this event, as an event callack      */
   /*          registered with the AUDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Connect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned long StreamFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Connect_Audio_Stream_t)(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned long StreamFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Audio Stream.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Disconnect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Disconnect_Audio_Stream_t)(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* local modules to determine if there are currently any connected   */
   /* Audio sessions of the specified role (specified by the first      */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of device address entries that the buffer will     */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with the total   */
   /* number of connected devices if the function is successful. The    */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters). This function returns a non-negative   */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer. This    */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Query_Audio_Connected_Devices(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Query_Audio_Connected_Devices_t)(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream state of the     */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream State of the Audio Stream (if*/
   /* this function is successful).                                     */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Query_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t *StreamState);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Query_Audio_Stream_State_t)(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t *StreamState);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream format of the    */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream Format of the Audio Stream   */
   /* (if this function is successful).                                 */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Query_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Query_Audio_Stream_Format_t)(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to start/suspend the specified Audio Stream.  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Change_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Change_Audio_Stream_State_t)(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the stream format of a currently connected (but */
   /* suspended) Audio Stream.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The stream format can ONLY be changed when the stream    */
   /*          state is stopped.                                        */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Change_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Change_Audio_Stream_Format_t)(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream configuration of */
   /* the specified Audio Stream.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* The final parameter will hold the Audio Stream Configuration of   */
   /* the Audio Stream (if this function is successful).                */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Query_Audio_Stream_Configuration(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Configuration_t *StreamConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Query_Audio_Stream_Configuration_t)(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Configuration_t *StreamConfiguration);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for Audio Manager */
   /* Connections (Audio Streams and Remote Control).  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Change_Incoming_Connection_Flags_t)(unsigned long ConnectionFlags);
#endif

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          AUDM_Query_Audio_Stream_Configuration() function.        */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Send_Encoded_Audio_Data(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Send_Encoded_Audio_Data_t)(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame);
#endif

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send, followed by flags which  */
   /* specify the format of the data (currently not used, this parameter*/
   /* is reserved for future additions), followed by the RTP Header     */
   /* Information.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          AUDM_Query_Audio_Stream_Configuration() function.        */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
   /* * NOTE * This is a low level function and allows the user to      */
   /*          specify the RTP Header Information for the outgoing data */
   /*          packet.  To use the default values for the RTP Header    */
   /*          Information use AUDM_Send_Encoded_Audio_Data() instead.  */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Send_RTP_Encoded_Audio_Data(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Send_RTP_Encoded_Audio_Data_t)(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a Remote Control connection to a remote      */
   /* device. This function returns zero if successful, or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Remote Control Connection Status Event (if specified).   */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          aetRemoteControlConnectionStatus event will be dispatched*/
   /*          to to denote the status of the connection.  This is the  */
   /*          ONLY way to receive this event, as an event callack      */
   /*          registered with the AUDM_Register_Event_Callback() or    */
   /*          AUDM_Register_Remote_Control_Event_Callback() functions  */
   /*          will NOT receive connection status events.               */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Connect_Remote_Control(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Connect_Remote_Control_t)(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Remote Control        */
   /* session.  This function returns zero if successful, or a negative */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Disconnect_Remote_Control(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Disconnect_Remote_Control_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Remote  */
   /* Control Target or Controller sessions (specified by the first     */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of device address entries that the buffer will     */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with the total   */
   /* number of connected devices if the function is successful. The    */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters). This function returns a non-negative   */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer. This    */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Query_Remote_Control_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Query_Remote_Control_Connected_Devices_t)(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is responsible for sending the specified   */
   /* Remote Control Command to the remote Device.  This function       */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* AUDM_Register_Remote_Control_Event_Callback() function), followed */
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Response Timeout (in milliseconds), followed by a */
   /* pointer to the actual Remote Control Message to send.  This       */
   /* function returns a positive, value if successful or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Transaction ID of the Remote Control Event that was  */
   /*          submitted.                                               */
   /* * NOTE * The response to the command, from the Target, will only  */
   /*          be delivered to the Callback associated with the         */
   /*          Remote Control Event Callback ID provided.               */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Send_Remote_Control_Command(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Send_Remote_Control_Command_t)(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData);
#endif

   /* The following function is responsible for sending the specified   */
   /* Remote Control Response to the remote Device.  This function      */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* AUDM_Register_Remote_Control_Event_Callback() function), followed */
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Transaction ID of the Remote Control Event,       */
   /* followed by a pointer to the actual Remote Control Response       */
   /* Message to send.  This function returns zero if successful or a   */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Send_Remote_Control_Response(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Send_Remote_Control_Response_t)(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Audio Manager    */
   /* Service.  This Callback will be dispatched by the Audio Manager   */
   /* when various Audio Manager Events occur.  This function accepts   */
   /* the Callback Function and Callback Parameter (respectively) to    */
   /* call when an Audio Manager Event needs to be dispatched.  This    */
   /* function returns a positive (non-zero) value if successful, or a  */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          AUDM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Register_Event_Callback(AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Register_Event_Callback_t)(AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Callback  */
   /* (registered via a successful call to the                          */
   /* AUDM_Register_Event_Callback() function).  This function accepts  */
   /* as input the Audio Manager Event Callback ID (return value from   */
   /* AUDM_Register_Event_Callback() function).                         */
BTPSAPI_DECLARATION void BTPSAPI AUDM_Un_Register_Event_Callback(unsigned int AudioManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_AUDM_Un_Register_Event_Callback_t)(unsigned int AudioManagerCallbackID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Audio     */
   /* Manager Service to explicitly process Data (either Source or      */
   /* Sink).  This Callback will be dispatched by the Audio Manager when*/
   /* various Audio Manager Events occur.  This function accepts Audio  */
   /* Stream Type and the Callback Function and Callback Parameter      */
   /* (respectively) to call when an Audio Manager Event needs to be    */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          AUDM_Send_Encoded_Audio_Data() function to send data (for*/
   /*          Audio Source).                                           */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Audio Stream Type.                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          AUDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Register_Data_Event_Callback(AUD_Stream_Type_t StreamType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Register_Data_Event_Callback_t)(AUD_Stream_Type_t StreamType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Callback  */
   /* (registered via a successful call to the                          */
   /* AUDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Audio Manager Data Event Callback ID (return */
   /* value from AUDM_Register_Data_Event_Callback() function).         */
BTPSAPI_DECLARATION void BTPSAPI AUDM_Un_Register_Data_Event_Callback(unsigned int AudioManagerDataCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_AUDM_Un_Register_Data_Event_Callback_t)(unsigned int AudioManagerDataCallbackID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Audio     */
   /* Manager Service to explicitly process Remote Control Data (either */
   /* Controller or Target).  This Callback will be dispatched by the   */
   /* Audio Manager when various Audio Manager Events occur.  This      */
   /* function accepts the Service Type (Target or Controller) and the  */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when an Audio Manager Remote Control Event needs to be dispatched.*/
   /* This function returns a positive (non-zero) value if successful,  */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          AUDM_Send_Remote_Control_Command() or                    */
   /*          AUDM_Send_Remote_Control_Response() functions to send    */
   /*          Remote Control Events.                                   */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Service Type.                                   */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          AUDM_Un_Register_Remote_Control_Event_Callback() function*/
   /*          to un-register the callback from this module.            */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Register_Remote_Control_Event_Callback(unsigned int ServiceType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Register_Remote_Control_Event_Callback_t)(unsigned int ServiceType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Remote Control  */
   /* Event Callback (registered via a successful call to the           */
   /* AUDM_Register_Remote_Control_Event_Callback() function).  This    */
   /* function accepts as input the Audio Manager Remote Control Event  */
   /* Callback ID (return value from                                    */
   /* AUDM_Register_Remote_Control_Event_Callback() function).          */
BTPSAPI_DECLARATION void BTPSAPI AUDM_Un_Register_Remote_Control_Event_Callback(unsigned int AudioManagerRemoteControlCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_AUDM_Un_Register_Remote_Control_Event_Callback_t)(unsigned int AudioManagerRemoteControlCallbackID);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* query and parse AVRCP services from a remote device. This first   */
   /* parameter is the Bluetooth Address of the remote device. The      */
   /* second parameter is a pointer to the structure where the parsed   */
   /* service information will be placed. This function returns zero if */
   /* successful and a negative return error code if there is an error. */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Query_Remote_Control_Services_Info(BD_ADDR_t RemoteDeviceAddress, AUDM_Remote_Control_Services_Info_t *ServicesInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Query_Remote_Control_Services_Info_t)(BD_ADDR_t RemoteDeviceAddress, AUDM_Remote_Control_Services_Info_t *ServicesInfo);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a Remote Control Browsing Channel connection */
   /* to a remote device. This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function requires that a standard Remote Control    */
   /*          Connection be setup before attempting to connect         */
   /*          browsing.                                                */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in     */
   /*          the Remote Control Browsing Connection Status Event (if  */
   /*          specified).                                              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          aetRemoteControlBrowsingConnectionStatus event           */
   /*          will be dispatched to to denote the status of            */
   /*          the connection.  This is the ONLY way to receive         */
   /*          this event, as an event callack registered               */
   /*          with the AUDM_Register_Event_Callback() or               */
   /*          AUDM_Register_Remote_Control_Event_Callback() functions  */
   /*          will NOT receive connection status events.               */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Connect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Connect_Remote_Control_Browsing_t)(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* Browsing session.  This function returns zero if successful, or a */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI AUDM_Disconnect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AUDM_Disconnect_Remote_Control_Browsing_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

#endif
