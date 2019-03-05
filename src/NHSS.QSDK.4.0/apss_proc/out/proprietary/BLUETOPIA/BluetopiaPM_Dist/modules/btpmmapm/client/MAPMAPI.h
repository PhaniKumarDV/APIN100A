/*****< mapmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAPMAPI - Message Access Profile API for Stonestreet One Bluetooth        */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/24/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __MAPMAPIH__
#define __MAPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "MAPMMSG.h"             /* Message Access IPC Message Definitions.   */

   /* The following enumerated type represents the Message Access       */
   /* Profile (MAP) Manager (MAPM) event types that are dispatched by   */
   /* this module.                                                      */
typedef enum
{
   /* Common/Connection Events.                                         */
   metMAPIncomingConnectionRequest,
   metMAPConnected,
   metMAPDisconnected,
   metMAPConnectionStatus,

   /* Message Access Client (MCE) Events.                               */
   metMAPEnableNotificationsResponse,
   metMAPGetFolderListingResponse,
   metMAPGetFolderListingSizeResponse,
   metMAPGetMessageListingResponse,
   metMAPGetMessageListingSizeResponse,
   metMAPGetMessageResponse,
   metMAPSetMessageStatusResponse,
   metMAPPushMessageResponse,
   metMAPUpdateInboxResponse,
   metMAPSetFolderResponse,

   /* Message Access Client (MCE) Notification Events.                  */
   metMAPNotificationIndication,

   /* Message Access Server (MSE) Events.                               */
   metMAPEnableNotificationsIndication,
   metMAPGetFolderListingRequest,
   metMAPGetFolderListingSizeRequest,
   metMAPGetMessageListingRequest,
   metMAPGetMessageListingSizeRequest,
   metMAPGetMessageRequest,
   metMAPSetMessageStatusRequest,
   metMAPPushMessageRequest,
   metMAPUpdateInboxRequest,
   metMAPSetFolderRequest,

   /* Message Access Server (MSE) Notification Events.                  */
   metMAPNotificationConfirmation
} MAPM_Event_Type_t;

   /* Common/Connection Events.                                         */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPIncomingConnectionRequest */
   /* event.                                                            */
typedef struct _tagMAPM_Incoming_Connection_Request_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
} MAPM_Incoming_Connection_Request_Event_Data_t;

#define MAPM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE       (sizeof(MAPM_Incoming_Connection_Request_Event_Data_t))

   /* The following event is dispatched when a Hands Free connection    */
   /* occurs.  The ConnectionType member specifies which local MAP role */
   /* type has been connected to and the RemoteDeviceAddress member     */
   /* specifies the remote Bluetooth device that has connected to the   */
   /* specified MAP Role.  Finally the Instance ID identifies the server*/
   /* with which the remote device connected.                           */
typedef struct _tagMAPM_Connected_Event_Data_t
{
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
} MAPM_Connected_Event_Data_t;

#define MAPM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(MAPM_Connected_Event_Data_t))

   /* The following event is dispatched when a remote device disconnects*/
   /* from the local device (for the specified MAP role).  The          */
   /* ConnectionType member identifies the local MAP role type being    */
   /* disconnected and the RemoteDeviceAddress member specifies the     */
   /* Bluetooth device address of the device that disconnected from the */
   /* profile.  The InstanceID member identifies the actual MAP         */
   /* connection that was disconnected.                                 */
typedef struct _tagMAPM_Disconnected_Event_Data_t
{
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
} MAPM_Disconnected_Event_Data_t;

#define MAPM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(MAPM_Disconnected_Event_Data_t))

   /* The following event is dispatched when a client receives the      */
   /* connection response from a remote server which was previously     */
   /* attempted to be connected to.  The ConnectionType member specifies*/
   /* the local client that has requested the connection, the           */
   /* RemoteDeviceAddress member specifies the remote device that was   */
   /* attempted to be connected to, the InstanceID specifies the        */
   /* Instance ID that defines the connection, and the ConnectionStatus */
   /* member represents the connection status of the request.           */
typedef struct _tagMAPM_Connection_Status_Event_Data_t
{
   MAPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   unsigned int           ConnectionStatus;
} MAPM_Connection_Status_Event_Data_t;

#define MAPM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(MAPM_Connection_Status_Event_Data_t))

   /* Message Access Client (MCE) Events.                               */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* metMAPEnableNotificationsResponse event.  The following event is  */
   /* dispatched when an attempt to register the notification server    */
   /* with a remote MSE completes.  The RemoteDeviceAddress member      */
   /* specifies the Bluetooth device address of the remote device.  The */
   /* InstanceID member specifies which Server instance to use on the   */
   /* remote device.  The Status member indicates whether the           */
   /* registration succeeded.                                           */
typedef struct _tagMAPM_Enable_Notifications_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int ResponseStatusCode;
} MAPM_Enable_Notifications_Response_Event_Data_t;

#define MAPM_ENABLE_NOTIFICATIONS_RESPONSE_EVENT_DATA_SIZE     (sizeof(MAPM_Enable_Notifications_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPGetFolderListingResponse  */
   /* event.  The following event is dispatched when a response to a get*/
   /* folder listing request is received.  The RemoteDeviceAddress      */
   /* member specifies the Bluetooth device address of the remote       */
   /* device.  The InstanceID member specifies which Server instance to */
   /* use on the remote device.  The Status member is the response      */
   /* indicated by the device.  The Final member signifies whether this */
   /* is the last event for the response.  The FolderListingLength      */
   /* member indicates the amount of the data contained in the          */
   /* FolderListingData member.                                         */
typedef struct _tagMAPM_Get_Folder_Listing_Response_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  InstanceID;
   unsigned int  ResponseStatusCode;
   Boolean_t     Final;
   unsigned int  FolderListingLength;
   Byte_t       *FolderListingData;
} MAPM_Get_Folder_Listing_Response_Event_Data_t;

#define MAPM_GET_FOLDER_LISTING_RESPONSE_EVENT_DATA_SIZE       (sizeof(MAPM_Get_Folder_Listing_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* metMAPGetFolderListingSizeResponse event.  The following event is */
   /* dispatched when a response to a get folder listing size request is*/
   /* received.  The RemoteDeviceAddress member specifies the Bluetooth */
   /* device address of the remote device.  The InstanceID member       */
   /* specifies which Server instance to use on the remote device.  The */
   /* Status member is the response indicated by the device.  The       */
   /* NumberOfFolders is the returned size.                             */
typedef struct _tagMAPM_Get_Folder_Listing_Size_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int ResponseStatusCode;
   Word_t       NumberOfFolders;
} MAPM_Get_Folder_Listing_Size_Response_Event_Data_t;

#define MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_EVENT_DATA_SIZE  (sizeof(MAPM_Get_Folder_Listing_Size_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPGetMessageListingResponse */
   /* event.  The following event is dispatched when a response to a get*/
   /* message listing request is received.  The RemoteDeviceAddress     */
   /* member specifies the Bluetooth device address of the remote       */
   /* device.  The InstanceID member specifies which Server instance to */
   /* use on the remote device.  The Status member is the response      */
   /* indicated by the device.  The Final member signifies whether this */
   /* is the event packet for the response.  The NewMessage member      */
   /* indicates of any new messages have been received.  The            */
   /* MessageListingLength member specifies how much data is present in */
   /* the MessageListingData member.                                    */
typedef struct _tagMAPM_Get_Message_Listing_Response_Event_Data_t
{
   BD_ADDR_t       RemoteDeviceAddress;
   unsigned int    InstanceID;
   unsigned int    ResponseStatusCode;
   Boolean_t       NewMessage;
   MAP_TimeDate_t  MSETime;
   Word_t          NumberOfMessages;
   Boolean_t       Final;
   unsigned int    MessageListingLength;
   Byte_t         *MessageListingData;
} MAPM_Get_Message_Listing_Response_Event_Data_t;

#define MAPM_GET_MESSAGE_LISTING_RESPONSE_EVENT_DATA_SIZE      (sizeof(MAPM_Get_Message_Listing_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPMessageListingSizeResponse*/
   /* event.  The following event is dispatched when a response to a get*/
   /* message listing size request is received.  The RemoteDeviceAddress*/
   /* member specifies the Bluetooth device address of the remote       */
   /* device.  The InstanceID member specifies which Server instance to */
   /* use on the remote device.  The Status member is the response      */
   /* indicated by the device.  The NumberOfMessages is the returned    */
   /* size.                                                             */
typedef struct _tagMAPM_Get_Message_Listing_Size_Response_Event_Data_t
{
   BD_ADDR_t      RemoteDeviceAddress;
   unsigned int   InstanceID;
   unsigned int   ResponseStatusCode;
   Boolean_t      NewMessage;
   MAP_TimeDate_t MSETime;
   Word_t         NumberOfMessages;
} MAPM_Get_Message_Listing_Size_Response_Event_Data_t;

#define MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_EVENT_DATA_SIZE (sizeof(MAPM_Get_Message_Listing_Size_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPGetMessageResponse event. */
   /* The following event is dispatched when a response to a get message*/
   /* request is received.  The RemoteDeviceAddress member specifies the*/
   /* Bluetooth device address of the remote device.  The InstanceID    */
   /* member specifies which Server instance to use on the remote       */
   /* device.  The Status member is the response indicated by the       */
   /* device.  The Final member signifies whether this is the last data */
   /* event for the response.  The FractionalType parameter identifies  */
   /* the portion of the message that is being received.  The           */
   /* MessageDataLength member specifies how much data was received.    */
   /* Finally the MessageData member points to the data that was        */
   /* received.                                                         */
typedef struct _tagMAPM_Get_Message_Response_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   unsigned int           ResponseStatusCode;
   MAP_Fractional_Type_t  FractionalType;
   Boolean_t              Final;
   unsigned int           MessageDataLength;
   Byte_t                *MessageData;
} MAPM_Get_Message_Response_Event_Data_t;

#define MAPM_GET_MESSAGE_RESPONSE_EVENT_DATA_SIZE              (sizeof(MAPM_Get_Message_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPSetMessageStatusResponse  */
   /* event.  The following event is dispatched when a response to a set*/
   /* message status request is received.  The RemoteDeviceAddress      */
   /* member specifies the Bluetooth device address of the remote       */
   /* device.  The InstanceID member specifies which Server instance to */
   /* use on the remote device.  The Status member is the response      */
   /* indicated by the device.                                          */
typedef struct _tagMAPM_Set_Message_Status_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int ResponseStatusCode;
} MAPM_Set_Message_Status_Response_Event_Data_t;

#define MAPM_SET_MESSAGE_STATUS_RESPONSE_EVENT_DATA_SIZE       (sizeof(MAPM_Set_Message_Status_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPPushMessageResponse event.*/
   /* The following event is dispatched when a response to a push       */
   /* message request is received.  The RemoteDeviceAddress member      */
   /* specifies the Bluetooth device address of the remote device.  The */
   /* InstanceID member specifies which Server instance to use on the   */
   /* remote device.  The Status member is the response indicated by the*/
   /* device.  The MessageHandle is a string of ASCII encoded HEX       */
   /* characters used to indentify the message (NULL terminated).       */
typedef struct _tagMAPM_Push_Message_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int ResponseStatusCode;
   char         MessageHandle[MAP_MESSAGE_HANDLE_LENGTH + 1];
} MAPM_Push_Message_Response_Event_Data_t;

#define MAPM_PUSH_MESSAGE_RESPONSE_EVENT_DATA_SIZE             (sizeof(MAPM_Push_Message_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPUpdateInboxResponse event.*/
   /* The following event is dispatched when a response to an update    */
   /* inbox request is received.  The RemoteDeviceAddress member        */
   /* specifies the Bluetooth device address of the remote device.  The */
   /* InstanceID member specifies which Server instance to use on the   */
   /* remote device.  The Status member is the response indicated by the*/
   /* device.                                                           */
typedef struct _tagMAPM_Update_Inbox_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int ResponseStatusCode;
} MAPM_Update_Inbox_Response_Event_Data_t;

#define MAPM_UPDATE_INBOX_RESPONSE_EVENT_DATA_SIZE             (sizeof(MAPM_Update_Inbox_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPSetFolderResponse event.  */
   /* The following event is dispatched when a response to a set folder */
   /* request is received.  The RemoteDeviceAddress member specifies the*/
   /* Bluetooth device address of the remote device.  The InstanceID    */
   /* member specifies which Server instance to use on the remote       */
   /* device.  The Status member is the response indicated by the       */
   /* device.  The CurrentPath member represents the current path (which*/
   /* may be an unexpected path if a call to Set_Folder_Absolute() did  */
   /* not complete successfully).                                       */
   /* * NOTE * The CurrentPath member points to a NULL terminated, UTF-8*/
   /*          encoded ASCII string.                                    */
typedef struct _tagMAPM_Set_Folder_Response_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  InstanceID;
   unsigned int  ResponseStatusCode;
   char         *CurrentPath;
} MAPM_Set_Folder_Response_Event_Data_t;

#define MAPM_SET_FOLDER_RESPONSE_EVENT_DATA_SIZE               (sizeof(MAPM_Set_Folder_Response_Event_Data_t))

   /* Message Access Client (MCE) Notification Events.                  */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPNotificationIndication    */
   /* event.  The following event is dispatched when a remote           */
   /* notification client sends an event to the notification server.    */
   /* The RemoteDeviceAddress member specifies the Bluetooth device     */
   /* address of the remote device.  The InstanceID member specifies    */
   /* which Server instance to use on the remote device.  The           */
   /* MASInstanceID specifies the instance of the MSE on the remote     */
   /* device.  The Final member indicates whether this is the last event*/
   /* for this request.  The EventReportLength member indicates the     */
   /* length of the data buffer.  The EventReportData parameter is a    */
   /* pointer to the event data.                                        */
typedef struct _tagMAPM_Notification_Indication_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  InstanceID;
   Boolean_t     Final;
   unsigned int  EventReportLength;
   Byte_t       *EventReportData;
} MAPM_Notification_Indication_Event_Data_t;

#define MAPM_NOTIFICATION_INDICATION_EVENT_DATA_SIZE           (sizeof(MAPM_Notification_Indication_Event_Data_t))

   /* Message Access Server (MSE) Events.                               */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metEnableNotificationsRequest   */
   /* event.  The following event is dispatched when a request to       */
   /* enabled/disable notifications is received.  The                   */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the remote device.  The InstanceID member specifies which      */
   /* Server instance to use.  The Enabled field indicates whether      */
   /* notifications or being enabled or disabled.                       */
typedef struct _tagMAPM_Enable_Notifications_Indication_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   Boolean_t    Enabled;
} MAPM_Enable_Notifications_Indication_Event_Data_t;

#define MAPM_ENABLE_NOTIFICATIONS_INDICATION_EVENT_DATA_SIZE   (sizeof(MAPM_Enable_Notifications_Indication_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPGetFolderListingRequest   */
   /* event.  The following event is dispatched when a request for a    */
   /* folder listing is received.  The RemoteDeviceAddress member       */
   /* specifies the Bluetooth device address of the remote device.  The */
   /* InstanceID member specifies which Server instance to use.  The    */
   /* MaxListCount parameters specifies the maximum amount of entries to*/
   /* include in the response.  The ListStartOffset member specifies an */
   /* offset into the listing to begin.                                 */
typedef struct _tagMAPM_Get_Folder_Listing_Request_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   Word_t       MaxListCount;
   Word_t       ListStartOffset;
} MAPM_Get_Folder_Listing_Request_Event_Data_t;

#define MAPM_GET_FOLDER_LISTING_REQUEST_EVENT_DATA_SIZE        (sizeof(MAPM_Get_Folder_Listing_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* metMAPGetFolderListingSizeRequest event.  The following event is  */
   /* dispatched when a request for the folder listing size is received.*/
   /* The RemoteDeviceAddress member specifies the Bluetooth device     */
   /* address of the remote device.  The InstanceID member specifies    */
   /* which Server instance to use.                                     */
typedef struct _tagMAPM_Get_Folder_Listing_Size_Request_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
} MAPM_Get_Folder_Listing_Size_Request_Event_Data_t;

#define MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_EVENT_DATA_SIZE   (sizeof(MAPM_Get_Folder_Listing_Size_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPGetMessageListingRequest  */
   /* event.  The following event is dispatched when a request for a    */
   /* message listing is received.  The RemoteDeviceAddress member      */
   /* specifies the Bluetooth device address of the remote device.  The */
   /* InstanceID member specifies which Server instance to use.  The    */
   /* FolderName member specifies the folder to pull the listing from.  */
   /* A NULL value indicates the current folder.  The MaxListCount      */
   /* parameter specifies the maximum amount of entries to include in   */
   /* the response.  The ListStartOffset member specifies an offset into*/
   /* the listing to begin.  The ListingInfo member is a structure that */
   /* contains information about what is include in the request and what*/
   /* should be included in the response.                               */
   /* * NOTE * If the FolderName member is NON-NULL it will point to a  */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
typedef struct _tagMAPM_Get_Message_Listing_Request_Event_Data_t
{
   BD_ADDR_t                   RemoteDeviceAddress;
   unsigned int                InstanceID;
   char                       *FolderName;
   Word_t                      MaxListCount;
   Word_t                      ListStartOffset;
   MAP_Message_Listing_Info_t  ListingInfo;
} MAPM_Get_Message_Listing_Request_Event_Data_t;

#define MAPM_GET_MESSAGE_LISTING_REQUEST_EVENT_DATA_SIZE       (sizeof(MAPM_Get_Message_Listing_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* metMAPGetMessageListingSizeRequest event.  The following event is */
   /* dispatched when a request for the message listing size is         */
   /* received.  The RemoteDeviceAddress member specifies the Bluetooth */
   /* device address of the remote device.  The InstanceID member       */
   /* specifies which Server instance to use.  The FolderName member    */
   /* specifies the folder to pull the listing size from.  A NULL value */
   /* indicates the current folder.  The ListingInfo member is a        */
   /* structure that contains information about what is include in the  */
   /* request and what should be included in the response.              */
   /* * NOTE * If the FolderName member is NON-NULL it will point to a  */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
typedef struct _tagMAPM_Get_Message_Listing_Size_Request_Event_Data_t
{
   BD_ADDR_t                   RemoteDeviceAddress;
   unsigned int                InstanceID;
   char                       *FolderName;
   MAP_Message_Listing_Info_t  ListingInfo;
} MAPM_Get_Message_Listing_Size_Request_Event_Data_t;

#define MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_EVENT_DATA_SIZE  (sizeof(MAPM_Get_Message_Listing_Size_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPGetMessageRequest event.  */
   /* The following event is dispatched when a request to get a message */
   /* is received.  The RemoteDeviceAddress member specifies the        */
   /* Bluetooth device address of the remote device.  The InstanceID    */
   /* member specifies which Server instance to use.  The Attachment    */
   /* member specifies whether any attachments should be included.  The */
   /* CharSet member indicates the format of the message.  The          */
   /* FractionalType member indicates what message fragment is          */
   /* requested.  The MessageHandle specifies the message requested.    */
typedef struct _tagMAPM_Get_Message_Request_Event_Data_t
{
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             Attachment;
   MAP_CharSet_t         CharSet;
   MAP_Fractional_Type_t FractionalType;
   char                  MessageHandle[MAP_MESSAGE_HANDLE_LENGTH + 1];
} MAPM_Get_Message_Request_Event_Data_t;

#define MAPM_GET_MESSAGE_REQUEST_EVENT_DATA_SIZE               (sizeof(MAPM_Get_Message_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPSetMessageStatusRequest   */
   /* event.  The following event is dispatched when a request to set a */
   /* message's status is received.  The RemoteDeviceAddress member     */
   /* specifies the Bluetooth device address of the remote device.  The */
   /* InstanceID member specifies which Server instance to use.  The    */
   /* MessageHandle member specifies what message to set.  The          */
   /* StatusIndicator member indicates which status to set.  The        */
   /* StatusValue parameter indicates the new status.                   */
typedef struct _tagMAPM_Set_Message_Status_Request_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   char                   MessageHandle[MAP_MESSAGE_HANDLE_LENGTH + 1];
   MAP_Status_Indicator_t StatusIndicator;
   Boolean_t              StatusValue;
} MAPM_Set_Message_Status_Request_Event_Data_t;

#define MAPM_SET_MESSAGE_STATUS_REQUEST_EVENT_DATA_SIZE        (sizeof(MAPM_Set_Message_Status_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPPushMessageRequest event. */
   /* The following event is dispatched when a request to push a message*/
   /* is received.  The RemoteDeviceAddress member specifies the        */
   /* Bluetooth device address of the remote device.  The InstanceID    */
   /* member specifies which Server instance to use.  The FolderName    */
   /* indicates what folder to push the message.  If NULL, the current  */
   /* folder is to be used.  Transparent specifies whether a copy should*/
   /* be saved in the sent folder.  Retry indicates if the message      */
   /* should be retried if the sending fails.  CharSet indicates the    */
   /* format of the message.  MessageDataLength specifies the length of */
   /* the data buffer.                                                  */
   /* * NOTE * If the FolderName member is NON-NULL it will point to a  */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
typedef struct _tagMAPM_Push_Message_Request_Event_Data_t
{
   BD_ADDR_t       RemoteDeviceAddress;
   unsigned int    InstanceID;
   char           *FolderName;
   Boolean_t       Transparent;
   Boolean_t       Retry;
   MAP_CharSet_t   CharSet;
   Boolean_t       Final;
   unsigned int    MessageDataLength;
   Byte_t         *MessageData;
} MAPM_Push_Message_Request_Event_Data_t;

#define MAPM_PUSH_MESSAGE_REQUEST_EVENT_DATA_SIZE              (sizeof(MAPM_Push_Message_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPUpdateInboxRequest event. */
   /* The following event is dispatched when a request to update the    */
   /* inbox is received.  The RemoteDeviceAddress member specifies the  */
   /* Bluetooth device address of the remote device.  The InstanceID    */
   /* member specifies which Server instance to use.                    */
typedef struct _tagMAPM_Update_Inbox_Request_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
} MAPM_Update_Inbox_Request_Event_Data_t;

#define MAPM_UPDATE_INBOX_REQUEST_EVENT_DATA_SIZE              (sizeof(MAPM_Update_Inbox_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPSetFolderRequest event.   */
   /* The following event is dispatched when a request to set the folder*/
   /* is received.  The RemoteDeviceAddress member specifies the        */
   /* Bluetooth device address of the remote device.  The InstanceID    */
   /* member specifies which Server instance to use.  The PathOption    */
   /* member indicates the location of the new directory with respect to*/
   /* the current.  If PathOption is sfDown, then FolderName indicates  */
   /* the name of the new directory.                                    */
   /* * NOTE * If the FolderName member is NON-NULL it will point to a  */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
typedef struct _tagMAPM_Set_Folder_Request_Event_Data_t
{
   BD_ADDR_t                RemoteDeviceAddress;
   unsigned int             InstanceID;
   MAP_Set_Folder_Option_t  PathOption;
   char                    *FolderName;
   char                    *NewPath;
} MAPM_Set_Folder_Request_Event_Data_t;

#define MAPM_SET_FOLDER_REQUEST_EVENT_DATA_SIZE                (sizeof(MAPM_Set_Folder_Request_Event_Data_t))

   /* Message Access Server (MSE) Notification Events.                  */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a metMAPNotificationConfirmation  */
   /* event.  The following event is dispatched when a remote           */
   /* notification server sends an event response back to the           */
   /* notification server.  The RemoteDeviceAddress member specifies the*/
   /* Bluetooth device address of the remote device.  The InstanceID    */
   /* member specifies which server instance the response was received  */
   /* on.  The ResponseStatusCode member holds the response code from   */
   /* the remote device.                                                */
typedef struct _tagMAPM_Notification_Confirmation_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int ResponseStatusCode;
} MAPM_Notification_Confirmation_Event_Data_t;

#define MAPM_NOTIFICATION_CONFIRMATION_EVENT_DATA_SIZE         (sizeof(MAPM_Notification_Confirmation_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Message Access Manager Event (and Event Data) of a Message Access */
   /* Manager Event.                                                    */
typedef struct _tagMAPM_Event_Data_t
{
   MAPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      /* Common/Connection Events.                                      */
      MAPM_Incoming_Connection_Request_Event_Data_t       IncomingConnectionRequestEventData;
      MAPM_Connected_Event_Data_t                         ConnectedEventData;
      MAPM_Disconnected_Event_Data_t                      DisconnectedEventData;
      MAPM_Connection_Status_Event_Data_t                 ConnectionStatusEventData;

      /* Message Access Client (MCE) Events.                            */
      MAPM_Enable_Notifications_Response_Event_Data_t     EnableNotificationsResponseEventData;
      MAPM_Get_Folder_Listing_Response_Event_Data_t       GetFolderListingResponseEventData;
      MAPM_Get_Folder_Listing_Size_Response_Event_Data_t  GetFolderListingSizeResponseEventData;
      MAPM_Get_Message_Listing_Response_Event_Data_t      GetMessageListingResponseEventData;
      MAPM_Get_Message_Listing_Size_Response_Event_Data_t GetMessageListingSizeResponseEventData;
      MAPM_Get_Message_Response_Event_Data_t              GetMessageResponseEventData;
      MAPM_Set_Message_Status_Response_Event_Data_t       SetMessageStatusResponseEventData;
      MAPM_Push_Message_Response_Event_Data_t             PushMessageResponseEventData;
      MAPM_Update_Inbox_Response_Event_Data_t             UpdateInboxResponseEventData;
      MAPM_Set_Folder_Response_Event_Data_t               SetFolderResponseEventData;

      /* Message Access Client (MCE) Notification Events.               */
      MAPM_Notification_Indication_Event_Data_t           NotificationIndicationEventData;

      /* Message Access Server (MSE) Events.                            */
      MAPM_Enable_Notifications_Indication_Event_Data_t   EnableNotificationsIndicationEventData;
      MAPM_Get_Folder_Listing_Request_Event_Data_t        GetFolderListingRequestEventData;
      MAPM_Get_Folder_Listing_Size_Request_Event_Data_t   GetFolderListingSizeRequestEventData;
      MAPM_Get_Message_Listing_Request_Event_Data_t       GetMessageListingRequestEventData;
      MAPM_Get_Message_Listing_Size_Request_Event_Data_t  GetMessageListingSizeRequestEventData;
      MAPM_Get_Message_Request_Event_Data_t               GetMessageRequestEventData;
      MAPM_Set_Message_Status_Request_Event_Data_t        SetMessageStatusRequestEventData;
      MAPM_Push_Message_Request_Event_Data_t              PushMessageRequestEventData;
      MAPM_Update_Inbox_Request_Event_Data_t              UpdateInboxRequestEventData;
      MAPM_Set_Folder_Request_Event_Data_t                SetFolderRequestEventData;

      /* Message Access Server (MSE) Notification Events.               */
      MAPM_Notification_Confirmation_Event_Data_t         NotificationConfirmationEventData;
   } EventData;
} MAPM_Event_Data_t;

#define MAPM_EVENT_DATA_SIZE                                   (sizeof(MAPM_Event_Data_t))

   /* The following declared type represents the prototype function for */
   /* an event and data callback. This function will be called whenever */
   /* the Messagae Access Manager dispatches an event. This function    */
   /* passes to the caller the Message Access Manager event and the     */
   /* callback parameter that was specified when this callback was      */
   /* installed. The caller is free to use the contents of the EventData*/
   /* ONLY in the context of this callback. If the caller requires the  */
   /* data for a longer period of time, then the callback function      */
   /* MUST copy the data into another data buffer. This function is     */
   /* guaranteed NOT to be invoked more than once simultaneously for the*/
   /* specified installed callback (i.e. this function DOES NOT have    */
   /* be reentrant). Because of this, the processing in this function   */
   /* should be as efficient as possible. It should also be noted that  */
   /* this function is called in the thread context of a thread that the*/
   /* user does NOT own. Therefore, processing in this function should  */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another message will not be processed while this function call is */
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT block and wait for events that can*/
   /*          only be satisfied by receiving other Events.  A deadlock */
   /*          WILL occur because NO event callbacks will be issued     */
   /*          while this function is currently outstanding.            */
typedef void (BTPSAPI *MAPM_Event_Callback_t)(MAPM_Event_Data_t *EventData, void *CallbackParameter);

   /* Message Access Module Installation/Support Functions.             */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Message Access Profile (MAP)       */
   /* Manager (MAPM) module.  This function should be registered with   */
   /* the Bluetopia Platform Manager module handler and will be called  */
   /* when the Platform Manager is initialized (or shut down).          */
void BTPSAPI MAPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI MAPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* Message Access Profile (MAP) Manager (MAPM) Common/Connection     */
   /* Functions.                                                        */

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming MAP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A MAP Connected   */
   /*          event will be dispatched to signify the actual result.   */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Connection_Request_Response_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Accept);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* to register a MAP Server (MSE) on a specified RFCOMM Server Port. */
   /* This function accepts as it's parameter's the RFCOMM Server Port  */
   /* to register the server on, followed by the incoming connection    */
   /* flags to apply to incoming connections.  The third and fourth     */
   /* parameters specify the required MAP Information (MAP Server       */
   /* Instance ID - must be unique on the device, followed by the       */
   /* supported MAP Message Types).  The final two parameters specify   */
   /* the MAP Manager Event Callback function and Callback parameter    */
   /* which will be used when MAP Manager events need to be dispatched  */
   /* for the specified MAP Server.  This function returns zero if      */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * This function is only applicable to MAP Servers.  It will*/
   /*          not allow the ability to register a MAP Notification     */
   /*          Server (Notification Servers are handled internal to the */
   /*          MAP Manager module).                                     */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Register_Server(unsigned int ServerPort, unsigned long ServerFlags, unsigned int InstanceID, unsigned long SupportedMessageTypes, MAPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Register_Server_t)(unsigned int ServerPort, unsigned long ServerFlags, unsigned int InstanceID, unsigned long SupportedMessageTypes, MAPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered MAP server (registered via a  */
   /* successful call the the MAP_Register_Server() function).  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Un_Register_Server(unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Un_Register_Server_t)(unsigned int InstanceID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to Register an SDP Service Record for a previously        */
   /* registered MAP Server.  This function returns a positive,         */
   /* non-zero, value if successful, or a negative return error code if */
   /* there was an error.  If this function is successful, the value    */
   /* that is returned represents the SDP Service Record Handle of the  */
   /* Service Record that was added to the SDP Database.  The           */
   /* ServiceName parameter is a pointer to a NULL terminated UTF-8     */
   /* encoded string.                                                   */
BTPSAPI_DECLARATION long BTPSAPI MAPM_Register_Service_Record(unsigned int InstanceID, char *ServiceName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef long (BTPSAPI *PFN_MAPM_Register_Service_Record_t)(unsigned int InstanceID, char *ServiceName);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered SDP Service Record.*/
   /* This function accepts the Instance ID of the MAP Server that is to*/
   /* have the Service Record Removed.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Un_Register_Service_Record(unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Un_Register_Service_Record_t)(unsigned int InstanceID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the details of MAP services offered by a remote  */
   /* Message Access Server device. This function accepts the remote    */
   /* device address of the device whose SDP records will be parsed     */
   /* and the buffer which will hold the parsed service details. This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
   /* * NOTE * When this function is successful, the provided buffer    */
   /*          will be populated with the parsed service details. This  */
   /*          buffer MUST be passed to                                 */
   /*          MAPM_Free_Message_Access_Service_Info() in order to      */
   /*          release any resources that were allocated during the     */
   /*          query process.                                           */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Parse_Remote_Message_Access_Services(BD_ADDR_t RemoteDeviceAddress, MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Parse_Remote_Message_Access_Services_t)(BD_ADDR_t RemoteDeviceAddress, MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to free all resources that were allocated to query the    */
   /* service details of a remote Message Access Server device. See the */
   /* MAPM_Query_Remote_Message_Access_Services() function for more     */
   /* information.                                                      */
BTPSAPI_DECLARATION void BTPSAPI MAPM_Free_Parsed_Message_Access_Service_Info(MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_MAPM_Free_Parsed_Message_Access_Service_Info_t)(MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Message Access Server device.  The */
   /* first parameter to this function specifies the connection type to */
   /* make (either Notification or Message Access).  The                */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The InstancedID    */
   /* member *MUST* specify the Remote Instance ID of the remote MAP    */
   /* server that is to be connected with.  The ConnectionFlags         */
   /* parameter specifies whether authentication or encryption should be*/
   /* used to create this connection.  The CallbackFunction is the      */
   /* function that will be registered for all future events regarding  */
   /* this connection.  The CallbackParameter is a parameter which will */
   /* be included in every callback.  This function returns zero if     */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Message Access Manager Event Callback supplied.          */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Connect_Remote_Device(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned int InstanceID, unsigned long ConnectionFlags, MAPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Connect_Remote_Device_t)(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned int InstanceID, unsigned long ConnectionFlags, MAPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function exists to close an active Message Access   */
   /* connection that was previously opened by a successful call to     */
   /* MAPM_Connect_Server() function or by a metConnectServer.          */
   /* This function accepts the RemoteDeviceAddress. The                */
   /* InstanceID parameter specifies which server instance to use.      */
   /* The ConnectionType parameter indicates what type of connection    */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
   /* * NOTE * Since there can only be one notification connection      */
   /*          between two devices, a call to this function with a      */
   /*          notification connectionType will not necessarily close   */
   /*          the connection. Once all instances close their           */
   /*          connections, a metDisconnected event will signify the    */
   /*          connection is down.                                      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Disconnect(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Disconnect_t)(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
#endif

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding MAPM profile client or notification client request.   */
   /* This function accepts as input the connection type of the remote  */
   /* connection, followed by the remote device address of the device to*/
   /* abort the current operation, followed by the InstanceID parameter.*/
   /* Together these parameters specify which connection is to have the */
   /* Abort issued.  This function returns zero if successful, or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Abort(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Abort_t)(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
#endif

   /* Message Access Client (MCE) Functions.                            */

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current folder.  The first parameter is the  */
   /* Bluetooth address of the device whose connection we are querying. */
   /* The InstanceID parameter specifies which server instance on the   */
   /* remote device to use.  The second parameter is the size of the    */
   /* Buffer that is available to store the current path.  The final    */
   /* parameter is the buffer to copy the path in to.  This function    */
   /* returns a positive (or zero) value representing the total length  */
   /* of the path string (excluding the NULL character) if successful   */
   /* and a negative return error code if there was an error.           */
   /* * NOTE * If the current path is at root, then the Buffer will     */
   /*          contain an empty string and the length will be zero.     */
   /* * NOTE * If the supplied buffer was not large enough to hold the  */
   /*          returned size, it will still be NULL-terminated but will */
   /*          not contain the complete path.                           */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Query_Current_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int BufferSize, char *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Query_Current_Folder_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int BufferSize, char *Buffer);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to create and enable a Notification server for a specified*/
   /* connection.  The RemoteDeviceAddress parameter specifies what     */
   /* connected device this server should be associated with.  The      */
   /* InstanceID parameter specifies which server instance on the remote*/
   /* device to use.  The ServerPort parameter is the local RFCOMM port */
   /* on which to open the server.  The Callback Function and Parameter */
   /* will be called for all events related to this notification server.*/
   /* * NOTE * A successful call to this function does not indicate that*/
   /*          notifications have been succesfully enabled.  The caller */
   /*          should check the result of the                           */
   /*          metMAPEnableNotificationsResponse event.                 */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Enable_Notifications(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Enabled);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Enable_Notifications_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Enabled);
#endif

   /* The following function generates a MAP Set Folder Request to the  */
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  The PathOption*/
   /* parameter contains an enumerated value that indicates the type of */
   /* path change to request.  The FolderName parameter contains the    */
   /* folder name to include with this Set Folder request.  This value  */
   /* can be NULL if no name is required for the selected PathOption.   */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Set_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Set_Folder_Option_t PathOption, char *FolderName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Set_Folder_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Set_Folder_Option_t PathOption, char *FolderName);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules set the folder to an absolute path.  This function        */
   /* generates a sequence of MAP Set Folder Requests, navigating to the*/
   /* supplied path.  The RemoteDeviceAddress is the address of the     */
   /* remote server.  The InstanceID parameter specifies which server   */
   /* instance on the remote device to use.  The FolderName parameter is*/
   /* a string containing containg a path from the root to the desired  */
   /* folder (i.e. telecom/msg/inbox).  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
   /* * NOTE * If an error occurs during one of the chained request, the*/
   /*          confirmation event will note the status and will contain */
   /*          the current path left from the last successful request.  */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Set_Folder_Absolute(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Set_Folder_Absolute_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName);
#endif

   /* The following function generates a MAP Get Folder Listing Request */
   /* to the specified remote MAP Server. The RemoteDeviceAddress       */
   /* is the address of the remote server. The InstanceID parameter     */
   /* specifies which server instance on the remote device to use.      */
   /* The MaxListCount is positive, non-zero integer representing the   */
   /* maximum amount of folder entries to return. The ListStartOffset   */
   /* signifies an offset to request. This function returns zero if     */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Get_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Get_Folder_Listing_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to simply request the size of folder listing. It accepts  */
   /* as a parameter the address of the remote server. The InstanceID   */
   /* parameter specifies which server instance on the remote device    */
   /* to use. This function returns zero if successful and a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Get_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Get_Folder_Listing_Size_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
#endif

   /* The following function generates a MAP Get Message Listing Request*/
   /* to the specified remote MAP Server.  The RemoteDeviceAddress is   */
   /* the address of the remote server.  The InstanceID parameter       */
   /* specifies which server instance on the remote device to use.  The */
   /* FolderName parameter specifies the direct sub-directory to pull   */
   /* the listing from.  If this is NULL, the listing will be from the  */
   /* current directory.  The MaxListCount is a positive, non-zero      */
   /* integer representing the maximum amount of folder entries to      */
   /* return.  The ListStartOffset signifies an offset to request.  The */
   /* ListingInfo parameter is an optional parameter which, if          */
   /* specified, points to a structure which contains a set of filters  */
   /* to use when pulling the listing.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Get_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Get_Message_Listing_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to simply request the size of a message listing.  It      */
   /* accepts as a parameter the address of the remote server and the   */
   /* folder name from which to pull the listing.  A value of NULL      */
   /* indicates the current folder should be used.  The InstanceID      */
   /* parameter specifies which server instance on the remote device to */
   /* use.  The ListingInfo parameter is an optional parameter which, if*/
   /* specified, points to a structure which contains a set of filters  */
   /* to use when pulling the listing.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* return error code if there was an error.                          */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Get_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, MAP_Message_Listing_Info_t *ListingInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Get_Message_Listing_Size_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, MAP_Message_Listing_Info_t *ListingInfo);
#endif

   /* The following function generates a MAP Get Message Request to     */
   /* the specified remote MAP Server. The RemoteDeviceAddress is       */
   /* the address of the remote server. The InstanceID parameter        */
   /* specifies which server instance on the remote device to use. The  */
   /* MessageHandle is a 16 byte NULL terminated string containing      */
   /* Unicode hexadecimal characters which identifies the message.      */
   /* The Attachment parameter indicates whether any attachments to     */
   /* the message should be included in the response. The CharSet and   */
   /* FractionalType parameters specify the format of the response. This*/
   /* function returns zero if successful and a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Get_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Get_Message_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType);
#endif

   /* The following function generates a MAP Set Message Status Request */
   /* to the specified remote MAP Server.  The RemoteDeviceAddress is   */
   /* the address of the remote server.  The InstanceID parameter       */
   /* specifies which server instance on the remote device to use.  The */
   /* MessageHandle is a 16 byte NULL terminated string containing      */
   /* Unicode hexadecimal characters which identifies the message.  The */
   /* StatusIndicator signifies which indicator to be set.  The         */
   /* StatusValue is the value to set.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Set_Message_Status(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Set_Message_Status_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue);
#endif

   /* The following function generates a MAP Push Message Request to the*/
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  The FolderName*/
   /* parameter specifies the direct sub-directory to pull the listing  */
   /* from.  If this is NULL, the listing will be from the current      */
   /* directory.  The Transparent parameter indicates whether a copy    */
   /* should be placed in the sent folder.  The Retry parameter         */
   /* indicates if any retries should be attempted if sending fails.    */
   /* The CharSet specifies the format of the message.  The DataLength  */
   /* parameter indicates the length of the supplied data.  The         */
   /* DataBuffer parameter is a pointer to the data to send.  The Final */
   /* parameter indicates if the buffer supplied is all of the data to  */
   /* be sent.  This function returns zero if successful and a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Push_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Push_Message_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);
#endif

   /* The following function generates a MAP Update Inbox Request to the*/
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the response result to         */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAPM request active at */
   /*          any one time.  Because of this, another MAPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the MAPM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the MAPM event callback*/
   /*          that was registered when the MAPM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Update_Inbox(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Update_Inbox_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
#endif

   /* Message Access Server (MSE) Functions.                            */

   /* The following function generates a MAP Set Notification           */
   /* Registration Response to the specified remote MAP Client.  The    */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode Parameter is the OBEX Response   */
   /* Code to send with the response.  This function returns zero if    */
   /* successful and a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI MAPM_Enable_Notifications_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Enable_Notifications_Confirmation_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);
#endif

   /* The following function generates a MAP Set Folder Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Set_Folder_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Set_Folder_Confirmation_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);
#endif

   /* The following function generates a MAP Folder Listing Response to */
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The DataLength parameter specifies the length of the   */
   /* data.  The Buffer contains the data to be sent.  This function    */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Send_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, unsigned int FolderListingLength, Byte_t *FolderListing, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Send_Folder_Listing_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, unsigned int FolderListingLength, Byte_t *FolderListing, Boolean_t Final);
#endif

   /* The following function generates a Folder Listing Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The Size of the size of the listing to return.  This   */
   /* function returns zero if successful and a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Send_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t FolderCount);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Send_Folder_Listing_Size_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t FolderCount);
#endif

   /* The following function generates a MAP Message Listing Response to*/
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The MessageCount supplies the number of messages in the*/
   /* listing.  The NewMessage parameter indicates if there are new     */
   /* messages since the last pull.  CurrentTime indicates the time of  */
   /* the response.  The DataLength parameter specifies the length of   */
   /* the data.  The Buffer contains the data to be sent.  This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Send_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime, unsigned int MessageListingLength, Byte_t *MessageListing, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Send_Message_Listing_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime, unsigned int MessageListingLength, Byte_t *MessageListing, Boolean_t Final);
#endif

   /* The following function generates a MAP Message Listing Size       */
   /* Response to the specified remote MAP Client.  The                 */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode parameter is the OBEX Response   */
   /* Code to send with the response.  The Size parameter is the size of*/
   /* the message listing to return.  This function returns zero if     */
   /* successful and a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI MAPM_Send_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Send_Message_Listing_Size_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime);
#endif

   /* The following function generates a MAP Get Message Response       */
   /* Response to the specified remote MAP Client.  The                 */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode parameter is the OBEX Response   */
   /* Code to send with the response.  FractionalType indicates what    */
   /* sort of framented response this is.  The DataLength parameter     */
   /* specifies the length of the data.  The Buffer contains the data to*/
   /* be sent.  This function returns zero if successful and a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * Specifying the FractionalType as ftUnfragmented causes no*/
   /*          FractionalType Header to be added to the OBEX Header     */
   /*          List.  This is the value that should be specified for a a*/
   /*          message that is non-fragmented.  Note that if the Get    */
   /*          Message Indication specified a non-fragmented            */
   /*          FractionalType then you must respond with the correct    */
   /*          non-fragmented FractionalType (i.e. ftMore or ftLast).   */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Send_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, MAP_Fractional_Type_t FractionalType, unsigned int MessageLength, Byte_t *Message, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Send_Message_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, MAP_Fractional_Type_t FractionalType, unsigned int MessageLength, Byte_t *Message, Boolean_t Final);
#endif

   /* The following function generates a MAP Set Message Status Response*/
   /* to the specified remote MAP Client.  The RemoteDeviceAddress is   */
   /* the address of the remote client.  The InstanceID parameter       */
   /* specifies which server instance on the local device to use.  The  */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Set_Message_Status_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Set_Message_Status_Confirmation_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);
#endif

   /* The following function generates a MAP Push Message Response to   */
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The message handle is the handle for the client to     */
   /* refer to the message.  This function returns zero if successful   */
   /* and a negative return error code if there was an error.           */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Push_Message_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, char *MessageHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Push_Message_Confirmationt)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, char *MessageHandle);
#endif

   /* The following function generates a MAP Update Inbox Response to   */
   /* the specified remote MAP Client. The RemoteDeviceAddress is the   */
   /* address of the remote client. The InstanceID parameter specifies  */
   /* which server instance on the local device to use. The ResponseCode*/
   /* parameter is the OBEX Response Code to send with the response.    */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Update_Inbox_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Update_Inbox_Confirmation_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);
#endif

   /* Message Access Server (MSE) Notification Functions.               */

   /* The following function generates a MAP Send Event Request to the  */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The DataLength */
   /* Parameter specifies the length of the data.  The Buffer contains  */
   /* the data to be sent.  This function returns zero if successful and*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
BTPSAPI_DECLARATION int BTPSAPI MAPM_Send_Notification(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int DataLength, Byte_t *EventData, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MAPM_Send_Notification_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int DataLength, Byte_t *EventData, Boolean_t Final);
#endif

#endif
