/*****< btpmmapm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMAPM - Message Access Manager for Stonestreet One Bluetooth           */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/03/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMMAPM.h"            /* BTPM MAPM Manager Prototypes/Constants.   */
#include "MAPMAPI.h"             /* MAPM Manager Prototypes/Constants.        */
#include "MAPMMSG.h"             /* BTPM MAPM Manager Message Formats.        */
#include "MAPMGR.h"              /* MAPM Manager Impl. Prototypes/Constants.  */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following constant represents the timeout (in milli-seconds)  */
   /* to wait for a Serial Port to close when it is closed by the local */
   /* host.                                                             */
#define MAXIMUM_MAP_PORT_DELAY_TIMEOUT_MS                      (BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_CLOSE_DELAY_TIME_MS * BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_CLOSE_DELAY_RETRIES)

   /* The following constant represents the number of times that this   */
   /* module will attempt to retry waiting for the Port to Disconnect   */
   /* (before attempting to connect to a remote port) if it is          */
   /* connected.                                                        */
#define MAXIMUM_MAP_PORT_OPEN_DELAY_RETRY                      (BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_RETRIES)

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking connections.                          */
typedef enum
{
   csIdle,
   csAuthorizing,
   csAuthenticating,
   csEncrypting,
   csConnectingWaiting,
   csConnectingDevice,
   csConnecting,
   csConnected
} Connection_State_t;

   /* The following enumerated type is used to denote the current MAP   */
   /* operation that is currently on-going.                             */
typedef enum
{
   coNone,
   coAbort,
   coEnableNotifications,
   coSetFolder,
   coSetFolderAbsolute,
   coGetFolderListing,
   coGetFolderListingSize,
   coGetMessageListing,
   coGetMessageListingSize,
   coGetMessage,
   coSetMessageStatus,
   coPushMessage,
   coUpdateInbox,
   coSendEvent
} Current_Operation_t;

   /* Structure which is used to track connection and port info for this*/
   /* module.                                                           */
typedef struct _tagMAPM_Entry_Info_t
{
   unsigned int                  TrackingID;
   unsigned int                  MAPID;
   unsigned int                  InstanceID;
   unsigned int                  PortNumber;
   unsigned int                  ClientID;
   unsigned long                 ConnectionFlags;
   unsigned long                 Flags;
   unsigned long                 SupportedMessageTypes;
   BD_ADDR_t                     RemoteDeviceAddress;
   Event_t                       ConnectionEvent;
   unsigned int                  ConnectionStatus;
   unsigned int                  ConnectionTimerID;
   unsigned int                  ConnectionTimerCount;
   Connection_State_t            ConnectionState;
   DWord_t                       ServiceRecordHandle;
   Current_Operation_t           CurrentOperation;
   char                         *CurrentPath;
   char                         *PendingPath;
   int                           PendingPathOffset;
   Boolean_t                     DataFinal;
   char                          DataMessageHandle[MAP_MESSAGE_HANDLE_LENGTH + 1];
   Byte_t                       *DataBuffer;
   unsigned int                  DataBufferSize;
   unsigned int                  DataBufferSent;
   MAPM_Event_Callback_t         CallbackFunction;
   void                         *CallbackParameter;
   struct _tagMAPM_Entry_Info_t *NextMAPMEntryInfoPtr;
} MAPM_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* MAPM_Entry_Info_t structure to denote various state information.  */
#define MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED            0x00000001
#define MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT                    0x00000010
#define MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION      0x00000020

#define MAPM_ENTRY_INFO_FLAGS_SERVER                           0x80000000

   /* The following enumerated type is used with the DEVM_Status_t      */
   /* structure to denote actual type of the status type information.   */
typedef enum
{
   dstAuthentication,
   dstEncryption,
   dstConnection
} DEVM_Status_Type_t;

   /* The following structure is used to track Notification Server      */
   /* connection state (if the local device is the MCE).                */
typedef struct _tagMAPM_Notification_Info_t
{
   unsigned int  NotificationServerPort;
   unsigned int  FakeSPPPortID;
   unsigned int  NumberOfConnections;
   long          MAPNotificationServerSDPRecord;
   char         *NotificationServiceName;
} MAPM_Notification_Info_t;

#define MAPM_NOTIFICATION_INFO_DATA_SIZE                       (sizeof(MAPM_Notification_Info_t))

   /* Internal Variables to this Module (remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current power state of the device.       */
static Boolean_t CurrentPowerState;

   /* The following variables hold the configurable parameters for this */
   /* module (and the current values of the parameters).                */
static MAPM_Notification_Info_t NotificationInfo;

   /* Variable which is used to hold the next (unique) tracking ID.     */
static unsigned int NextTrackingID;

   /* Variable which holds a pointer to the first element in the Message*/
   /* Access Info list (which stores information about all open ports   */
   /* and connections).                                                 */
static MAPM_Entry_Info_t *MAPMEntryInfoList;

   /* Variable which holds a pointer to the first element in the        */
   /* Notification Message Access Info list (which stores information   */
   /* about all open Notification connections).                         */
static MAPM_Entry_Info_t *MAPMEntryInfoList_Notification;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextTrackingID(void);

static MAPM_Entry_Info_t *AddMAPMEntryInfoEntry(MAPM_Entry_Info_t **ListHead, MAPM_Entry_Info_t *EntryToAdd);
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByTrackingID(MAPM_Entry_Info_t **ListHead, unsigned int TrackingID);
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByConnection(MAPM_Entry_Info_t **ListHead, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Server);
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByMAPID(MAPM_Entry_Info_t **ListHead, unsigned int MAPID);
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByServerInstanceID(MAPM_Entry_Info_t **ListHead, unsigned int InstanceID);
static MAPM_Entry_Info_t *DeleteMAPMEntryInfoEntry(MAPM_Entry_Info_t **ListHead, unsigned int TrackingID);
static void FreeMAPMEntryInfoEntryMemory(MAPM_Entry_Info_t *EntryToFree);
static void FreeMAPMEntryInfoList(MAPM_Entry_Info_t **ListHead);

static void CleanupLocalNotificationServer(MAPM_Notification_Info_t *NotificationInfo);
static int HandleNotificationEnableRequest(MAPM_Entry_Info_t *MAPEntry, MAPM_Notification_Info_t *NotificationInfo);
static Boolean_t DeleteMAPMNotificationEntry(BD_ADDR_t BD_ADDR);
static void HandleNotificationDisconnection(MAPM_Entry_Info_t *MAPEntry, MAPM_Notification_Info_t *NotificationInfo);

static void CleanupMAPMEntryInfo(MAPM_Entry_Info_t *EntryToCleanup);
static Word_t *ConvertUTF8ToUnicode(char *UTFString);
static char *ConvertUnicodeToUTF8(Word_t *UnicodeString);

static Boolean_t ServiceRecordContainsServiceClass(SDP_Service_Attribute_Response_Data_t *ServiceRecord, UUID_128_t ServiceClass);
static SDP_Data_Element_t *FindSDPAttribute(SDP_Service_Attribute_Response_Data_t *ServiceRecord, Word_t AttributeID);
static int ConvertSDPDataElementToUUID128(SDP_Data_Element_t DataElement, UUID_128_t *UUID);

static Boolean_t BuildPendingFolder(MAP_Set_Folder_Option_t PathOption, Word_t *FolderName, char *CurrentPath, char **PendingFolder);

static Boolean_t MapResponseStatusCodeToResponseCode(unsigned int ResponseStatusCode, Byte_t *ResponseCode);
static unsigned int MapResponseCodeToResponseStatusCode(Byte_t ResponseCode);

static void IssuePendingAbort(MAPM_Entry_Info_t *MAPMEntryInfo);

static void ProcessConnectionResponseMessage(MAPM_Connection_Request_Response_Request_t *Message);
static void ProcessRegisterServerMessage(MAPM_Register_Server_Request_t *Message);
static void ProcessUnRegisterServerMessage(MAPM_Un_Register_Server_Request_t *Message);
static void ProcessRegisterServiceRecordMessage(MAPM_Register_Service_Record_Request_t *Message);
static void ProcessUnRegisterServiceRecordMessage(MAPM_Un_Register_Service_Record_Request_t *Message);
static void ProcessParseRemoteMessageAccessServicesMessage(MAPM_Parse_Remote_Message_Access_Services_Request_t *Message);
static void ProcessConnectRemoteDeviceMessage(MAPM_Connect_Remote_Device_Request_t *Message);
static void ProcessDisconnectMessage(MAPM_Disconnect_Request_t *Message);
static void ProcessAbortMessage(MAPM_Abort_Request_t *Message);
static void ProcessQueryCurrentFolderMessage(MAPM_Query_Current_Folder_Request_t *Message);
static void ProcessEnableNotificationsMessage(MAPM_Enable_Notifications_Request_t *Message);
static void ProcessGetFolderListingMessage(MAPM_Get_Folder_Listing_Request_t *Message);
static void ProcessGetFolderListingSizeMessage(MAPM_Get_Folder_Listing_Size_Request_t *Message);
static void ProcessGetMessageListingMessage(MAPM_Get_Message_Listing_Request_t *Message);
static void ProcessGetMessageListingSizeMessage(MAPM_Get_Message_Listing_Size_Request_t *Message);
static void ProcessGetMessageMessage(MAPM_Get_Message_Request_t *Message);
static void ProcessSetMessageStatusMessage(MAPM_Set_Message_Status_Request_t *Message);
static void ProcessPushMessageMessage(MAPM_Push_Message_Request_t *Message);
static void ProcessUpdateInboxMessage(MAPM_Update_Inbox_Request_t *Message);
static void ProcessSetFolderMessage(MAPM_Set_Folder_Request_t *Message);
static void ProcessSetFolderAbsoluteMessage(MAPM_Set_Folder_Absolute_Request_t *Message);
static void ProcessEnableNotificationsConfirmationMessage(MAPM_Enable_Notifications_Confirmation_Request_t *Message);
static void ProcessSendFolderListingMessage(MAPM_Send_Folder_Listing_Request_t *Message);
static void ProcessSendFolderListingSizeMessage(MAPM_Send_Folder_Listing_Size_Request_t *Message);
static void ProcessSendMessageListingMessage(MAPM_Send_Message_Listing_Request_t *Message);
static void ProcessSendMessageListingSizeMessage(MAPM_Send_Message_Listing_Size_Request_t *Message);
static void ProcessSendMessageMessage(MAPM_Send_Message_Request_t *Message);
static void ProcessSendMessageStatusMessage(MAPM_Message_Status_Confirmation_Request_t *Message);
static void ProcessPushMessageConfirmationMessage(MAPM_Push_Message_Confirmation_Request_t *Message);
static void ProcessUpdateInboxConfirmationMessage(MAPM_Update_Inbox_Confirmation_Request_t *Message);
static void ProcessSetFolderConfirmationMessage(MAPM_Set_Folder_Confirmation_Request_t *Message);
static void ProcessSendNotificationMessage(MAPM_Send_Notification_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessOpenRequestIndicationEvent(MAP_Open_Request_Indication_Data_t *OpenRequestIndicationData);
static void ProcessOpenPortIndicationEvent(MAP_Open_Port_Indication_Data_t *OpenPortIndicationData);
static void ProcessOpenPortConfirmationEvent(MAP_Open_Port_Confirmation_Data_t *OpenPortConfirmationData);
static void ProcessClosePortIndicationEvent(MAP_Close_Port_Indication_Data_t *ClosePortIndicationData);
static void ProcessSendEventIndicationEvent(MAP_Send_Event_Indication_Data_t *SendEventIndicationData);
static void ProcessSendEventConfirmationEvent(MAP_Send_Event_Confirmation_Data_t *SendEventConfirmationData);
static void ProcessNotificationRegistrationIndicationEvent(MAP_Notification_Registration_Indication_Data_t *NotificationRegistrationIndicationData);
static void ProcessNotificationRegistrationConfirmationEvent(MAP_Notification_Registration_Confirmation_Data_t *NotificationRegistrationConfirmationData);
static void ProcessGetFolderListingIndicationEvent(MAP_Get_Folder_Listing_Indication_Data_t *GetFolderListingIndicationData);
static void ProcessGetFolderListingConfirmationEvent(MAP_Get_Folder_Listing_Confirmation_Data_t *GetFolderListingConfirmationData);
static void ProcessGetMessageListingIndicationEvent(MAP_Get_Message_Listing_Indication_Data_t *GetMessageListingIndicationData);
static void ProcessGetMessageListingConfirmationEvent(MAP_Get_Message_Listing_Confirmation_Data_t *GetMessageListingConfirmationData);
static void ProcessGetMessageIndicationEvent(MAP_Get_Message_Indication_Data_t *GetMessageIndicationData);
static void ProcessGetMessageConfirmationEvent(MAP_Get_Message_Confirmation_Data_t *GetMessageConfirmationData);
static void ProcessSetMessageStatusIndicationEvent(MAP_Set_Message_Status_Indication_Data_t *SetMessageStatusIndicationData);
static void ProcessSetMessageStatusConfirmationEvent(MAP_Set_Message_Status_Confirmation_Data_t *SetMessageStatusConfirmationData);
static void ProcessPushMessageIndicationEvent(MAP_Push_Message_Indication_Data_t *PushMessageIndicationData);
static void ProcessPushMessageConfirmationEvent(MAP_Push_Message_Confirmation_Data_t *PushMessageConfirmationData);
static void ProcessUpdateInboxIndicationEvent(MAP_Update_Inbox_Indication_Data_t *UpdateInboxIndicationData);
static void ProcessUpdateInboxConfirmationEvent(MAP_Update_Inbox_Confirmation_Data_t *UpdateInboxConfirmationData);
static void ProcessSetFolderIndicationEvent(MAP_Set_Folder_Indication_Data_t *SetFolderIndicationData);
static void ProcessSetFolderConfirmationEvent(MAP_Set_Folder_Confirmation_Data_t *SetFolderConfirmationData);
static void ProcessAbortIndicationEvent(MAP_Abort_Indication_Data_t *AbortIndicationData);
static void ProcessAbortConfirmationEvent(MAP_Abort_Confirmation_Data_t *AbortConfirmationData);

static void ProcessMAPEvent(MAPM_MAP_Event_Data_t *MAPEventData);

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status);

static void BTPSAPI BTPMDispatchCallback_MAPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MAP(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter);

static void BTPSAPI MessageAccessManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

static void BTPSAPI SPPM_Event_Callback(SPPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique, Tracking ID that can be used to track a MAP    */
   /* Entry (client or server).                                         */
static unsigned int GetNextTrackingID(void)
{
   unsigned int ret_val;

   ret_val = ++NextTrackingID;

   if(NextTrackingID & 0x80000000)
      NextTrackingID = 1;

   return(ret_val);
}

   /* The following function adds the specified entry to the specified  */
   /* list.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the entry passed into this function.   */
   /* This function will return NULL if NO entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* list head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            Tracking ID is the same as an entry already in the     */
   /*            list.  When this occurs, this function returns NULL.   */
static MAPM_Entry_Info_t *AddMAPMEntryInfoEntry(MAPM_Entry_Info_t **ListHead, MAPM_Entry_Info_t *EntryToAdd)
{
   MAPM_Entry_Info_t *AddedEntry = NULL;
   MAPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that the parameters are not NULL.                           */
   if((ListHead) && (EntryToAdd))
   {
      /* Allocate memory for the entry data structure.                  */
      AddedEntry = (MAPM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(MAPM_Entry_Info_t));

      if(AddedEntry)
      {
         /* Copy the entry data to the newly allocated memory.          */
         *AddedEntry = *EntryToAdd;

         AddedEntry->NextMAPMEntryInfoPtr = NULL;

         /* Check if the list is empty.                                 */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Find the last element.                                   */
            while(tmpEntry)
            {
               if(tmpEntry->TrackingID == AddedEntry->TrackingID)
               {
                  /* The entry is already in the list.  Free the memory */
                  /* and and set added entry to NULL.                   */
                  FreeMAPMEntryInfoEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* If there is another entry, point to that entry.  If*/
                  /* not, end the search.                               */
                  if(tmpEntry->NextMAPMEntryInfoPtr)
                     tmpEntry = tmpEntry->NextMAPMEntryInfoPtr;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Add the entry to the end of the list.                 */
               tmpEntry->NextMAPMEntryInfoPtr = AddedEntry;
            }
         }
         else
            *ListHead = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified MAP Entry (based on Tracking ID).  This function returns*/
   /* NULL if either the list head is invalid, the Tracking ID is       */
   /* invalid or the specified entry was NOT found.                     */
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByTrackingID(MAPM_Entry_Info_t **ListHead, unsigned int TrackingID)
{
   MAPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", TrackingID));

   /* Check that the parameters appear to be semi-valid.                */
   if((ListHead) && (TrackingID))
   {
      /* Search the list for the specified entry.                       */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TrackingID != TrackingID))
         FoundEntry = FoundEntry->NextMAPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified connection.  This function returns NULL if either the   */
   /* list head is invalid, the Remote Device is invalid, the Instance  */
   /* ID is invalid, or the specified entry was NOT found.              */
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByConnection(MAPM_Entry_Info_t **ListHead, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Server)
{
   unsigned long      ServerFlags;
   MAPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", InstanceID));

   /* Check if ListHead is not NULL and the remote address appears to be*/
   /* semi-valid.                                                       */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* Note the Server Flags to test against when searching.          */
      ServerFlags = Server?MAPM_ENTRY_INFO_FLAGS_SERVER:0;

      /* Search the list for the specified entry.                       */
      FoundEntry  = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->RemoteDeviceAddress, RemoteDeviceAddress)) || (FoundEntry->InstanceID != InstanceID) || ((FoundEntry->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER) != ServerFlags)))
         FoundEntry = FoundEntry->NextMAPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified MAP ID.  This function returns NULL if either the list  */
   /* head is invalid, the MAP ID is invalid, or the specified MAP ID   */
   /* was NOT found.                                                    */
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByMAPID(MAPM_Entry_Info_t **ListHead, unsigned int MAPID)
{
   MAPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", MAPID));

   /* Check that the parameters appear to be semi-valid.                */
   if((ListHead) && (MAPID))
   {
      /* Search the list for the specified entry.                       */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->MAPID != MAPID))
         FoundEntry = FoundEntry->NextMAPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Server Instance ID.  This function returns NULL if      */
   /* either the list head is invalid, the Instance ID is invalid, or   */
   /* the specified MAP Entry was NOT found.                            */
static MAPM_Entry_Info_t *SearchMAPMEntryInfoByServerInstanceID(MAPM_Entry_Info_t **ListHead, unsigned int InstanceID)
{
   MAPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", InstanceID));

   /* Check that the parameters appear to be semi-valid.                */
   if(ListHead)
   {
      /* Search the list for the specified entry.                       */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!(FoundEntry->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)) || (FoundEntry->InstanceID != InstanceID)))
         FoundEntry = FoundEntry->NextMAPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified MAP entry           */
   /* information list for the specified entry and removes it from the  */
   /* List.  This function returns NULL if either the MAP entry         */
   /* information list head is invalid, the entry is invalid, or the    */
   /* specified entry was NOT present in the list.  The entry returned  */
   /* will have the next entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeMAPMEntryInfoEntryMemory().                           */
static MAPM_Entry_Info_t *DeleteMAPMEntryInfoEntry(MAPM_Entry_Info_t **ListHead, unsigned int TrackingID)
{
   MAPM_Entry_Info_t *FoundEntry = NULL;
   MAPM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that ListHead is not NULL.                                  */
   if(ListHead)
   {
      /* Search the list for the specified entry.                       */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TrackingID != TrackingID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextMAPMEntryInfoPtr;
      }

      /* Check if the specified entry was found.                        */
      if(FoundEntry)
      {
         /* Check if the entry was the first entry in the list.         */
         if(LastEntry)
            LastEntry->NextMAPMEntryInfoPtr = FoundEntry->NextMAPMEntryInfoPtr;
         else
            *ListHead = FoundEntry->NextMAPMEntryInfoPtr;

         FoundEntry->NextMAPMEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* This function frees the specified Message Access entry information*/
   /* member.  No check is done on this entry other than making sure it */
   /* not NULL.                                                         */
static void FreeMAPMEntryInfoEntryMemory(MAPM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and frees all memory of) every    */
   /* element of the specified Message Access entry information list.   */
   /* The list head pointer is set to NULL.                             */
static void FreeMAPMEntryInfoList(MAPM_Entry_Info_t **ListHead)
{
   MAPM_Entry_Info_t *EntryToFree;
   MAPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Traverse the list and free every element.                      */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextMAPMEntryInfoPtr;

         /* Clean up any resources that might have been allocated.      */
         CleanupMAPMEntryInfo(tmpEntry);

         FreeMAPMEntryInfoEntryMemory(tmpEntry);
      }

      /* Set ListHead to NULL.                                          */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is a utility function which is used to     */
   /* cleanup the local Notification Server.                            */
static void CleanupLocalNotificationServer(MAPM_Notification_Info_t *NotificationInfo)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(NotificationInfo)
   {
      /* Un register the local SDP record if possible.                  */
      if(NotificationInfo->MAPNotificationServerSDPRecord)
      {
         _MAP_Un_Register_SDP_Record(0, NotificationInfo->MAPNotificationServerSDPRecord);

         NotificationInfo->MAPNotificationServerSDPRecord = 0;
      }

      /* Clean up the holder SPP Server Port.                           */
      if(NotificationInfo->FakeSPPPortID)
      {
         SPPM_UnRegisterServerPort(NotificationInfo->FakeSPPPortID);

         NotificationInfo->FakeSPPPortID = 0;
      }

      /* Clear the Connection Count.                                    */
      NotificationInfo->NumberOfConnections = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which is used to     */
   /* handle a Notification Enable request sent by the local device (so */
   /* Local device is MCE).  This returns zero on success or a negative */
   /* error code.                                                       */
static int HandleNotificationEnableRequest(MAPM_Entry_Info_t *MAPEntry, MAPM_Notification_Info_t *NotificationInfo)
{
   int                ret_val;
   int                Result;
   long               RecordHandle;
   Boolean_t          ServerPortAllocated;
   Boolean_t          ServerNeeded;
   MAPM_Entry_Info_t  EntryToAdd;
   MAPM_Entry_Info_t *tmpMAPMEntryInfo;
   MAPM_Entry_Info_t *AddedEntry;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((MAPEntry) && (NotificationInfo))
   {
      /* Walk the MAPM connection list and see if any other devices have*/
      /* registered for notifications to this device.                   */
      ServerNeeded     = TRUE;
      tmpMAPMEntryInfo = MAPMEntryInfoList;
      while((tmpMAPMEntryInfo) && (ServerNeeded))
      {
         /* Verify that this is an MCE connection.                      */
         if((tmpMAPMEntryInfo != MAPEntry) && (!(tmpMAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)))
         {
            /* Verify that notifications our enabled for this MCE       */
            /* connection (or an enable is pending).                    */
            if(tmpMAPMEntryInfo->Flags & (MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED | MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION))
            {
               /* Verify that the BD_ADDR of the MCE connection is same */
               /* as BD_ADDR that is attempting to connection.          */
               if(COMPARE_BD_ADDR(tmpMAPMEntryInfo->RemoteDeviceAddress, MAPEntry->RemoteDeviceAddress))
                  ServerNeeded = FALSE;
            }
         }

         tmpMAPMEntryInfo = tmpMAPMEntryInfo->NextMAPMEntryInfoPtr;
      }

      /* Only allocate a MAPM Notification Server if these is the MCE   */
      /* connect to the specific BD_ADDR (REMOTE DEVICE!) that is       */
      /* enabling notifications.                                        */
      if(ServerNeeded)
      {
         /* Flag that we have not yet allocated a server port.          */
         ServerPortAllocated = FALSE;

         /* Now go ahead and register a open a Notification Server and  */
         /* also a SDP record if necessary.                             */
         if(NotificationInfo->NotificationServerPort == 0)
         {
            /* Request a free SPP Port.                                 */
            ret_val = SPPM_FindFreeServerPort();
            if(ret_val > 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Server Port Number: %u.\n", ret_val));

               NotificationInfo->NotificationServerPort = (unsigned int)ret_val;

               ServerPortAllocated                      = TRUE;

               ret_val                                  = 0;
            }
         }
         else
            ret_val = 0;

         /* Continue only if no error has occurred.                     */
         if(!ret_val)
         {
            /* Initialize the Notification Connection entry.            */
            BTPS_MemInitialize(&EntryToAdd, 0, sizeof(MAPM_Entry_Info_t));

            /* Go ahead and register the MAP Notification Server.       */
            ret_val = _MAP_Open_Message_Notification_Server(NotificationInfo->NotificationServerPort, MAPEntry->MAPID);
            if(ret_val > 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Server Port ID: %u.\n", ret_val));

               /* Close any fake SPP Port that we have opened to save   */
               /* our SPP Port #.                                       */
               if(NotificationInfo->FakeSPPPortID)
               {
                  SPPM_UnRegisterServerPort(NotificationInfo->FakeSPPPortID);

                  NotificationInfo->FakeSPPPortID = 0;
               }

               /* Initialize the entry to add to the Notification list. */
               BTPS_MemInitialize(&EntryToAdd, 0, sizeof(MAPM_Entry_Info_t));

               EntryToAdd.TrackingID            = GetNextTrackingID();
               EntryToAdd.ClientID              = 0;
               EntryToAdd.Flags                 = MAPM_ENTRY_INFO_FLAGS_SERVER;
               EntryToAdd.ConnectionState       = csIdle;
               EntryToAdd.ConnectionFlags       = 0;
               EntryToAdd.CurrentOperation      = coNone;
               EntryToAdd.InstanceID            = 0;
               EntryToAdd.PortNumber            = NotificationInfo->NotificationServerPort;
               EntryToAdd.SupportedMessageTypes = 0;
               EntryToAdd.MAPID                 = (unsigned int)ret_val;

               /* Add the Notification Connection Entry.                */
               AddedEntry = AddMAPMEntryInfoEntry(&MAPMEntryInfoList_Notification, &EntryToAdd);
               if(AddedEntry)
               {
                  /* If we haven't already done so register a           */
                  /* Notification Server SDP Record.                    */
                  if(NotificationInfo->MAPNotificationServerSDPRecord == 0)
                  {
                     /* Now go ahead and register the SDP Record.       */
                     RecordHandle = _MAP_Register_Message_Notification_Server_SDP_Record(AddedEntry->MAPID, NotificationInfo->NotificationServiceName);
                     if(RecordHandle > 0)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Server SDP Record: %ld.\n", RecordHandle));

                        /* Save the SDP Record Handle.                  */
                        NotificationInfo->MAPNotificationServerSDPRecord = RecordHandle;

                        /* return success to the caller.                */
                        ret_val                                          = 0;
                     }
                     else
                     {
                        /* Re-open our Fake SPP Port if necessary.      */
                        if(!ServerPortAllocated)
                        {
                           /* Register a fake Notification Server SPP   */
                           /* Port.                                     */
                           Result = SPPM_RegisterServerPort(NotificationInfo->NotificationServerPort, SPPM_REGISTER_SERVER_PORT_FLAGS_REQUIRE_AUTHORIZATION, SPPM_Event_Callback, (void *)NotificationInfo);
                           if(Result > 0)
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Server Port ID: %u.\n", (unsigned int)Result));

                              NotificationInfo->FakeSPPPortID = (unsigned int)Result;
                           }
                           else
                              DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Failed to get SPP Port for Notification Server %d.\n", Result));
                        }

                        /* Close the Notification Server that we just   */
                        /* opened.                                      */
                        _MAP_Close_Server(AddedEntry->MAPID);

                        /* Delete the Notification Connection Entry.    */
                        if((AddedEntry = DeleteMAPMEntryInfoEntry(&MAPMEntryInfoList_Notification, AddedEntry->TrackingID)) != NULL)
                           FreeMAPMEntryInfoEntryMemory(AddedEntry);

                        /* Return the error.                            */
                        ret_val = (int)RecordHandle;
                     }
                  }
                  else
                  {
                     /* SDP record already register so just return      */
                     /* success to the caller.                          */
                     ret_val = 0;
                  }

                  /* Continue only if no error has occurred.            */
                  if(!ret_val)
                  {
                     /* Increment the reference count for number of     */
                     /* notification connections.                       */
                     NotificationInfo->NumberOfConnections++;
                  }
               }
               else
               {
                  /* Re-open our Fake SPP Port if necessary.            */
                  if(!ServerPortAllocated)
                  {
                     /* Register a fake Notification Server SPP Port.   */
                     Result = SPPM_RegisterServerPort(NotificationInfo->NotificationServerPort, SPPM_REGISTER_SERVER_PORT_FLAGS_REQUIRE_AUTHORIZATION, SPPM_Event_Callback, (void *)NotificationInfo);
                     if(Result > 0)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Server Port ID: %u.\n", (unsigned int)Result));

                        NotificationInfo->FakeSPPPortID = (unsigned int)Result;
                     }
                     else
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Failed to get SPP Port for Notification Server %d.\n", Result));
                  }

                  /* Close the Notification Server that we just opened. */
                  _MAP_Close_Server(EntryToAdd.MAPID);

                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
            }
         }

         /* If we failed to open the notification server and we         */
         /* allocated a Server Port Number then we need to un-allocate  */
         /* it.                                                         */
         if((ret_val) && (ServerPortAllocated))
            NotificationInfo->NotificationServerPort = 0;
      }
      else
      {
         /* Nothing to do except return success.                        */
         ret_val = 0;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* delete MAPM Notification Entries by BD_ADDR.                      */
static Boolean_t DeleteMAPMNotificationEntry(BD_ADDR_t BD_ADDR)
{
   Boolean_t          ret_val = FALSE;
   MAPM_Entry_Info_t *EntryToDelete;
   MAPM_Entry_Info_t *TempEntry;

   /* Walk the list to find the notification server that we should      */
   /* disconnect.                                                       */
   EntryToDelete = MAPMEntryInfoList_Notification;
   while((EntryToDelete) && (!ret_val))
   {
      /* Verify that this is a notification server.                     */
      if(EntryToDelete->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
      {
         TempEntry     = EntryToDelete;

         EntryToDelete = EntryToDelete->NextMAPMEntryInfoPtr;

         if(COMPARE_BD_ADDR(TempEntry->RemoteDeviceAddress, BD_ADDR))
         {
            /* Delete the entry from the list.                          */
            if((TempEntry = DeleteMAPMEntryInfoEntry(&MAPMEntryInfoList_Notification, TempEntry->TrackingID)) != NULL)
            {
               /* Close the server allocated for this entry.            */
               _MAP_Close_Server(TempEntry->MAPID);

               /* Return success.                                       */
               ret_val = TRUE;

               /* Cleanup resources.                                    */
               CleanupMAPMEntryInfo(TempEntry);
               FreeMAPMEntryInfoEntryMemory(TempEntry);
            }
         }
      }
      else
         EntryToDelete = EntryToDelete->NextMAPMEntryInfoPtr;
   }

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* handle Notification Disconnections.                               */
static void HandleNotificationDisconnection(MAPM_Entry_Info_t *MAPEntry, MAPM_Notification_Info_t *NotificationInfo)
{
   int                Result;
   BD_ADDR_t          NullAddr;
   Boolean_t          Disconnected;
   MAPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((MAPEntry) && (NotificationInfo))
   {
      /* Assign a NULL BD_ADDR for comparison purposes.                 */
      ASSIGN_BD_ADDR(NullAddr, 0, 0, 0, 0, 0, 0);

      /* Walk the list of MCE connections and see if any other          */
      /* connections have enabled notifications to this device.         */
      tmpEntry     = MAPMEntryInfoList;
      Disconnected = TRUE;
      while((tmpEntry) && (Disconnected))
      {
         /* Verify that this is not same connection and that it is a MCE*/
         /* connection.                                                 */
         if((tmpEntry != MAPEntry) && (!(tmpEntry->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)))
         {
            /* Verify that the BD_ADDR is the same.                     */
            if(COMPARE_BD_ADDR(tmpEntry->RemoteDeviceAddress, MAPEntry->RemoteDeviceAddress))
            {
               /* If this MCE connection has enabled notifications or is*/
               /* pending enable notifications we should NOT close an   */
               /* open Notification Server.                             */
               if(tmpEntry->Flags & (MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED | MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION))
                  Disconnected = FALSE;
            }
         }

         tmpEntry = tmpEntry->NextMAPMEntryInfoPtr;
      }

      /* If we actually disconnected completly do some cleanup.         */
      if(Disconnected)
      {
         /* Verify that we can delete a open notification entry.        */
         if(((DeleteMAPMNotificationEntry(MAPEntry->RemoteDeviceAddress)) || (DeleteMAPMNotificationEntry(NullAddr))))
         {
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Number of Current local Notification Servers: %u.\n", (unsigned int)NotificationInfo->NumberOfConnections));

            /* If the number of Notification Servers just hit zero we   */
            /* need to un-register the SDP record and open a fake SPP   */
            /* Port.                                                    */
            if(--(NotificationInfo->NumberOfConnections) == 0)
            {
               /* Register a fake Notification Server SPP Port.         */
               Result = SPPM_RegisterServerPort(NotificationInfo->NotificationServerPort, SPPM_REGISTER_SERVER_PORT_FLAGS_REQUIRE_AUTHORIZATION, SPPM_Event_Callback, (void *)NotificationInfo);
               if(Result > 0)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Fake SPP Port ID: %u.\n", (unsigned int)Result));

                  NotificationInfo->FakeSPPPortID = (unsigned int)Result;
               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Failed to get SPP Port for Notification Server %d.\n", Result));

               /* Un-register our SDP MAP Notification Server SDP       */
               /* Record.                                               */
               _MAP_Un_Register_SDP_Record(MAPEntry->MAPID, NotificationInfo->MAPNotificationServerSDPRecord);

               NotificationInfo->MAPNotificationServerSDPRecord = 0;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to free  */
   /* any resources that might have been allocated for the specified MAP*/
   /* Entry.  This Entry *DOES NOT* free the Entry itself, just any     */
   /* resources that have not been freed by the Entry.                  */
static void CleanupMAPMEntryInfo(MAPM_Entry_Info_t *EntryToCleanup)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToCleanup)
   {
      /* Close any currently executing timer.                           */
      if(EntryToCleanup->ConnectionTimerID)
         TMR_StopTimer(EntryToCleanup->ConnectionTimerID);

      /* Free any Events that might have been allocated.                */
      if(EntryToCleanup->ConnectionEvent)
         BTPS_CloseEvent(EntryToCleanup->ConnectionEvent);

      /* If there is a current path that has been stored, we need to    */
      /* free it at this time.                                          */
      if(EntryToCleanup->CurrentPath)
         BTPS_FreeMemory(EntryToCleanup->CurrentPath);

      if(EntryToCleanup->PendingPath)
         BTPS_FreeMemory(EntryToCleanup->PendingPath);

      /* If there was an on-going transfer we need to clean this up as  */
      /* well.                                                          */
      if(EntryToCleanup->DataBufferSent)
         BTPS_FreeMemory(EntryToCleanup->DataBuffer);

      /* Flag the resources that were freed (above) as not being        */
      /* present.                                                       */
      EntryToCleanup->ConnectionEvent   = NULL;
      EntryToCleanup->CurrentPath       = NULL;
      EntryToCleanup->PendingPath       = NULL;
      EntryToCleanup->DataBuffer        = NULL;
      EntryToCleanup->ConnectionTimerID = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* convert the specified string from UTF-8 to Unicode.  This function*/
   /* allocates an array of Word_t's to hold the string and returns the */
   /* string (if successful).  If un-successful this function returns   */
   /* NULL.  If this function is successful (i.e. returns non-NULL, it  */
   /* is the callers responsiblity to to free the memory allocated using*/
   /* the BTPS_FreeMemory() function.                                   */
   /* * NOTE * The returned string is NULL terminated (i.e. ends with   */
   /*          0x0000).                                                 */
static Word_t *ConvertUTF8ToUnicode(char *UTFString)
{
   Word_t       *ret_val;
   unsigned int  StringLength;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((UTFString) && ((StringLength = (BTPS_StringLength(UTFString) + 1)) != 1))
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Converting: %s\n", UTFString));

      if((ret_val = (Word_t *)BTPS_AllocateMemory(StringLength*sizeof(Word_t))) != NULL)
      {
         /* Build the Unicode string.                                   */
         /* * NOTE * We will currently just convert the string directly */
         /*          from ASCII to Unicode (as ASCII).                  */
         while(StringLength--)
            ret_val[StringLength] = (Word_t)UTFString[StringLength];
      }
   }
   else
      ret_val = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the specified string from Unicode to UTF-8.  This function*/
   /* allocates an array of char's to hold the string and returns the   */
   /* string (if successful).  If un-successful this function returns   */
   /* NULL.  If this function is successful (i.e. returns non-NULL, it  */
   /* is the callers responsiblity to to free the memory allocated using*/
   /* the BTPS_FreeMemory() function.                                   */
   /* * NOTE * The returned string is NULL terminated (i.e.  ends with  */
   /*          '\0').                                                   */
static char *ConvertUnicodeToUTF8(Word_t *UnicodeString)
{
   char         *ret_val;
   unsigned int  StringLength;
   unsigned int  Index;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(UnicodeString)
   {
      /* Determine the string length.                                   */
      StringLength = 0;
      while(UnicodeString[StringLength])
      {
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unicode String: 0x%04X (%c)\n", UnicodeString[StringLength], (char)(UnicodeString[StringLength])));

         StringLength++;
      }

      if(StringLength)
      {
         if((ret_val = (char *)BTPS_AllocateMemory(StringLength+1)) != NULL)
         {
            /* Build the UTF-8 string.                                  */
            /* * NOTE * We will currently just convert the string       */
            /*          directly from Unicode to ASCII (as ASCII).      */
            for(Index=0;Index<StringLength;Index++)
               ret_val[Index] = (char)(UnicodeString[Index]);

            ret_val[Index] = '\0';

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Converted: %s\n", ret_val));
         }
      }
      else
         ret_val = NULL;
   }
   else
      ret_val = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* determine whether a service record contains a given Service Class */
   /* UUID. This function returns TRUE if the given Service Class UUID  */
   /* was found in the Service Class List attribute of the service      */
   /* record.                                                           */
static Boolean_t ServiceRecordContainsServiceClass(SDP_Service_Attribute_Response_Data_t *ServiceRecord, UUID_128_t ServiceClass)
{
   Boolean_t           ret_val;
   UUID_128_t          RecordServiceClass;
   unsigned int        ServiceClassIndex;
   SDP_Data_Element_t *ServiceClassAttribute;

   ret_val = FALSE;

   if(ServiceRecord)
   {
      if((ServiceClassAttribute = FindSDPAttribute(ServiceRecord, SDP_ATTRIBUTE_ID_SERVICE_CLASS_ID_LIST)) != NULL)
      {
         /* The Service Class Attribute has been located. Make sure that*/
         /* it contains a Sequence, as required.                        */
         if(ServiceClassAttribute->SDP_Data_Element_Type == deSequence)
         {
            /* The attribute looks valid. Now search for the appropriate*/
            /* Service Class UUID.                                      */
            for(ServiceClassIndex = 0; ServiceClassIndex < ServiceClassAttribute->SDP_Data_Element_Length; ServiceClassIndex++)
            {
               /* Normalize the Service Class to a 128-bit UUID for     */
               /* comparison.                                           */
               if(ConvertSDPDataElementToUUID128(ServiceClassAttribute->SDP_Data_Element.SDP_Data_Element_Sequence[ServiceClassIndex], &RecordServiceClass) == 0)
               {
                  /* The Service Class UUID was successfully located, so*/
                  /* see if it is the Service Class we are looking for. */
                  if(COMPARE_UUID_128(RecordServiceClass, ServiceClass))
                  {
                     /* The Service Class UUID has been found in the    */
                     /* current service record.                         */
                     ret_val = TRUE;
                     break;
                  }
               }
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to locate*/
   /* a particular Attribute within a parsed SDP record. This function  */
   /* returns a pointer to the Attribute data, or NULL if the attribute */
   /* does not exist in the given SDP record.                           */
static SDP_Data_Element_t *FindSDPAttribute(SDP_Service_Attribute_Response_Data_t *ServiceRecord, Word_t AttributeID)
{
   unsigned int        AttributeIndex;
   SDP_Data_Element_t *AttributePtr = NULL;

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(ServiceRecord)
   {
      /* Loop through all available attributes in the record to find the*/
      /* requested attribute.                                           */
      for(AttributeIndex=0;AttributeIndex<(unsigned int)ServiceRecord->Number_Attribute_Values;AttributeIndex++)
      {
         /* Check whether we have found the requested attribute.        */
         if(ServiceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == AttributeID)
         {
            /* The attribute has been found. Save the attribute and stop*/
            /* searching the record.                                    */
            AttributePtr = ServiceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;
            break;
         }
      }
   }

   return(AttributePtr);
}

   /* The following function is a utility function that exists to       */
   /* convert an SDP Data Element, which contains a UUID, into a 128-bit*/
   /* UUID. This function returns zero if successful, or a negative     */
   /* error code on failure.                                            */
static int ConvertSDPDataElementToUUID128(SDP_Data_Element_t DataElement, UUID_128_t *UUID)
{
   int ret_val;

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UUID)
   {
      switch(DataElement.SDP_Data_Element_Type)
      {
         case deUUID_128:
            *UUID   = DataElement.SDP_Data_Element.UUID_128;
            ret_val = 0;
            break;
         case deUUID_32:
            SDP_ASSIGN_BASE_UUID(*UUID);
            ASSIGN_SDP_UUID_32_TO_SDP_UUID_128(*UUID, DataElement.SDP_Data_Element.UUID_32);
            ret_val = 0;
            break;
         case deUUID_16:
            SDP_ASSIGN_BASE_UUID(*UUID);
            ASSIGN_SDP_UUID_16_TO_SDP_UUID_128(*UUID, DataElement.SDP_Data_Element.UUID_16);
            ret_val = 0;
            break;
         default:
            /* No other data type is allowed for this parameter, so     */
            /* return an error.                                         */
            ASSIGN_SDP_UUID_128(*UUID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            break;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is a utility function that exists create   */
   /* the new Pending Folder given the current folder/path, as well as  */
   /* the options that are to be applied.  This function returns TRUE if*/
   /* the Folder was able to be built successfully, or FALSE if there   */
   /* was an error building the Pending Folder.                         */
   /* * NOTE * If this function returns TRUE then PendingFolder will    */
   /*          contain a pointer to the new current pending folder      */
   /*          (note this pointer could be NULL if the new current      */
   /*          pending folder is root).                                 */
static Boolean_t BuildPendingFolder(MAP_Set_Folder_Option_t PathOption, Word_t *FolderName, char *CurrentPath, char **PendingFolder)
{
   int        Index;
   char      *Path;
   Boolean_t  ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(PendingFolder)
   {
      /* Flag success.                                                  */
      ret_val = TRUE;

      /* * NOTE * Here are the options to test for PathOption:          */
      /*             - sfRoot - FolderName ignored                      */
      /*             - sfDown - FolderName required                     */
      /*             - sfUp   - FolderName optional                     */
      if(PathOption == sfRoot)
         *PendingFolder = NULL;
      else
      {
         if(PathOption == sfDown)
         {
            if((FolderName) && ((Path = ConvertUnicodeToUTF8(FolderName)) != NULL))
            {
               /* OK, now we need to append the current path to the     */
               /* current path.                                         */
               if((*PendingFolder = BTPS_AllocateMemory((CurrentPath?BTPS_StringLength(CurrentPath):0) + BTPS_StringLength(Path) + 2)) != NULL)
               {
                  /* Memory allocated, build the path.                  */
                  if((CurrentPath) && (BTPS_StringLength(CurrentPath)))
                  {
                     BTPS_StringCopy(*PendingFolder, CurrentPath);

                     if(CurrentPath[BTPS_StringLength(CurrentPath) - 1] != '/')
                        BTPS_StringCopy(&((*PendingFolder)[BTPS_StringLength(*PendingFolder)]), "/");
                  }
                  else
                     (*PendingFolder)[0] = '\0';

                  BTPS_StringCopy(&((*PendingFolder)[BTPS_StringLength(*PendingFolder)]), Path);
               }
               else
                  ret_val = FALSE;

               /* Free the memory that was allocated for the UTF8       */
               /* conversion of the folder.                             */
               BTPS_FreeMemory(Path);
            }
            else
               ret_val = FALSE;
         }
         else
         {
            /* Move up a directory.                                     */
            if((CurrentPath) && (BTPS_StringLength(CurrentPath)))
            {
               Index = BTPS_StringLength(CurrentPath) - 1;
               while((Index >= 0) && (CurrentPath[Index] != '/'))
                  Index--;

               if(Index)
                  Index--;
            }
            else
               Index = 0;

            /* Index holds the length of the current path (to the next  */
            /* higher directory) SANS the delimiter.                    */
            if(FolderName)
            {
               if((Path = ConvertUnicodeToUTF8(FolderName)) == NULL)
                  ret_val = FALSE;
            }
            else
               Path = NULL;

            if((ret_val) && (CurrentPath))
            {
               /* Simply move up a directory.                           */
               if(Index >= 0)
               {
                  if((*PendingFolder = BTPS_AllocateMemory(Index + (Path?BTPS_StringLength(Path):0) + 3)) != NULL)
                  {
                     /* Copy the entire folder over.                    */
                     (*PendingFolder)[Index+1] = '\0';
                     while(Index >= 0)
                     {
                        (*PendingFolder)[Index] = CurrentPath[Index];

                        Index--;
                     }

                     /* Check to see if we need to append a path.       */
                     if(Path)
                     {
                        BTPS_StringCopy(&((*PendingFolder)[BTPS_StringLength(*PendingFolder)]), "/");

                        BTPS_StringCopy(&((*PendingFolder)[BTPS_StringLength(*PendingFolder)]), Path);

                        /* Free the memory that was allocated for the   */
                        /* UTF8 conversion of the folder.               */
                        BTPS_FreeMemory(Path);
                     }
                  }
                  else
                     ret_val = FALSE;
               }
               else
                  *PendingFolder = NULL;
            }
         }
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the Bluetopia PM Response Status Codes to the correct OBEX*/
   /* Response Code.  This function returns TRUE if the mapping was able*/
   /* to made successfully or FALSE if there was invalid parameter.     */
static Boolean_t MapResponseStatusCodeToResponseCode(unsigned int ResponseStatusCode, Byte_t *ResponseCode)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d \n", ResponseStatusCode));

   /* Check to make sure that we have a buffer to store the result into.*/
   if(ResponseCode)
   {
      /* Initialize success.                                            */
      ret_val = TRUE;

      /* Next, map the Status to the correct code.                      */
      switch(ResponseStatusCode)
      {
         case MAPM_RESPONSE_STATUS_CODE_SUCCESS:
            *ResponseCode = MAP_OBEX_RESPONSE_OK;
            break;
         case MAPM_RESPONSE_STATUS_CODE_NOT_FOUND:
            *ResponseCode = MAP_OBEX_RESPONSE_NOT_FOUND;
            break;
         case MAPM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE:
            *ResponseCode = MAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE;
            break;
         case MAPM_RESPONSE_STATUS_CODE_BAD_REQUEST:
            *ResponseCode = MAP_OBEX_RESPONSE_BAD_REQUEST;
            break;
         case MAPM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED:
            *ResponseCode = MAP_OBEX_RESPONSE_NOT_IMPLEMENTED;
            break;
         case MAPM_RESPONSE_STATUS_CODE_UNAUTHORIZED:
            *ResponseCode = MAP_OBEX_RESPONSE_UNAUTHORIZED;
            break;
         case MAPM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED:
            *ResponseCode = MAP_OBEX_RESPONSE_PRECONDITION_FAILED;
            break;
         case MAPM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE:
            *ResponseCode = MAP_OBEX_RESPONSE_NOT_ACCEPTABLE;
            break;
         case MAPM_RESPONSE_STATUS_CODE_FORBIDDEN:
            *ResponseCode = MAP_OBEX_RESPONSE_FORBIDDEN;
            break;
         case MAPM_RESPONSE_STATUS_CODE_SERVER_ERROR:
            *ResponseCode = MAP_OBEX_RESPONSE_SERVER_ERROR;
            break;
         case MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED:
         case MAPM_RESPONSE_STATUS_CODE_DEVICE_POWER_OFF:
         case MAPM_RESPONSE_STATUS_CODE_UNKNOWN:
         default:
            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the OBEX Response to the correct Bluetopia PM Response    */
   /* Status Codes.                                                     */
static unsigned int MapResponseCodeToResponseStatusCode(Byte_t ResponseCode)
{
   unsigned int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d (0x%02X) \n", (int)ResponseCode, (int)ResponseCode));

   /* Next, map the OBEX Response to the correct Status Code.           */
   switch(ResponseCode)
   {
      case MAP_OBEX_RESPONSE_OK:
      case MAP_OBEX_RESPONSE_CONTINUE:
         ret_val = MAPM_RESPONSE_STATUS_CODE_SUCCESS;
         break;
      case MAP_OBEX_RESPONSE_NOT_FOUND:
         ret_val = MAPM_RESPONSE_STATUS_CODE_NOT_FOUND;
         break;
      case MAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE:
         ret_val = MAPM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE;
         break;
      case MAP_OBEX_RESPONSE_BAD_REQUEST:
         ret_val = MAPM_RESPONSE_STATUS_CODE_BAD_REQUEST;
         break;
      case MAP_OBEX_RESPONSE_NOT_IMPLEMENTED:
         ret_val = MAPM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED;
         break;
      case MAP_OBEX_RESPONSE_UNAUTHORIZED:
         ret_val = MAPM_RESPONSE_STATUS_CODE_UNAUTHORIZED;
         break;
      case MAP_OBEX_RESPONSE_PRECONDITION_FAILED:
         ret_val = MAPM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED;
         break;
      case MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:
         ret_val = MAPM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE;
         break;
      case MAP_OBEX_RESPONSE_FORBIDDEN:
         ret_val = MAPM_RESPONSE_STATUS_CODE_FORBIDDEN;
         break;
      case MAP_OBEX_RESPONSE_SERVER_ERROR:
         ret_val = MAPM_RESPONSE_STATUS_CODE_SERVER_ERROR;
         break;
      default:
         ret_val = MAPM_RESPONSE_STATUS_CODE_UNKNOWN;
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* actually format and send an Abort Request to the remote device    */
   /* specified by the Message Access Manager Entry.                    */
static void IssuePendingAbort(MAPM_Entry_Info_t *MAPMEntryInfo)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure the input the parameters appear to be semi-valid.*/
   if((MAPMEntryInfo) && (MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
   {
      /* Simply issue the Abort.                                        */
      _MAP_Abort_Request(MAPMEntryInfo->MAPID);

      /* Clear the Pending Abort flag.                                  */
      MAPMEntryInfo->Flags            &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT);

      /* Clear any current operation.                                   */
      MAPMEntryInfo->CurrentOperation  = coNone;

      /* Finally, clear any queued data (if present).                   */
      if(MAPMEntryInfo->DataBuffer)
      {
         BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

         MAPMEntryInfo->DataBuffer = NULL;
      }

      /* If we were in the middle of changing Path's, go ahead and free */
      /* the pending path operation.                                    */
      if(MAPMEntryInfo->PendingPath)
      {
         BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

         MAPMEntryInfo->PendingPath = NULL;
      }
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("No MAPM Entry or no Pending Abort queued.\n"));

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Message Access     */
   /* connection request response message and responds to the message   */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessConnectionResponseMessage(MAPM_Connection_Request_Response_Request_t *Message)
{
   int                                          Result;
   Boolean_t                                    Authenticate;
   Boolean_t                                    Encrypt;
   MAPM_Entry_Info_t                           *MAPMEntryInfo;
   MAPM_Connection_Request_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Next, determine if the Port is in the correct state.        */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
         {
            /* Verify that this message is coming from the Client that  */
            /* registered the server or opened the connection to the    */
            /* remote device.                                           */
            if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
            {
               if(MAPMEntryInfo->ConnectionState == csAuthorizing)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: 0x%08X (%d), %d\n", Message->InstanceID, Message->InstanceID, Message->Accept));

                  /* If the client has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(Message->Accept)
                  {
                     /* Determine if Authentication and/or Encryption is*/
                     /* required for this link.                         */
                     if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION)
                        Authenticate = TRUE;
                     else
                        Authenticate = FALSE;

                     if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION)
                        Encrypt = TRUE;
                     else
                        Encrypt = FALSE;

                     if((Authenticate) || (Encrypt))
                     {
                        if(Encrypt)
                           Result = DEVM_EncryptRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, 0);
                        else
                           Result = DEVM_AuthenticateRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, 0);
                     }
                     else
                        Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

                     if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                     {
                        /* Authorization not required, and we are       */
                        /* already in the correct state.                */
                        if((Result = _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, TRUE)) != 0)
                        {
                           /* Failure, go ahead and try to disconnect it*/
                           /* (will probably fail as well).             */
                           MAPMEntryInfo->ConnectionState = csIdle;

                           _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                        }
                     }
                     else
                     {
                        /* If we were successfully able to Authenticate */
                        /* and/or Encrypt, then we need to set the      */
                        /* correct state.                               */
                        if(!Result)
                        {
                           if(Encrypt)
                              MAPMEntryInfo->ConnectionState = csEncrypting;
                           else
                              MAPMEntryInfo->ConnectionState = csAuthenticating;

                           /* Flag success.                             */
                           Result = 0;
                        }
                        else
                        {
                           /* Error, reject the request.                */
                           if(_MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE))
                           {
                              /* Failure, go ahead and try to disconnect*/
                              /* it (will probably fail as well).       */
                              MAPMEntryInfo->ConnectionState = csIdle;

                              _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                           }
                        }
                     }
                  }
                  else
                  {
                     /* Rejection - Simply respond to the request.      */
                     _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);

                     MAPMEntryInfo->ConnectionState = csIdle;

                     _MAP_Close_Connection(MAPMEntryInfo->MAPID);

                     /* Flag success.                                   */
                     Result = 0;
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
         }
         else
            Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_NOT_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Message Access     */
   /* register server message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessRegisterServerMessage(MAPM_Register_Server_Request_t *Message)
{
   int                              Result;
   MAPM_Entry_Info_t                MAPMEntryInfo;
   MAPM_Entry_Info_t               *MAPMEntryInfoPtr;
   MAPM_Register_Server_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify that the specified Instance ID appears to be         */
         /* semi-valid.                                                 */
         if((Message->InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (Message->InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE))
         {
            /* First, let's make sure there is NO server that is        */
            /* registered for the same Instance ID.                     */
            if(!SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, Message->InstanceID))
            {
               BTPS_MemInitialize(&MAPMEntryInfo, 0, sizeof(MAPM_Entry_Info_t));

               MAPMEntryInfo.TrackingID            = GetNextTrackingID();
               MAPMEntryInfo.ClientID              = Message->MessageHeader.AddressID;
               MAPMEntryInfo.Flags                 = MAPM_ENTRY_INFO_FLAGS_SERVER;
               MAPMEntryInfo.ConnectionState       = csIdle;
               MAPMEntryInfo.ConnectionFlags       = Message->ServerFlags;
               MAPMEntryInfo.CurrentOperation      = coNone;
               MAPMEntryInfo.InstanceID            = Message->InstanceID;
               MAPMEntryInfo.PortNumber            = Message->ServerPort;
               MAPMEntryInfo.SupportedMessageTypes = Message->SupportedMessageTypes;

               if((MAPMEntryInfoPtr = AddMAPMEntryInfoEntry(&MAPMEntryInfoList, &MAPMEntryInfo)) != NULL)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Register Server Port: %u, Instance: %u, Flags: 0x%08lX\n", Message->ServerPort, Message->InstanceID, Message->ServerFlags));

                  /* Next, attempt to register the Server.              */
                  if((Result = _MAP_Open_Message_Access_Server(Message->ServerPort)) > 0)
                  {
                     /* Note the returned MAP ID.                       */
                     MAPMEntryInfoPtr->MAPID = (unsigned int)Result;

                     /* Flag success.                                   */
                     Result                  = 0;
                  }
                  else
                  {
                     /* Error opening port, go ahead and delete the     */
                     /* entry that was added.                           */
                     if((MAPMEntryInfoPtr = DeleteMAPMEntryInfoEntry(&MAPMEntryInfoList, MAPMEntryInfoPtr->TrackingID)) != NULL)
                     {
                        CleanupMAPMEntryInfo(MAPMEntryInfoPtr);

                        FreeMAPMEntryInfoEntryMemory(MAPMEntryInfoPtr);
                     }
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_DUPLICATE_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_REGISTER_SERVER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;
      ResponseMessage.InstanceID                   = Message->InstanceID;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* un-register server message and responds to the message            */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessUnRegisterServerMessage(MAPM_Un_Register_Server_Request_t *Message)
{
   int                                 Result;
   MAPM_Entry_Info_t                  *MAPMEntryInfo;
   MAPM_Un_Register_Server_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Next, attempt to search for the MAP Server Instance to      */
         /* un-register.                                                */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, Message->InstanceID)) != NULL)
         {
            /* Verify that this message is coming from the Client that  */
            /* registered the server or opened the connection to the    */
            /* remote device.                                           */
            if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
            {
               /* Delete the MAP Server Entry from the MAP Entry List.  */
               if((MAPMEntryInfo = DeleteMAPMEntryInfoEntry(&MAPMEntryInfoList, MAPMEntryInfo->MAPID)) != NULL)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Delete Server: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Reject any incoming connection that might be in    */
                  /* progress.                                          */
                  if((MAPMEntryInfo->ConnectionState == csAuthenticating) || (MAPMEntryInfo->ConnectionState == csAuthorizing) || (MAPMEntryInfo->ConnectionState == csEncrypting))
                     _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);

                  /* If there was a Service Record Registered, go ahead */
                  /* and make sure it is freed.                         */
                  if(MAPMEntryInfo->ServiceRecordHandle)
                     _MAP_Un_Register_SDP_Record(MAPMEntryInfo->MAPID, MAPMEntryInfo->ServiceRecordHandle);

                  /* Next, go ahead and Un-Register the Server.         */
                  _MAP_Close_Server(MAPMEntryInfo->MAPID);

                  /* Clean up any resources that were allocated for this*/
                  /* entry.                                             */
                  CleanupMAPMEntryInfo(MAPMEntryInfo);

                  /* All finished, free any memory that was allocated   */
                  /* for the server.                                    */
                  FreeMAPMEntryInfoEntryMemory(MAPMEntryInfo);

                  /* Flag success.                                      */
                  Result = 0;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
         }
         else
            Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_UN_REGISTER_SERVER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* register server record message and responds to the message        */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessRegisterServiceRecordMessage(MAPM_Register_Service_Record_Request_t *Message)
{
   long                                     Result;
   MAPM_Entry_Info_t                       *MAPMEntryInfo;
   MAPM_Register_Service_Record_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify that a Service name was specified.                   */
         /* * NOTE * The length should include the NULL terminator.     */
         if(Message->ServiceNameLength > 1)
         {
            /* NULL terminate the string, just to be safe.              */
            Message->ServiceName[Message->ServiceNameLength - 1] = '\0';

            /* Next, attempt to find the correct MAP Server Entry from  */
            /* the MAP Entry List.                                      */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, Message->InstanceID)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Register SDP Record : 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Verify that there is no Service Record already     */
                  /* registered.                                        */
                  if(!MAPMEntryInfo->ServiceRecordHandle)
                  {
                     if((Result = _MAP_Register_Message_Access_Server_SDP_Record(MAPMEntryInfo->MAPID, Message->ServiceName, (Byte_t)MAPMEntryInfo->InstanceID, (Byte_t)MAPMEntryInfo->SupportedMessageTypes)) > 0)
                     {
                        /* Success.  Note the handle that was returned. */
                        MAPMEntryInfo->ServiceRecordHandle = (unsigned long)Result;
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_SERVICE_RECORD_ALREADY_REGISTERED;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_REGISTER_SERVICE_RECORD_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;
         ResponseMessage.ServiceRecordHandle       = (unsigned long)Result;
      }
      else
      {
         ResponseMessage.Status                    = (int)Result;
         ResponseMessage.ServiceRecordHandle       = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* un-register server record message and responds to the message     */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessUnRegisterServiceRecordMessage(MAPM_Un_Register_Service_Record_Request_t *Message)
{
   int                                         Result;
   MAPM_Entry_Info_t                          *MAPMEntryInfo;
   MAPM_Un_Register_Service_Record_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Next, attempt to delete the MAP Server Entry from the MAP   */
         /* Entry List.                                                 */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, Message->InstanceID)) != NULL)
         {
            /* Verify that this message is coming from the Client that  */
            /* registered the server or opened the connection to the    */
            /* remote device.                                           */
            if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Un-Register SDP Record : 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

               /* Verify that there is a Service Record registered.     */
               if(MAPMEntryInfo->ServiceRecordHandle)
               {
                  if((Result = _MAP_Un_Register_SDP_Record(MAPMEntryInfo->MAPID, MAPMEntryInfo->ServiceRecordHandle)) == 0)
                  {
                     /* Success.  Note the handle that was returned.    */
                     MAPMEntryInfo->ServiceRecordHandle = 0;
                  }
               }
               else
                  Result = 0;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
         }
         else
            Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_UN_REGISTER_SERVICE_RECORD_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* parse remote message access services message and responds to the  */
   /* message accordingly. This function does not verify the integrity  */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessParseRemoteMessageAccessServicesMessage(MAPM_Parse_Remote_Message_Access_Services_Request_t *Message)
{
   int                                                   Result;
   unsigned int                                          Index;
   unsigned int                                          ReservedSize;
   MAPM_Parsed_Message_Access_Service_Info_t             ServiceInfo;
   MAPM_Parse_Remote_Message_Access_Services_Response_t  ErrorResponseMessage;
   MAPM_Parse_Remote_Message_Access_Services_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */
      ResponseMessage = NULL;
      ReservedSize    = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Next, attempt to parse the Service Records of the specified */
         /* remote device.                                              */
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Parse Service Record of Remote Message Access Server\n"));

         if((Result = MAPM_Parse_Remote_Message_Access_Services(Message->RemoteDeviceAddress, &ServiceInfo)) == 0)
         {
            ReservedSize = 0;

            for(Index = 0; Index < ServiceInfo.NumberServices; Index++)
            {
               if(ServiceInfo.ServiceDetails[Index].ServiceName)
               {
                  /* Store the string size in place of the string       */
                  /* pointer. The pointers will be rebuilt on the       */
                  /* receiving client side.                             */
                  ReservedSize += (BTPS_StringLength(ServiceInfo.ServiceDetails[Index].ServiceName) + 1);
               }
            }

            if((ResponseMessage = (MAPM_Parse_Remote_Message_Access_Services_Response_t *)BTPS_AllocateMemory(MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_RESPONSE_SIZE(ServiceInfo.NumberServices, ReservedSize))) != NULL)
            {
               ResponseMessage->NumberServices       = ServiceInfo.NumberServices;
               ResponseMessage->ReservedBufferLength = ReservedSize;

               for(Index = 0; Index < ServiceInfo.NumberServices; Index++)
               {
                  ((MAPM_Parse_Response_Service_Details_t *)(ResponseMessage->VariableData))[Index].ServerPort            = ServiceInfo.ServiceDetails[Index].ServerPort;
                  ((MAPM_Parse_Response_Service_Details_t *)(ResponseMessage->VariableData))[Index].InstanceID            = ServiceInfo.ServiceDetails[Index].InstanceID;
                  ((MAPM_Parse_Response_Service_Details_t *)(ResponseMessage->VariableData))[Index].SupportedMessageTypes = ServiceInfo.ServiceDetails[Index].SupportedMessageTypes;

                  if(ServiceInfo.ServiceDetails->ServiceName)
                     ((MAPM_Parse_Response_Service_Details_t *)(ResponseMessage->VariableData))[Index].ServiceNameBytes   = (BTPS_StringLength(ServiceInfo.ServiceDetails->ServiceName) + 1);
                  else
                     ((MAPM_Parse_Response_Service_Details_t *)(ResponseMessage->VariableData))[Index].ServiceNameBytes   = 0;
               }

               if((ReservedSize) && (ServiceInfo.RESERVED))
                  BTPS_MemCopy((ResponseMessage->VariableData + (ServiceInfo.NumberServices * sizeof(MAPM_Parse_Response_Service_Details_t))), ServiceInfo.RESERVED, ReservedSize);
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

            MAPM_Free_Parsed_Message_Access_Service_Info(&ServiceInfo);
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      if(ResponseMessage)
      {
         ResponseMessage->MessageHeader                = Message->MessageHeader;

         ResponseMessage->MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

         ResponseMessage->MessageHeader.MessageLength  = MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_RESPONSE_SIZE(ResponseMessage->NumberServices, ReservedSize) - BTPM_MESSAGE_HEADER_SIZE;

         ResponseMessage->Status                       = Result;

         MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

         /* Free the memory that was allocated because are finished with*/
         /* it.                                                         */
         BTPS_FreeMemory(ResponseMessage);
      }
      else
      {
         ErrorResponseMessage.MessageHeader                = Message->MessageHeader;

         ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

         ErrorResponseMessage.MessageHeader.MessageLength  = MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_RESPONSE_SIZE(0, 0) - BTPM_MESSAGE_HEADER_SIZE;

         ErrorResponseMessage.Status                       = Result;

         ErrorResponseMessage.NumberServices               = 0;

         ErrorResponseMessage.ReservedBufferLength         = 0;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }
}

   /* The following function processes the specified Message Access     */
   /* connect remote device message and responds to the message         */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessConnectRemoteDeviceMessage(MAPM_Connect_Remote_Device_Request_t *Message)
{
   int                                    Result;
   Boolean_t                              Notification;
   unsigned int                           MAPID;
   MAPM_Entry_Info_t                      MAPMEntryInfo;
   MAPM_Entry_Info_t                     *MAPMEntryInfoPtr;
   MAPM_Connect_Remote_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Next, verify that the message parameters appear to be       */
         /* semi-valid.                                                 */
         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && (Message->ServerPort >= MAP_PORT_NUMBER_MINIMUM) && (Message->ServerPort <= MAP_PORT_NUMBER_MAXIMUM) && (Message->InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (Message->InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE))
         {
            /* Initialize success.                                      */
            Result = 0;

            /* Initialize the MAP ID to 0.                              */
            MAPID  = 0;

            /* Determine if this a Client connection or a Notification  */
            /* connection.                                              */
            if((Message->ConnectionType == mctNotificationServer) || (Message->ConnectionType == mctNotificationClient))
            {
               if((MAPMEntryInfoPtr = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, Message->InstanceID)) != NULL)
               {
                  /* Verify that this message is coming from the Client */
                  /* that registered the server or opened the connection*/
                  /* to the remote device.                              */
                  if(MAPMEntryInfoPtr->ClientID == Message->MessageHeader.AddressID)
                  {
                     /* Instance ID found.  Verify that Notifications   */
                     /* have been enabled.                              */
                     if(MAPMEntryInfoPtr->Flags & MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED)
                     {
                        /* We need to connect to the Notification Server*/
                        /* based on the existing MAP ID, so let's go    */
                        /* ahead and note it.                           */
                        MAPID        = MAPMEntryInfoPtr->MAPID;

                        Notification = TRUE;
                     }
                     else
                        Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOTIFICATIONS_NOT_ENABLED;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
            }
            else
               Notification = FALSE;

            if(!Result)
            {
               /* Next, make sure that we do not already have a         */
               /* connection to the specified device.                   */
               if((MAPMEntryInfoPtr = SearchMAPMEntryInfoByConnection(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) == NULL)
               {
                  /* Entry is not present, go ahead and create a new    */
                  /* entry.                                             */
                  BTPS_MemInitialize(&MAPMEntryInfo, 0, sizeof(MAPM_Entry_Info_t));

                  MAPMEntryInfo.TrackingID          = GetNextTrackingID();
                  MAPMEntryInfo.ClientID            = Message->MessageHeader.AddressID;
                  MAPMEntryInfo.RemoteDeviceAddress = Message->RemoteDeviceAddress;
                  MAPMEntryInfo.PortNumber          = Message->ServerPort;
                  MAPMEntryInfo.InstanceID          = Message->InstanceID;
                  MAPMEntryInfo.ConnectionState     = csIdle;
                  MAPMEntryInfo.ConnectionFlags     = Message->ConnectionFlags;
                  MAPMEntryInfo.CurrentOperation    = coNone;

                  if((MAPMEntryInfoPtr = AddMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), &MAPMEntryInfo)) != NULL)
                  {
                     /* First, let's wait for the Port to disconnect.   */
                     if(!SPPM_WaitForPortDisconnection(Message->ServerPort, FALSE, Message->RemoteDeviceAddress, MAXIMUM_MAP_PORT_DELAY_TIMEOUT_MS))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device.     */
                        if(Message->ConnectionFlags & MAPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                           MAPMEntryInfoPtr->ConnectionState = csEncrypting;
                        else
                        {
                           if(Message->ConnectionFlags & MAPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                              MAPMEntryInfoPtr->ConnectionState = csAuthenticating;
                           else
                              MAPMEntryInfoPtr->ConnectionState = csConnectingDevice;
                        }

                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Device\n"));

                        Result = DEVM_ConnectWithRemoteDevice(Message->RemoteDeviceAddress, (MAPMEntryInfoPtr->ConnectionState == csConnectingDevice)?0:((MAPMEntryInfoPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                        if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                        {
                           /* Check to see if we need to actually issue */
                           /* the Remote connection.                    */
                           if(Result < 0)
                           {
                              /* Set the state to connecting remote     */
                              /* device.                                */
                              MAPMEntryInfoPtr->ConnectionState = csConnecting;

                              /* We need to handle the connection       */
                              /* differently based upon if the user is  */
                              /* requesting to connect to a notification*/
                              /* server.                                */
                              if(Notification)
                              {
                                 /* Notification server.  We need to    */
                                 /* determine the MAP ID that has       */
                                 /* registered for Notifications.       */
                                 Result = _MAP_Open_Remote_Message_Notification_Server_Port(MAPID, Message->ServerPort);
                              }
                              else
                                 Result = _MAP_Open_Remote_Message_Access_Server_Port(Message->RemoteDeviceAddress, Message->ServerPort);

                              if(Result < 0)
                                 Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
                              else
                              {
                                 /* Note the MAP ID.                    */
                                 MAPMEntryInfoPtr->MAPID = (unsigned int)Result;

                                 /* Flag success.                       */
                                 Result                  = 0;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Move the state to the connecting Waiting     */
                        /* state.                                       */
                        MAPMEntryInfoPtr->ConnectionState = csConnectingWaiting;

                        if((BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS) && (MAXIMUM_MAP_PORT_OPEN_DELAY_RETRY))
                        {
                           /* Port is NOT disconnected, go ahead and    */
                           /* start a timer so that we can continue to  */
                           /* check for the Port Disconnection.         */
                           Result = TMR_StartTimer((void *)MAPMEntryInfoPtr->TrackingID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                           /* If the timer was started, go ahead and    */
                           /* note the Timer ID.                        */
                           if(Result > 0)
                           {
                              MAPMEntryInfoPtr->ConnectionTimerID = (unsigned int)Result;

                              Result                              = 0;
                           }
                           else
                              Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
                        }
                        else
                           Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_ALREADY_CONNECTED;
                     }

                     /* If there was an error, go ahead and delete the  */
                     /* entry that was added.                           */
                     if(Result)
                     {
                        if((MAPMEntryInfoPtr = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfoPtr->TrackingID)) != NULL)
                        {
                           CleanupMAPMEntryInfo(MAPMEntryInfoPtr);

                           FreeMAPMEntryInfoEntryMemory(MAPMEntryInfoPtr);
                        }
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
               {
                  if(MAPMEntryInfoPtr->ConnectionState == csConnected)
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_ALREADY_CONNECTED;
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_CONNECTION_IN_PROGRESS;
               }
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;
      ResponseMessage.InstanceID                   = Message->InstanceID;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Message Access     */
   /* disconnect device message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessDisconnectMessage(MAPM_Disconnect_Request_t *Message)
{
   int                         Result;
   Boolean_t                   Notification;
   Boolean_t                   Server;
   Boolean_t                   PerformDisconnect;
   unsigned int                ServerPort;
   MAPM_Entry_Info_t          *MAPMEntryInfo;
   MAPM_Disconnect_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Go ahead and note if this is a notification abort or a   */
            /* normal MAP abort.                                        */
            if((Message->ConnectionType == mctNotificationServer) || (Message->ConnectionType == mctNotificationClient))
               Notification = TRUE;
            else
               Notification = FALSE;

            if((Message->ConnectionType == mctNotificationServer) || (Message->ConnectionType == mctMessageAccessServer))
               Server = TRUE;
            else
               Server = FALSE;

            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), Message->RemoteDeviceAddress, Message->InstanceID, Server)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Connection: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Next, go ahead and close the connection.           */
                  if((Server) && (MAPMEntryInfo->ConnectionState == csIdle))
                     Result = 0;
                  else
                  {
                     if(Server)
                     {
                        CleanupMAPMEntryInfo(MAPMEntryInfo);

                        MAPMEntryInfo->ConnectionState = csIdle;

                        Result = _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                     }
                     else
                     {
                        /* Not if this is a Message Access Client and we*/
                        /* need to check to see if we should cleanup the*/
                        /* local Notification Server state.             */
                        if(Notification == FALSE)
                        {
                           /* If we have enabled (or are pending an     */
                           /* enable for notifications) on this MCE     */
                           /* connection handle the disconnect.         */
                           if(MAPMEntryInfo->Flags & (MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED | MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION))
                              HandleNotificationDisconnection(MAPMEntryInfo, &NotificationInfo);
                        }

                        switch(MAPMEntryInfo->ConnectionState)
                        {
                           case csAuthorizing:
                           case csAuthenticating:
                           case csEncrypting:
                              /* Should not occur.                      */
                              PerformDisconnect = FALSE;
                              break;
                           case csConnectingWaiting:
                              if(MAPMEntryInfo->ConnectionTimerID)
                                 TMR_StopTimer(MAPMEntryInfo->ConnectionTimerID);

                              PerformDisconnect = FALSE;
                              break;
                           case csConnectingDevice:
                              PerformDisconnect = FALSE;
                              break;
                           default:
                           case csConnecting:
                           case csConnected:
                              PerformDisconnect = TRUE;
                              break;
                        }

                        if(PerformDisconnect)
                        {
                           /* Nothing really to do other than to        */
                           /* disconnect the device.                    */
                           Result = _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                        }
                        else
                           Result = 0;

                        /* If this is a client, we need to go ahead and */
                        /* delete the entry.                            */
                        if(!Result)
                        {
                           /* Note the Port Number before we delete the */
                           /* MAP Entry (we will use it below after we  */
                           /* free the entry).                          */
                           ServerPort = MAPMEntryInfo->PortNumber;

                           if((MAPMEntryInfo = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfo->TrackingID)) != NULL)
                           {
                              /* All finished, free any memory that was */
                              /* allocated for the server.              */
                              CleanupMAPMEntryInfo(MAPMEntryInfo);

                              FreeMAPMEntryInfoEntryMemory(MAPMEntryInfo);
                           }

                           /* Go ahead and give the port some time to   */
                           /* disconnect (since it was initiated        */
                           /* locally).                                 */
                           SPPM_WaitForPortDisconnection(ServerPort, FALSE, Message->RemoteDeviceAddress, MAXIMUM_MAP_PORT_DELAY_TIMEOUT_MS);
                        }
                     }
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_DISCONNECT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* abort message and responds to the message accordingly.  This      */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessAbortMessage(MAPM_Abort_Request_t *Message)
{
   int                              Result;
   Boolean_t                        Notification;
   MAPM_Entry_Info_t               *MAPMEntryInfo;
   MAPM_Register_Server_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Go ahead and note if this is a notification abort or a   */
            /* normal MAP abort.                                        */
            if((Message->ConnectionType == mctNotificationServer) || (Message->ConnectionType == mctNotificationClient))
               Notification = TRUE;
            else
               Notification = FALSE;

            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Abort: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is an on-going   */
                  /* operation.                                         */
                  if((MAPMEntryInfo->CurrentOperation != coNone) && (!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)))
                  {
                     /* Operation in progress, go ahead and send the    */
                     /* Abort.                                          */
                     if((Result = _MAP_Abort_Request(MAPMEntryInfo->MAPID)) == 0)
                        MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;
                  }
                  else
                  {
                     if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                        Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_ABORT_OPERATION_IN_PROGRESS;
                     else
                        Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_NO_OPERATION_IN_PROGRESS;
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_ABORT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* query current folder message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessQueryCurrentFolderMessage(MAPM_Query_Current_Folder_Request_t *Message)
{
   int                                   Result;
   MAPM_Entry_Info_t                    *MAPMEntryInfo;
   MAPM_Query_Current_Folder_Response_t  ErrorResponseMessage;
   MAPM_Query_Current_Folder_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Flag that we have not allocated a message response.            */
      ResponseMessage = NULL;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Folder: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Current Folder is present, go ahead and copy as    */
                  /* much as we have space for.                         */
                  if(MAPMEntryInfo->CurrentPath)
                     Result = BTPS_StringLength(MAPMEntryInfo->CurrentPath) + 1;
                  else
                     Result = 1;

                  /* Now let's attempt to allocate memory to hold the   */
                  /* entire path.                                       */
                  if((ResponseMessage = (MAPM_Query_Current_Folder_Response_t *)BTPS_AllocateMemory(MAPM_QUERY_CURRENT_FOLDER_RESPONSE_SIZE(Result))) != NULL)
                  {
                     BTPS_MemInitialize(ResponseMessage, 0, MAPM_QUERY_CURRENT_FOLDER_RESPONSE_SIZE(Result));

                     if(MAPMEntryInfo->CurrentPath)
                        BTPS_StringCopy(ResponseMessage->FolderName, MAPMEntryInfo->CurrentPath);

                     ResponseMessage->FolderNameLength = Result;

                     Result                            = 0;
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      if(ResponseMessage)
      {
         ResponseMessage->MessageHeader                = Message->MessageHeader;

         ResponseMessage->MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

         ResponseMessage->MessageHeader.MessageLength  = MAPM_QUERY_CURRENT_FOLDER_RESPONSE_SIZE(ResponseMessage->FolderNameLength) - BTPM_MESSAGE_HEADER_SIZE;

         ResponseMessage->Status                       = Result;

         MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

         /* Free the memory that was allocated because are finished with*/
         /* it.                                                         */
         BTPS_FreeMemory(ResponseMessage);
      }
      else
      {
         ErrorResponseMessage.MessageHeader                = Message->MessageHeader;

         ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

         ErrorResponseMessage.MessageHeader.MessageLength  = MAPM_QUERY_CURRENT_FOLDER_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

         ErrorResponseMessage.Status                       = Result;
         ErrorResponseMessage.FolderNameLength             = 0;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }
}

   /* The following function processes the specified Message Access     */
   /* enable notifications message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableNotificationsMessage(MAPM_Enable_Notifications_Request_t *Message)
{
   int                                   Result;
   MAPM_Entry_Info_t                    *MAPMEntryInfo;
   MAPM_Enable_Notifications_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Enable Notifications: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* Handle the Notification Enable/Disable Request. */
                     if((Message->Enable == FALSE) || (!(Result = HandleNotificationEnableRequest(MAPMEntryInfo, &NotificationInfo))))
                     {
                        /* No operation in progress, go ahead and       */
                        /* enable/disable notifications.                */
                        if((Result = _MAP_Set_Notification_Registration_Request(MAPMEntryInfo->MAPID, Message->Enable)) == 0)
                        {
                           if(Message->Enable)
                              MAPMEntryInfo->Flags |= (unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION;
                           else
                              MAPMEntryInfo->Flags &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION);

                           MAPMEntryInfo->CurrentOperation = coEnableNotifications;
                        }
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_ENABLE_NOTIFICATIONS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access get */
   /* folder listing message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessGetFolderListingMessage(MAPM_Get_Folder_Listing_Request_t *Message)
{
   int                                 Result;
   MAPM_Entry_Info_t                  *MAPMEntryInfo;
   MAPM_Get_Folder_Listing_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Folder Listing: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Get Folder Listing command.       */
                     if((Result = _MAP_Get_Folder_Listing_Request(MAPMEntryInfo->MAPID, Message->MaxListCount, Message->ListStartOffset)) == 0)
                        MAPMEntryInfo->CurrentOperation = coGetFolderListing;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_GET_FOLDER_LISTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access get */
   /* folder listing size message and responds to the message           */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessGetFolderListingSizeMessage(MAPM_Get_Folder_Listing_Size_Request_t *Message)
{
   int                                      Result;
   MAPM_Entry_Info_t                       *MAPMEntryInfo;
   MAPM_Get_Folder_Listing_Size_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Folder Listing Size: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Get Folder Listing Size command.  */
                     if((Result = _MAP_Get_Folder_Listing_Request(MAPMEntryInfo->MAPID, 0, 0)) == 0)
                        MAPMEntryInfo->CurrentOperation = coGetFolderListingSize;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access get */
   /* message listing message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessGetMessageListingMessage(MAPM_Get_Message_Listing_Request_t *Message)
{
   int                                  Result;
   Word_t                              *FolderName;
   unsigned int                         Index;
   MAPM_Entry_Info_t                   *MAPMEntryInfo;
   MAPM_Get_Message_Listing_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Message Listing: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* Before going any further, let's fix up all the  */
                     /* Pointers and make sure all strings are NULL     */
                     /* terminated.                                     */
                     Message->ListingInfo.FilterRecipient  = NULL;
                     Message->ListingInfo.FilterOriginator = NULL;

                     Index                                 = 0;
                     if(Message->ListingInfoPresent)
                     {
                        if(Message->FilterRecipientLength)
                        {
                           Message->ListingInfo.FilterRecipient  = (char *)&(Message->VariableData[Index]);
                           Index                                += Message->FilterRecipientLength;

                           /* Finally make sure the Filter Recipient is */
                           /* NULL terminated.                          */
                           Message->VariableData[Index - 1]      = '\0';
                        }

                        if(Message->FilterOriginatorLength)
                        {
                           Message->ListingInfo.FilterOriginator  = (char *)&(Message->VariableData[Index]);
                           Index                                 += Message->FilterOriginatorLength;

                           /* Finally make sure the Filter Originator is*/
                           /* NULL terminated.                          */
                           Message->VariableData[Index - 1]      = '\0';
                        }
                     }

                     /* Make sure that the Folder Name is NULL          */
                     /* terminated.                                     */
                     if(Message->FolderNameLength)
                        Message->VariableData[Index + Message->FolderNameLength - 1] = '\0';

                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     /* Initialize success.                             */
                     Result = 0;
                     if((!Message->FolderNameLength) || ((Message->FolderNameLength) && (!BTPS_StringLength((char *)&(Message->VariableData[Index])))))
                        FolderName = NULL;
                     else
                     {
                        if((FolderName = ConvertUTF8ToUnicode((char *)&(Message->VariableData[Index]))) == NULL)
                           Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_FOLDER_NAME;
                     }

                     /* Folder Name converted, go ahead and attempt to  */
                     /* Get the Message Listing.                        */
                     if(!Result)
                     {
                        if((Result = _MAP_Get_Message_Listing_Request(MAPMEntryInfo->MAPID, FolderName, Message->MaxListCount, Message->ListStartOffset, Message->ListingInfoPresent?&(Message->ListingInfo):NULL)) == 0)
                           MAPMEntryInfo->CurrentOperation = coGetMessageListing;
                     }

                     /* Free any memory that was allocated for the      */
                     /* Folder Name.                                    */
                     if(FolderName)
                        BTPS_FreeMemory(FolderName);
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_GET_MESSAGE_LISTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access get */
   /* message listing size message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessGetMessageListingSizeMessage(MAPM_Get_Message_Listing_Size_Request_t *Message)
{
   int                              Result;
   Word_t                          *FolderName;
   unsigned int                     Index;
   MAPM_Entry_Info_t               *MAPMEntryInfo;
   MAPM_Register_Server_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Message Listing: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* Before going any further, let's fix up all the  */
                     /* Pointers and make sure all strings are NULL     */
                     /* terminated.                                     */
                     Message->ListingInfo.FilterRecipient  = NULL;
                     Message->ListingInfo.FilterOriginator = NULL;

                     Index                                 = 0;
                     if(Message->ListingInfoPresent)
                     {
                        if(Message->FilterRecipientLength)
                        {
                           Message->ListingInfo.FilterRecipient  = (char *)&(Message->VariableData[Index]);
                           Index                                += Message->FilterRecipientLength;

                           /* Finally make sure the Filter Recipient is */
                           /* NULL terminated.                          */
                           Message->VariableData[Index - 1]      = '\0';
                        }

                        if(Message->FilterOriginatorLength)
                        {
                           Message->ListingInfo.FilterOriginator  = (char *)&(Message->VariableData[Index]);
                           Index                                 += Message->FilterOriginatorLength;

                           /* Finally make sure the Filter Originator is*/
                           /* NULL terminated.                          */
                           Message->VariableData[Index - 1]      = '\0';
                        }
                     }

                     /* Make sure that the Folder Name is NULL          */
                     /* terminated.                                     */
                     if(Message->FolderNameLength)
                        Message->VariableData[Index + Message->FolderNameLength - 1] = '\0';

                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     /* Initialize success.                             */
                     Result = 0;
                     if((!Message->FolderNameLength) || ((Message->FolderNameLength) && (!BTPS_StringLength((char *)&(Message->VariableData[Index])))))
                        FolderName = NULL;
                     else
                     {
                        if((FolderName = ConvertUTF8ToUnicode((char *)&(Message->VariableData[Index]))) == NULL)
                           Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_FOLDER_NAME;
                     }

                     /* Folder Name converted, go ahead and attempt to  */
                     /* Get the Message Listing Size.                   */
                     if(!Result)
                     {
                        if((Result = _MAP_Get_Message_Listing_Request(MAPMEntryInfo->MAPID, FolderName, 0, 0, Message->ListingInfoPresent?&(Message->ListingInfo):NULL)) == 0)
                           MAPMEntryInfo->CurrentOperation = coGetMessageListingSize;
                     }

                     /* Free any memory that was allocated for the      */
                     /* Folder Name.                                    */
                     if(FolderName)
                        BTPS_FreeMemory(FolderName);
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access get */
   /* message message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessGetMessageMessage(MAPM_Get_Message_Request_t *Message)
{
   int                          Result;
   MAPM_Entry_Info_t           *MAPMEntryInfo;
   MAPM_Get_Message_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */

         /* Make sure the Message Handle is NULL terminated.            */
         Message->MessageHandle[MAP_MESSAGE_HANDLE_LENGTH] = '\0';

         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && (BTPS_StringLength(Message->MessageHandle)) && (BTPS_StringLength(Message->MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Message: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Get Message command.              */
                     if((Result = _MAP_Get_Message_Request(MAPMEntryInfo->MAPID, Message->MessageHandle, Message->Attachment, Message->CharSet, Message->FractionalType)) == 0)
                        MAPMEntryInfo->CurrentOperation = coGetMessage;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
         {
            if((!BTPS_StringLength(Message->MessageHandle)) ||  (BTPS_StringLength(Message->MessageHandle) > MAP_MESSAGE_HANDLE_LENGTH))
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_MESSAGE_HANDLE;
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SET_MESSAGE_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access set */
   /* message status message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSetMessageStatusMessage(MAPM_Set_Message_Status_Request_t *Message)
{
   int                                 Result;
   MAPM_Entry_Info_t                  *MAPMEntryInfo;
   MAPM_Set_Message_Status_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */

         /* Make sure the Message Handle is NULL terminated.            */
         Message->MessageHandle[MAP_MESSAGE_HANDLE_LENGTH] = '\0';

         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && (BTPS_StringLength(Message->MessageHandle)) && (BTPS_StringLength(Message->MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Message Status: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Set Message Status command.       */
                     if((Result = _MAP_Set_Message_Status_Request(MAPMEntryInfo->MAPID, Message->MessageHandle, Message->StatusIndicator, Message->StatusValue)) == 0)
                        MAPMEntryInfo->CurrentOperation = coSetMessageStatus;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
         {
            if((!BTPS_StringLength(Message->MessageHandle)) ||  (BTPS_StringLength(Message->MessageHandle) > MAP_MESSAGE_HANDLE_LENGTH))
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_MESSAGE_HANDLE;
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SET_MESSAGE_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access push*/
   /* message message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessPushMessageMessage(MAPM_Push_Message_Request_t *Message)
{
   int                           Result;
   Word_t                       *FolderName;
   unsigned int                  Index;
   MAPM_Entry_Info_t            *MAPMEntryInfo;
   MAPM_Push_Message_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Push Message: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coPushMessage))
                  {
                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     Index = 0;
                     if(Message->FolderNameLength)
                     {
                        Index                            += Message->FolderNameLength;

                        /* Finally make sure the Folder Name is NULL    */
                        /* terminated.                                  */
                        Message->VariableData[Index - 1]  = '\0';
                     }

                     /* Initialize success.                             */
                     Result = 0;
                     if((!Message->FolderNameLength) || ((Message->FolderNameLength) && (!BTPS_StringLength((char *)Message->VariableData))))
                        FolderName = NULL;
                     else
                     {
                        if((FolderName = ConvertUTF8ToUnicode((char *)(Message->VariableData))) == NULL)
                           Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_FOLDER_NAME;
                     }

                     /* Folder Name converted, go ahead and attempt to  */
                     /* Push the Message.                               */
                     if(!Result)
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        if((MAPMEntryInfo->DataBufferSize = Message->MessageLength) != 0)
                        {
                           /* Free any current data we have buffered    */
                           /* (should be none).                         */
                           if(MAPMEntryInfo->DataBuffer)
                           {
                              BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                              MAPMEntryInfo->DataBuffer = NULL;
                           }

                           /* Go ahead and allocate the buffer (we will */
                           /* not copy it yet, but we will allocate it  */
                           /* so that we don't get an error *AFTER* we  */
                           /* send the first part of the data.          */
                           if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(Message->MessageLength)) == NULL)
                              Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }

                        /* Flag that we have not sent any data at this  */
                        /* point.                                       */
                        MAPMEntryInfo->DataBufferSent = 0;

                        if(!Result)
                        {
                           if((Result = _MAP_Push_Message_Request(MAPMEntryInfo->MAPID, FolderName, Message->Transparent, Message->Retry, Message->CharSet, Message->MessageLength, &(Message->VariableData[Index]), &(MAPMEntryInfo->DataBufferSent), Message->Final)) == 0)
                           {
                              /* Go ahead and clear out the current     */
                              /* Message Handle (it needs to be cached  */
                              /* for responses).                        */
                              if(MAPMEntryInfo->CurrentOperation == coNone)
                                 BTPS_MemInitialize(MAPMEntryInfo->DataMessageHandle, 0, sizeof(MAPMEntryInfo->DataMessageHandle));

                              /* Flag that a Push Message Operation is  */
                              /* in progress.                           */
                              MAPMEntryInfo->CurrentOperation = coPushMessage;

                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if(MAPMEntryInfo->DataBufferSent != Message->MessageLength)
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, &(Message->VariableData[Index]), Message->MessageLength);
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((Result) || (MAPMEntryInfo->DataBufferSent == Message->MessageLength))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }

                     /* Free any memory that was allocated for the      */
                     /* Folder Name.                                    */
                     if(FolderName)
                        BTPS_FreeMemory(FolderName);
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_PUSH_MESSAGE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* update inbox message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessUpdateInboxMessage(MAPM_Update_Inbox_Request_t *Message)
{
   int                           Result;
   MAPM_Entry_Info_t            *MAPMEntryInfo;
   MAPM_Update_Inbox_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Update Inbox: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Update Inbox command.             */
                     if((Result = _MAP_Update_Inbox_Request(MAPMEntryInfo->MAPID)) == 0)
                        MAPMEntryInfo->CurrentOperation = coUpdateInbox;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_UPDATE_INBOX_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access set */
   /* folder message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessSetFolderMessage(MAPM_Set_Folder_Request_t *Message)
{
   int                         Result;
   char                       *PendingPath;
   Word_t                     *FolderName;
   MAPM_Entry_Info_t          *MAPMEntryInfo;
   MAPM_Set_Folder_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         /* * NOTE * Here are the options to test for PathOption:       */
         /*             - sfRoot - FolderName ignored                   */
         /*             - sfDown - FolderName required                  */
         /*             - sfUp   - FolderName optional                  */
         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && ((Message->PathOption != sfDown) || (((Message->PathOption == sfDown) && (Message->FolderNameLength)))))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Path: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     /* First make sure the Folder Name is NULL         */
                     /* terminated.                                     */
                     if(Message->FolderNameLength)
                        Message->FolderName[Message->FolderNameLength - 1]  = '\0';

                     /* Go ahead and convert the folder name from UTF-8 */
                     /* to Unicode.                                     */
                     if(Message->PathOption == sfRoot)
                        FolderName = NULL;
                     else
                     {
                        if((Message->FolderNameLength) && (BTPS_StringLength(Message->FolderName)))
                           FolderName = ConvertUTF8ToUnicode(Message->FolderName);
                        else
                           FolderName = NULL;
                     }

                     /* Go ahead and attempt to build the Pending Path. */
                     if(BuildPendingFolder(Message->PathOption, FolderName, MAPMEntryInfo->CurrentPath, &PendingPath))
                     {
                        /* Free any current Pending Set Path Information*/
                        /* (should already be clear).                   */
                        if(MAPMEntryInfo->PendingPath)
                        {
                           BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                           MAPMEntryInfo->PendingPath = NULL;
                        }

                        /* Set the new Pending Path.                    */
                        MAPMEntryInfo->PendingPath = PendingPath;

                        if((Result = _MAP_Set_Folder_Request(MAPMEntryInfo->MAPID, Message->PathOption, FolderName)) == 0)
                           MAPMEntryInfo->CurrentOperation = coSetFolder;
                        else
                        {
                           /* Error free any resources that were        */
                           /* allocated.                                */
                           if(MAPMEntryInfo->PendingPath)
                           {
                              BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                              MAPMEntryInfo->PendingPath = NULL;
                           }
                        }
                     }
                     else
                        Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_PATH;

                     /* Free any memory that was allocated to hold the  */
                     /* Unicode Folder Name.                            */
                     if(FolderName)
                        BTPS_FreeMemory(FolderName);
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SET_FOLDER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access set */
   /* folder (absolute) message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSetFolderAbsoluteMessage(MAPM_Set_Folder_Absolute_Request_t *Message)
{
   int                                  Result;
   MAPM_Entry_Info_t                   *MAPMEntryInfo;
   MAPM_Set_Folder_Absolute_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Absolute Path: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* Go ahead and make sure the Folder Name is NULL  */
                     /* terminated.                                     */
                     if(Message->FolderNameLength)
                        Message->FolderName[Message->FolderNameLength - 1] = '\0';

                     /* Check to see if we can simply set the path to   */
                     /* the root.                                       */
                     if((!Message->FolderNameLength) || ((Message->FolderNameLength) && (!BTPS_StringLength(Message->FolderName))))
                        Result = MAPM_Set_Folder(Message->RemoteDeviceAddress, Message->InstanceID, sfRoot, NULL);
                     else
                     {
                        /* We need to set a non-NULL absolute path.     */

                        /* Free any current Pending Set Path Information*/
                        /* (should already be clear).                   */
                        if(MAPMEntryInfo->PendingPath)
                        {
                           BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                           MAPMEntryInfo->PendingPath = NULL;
                        }

                        /* Next, attempt to store the path we are trying*/
                        /* to set to the context.                       */
                        /* * NOTE * We will strip any potential leading */
                        /*          delimiter character ('/').          */
                        if((Message->FolderName) && (BTPS_StringLength(Message->FolderName)) && ((Message->FolderName[0] != '/') || ((Message->FolderName[0] == '/') && (Message->FolderName[1] != '\0'))))
                        {
                           if((MAPMEntryInfo->PendingPath = BTPS_AllocateMemory(BTPS_StringLength(Message->FolderName) + 1)) != NULL)
                           {
                              if(Message->FolderName[0] == '/')
                                 BTPS_StringCopy(MAPMEntryInfo->PendingPath, &(Message->FolderName[1]));
                              else
                                 BTPS_StringCopy(MAPMEntryInfo->PendingPath, Message->FolderName);

                              Result = 0;
                           }
                           else
                              Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }
                        else
                           Result = 0;

                        /* Pending Path Folder Name stored, go ahead and*/
                        /* attempt to set the current folder to root (to*/
                        /* start the process).                          */
                        if(!Result)
                        {
                           if((Result = _MAP_Set_Folder_Request(MAPMEntryInfo->MAPID, sfRoot, NULL)) == 0)
                           {
                              /* Flag that we are setting an absolute   */
                              /* path (as opposed to simply setting the */
                              /* path).                                 */
                              MAPMEntryInfo->CurrentOperation  = coSetFolderAbsolute;

                              /* Flag that we are currently setting the */
                              /* Root Path.                             */
                              MAPMEntryInfo->PendingPathOffset = -1;
                           }
                           else
                           {
                              /* Error free any resources that were     */
                              /* allocated.                             */
                              if(MAPMEntryInfo->PendingPath)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                                 MAPMEntryInfo->PendingPath = NULL;
                              }
                           }
                        }
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SET_FOLDER_ABSOLUTE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* enable notifications confirmation message and responds to the     */
   /* message accordingly.  This function does not verify the integrity */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableNotificationsConfirmationMessage(MAPM_Enable_Notifications_Confirmation_Request_t *Message)
{
   int                                                Result;
   Byte_t                                             ResponseCode;
   MAPM_Entry_Info_t                                 *MAPMEntryInfo;
   MAPM_Enable_Notifications_Confirmation_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Enable Confirmation: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coEnableNotifications)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Set Notification confirmation  */
                        /* response.                                    */
                        if((Result = _MAP_Set_Notification_Registration_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_ENABLE_NOTIFICATIONS_CONFIRMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access send*/
   /* folder listing message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendFolderListingMessage(MAPM_Send_Folder_Listing_Request_t *Message)
{
   int                                  Result;
   Byte_t                               ResponseCode;
   MAPM_Entry_Info_t                   *MAPMEntryInfo;
   MAPM_Send_Folder_Listing_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Folder Listing: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that the correct on-going   */
                  /* operation is in progress.                          */
                  if(MAPMEntryInfo->CurrentOperation == coGetFolderListing)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        /* * NOTE * There is no reason to worry about   */
                        /*          sending any data (or backing any    */
                        /*          data up if this is not a successful */
                        /*          response).                          */
                        if(Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                        {
                           /* Check to see if we need to map the final  */
                           /* bit into a continue (the lack of it).     */
                           /* * NOTE * This is required because there is*/
                           /*          No Final flag for responses (it  */
                           /*          is inherant with either an OK or */
                           /*          or CONTINUE being sent as the    */
                           /*          code).                           */
                           if(!Message->Final)
                              ResponseCode = MAP_OBEX_RESPONSE_CONTINUE;

                           if((MAPMEntryInfo->DataBufferSize = Message->FolderListingLength) != 0)
                           {
                              /* Free any current data we have buffered */
                              /* (should be none).                      */
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }

                              /* Go ahead and allocate the buffer (we   */
                              /* will not copy it yet, but we will      */
                              /* allocate it so that we don't get an    */
                              /* error *AFTER* we send the first part of*/
                              /* the data.                              */
                              if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(Message->FolderListingLength)) == NULL)
                                 Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                              else
                                 Result = 0;
                           }
                           else
                              Result = 0;

                           /* Flag that we have not sent any data at    */
                           /* this point.                               */
                           MAPMEntryInfo->DataBufferSent = 0;
                        }
                        else
                        {
                           /* There is no reason to send any data       */
                           /* because this is an error response.        */
                           Message->FolderListingLength = 0;

                           Result                       = 0;
                        }

                        if(!Result)
                        {
                           if((Result = _MAP_Get_Folder_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, NULL, Message->FolderListingLength, Message->FolderListingLength?Message->FolderListing:NULL, &(MAPMEntryInfo->DataBufferSent))) == 0)
                           {
                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if((Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent != Message->FolderListingLength))
                              {
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, Message->FolderListing, Message->FolderListingLength);

                                 MAPMEntryInfo->DataFinal = Message->Final;
                              }
                              else
                                 MAPMEntryInfo->CurrentOperation = coNone;
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((Result) || ((Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent == Message->FolderListingLength)))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SEND_FOLDER_LISTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access send*/
   /* folder listing size message and responds to the message           */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendFolderListingSizeMessage(MAPM_Send_Folder_Listing_Size_Request_t *Message)
{
   int                                       Result;
   Byte_t                                    ResponseCode;
   MAPM_Entry_Info_t                        *MAPMEntryInfo;
   MAPM_Send_Folder_Listing_Size_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Folder Listing Size: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coGetFolderListingSize)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Get Folder Listing Response    */
                        /* command.                                     */
                        if((Result = _MAP_Get_Folder_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, &(Message->FolderCount), 0, NULL, NULL)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SEND_FOLDER_LISTING_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access send*/
   /* message listing message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendMessageListingMessage(MAPM_Send_Message_Listing_Request_t *Message)
{
   int                                   Result;
   Byte_t                                ResponseCode;
   MAPM_Entry_Info_t                    *MAPMEntryInfo;
   MAPM_Send_Message_Listing_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Message Listing: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that the correct on-going   */
                  /* operation is in progress.                          */
                  if(MAPMEntryInfo->CurrentOperation == coGetMessageListing)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        /* * NOTE * There is no reason to worry about   */
                        /*          sending any data (or backing any    */
                        /*          data up if this is not a successful */
                        /*          response).                          */
                        if(Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                        {
                           /* Check to see if we need to map the final  */
                           /* bit into a continue (the lack of it).     */
                           /* * NOTE * This is required because there is*/
                           /*          No Final flag for responses (it  */
                           /*          is inherant with either an OK or */
                           /*          or CONTINUE being sent as the    */
                           /*          code).                           */
                           if(!Message->Final)
                              ResponseCode = MAP_OBEX_RESPONSE_CONTINUE;

                           if((MAPMEntryInfo->DataBufferSize = Message->MessageListingLength) != 0)
                           {
                              /* Free any current data we have buffered */
                              /* (should be none).                      */
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }

                              /* Go ahead and allocate the buffer (we   */
                              /* will not copy it yet, but we will      */
                              /* allocate it so that we don't get an    */
                              /* error *AFTER* we send the first part of*/
                              /* the data.                              */
                              if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(Message->MessageListingLength)) == NULL)
                                 Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                              else
                                 Result = 0;
                           }
                           else
                              Result = 0;

                           /* Flag that we have not sent any data at    */
                           /* this point.                               */
                           MAPMEntryInfo->DataBufferSent = 0;
                        }
                        else
                        {
                           /* There is no reason to send any data       */
                           /* because this is an error response.        */
                           Message->MessageListingLength = 0;

                           Result                        = 0;
                        }

                        if(!Result)
                        {
                           if((Result = _MAP_Get_Message_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, &(Message->MessageCount), Message->NewMessage, &(Message->CurrentTime), Message->MessageListingLength, Message->MessageListingLength?Message->MessageListing:NULL, &(MAPMEntryInfo->DataBufferSent))) == 0)
                           {
                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if((Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent != Message->MessageListingLength))
                              {
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, Message->MessageListing, Message->MessageListingLength);

                                 MAPMEntryInfo->DataFinal = Message->Final;
                              }
                              else
                                 MAPMEntryInfo->CurrentOperation = coNone;
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((Result) || ((Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent == Message->MessageListingLength)))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SEND_MESSAGE_LISTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access send*/
   /* message listing size message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendMessageListingSizeMessage(MAPM_Send_Message_Listing_Size_Request_t *Message)
{
   int                                        Result;
   Byte_t                                     ResponseCode;
   MAPM_Entry_Info_t                         *MAPMEntryInfo;
   MAPM_Send_Message_Listing_Size_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Message Listing Size: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coGetMessageListingSize)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Get Message Listing Response   */
                        /* command.                                     */
                        if((Result = _MAP_Get_Message_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, &(Message->MessageCount), Message->NewMessage, &(Message->CurrentTime), 0, NULL, NULL)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SEND_MESSAGE_LISTING_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access send*/
   /* message message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessSendMessageMessage(MAPM_Send_Message_Request_t *Message)
{
   int                           Result;
   Byte_t                        ResponseCode;
   MAPM_Entry_Info_t            *MAPMEntryInfo;
   MAPM_Send_Message_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Message: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that the correct on-going   */
                  /* operation is in progress.                          */
                  if(MAPMEntryInfo->CurrentOperation == coGetMessage)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        /* * NOTE * There is no reason to worry about   */
                        /*          sending any data (or backing any    */
                        /*          data up if this is not a successful */
                        /*          response).                          */
                        if(Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                        {
                           /* Check to see if we need to map the final  */
                           /* bit into a continue (the lack of it).     */
                           /* * NOTE * This is required because there is*/
                           /*          No Final flag for responses (it  */
                           /*          is inherant with either an OK or */
                           /*          or CONTINUE being sent as the    */
                           /*          code).                           */
                           if(!Message->Final)
                              ResponseCode = MAP_OBEX_RESPONSE_CONTINUE;

                           if((MAPMEntryInfo->DataBufferSize = Message->MessageDataLength) != 0)
                           {
                              /* Free any current data we have buffered */
                              /* (should be none).                      */
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }

                              /* Go ahead and allocate the buffer (we   */
                              /* will not copy it yet, but we will      */
                              /* allocate it so that we don't get an    */
                              /* error *AFTER* we send the first part of*/
                              /* the data.                              */
                              if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(Message->MessageDataLength)) == NULL)
                                 Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                              else
                                 Result = 0;
                           }
                           else
                              Result = 0;

                           /* Flag that we have not sent any data at    */
                           /* this point.                               */
                           MAPMEntryInfo->DataBufferSent = 0;
                        }
                        else
                        {
                           /* There is no reason to send any data       */
                           /* because this is an error response.        */
                           Message->MessageDataLength = 0;

                           Result                     = 0;
                        }

                        if(!Result)
                        {
                           if((Result = _MAP_Get_Message_Response(MAPMEntryInfo->MAPID, ResponseCode, Message->FractionalType, Message->MessageDataLength, Message->MessageDataLength?Message->MessageData:NULL, &(MAPMEntryInfo->DataBufferSent))) == 0)
                           {
                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if((Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent != Message->MessageDataLength))
                              {
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, Message->MessageData, Message->MessageDataLength);

                                 MAPMEntryInfo->DataFinal = Message->Final;
                              }
                              else
                                 MAPMEntryInfo->CurrentOperation = coNone;
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((Result) || ((Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent == Message->MessageDataLength)))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SEND_MESSAGE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access send*/
   /* message status message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendMessageStatusMessage(MAPM_Message_Status_Confirmation_Request_t *Message)
{
   int                                          Result;
   Byte_t                                       ResponseCode;
   MAPM_Entry_Info_t                           *MAPMEntryInfo;
   MAPM_Message_Status_Confirmation_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Message Status Confirmation: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coSetMessageStatus)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Set Message Status Response    */
                        /* command.                                     */
                        if((Result = _MAP_Set_Message_Status_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_MESSAGE_STATUS_CONFIRMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access push*/
   /* message confirmation message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessPushMessageConfirmationMessage(MAPM_Push_Message_Confirmation_Request_t *Message)
{
   int                                        Result;
   Byte_t                                     ResponseCode;
   MAPM_Entry_Info_t                         *MAPMEntryInfo;
   MAPM_Push_Message_Confirmation_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */

         /* Make sure the Message Handle is NULL terminated.            */
         Message->MessageHandle[MAP_MESSAGE_HANDLE_LENGTH] = '\0';

         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && ((Message->ResponseStatusCode != MAPM_RESPONSE_STATUS_CODE_SUCCESS) || ((Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (BTPS_StringLength(Message->MessageHandle)) && (BTPS_StringLength(Message->MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Push Message Confirmation: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coPushMessage)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Send Push Message Response     */
                        /* command.                                     */
                        if((Result = _MAP_Push_Message_Response(MAPMEntryInfo->MAPID, ResponseCode, Message->MessageHandle)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
         {
            if(Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
            {
               if((!BTPS_StringLength(Message->MessageHandle)) || (BTPS_StringLength(Message->MessageHandle) > MAP_MESSAGE_HANDLE_LENGTH))
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_MESSAGE_HANDLE;
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_PUSH_MESSAGE_CONFIRMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access     */
   /* update inbox confirmation message and responds to the message     */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessUpdateInboxConfirmationMessage(MAPM_Update_Inbox_Confirmation_Request_t *Message)
{
   int                                        Result;
   Byte_t                                     ResponseCode;
   MAPM_Entry_Info_t                         *MAPMEntryInfo;
   MAPM_Update_Inbox_Confirmation_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the input parameters appear to be semi-valid.        */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Update Inbox Confirmation: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coUpdateInbox)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Update Inbox Response command. */
                        if((Result = _MAP_Update_Inbox_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_UPDATE_INBOX_CONFIRMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access set */
   /* folder confirmation message and responds to the message           */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSetFolderConfirmationMessage(MAPM_Set_Folder_Confirmation_Request_t *Message)
{
   int                                      Result;
   Byte_t                                   ResponseCode;
   MAPM_Entry_Info_t                       *MAPMEntryInfo;
   MAPM_Set_Folder_Confirmation_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, Message->RemoteDeviceAddress, Message->InstanceID, TRUE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Folder Confirmation: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coSetFolder)
                  {
                     if(MapResponseStatusCodeToResponseCode(Message->ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Set Folder Response command.   */
                        if((Result = _MAP_Set_Folder_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                        {
                           /* If the Set Folder confirmation was        */
                           /* successful, we need to go ahead and note  */
                           /* the new Folder (this is stored in the     */
                           /* PendingPath member.                       */
                           if(Message->ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                           {
                              if(MAPMEntryInfo->CurrentPath)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->CurrentPath);

                                 MAPMEntryInfo->CurrentPath = NULL;
                              }

                              MAPMEntryInfo->CurrentPath = MAPMEntryInfo->PendingPath;

                              MAPMEntryInfo->PendingPath = NULL;
                           }

                           MAPMEntryInfo->CurrentOperation = coNone;
                        }
                     }
                     else
                        Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SET_FOLDER_CONFIRMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function processes the specified Message Access send*/
   /* notification message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendNotificationMessage(MAPM_Send_Notification_Request_t *Message)
{
   int                                Result;
   MAPM_Entry_Info_t                 *MAPMEntryInfo;
   MAPM_Send_Notification_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Verify the Message parameters appear to be semi-valid.      */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList_Notification, Message->RemoteDeviceAddress, Message->InstanceID, FALSE)) != NULL)
            {
               /* Verify that this message is coming from the Client    */
               /* that registered the server or opened the connection to*/
               /* the remote device.                                    */
               if(MAPMEntryInfo->ClientID == Message->MessageHeader.AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Notification: 0x%08X (%d)\n", Message->InstanceID, Message->InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coSendEvent))
                  {
                     /* Initialize the Result to indicate success.      */
                     Result = 0;

                     /* Determine if we need to back up the data we are */
                     /* sending.                                        */
                     if((MAPMEntryInfo->DataBufferSize = Message->EventDataLength) != 0)
                     {
                        /* Free any current data we have buffered       */
                        /* (should be none).                            */
                        if(MAPMEntryInfo->DataBuffer)
                        {
                           BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                           MAPMEntryInfo->DataBuffer = NULL;
                        }

                        /* Go ahead and allocate the buffer (we will not*/
                        /* copy it yet, but we will allocate it so that */
                        /* we don't get an error *AFTER* we send the    */
                        /* first part of the data.                      */
                        if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(Message->EventDataLength)) == NULL)
                           Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                     }

                     /* Flag that we have not sent any data at this     */
                     /* point.                                          */
                     MAPMEntryInfo->DataBufferSent = 0;

                     if(!Result)
                     {
                        if((Result = _MAP_Send_Event_Request(MAPMEntryInfo->MAPID, Message->EventDataLength, Message->EventData, &(MAPMEntryInfo->DataBufferSent), Message->Final)) == 0)
                        {
                           /* Flag that a Push Message Operation is in  */
                           /* progress.                                 */
                           MAPMEntryInfo->CurrentOperation = coSendEvent;

                           /* Copy any remaining data into the buffer   */
                           /* for future operations.                    */
                           if(MAPMEntryInfo->DataBufferSent != Message->EventDataLength)
                              BTPS_MemCopy(MAPMEntryInfo->DataBuffer, Message->EventData, Message->EventDataLength);
                        }

                        /* If there was an error or we sent all of the  */
                        /* data, then we need to free any buffer that   */
                        /* was allocated.                               */
                        if((Result) || (MAPMEntryInfo->DataBufferSent == Message->EventDataLength))
                        {
                           if(MAPMEntryInfo->DataBuffer)
                           {
                              BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                              MAPMEntryInfo->DataBuffer = NULL;
                           }
                        }
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = MAPM_SEND_NOTIFICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the MAP Manager lock */
   /*          held.  This function will release the lock before it     */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).     */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access connect response request.              */
               ProcessConnectionResponseMessage((MAPM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_REGISTER_SERVER:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Server Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_REGISTER_SERVER_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access register server request.               */
               ProcessRegisterServerMessage((MAPM_Register_Server_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Server Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_UN_REGISTER_SERVER_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access un-register server request.            */
               ProcessUnRegisterServerMessage((MAPM_Un_Register_Server_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Service Record Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(((MAPM_Register_Service_Record_Request_t *)Message)->ServiceNameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access register service record request.       */
               ProcessRegisterServiceRecordMessage((MAPM_Register_Service_Record_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVICE_RECORD:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Service Record Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_UN_REGISTER_SERVICE_RECORD_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access un-register service record request.    */
               ProcessUnRegisterServiceRecordMessage((MAPM_Un_Register_Service_Record_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Parse Remote Message Access Services Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access parse remote message access services   */
               /* request.                                              */
               ProcessParseRemoteMessageAccessServicesMessage((MAPM_Parse_Remote_Message_Access_Services_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access connect remote device request.         */
               ProcessConnectRemoteDeviceMessage((MAPM_Connect_Remote_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_DISCONNECT:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_DISCONNECT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access disconnect request.                    */
               ProcessDisconnectMessage((MAPM_Disconnect_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_ABORT:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_ABORT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access abort request.                         */
               ProcessAbortMessage((MAPM_Abort_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_QUERY_CURRENT_FOLDER:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Folder Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_QUERY_CURRENT_FOLDER_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access query current folder request.          */
               ProcessQueryCurrentFolderMessage((MAPM_Query_Current_Folder_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Notifications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_ENABLE_NOTIFICATIONS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access enable notifications request.          */
               ProcessEnableNotificationsMessage((MAPM_Enable_Notifications_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access get folder listing request.            */
               ProcessGetFolderListingMessage((MAPM_Get_Folder_Listing_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Size Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access get folder listing size request.       */
               ProcessGetFolderListingSizeMessage((MAPM_Get_Folder_Listing_Size_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(0, 0, 0))
            {
               if(((!(((MAPM_Get_Message_Listing_Request_t *)Message)->ListingInfoPresent)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(0, 0, ((MAPM_Get_Message_Listing_Request_t *)Message)->FolderNameLength))) || ((((MAPM_Get_Message_Listing_Request_t *)Message)->ListingInfoPresent) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(((MAPM_Get_Message_Listing_Request_t *)Message)->FilterRecipientLength, ((MAPM_Get_Message_Listing_Request_t *)Message)->FilterOriginatorLength, ((MAPM_Get_Message_Listing_Request_t *)Message)->FolderNameLength))))
               {
                  /* Size seems to be valid, go ahead and dispatch the  */
                  /* get message listing request.                       */
                  ProcessGetMessageListingMessage((MAPM_Get_Message_Listing_Request_t *)Message);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Size Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(0, 0, 0))
            {
               if(((!(((MAPM_Get_Message_Listing_Size_Request_t *)Message)->ListingInfoPresent)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(0, 0, ((MAPM_Get_Message_Listing_Size_Request_t *)Message)->FolderNameLength))) || ((((MAPM_Get_Message_Listing_Size_Request_t *)Message)->ListingInfoPresent) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(((MAPM_Get_Message_Listing_Size_Request_t *)Message)->FilterRecipientLength, ((MAPM_Get_Message_Listing_Size_Request_t *)Message)->FilterOriginatorLength, ((MAPM_Get_Message_Listing_Size_Request_t *)Message)->FolderNameLength))))
               {
                  /* Size seems to be valid, go ahead and dispatch the  */
                  /* get message listing size request.                  */
                  ProcessGetMessageListingSizeMessage((MAPM_Get_Message_Listing_Size_Request_t *)Message);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access get message request.                   */
               ProcessGetMessageMessage((MAPM_Get_Message_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Message Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_MESSAGE_STATUS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access set message status request.            */
               ProcessSetMessageStatusMessage((MAPM_Set_Message_Status_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Message Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_REQUEST_SIZE(0, 0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_REQUEST_SIZE(((MAPM_Push_Message_Request_t *)Message)->FolderNameLength, ((MAPM_Push_Message_Request_t *)Message)->MessageLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access push message request.                  */
               ProcessPushMessageMessage((MAPM_Push_Message_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_UPDATE_INBOX:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Inbox Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_UPDATE_INBOX_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access update inbox request.                  */
               ProcessUpdateInboxMessage((MAPM_Update_Inbox_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_FOLDER:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Folder Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_REQUEST_SIZE(((MAPM_Set_Folder_Request_t *)Message)->FolderNameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access set folder request.                    */
               ProcessSetFolderMessage((MAPM_Set_Folder_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_FOLDER_ABSOLUTE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Folder (Absolute) Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_ABSOLUTE_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_ABSOLUTE_REQUEST_SIZE(((MAPM_Set_Folder_Absolute_Request_t *)Message)->FolderNameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access set folder (absolute) request.         */
               ProcessSetFolderAbsoluteMessage((MAPM_Set_Folder_Absolute_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_CONFIRMATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Notifications Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_ENABLE_NOTIFICATIONS_CONFIRMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access enable notifications confirmation      */
               /* request.                                              */
               ProcessEnableNotificationsConfirmationMessage((MAPM_Enable_Notifications_Confirmation_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Folder Listing Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_FOLDER_LISTING_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_FOLDER_LISTING_REQUEST_SIZE(((MAPM_Send_Folder_Listing_Request_t *)Message)->FolderListingLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access send folder listing request.           */
               ProcessSendFolderListingMessage((MAPM_Send_Folder_Listing_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Folder Listing Size Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_FOLDER_LISTING_SIZE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access send folder listing size request.      */
               ProcessSendFolderListingSizeMessage((MAPM_Send_Folder_Listing_Size_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Message Listing Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_LISTING_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_LISTING_REQUEST_SIZE(((MAPM_Send_Message_Listing_Request_t *)Message)->MessageListingLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access send message listing request.          */
               ProcessSendMessageListingMessage((MAPM_Send_Message_Listing_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Message Listing Size Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_LISTING_SIZE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access send message listing size request.     */
               ProcessSendMessageListingSizeMessage((MAPM_Send_Message_Listing_Size_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SEND_MESSAGE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Message Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_REQUEST_SIZE(((MAPM_Send_Message_Request_t *)Message)->MessageDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access send message request.                  */
               ProcessSendMessageMessage((MAPM_Send_Message_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_MESSAGE_STATUS_CONFIRMATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Message Status Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_MESSAGE_STATUS_CONFIRMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access send message status request.           */
               ProcessSendMessageStatusMessage((MAPM_Message_Status_Confirmation_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_CONFIRMATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Message Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_CONFIRMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access push message confirmation request.     */
               ProcessPushMessageConfirmationMessage((MAPM_Push_Message_Confirmation_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_CONFIRMATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Inbox Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_UPDATE_INBOX_CONFIRMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access update inbox confirmation request.     */
               ProcessUpdateInboxConfirmationMessage((MAPM_Update_Inbox_Confirmation_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_FOLDER_CONFIRMATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Folder Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_CONFIRMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access set folder confirmation request.       */
               ProcessSetFolderConfirmationMessage((MAPM_Set_Folder_Confirmation_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SEND_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Notification Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_NOTIFICATION_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SEND_NOTIFICATION_REQUEST_SIZE(((MAPM_Send_Notification_Request_t *)Message)->EventDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access send notification request.             */
               ProcessSendNotificationMessage((MAPM_Send_Notification_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   unsigned int        LoopCount;
   MAPM_Entry_Info_t  *MAPMEntryInfo;
   MAPM_Entry_Info_t **_MAPMEntryInfoList;
   MAPM_Entry_Info_t  *tmpMAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* We need to loop through both lists as there could be client    */
      /* registrations in any of the lists.                             */
      LoopCount = 2;
      while(LoopCount--)
      {
         if(LoopCount)
         {
            MAPMEntryInfo      = MAPMEntryInfoList;
            _MAPMEntryInfoList = &MAPMEntryInfoList;
         }
         else
         {
            MAPMEntryInfo      = MAPMEntryInfoList_Notification;
            _MAPMEntryInfoList = &MAPMEntryInfoList_Notification;
         }

         while(MAPMEntryInfo)
         {
            /* Check to see if the current Client Information is the one*/
            /* that is being un-registered.                             */
            if(MAPMEntryInfo->ClientID == ClientID)
            {
               /* Note the next MAP Entry in the list (we are about to  */
               /* delete the current entry).                            */
               tmpMAPMEntryInfo = MAPMEntryInfo->NextMAPMEntryInfoPtr;

               /* Go ahead and delete the MAP Information Entry and     */
               /* clean up the resources.                               */
               if((MAPMEntryInfo = DeleteMAPMEntryInfoEntry(_MAPMEntryInfoList, MAPMEntryInfo->TrackingID)) != NULL)
               {
                  if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
                  {
                     /* Close any open server.                          */
                     if(MAPMEntryInfo->MAPID)
                        _MAP_Close_Server(MAPMEntryInfo->MAPID);
                  }
                  else
                  {
                     /* If we are connected, go ahead and close the     */
                     /* connection.                                     */
                     if(MAPMEntryInfo->MAPID)
                        _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                  }

                  CleanupMAPMEntryInfo(MAPMEntryInfo);

                  /* All finished with the memory so free the entry.    */
                  FreeMAPMEntryInfoEntryMemory(MAPMEntryInfo);
               }

               /* Go ahead and set the next MAP Information Entry (past */
               /* the one we just deleted).                             */
               MAPMEntryInfo = tmpMAPMEntryInfo;
            }
            else
               MAPMEntryInfo = MAPMEntryInfo->NextMAPMEntryInfoPtr;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* open request indication event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Message Access Manager information held.      */
static void ProcessOpenRequestIndicationEvent(MAP_Open_Request_Indication_Data_t *OpenRequestIndicationData)
{
   int                                Result;
   void                              *CallbackParameter;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   MAPM_Event_Data_t                  MAPMEventData;
   MAPM_Entry_Info_t                 *MAPMEntryInfo;
   MAPM_Entry_Info_t                 *tmpMAPMEntryInfo;
   MAPM_Event_Callback_t              EventCallback;
   MAPM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that the pointer to the event data is not NULL.             */
   if(OpenRequestIndicationData)
   {
      /* Check to see if we can find the indicated server instance.     */
      /* * NOTE * This entry could be either a MAP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, OpenRequestIndicationData->MAPID)) == NULL)
      {
         /* Search to see if this is an open to a local Notification    */
         /* Server.                                                     */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, OpenRequestIndicationData->MAPID)) != NULL)
         {
            /* Walk the MCE Connection list and verify that at least one*/
            /* MCE connection to this BD_ADDR has enabled notifications.*/
            tmpMAPMEntryInfo = MAPMEntryInfoList;
            while(tmpMAPMEntryInfo)
            {
               /* Verify that this is an MCE connection.                */
               if(!(tmpMAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER))
               {
                  /* Verify that notifications our enabled for this MCE */
                  /* connection (or an enable is pending).              */
                  if(tmpMAPMEntryInfo->Flags & (MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED | MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION))
                  {
                     /* Verify that the BD_ADDR of the MCE connection is*/
                     /* same as BD_ADDR that is attempting to           */
                     /* connection.                                     */
                     if(COMPARE_BD_ADDR(tmpMAPMEntryInfo->RemoteDeviceAddress, MAPMEntryInfo->RemoteDeviceAddress))
                        break;
                  }
               }

               tmpMAPMEntryInfo = tmpMAPMEntryInfo->NextMAPMEntryInfoPtr;
            }

            if(tmpMAPMEntryInfo)
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MAP Client Connection to this device has enabled notifications, accepting\n"));

               _MAP_Open_Request_Response(OpenRequestIndicationData->MAPID, TRUE);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MAP Client Connection has not enabled notifications to this device, rejecting\n"));

               _MAP_Open_Request_Response(OpenRequestIndicationData->MAPID, FALSE);
            }

            MAPMEntryInfo = NULL;
         }
      }

      if(MAPMEntryInfo)
      {
         /* Record the address of the remote device.                    */
         MAPMEntryInfo->RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;

         /* Check whether any connection flags are set.                 */
         if(!MAPMEntryInfo->ConnectionFlags)
         {
            /* Simply Accept the connection.                            */
            _MAP_Open_Request_Response(OpenRequestIndicationData->MAPID, TRUE);
         }
         else
         {
            /* Check if authorization is required.                      */
            if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHORIZATION)
            {
               MAPMEntryInfo->ConnectionState = csAuthorizing;

               /* Dispatch the event based upon the client registration */
               /* type.                                                 */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Dispatch the event locally.                        */

                  /* Event needs to be dispatched.  Go ahead and format */
                  /* the event.                                         */
                  BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                  /* Format the event data.                             */
                  MAPMEventData.EventType                                                        = metMAPIncomingConnectionRequest;
                  MAPMEventData.EventLength                                                      = MAPM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

                  MAPMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;
                  MAPMEventData.EventData.IncomingConnectionRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;

                  /* Note the Callback information.                     */
                  EventCallback                                                                  = MAPMEntryInfo->CallbackFunction;
                  CallbackParameter                                                              = MAPMEntryInfo->CallbackParameter;

                  /* Release the Lock so we can dispatch the event.     */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                        (*EventCallback)(&MAPMEventData, CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
               else
               {
                  /* Dispatch the event remotely.                       */

                  /* Format the message.                                */
                  BTPS_MemInitialize(&Message, 0, sizeof(Message));

                  Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
                  Message.MessageHeader.MessageLength   = (MAPM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  Message.RemoteDeviceAddress           = OpenRequestIndicationData->BD_ADDR;
                  Message.InstanceID                    = MAPMEntryInfo->InstanceID;

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&Message);
               }
            }
            else
            {
               /* Determine if authentication or encryption is required.*/
               if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Accept the connection.                             */
                  Result = _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, TRUE);

                  if(Result)
                     _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);
                  else
                  {
                     /* Update the current connection state.            */
                     MAPMEntryInfo->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* Set the connection state.                          */
                  if(!Result)
                  {
                     if(Encrypt)
                        MAPMEntryInfo->ConnectionState = csEncrypting;
                     else
                        MAPMEntryInfo->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Reject the request.                             */
                     _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);
                  }
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* open indication event that has been received with the specified   */
   /* information.  This function should be called with the lock        */
   /* protecting the Message Access Manager information held.           */
static void ProcessOpenPortIndicationEvent(MAP_Open_Port_Indication_Data_t *OpenPortIndicationData)
{
   void                            *CallbackParameter;
   MAPM_Event_Data_t                MAPMEventData;
   MAPM_Entry_Info_t               *MAPMEntryInfo;
   MAPM_Event_Callback_t            EventCallback;
   MAPM_Device_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(OpenPortIndicationData)
   {
      /* Find the list entry by the MAPID.                              */
      /* * NOTE * This entry could be either a MAP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, OpenPortIndicationData->MAPID)) == NULL)
      {
         /* If this a notification server, we need to actually map the  */
         /* Open Indication Event to the correct client entry (so we can*/
         /* dispatch the correct information.                           */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, OpenPortIndicationData->MAPID)) != NULL)
         {
            /* Flag that this Notification Server Entry is now          */
            /* connected.                                               */
            MAPMEntryInfo->ConnectionState     = csConnected;

            /* If this is a local notification server then we will just */
            /* save the BD_ADDR of the connection (and we won't dispatch*/
            /* a connection event).                                     */
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
            {
               /* Save the BD_ADDR of the connection.                   */
               MAPMEntryInfo->RemoteDeviceAddress = OpenPortIndicationData->BD_ADDR;

               /* Clear the MAP Entry Info so that no event is          */
               /* dispatched.                                           */
               MAPMEntryInfo                      = NULL;
            }
         }
      }

      if(MAPMEntryInfo)
      {
         /* Flag that this Server Entry is now connected.               */
         MAPMEntryInfo->ConnectionState     = csConnected;

         /* If this is a local notification server then we will just    */
         /* save the BD_ADDR of the connection (and we won't dispatch a */
         /* connection event).                                          */
         if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
         {
            /* Save the BD_ADDR of the connection.                      */
            MAPMEntryInfo->RemoteDeviceAddress = OpenPortIndicationData->BD_ADDR;
         }

         /* Dispatch the event based upon the client registration type. */
         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            /* Format the event data.                                   */
            MAPMEventData.EventType                                        = metMAPConnected;
            MAPMEventData.EventLength                                      = MAPM_CONNECTED_EVENT_DATA_SIZE;

            MAPMEventData.EventData.ConnectedEventData.ConnectionType      = mctMessageAccessServer;
            MAPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.ConnectedEventData.InstanceID          = MAPMEntryInfo->InstanceID;

            /* Note the Callback information.                           */
            EventCallback                                                  = MAPMEntryInfo->CallbackFunction;
            CallbackParameter                                              = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_DEVICE_CONNECTED;
            Message.MessageHeader.MessageLength   = (MAPM_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.ConnectionType                = mctMessageAccessServer;
            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Mesage     */
   /* Access open confirmation event that has been received with the    */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Message Access Manager information held.      */
static void ProcessOpenPortConfirmationEvent(MAP_Open_Port_Confirmation_Data_t *OpenPortConfirmationData)
{
   void                             *CallbackParameter;
   Boolean_t                         Notification;
   unsigned int                      ConnectionStatus;
   MAPM_Event_Data_t                 MAPMEventData;
   MAPM_Entry_Info_t                *MAPMEntryInfo;
   MAPM_Event_Callback_t             EventCallback;
   MAPM_Connection_Status_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(OpenPortConfirmationData)
   {
      /***************************** NOTE *******************************/
      /* * This function can be called internally to clean up failed  * */
      /* * connections (or connections that need to be closed).  It   * */
      /* * is possible that a MAP ID has not been assigned to the     * */
      /* * Entry during some portions of out-going connections.  To   * */
      /* * handle this case and allow the use of this function for    * */
      /* * cleanup, the Tracking ID will be used in place of the MAP  * */
      /* * ID.  This will be signified by the Most Significant bit of * */
      /* * of the MAP ID being set (this ID cannot occur as a normal  * */
      /* * MAP ID so there will be no conflicts).                     * */
      /***************************** NOTE *******************************/
      if(OpenPortConfirmationData->MAPID & 0x80000000)
      {
         OpenPortConfirmationData->MAPID &= 0x7FFFFFFF;

         if((MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, OpenPortConfirmationData->MAPID)) == NULL)
         {
            if((MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList_Notification, OpenPortConfirmationData->MAPID)) != NULL)
               Notification = TRUE;
            else
               Notification = FALSE;
         }
         else
            Notification = FALSE;
      }
      else
      {
         /* Find the list entry by the MAPID.                           */
         /* * NOTE * This entry could be either a MAP Server Entry or a */
         /*          Notification Server Entry.                         */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, OpenPortConfirmationData->MAPID)) == NULL)
         {
            if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, OpenPortConfirmationData->MAPID)) != NULL)
               Notification = TRUE;
            else
               Notification = FALSE;
         }
         else
            Notification = FALSE;
      }

      if(MAPMEntryInfo)
      {
         /* First, let's map the status to the correct Message Access   */
         /* Manager status.                                             */
         switch(OpenPortConfirmationData->OpenStatus)
         {
            case MAP_OPEN_STATUS_SUCCESS:
               ConnectionStatus = MAPM_CONNECTION_STATUS_SUCCESS;
               break;
            case MAP_OPEN_STATUS_CONNECTION_TIMEOUT:
               ConnectionStatus = MAPM_CONNECTION_STATUS_FAILURE_TIMEOUT;
               break;
            case MAP_OPEN_STATUS_CONNECTION_REFUSED:
               ConnectionStatus = MAPM_CONNECTION_STATUS_FAILURE_REFUSED;
               break;
            case MAP_OPEN_STATUS_UNKNOWN_ERROR:
            default:
               ConnectionStatus = MAPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
               break;
         }

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Status, %d %d\n", OpenPortConfirmationData->OpenStatus, ConnectionStatus));

         /* Check the connection status.                                */
         if(ConnectionStatus == MAPM_CONNECTION_STATUS_SUCCESS)
            MAPMEntryInfo->ConnectionState = csConnected;
         else
         {
            /* If this was not a synchronous connection, go ahead and   */
            /* delete it from the list (we will free the resources at   */
            /* the end).                                                */
            if(!MAPMEntryInfo->ConnectionEvent)
               MAPMEntryInfo = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfo->TrackingID);
         }

         if(MAPMEntryInfo)
         {
            /* Dispatch the event based upon the client registration    */
            /* type.                                                    */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* If this was a synchronous event we need to set the    */
               /* status and the event.                                 */
               if(MAPMEntryInfo->ConnectionEvent)
               {
                  /* Synchronous event, go ahead and set the correct    */
                  /* status, then set the event.                        */
                  MAPMEntryInfo->ConnectionStatus = ConnectionStatus;

                  BTPS_SetEvent(MAPMEntryInfo->ConnectionEvent);
               }
               else
               {
                  /* Event needs to be dispatched.  Go ahead and format */
                  /* the event.                                         */
                  BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                  /* Format the event data.                             */
                  MAPMEventData.EventType                                               = metMAPConnectionStatus;
                  MAPMEventData.EventLength                                             = MAPM_CONNECTION_STATUS_EVENT_DATA_SIZE;

                  MAPMEventData.EventData.ConnectionStatusEventData.ConnectionType      = Notification?mctNotificationClient:mctMessageAccessClient;
                  MAPMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
                  MAPMEventData.EventData.ConnectionStatusEventData.InstanceID          = MAPMEntryInfo->InstanceID;
                  MAPMEventData.EventData.ConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

                  /* Note the Callback information.                     */
                  EventCallback                                                         = MAPMEntryInfo->CallbackFunction;
                  CallbackParameter                                                     = MAPMEntryInfo->CallbackParameter;

                  /* Release the Lock so we can dispatch the event.     */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                        (*EventCallback)(&MAPMEventData, CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_CONNECTION_STATUS;
               Message.MessageHeader.MessageLength   = (MAPM_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               Message.ConnectionType                = Notification?mctNotificationClient:mctMessageAccessClient;
               Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               Message.InstanceID                    = MAPMEntryInfo->InstanceID;
               Message.ConnectionStatus              = ConnectionStatus;

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&Message);
            }

            /* If the entry was deleted, we need to free any resources  */
            /* that were allocated.                                     */
            if(ConnectionStatus != MAPM_CONNECTION_STATUS_SUCCESS)
            {
               CleanupMAPMEntryInfo(MAPMEntryInfo);

               FreeMAPMEntryInfoEntryMemory(MAPMEntryInfo);
            }
         }
      }

   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access close port indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Message Access Manager information held.      */
static void ProcessClosePortIndicationEvent(MAP_Close_Port_Indication_Data_t *ClosePortIndicationData)
{
   void                               *CallbackParameter;
   Boolean_t                           Notification;
   MAPM_Event_Data_t                   MAPMEventData;
   MAPM_Entry_Info_t                  *MAPMEntryInfo;
   MAPM_Event_Callback_t               EventCallback;
   MAPM_Device_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to ensure the event data pointer parameter is not NULL.     */
   if(ClosePortIndicationData)
   {
      /* Search by MAPID.                                               */
      /* Find the list entry by the MAPID.                              */
      /* * NOTE * This entry could be either a MAP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, ClosePortIndicationData->MAPID)) == NULL)
      {
         if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, ClosePortIndicationData->MAPID)) != NULL)
         {
            /* Flag that this is a notification connection that just    */
            /* disconnected.                                            */
            Notification = TRUE;

            /* Check to see if this is a notification Server or Client  */
            /* connection.                                              */
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
            {
               /* Cleanup information for the connection.               */
               CleanupMAPMEntryInfo(MAPMEntryInfo);

               MAPMEntryInfo->ConnectionState = csIdle;

               ASSIGN_BD_ADDR(MAPMEntryInfo->RemoteDeviceAddress, 0, 0, 0, 0, 0, 0);

               /* We don't dispatch connects or disconnects for         */
               /* notification servers so NULL the entry.               */
               MAPMEntryInfo = NULL;
            }
         }
         else
            Notification = FALSE;
      }
      else
      {
         /* Flag that this is not a notification connection.            */
         Notification = FALSE;

         /* If this is an MCE connection we may need to clean-up local  */
         /* Notification Server information if the local device has     */
         /* registered for notifications with the MSE.                  */
         if(!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER))
         {
            /* Check to see if notifications are enabled for this MCE   */
            /* connection.                                              */
            if(MAPMEntryInfo->Flags & (MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED | MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION))
               HandleNotificationDisconnection(MAPMEntryInfo, &NotificationInfo);
         }
         else
         {
            /* Set the state to idle.                                   */
            MAPMEntryInfo->ConnectionState = csIdle;

            /* Reset the Bluetooth Address.                             */
            ASSIGN_BD_ADDR(MAPMEntryInfo->RemoteDeviceAddress, 0, 0, 0, 0, 0, 0);
         }
      }

      if((MAPMEntryInfo) && ((MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER) || ((MAPMEntryInfo = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfo->TrackingID)) != NULL)))
      {
         /* Entry has been deleted.                                     */

         /* Next, determine the connection type (could be a client or   */
         /* server) and could be Notification as well.                  */
         if(Notification)
         {
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
               MAPMEventData.EventData.DisconnectedEventData.ConnectionType = mctNotificationServer;
            else
               MAPMEventData.EventData.DisconnectedEventData.ConnectionType = mctNotificationClient;
         }
         else
         {
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
               MAPMEventData.EventData.DisconnectedEventData.ConnectionType = mctMessageAccessServer;
            else
               MAPMEventData.EventData.DisconnectedEventData.ConnectionType = mctMessageAccessClient;
         }

         /* Dispatch the event based upon the client registration type. */
         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            /* Format the event data.                                   */
            MAPMEventData.EventType                                           = metMAPDisconnected;
            MAPMEventData.EventLength                                         = MAPM_DISCONNECTED_EVENT_DATA_SIZE;

            MAPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.DisconnectedEventData.InstanceID          = MAPMEntryInfo->InstanceID;

            /* Note the Callback information.                           */
            EventCallback                                                     = MAPMEntryInfo->CallbackFunction;
            CallbackParameter                                                 = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED;
            Message.MessageHeader.MessageLength   = (MAPM_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.ConnectionType                = MAPMEventData.EventData.DisconnectedEventData.ConnectionType;
            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }

         /* All finished with the MAP Entry, go ahead and clean it up.  */
         CleanupMAPMEntryInfo(MAPMEntryInfo);

         /* If this is a Client entry it has been deleted and we should */
         /* free the list entry memory.                                 */
         if(!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER))
            FreeMAPMEntryInfoEntryMemory(MAPMEntryInfo);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access send event indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Message Access Manager information held.      */
static void ProcessSendEventIndicationEvent(MAP_Send_Event_Indication_Data_t *SendEventIndicationData)
{
   void                                   *CallbackParameter;
   Byte_t                                  ResponseCode;
   unsigned int                            TrackingID;
   MAPM_Event_Data_t                       MAPMEventData;
   MAPM_Entry_Info_t                      *MAPMEntryInfo;
   MAPM_Entry_Info_t                      *_MAPMEntryInfo;
   MAPM_Event_Callback_t                   EventCallback;
   MAPM_Notification_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(SendEventIndicationData)
   {
      /* Search for the MAPID in the entry list (the MAP ID specifies a */
      /* Notification server).                                          */
      if(((_MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, SendEventIndicationData->MAPID)) != NULL) && ((_MAPMEntryInfo->CurrentOperation == coNone) || (_MAPMEntryInfo->CurrentOperation == coSendEvent)))
      {
         /* We have found the notification server.  Now we need to find */
         /* the MAP Client.                                             */
         if((((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, _MAPMEntryInfo->RemoteDeviceAddress, SendEventIndicationData->MASInstanceID, FALSE)) != NULL)) && (MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED))
         {
            /* Go ahead and initialize the response (either OK or a     */
            /* continue).                                               */
            if(SendEventIndicationData->Final)
            {
               ResponseCode                     = MAP_OBEX_RESPONSE_OK;
               _MAPMEntryInfo->CurrentOperation = coNone;
            }
            else
            {
               ResponseCode                     = MAP_OBEX_RESPONSE_CONTINUE;
               _MAPMEntryInfo->CurrentOperation = coSendEvent;
            }

            /* Process differently based upon the client registration   */
            /* type.                                                    */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                     = metMAPNotificationIndication;
               MAPMEventData.EventLength                                                   = MAPM_NOTIFICATION_INDICATION_EVENT_DATA_SIZE;

               MAPMEventData.EventData.NotificationIndicationEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.NotificationIndicationEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.NotificationIndicationEventData.Final               = SendEventIndicationData->Final;

               if((MAPMEventData.EventData.NotificationIndicationEventData.EventReportLength = SendEventIndicationData->DataLength) != 0)
                  MAPMEventData.EventData.NotificationIndicationEventData.EventReportData = SendEventIndicationData->DataBuffer;

               /* Note the Callback information.                        */
               TrackingID        = _MAPMEntryInfo->TrackingID;
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();

               /* Make sure that the Entry wasn't deleted during the    */
               /* callback.                                             */
               _MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList_Notification, TrackingID);
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */

               /* First, allocate a message of the correct size.        */
               if((Message = (MAPM_Notification_Indication_Message_t *)BTPS_AllocateMemory(MAPM_NOTIFICATION_INDICATION_MESSAGE_SIZE(SendEventIndicationData->DataLength))) != NULL)
               {
                  BTPS_MemInitialize(Message, 0, MAPM_NOTIFICATION_INDICATION_MESSAGE_SIZE(SendEventIndicationData->DataLength));

                  Message->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  Message->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_NOTIFICATION_INDICATION;
                  Message->MessageHeader.MessageLength   = MAPM_NOTIFICATION_INDICATION_MESSAGE_SIZE(SendEventIndicationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE;

                  Message->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  Message->InstanceID                    = MAPMEntryInfo->InstanceID;
                  Message->Final                         = SendEventIndicationData->Final;

                  if((Message->EventDataLength = SendEventIndicationData->DataLength) != 0)
                     BTPS_MemCopy(Message->EventData, SendEventIndicationData->DataBuffer, SendEventIndicationData->DataLength);

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)Message);

                  /* Go ahead and free the memory because we are        */
                  /* finished with it.                                  */
                  BTPS_FreeMemory(Message);
               }
               else
               {
                  /* Error allocating memory, flag an error.            */
                  ResponseCode                     = MAP_OBEX_RESPONSE_SERVER_ERROR;

                  _MAPMEntryInfo->CurrentOperation = coNone;
               }
            }

            if(_MAPMEntryInfo)
            {
               /* Respond to the request.                               */
               if(_MAP_Send_Event_Response(SendEventIndicationData->MAPID, ResponseCode))
                  _MAPMEntryInfo->CurrentOperation = coNone;
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find corresponding Message Access Client: %d\n", (int)SendEventIndicationData->MASInstanceID));

            _MAP_Send_Event_Response(SendEventIndicationData->MAPID, (Byte_t)MAP_OBEX_RESPONSE_BAD_REQUEST);

            _MAPMEntryInfo->CurrentOperation = coNone;
         }
      }
      else
      {
         if(!_MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", SendEventIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)_MAPMEntryInfo->CurrentOperation));

         _MAP_Send_Event_Response(SendEventIndicationData->MAPID, (Byte_t)(_MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access send event confirmation event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessSendEventConfirmationEvent(MAP_Send_Event_Confirmation_Data_t *SendEventConfirmationData)
{
   void                                     *CallbackParameter;
   Boolean_t                                 DispatchEvent;
   unsigned int                              DataLength;
   MAPM_Event_Data_t                         MAPMEventData;
   MAPM_Entry_Info_t                        *MAPMEntryInfo;
   MAPM_Entry_Info_t                        *_MAPMEntryInfo;
   MAPM_Event_Callback_t                     EventCallback;
   MAPM_Notification_Confirmation_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(SendEventConfirmationData)
   {
      /* Search for the MAPID in the entry list (the MAP ID specifies a */
      /* Notification client).                                          */
      if(((_MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, SendEventConfirmationData->MAPID)) != NULL) && (_MAPMEntryInfo->CurrentOperation == coSendEvent))
      {
         /* We have found the notification client.  Now we need to find */
         /* the MAP Server.                                             */
         if(((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, _MAPMEntryInfo->RemoteDeviceAddress, _MAPMEntryInfo->InstanceID, TRUE)) != NULL))
         {
            /* Flag that we do not need to dispatch the event           */
            /* confirmation (this might change below).                  */
            DispatchEvent = FALSE;

            /* Determine if we need to send more data.                  */
            if(((SendEventConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_OK) || (SendEventConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)) && (_MAPMEntryInfo->DataBufferSize) && (_MAPMEntryInfo->DataBuffer) && (!(_MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)))
            {
               /* Calculate the remaining data to send.                 */
               DataLength = MAPMEntryInfo->DataBufferSize - MAPMEntryInfo->DataBufferSent;

               if(_MAP_Send_Event_Request(_MAPMEntryInfo->MAPID, DataLength, &(_MAPMEntryInfo->DataBuffer[_MAPMEntryInfo->DataBufferSent]), &DataLength, _MAPMEntryInfo->DataFinal) > 0)
               {
                  _MAPMEntryInfo->DataBufferSent += DataLength;

                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)_MAPMEntryInfo->MAPID, _MAPMEntryInfo->DataBufferSent));
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)_MAPMEntryInfo->MAPID, _MAPMEntryInfo->DataBufferSent));

                  /* Flag that we have sent all the data (so it can be  */
                  /* freed below).                                      */
                  _MAPMEntryInfo->DataBufferSent = _MAPMEntryInfo->DataBufferSize;

                  DispatchEvent                  = TRUE;
               }

               /* Free any memory that was allocated (if we have sent   */
               /* all the data).                                        */
               if((MAPMEntryInfo) && (_MAPMEntryInfo->DataBufferSent == _MAPMEntryInfo->DataBufferSize))
               {
                  if(_MAPMEntryInfo->DataBuffer)
                  {
                     BTPS_FreeMemory(_MAPMEntryInfo->DataBuffer);

                     _MAPMEntryInfo->DataBuffer = NULL;
                  }
               }
            }
            else
            {
               DispatchEvent = TRUE;

               /* If we have a pending abort, let's go ahead and clean  */
               /* up everything.                                        */
               if(_MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               {
                  if(_MAPMEntryInfo->DataBuffer)
                  {
                     BTPS_FreeMemory(_MAPMEntryInfo->DataBuffer);

                     _MAPMEntryInfo->DataBuffer = NULL;
                  }
               }
            }

            if(DispatchEvent)
            {
               /* Determine if we need to clear the state.              */
               if((MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT) || (SendEventConfirmationData->ResponseCode != MAP_OBEX_RESPONSE_CONTINUE))
                  MAPMEntryInfo->CurrentOperation = coNone;

               /* Process differently based upon the client registration*/
               /* type.                                                 */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Dispatch the event locally.                        */

                  /* Event needs to be dispatched.  Go ahead and format */
                  /* the event.                                         */
                  BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                  /* Format the event data.                             */
                  MAPMEventData.EventType                                                       = metMAPNotificationConfirmation;
                  MAPMEventData.EventLength                                                     = MAPM_NOTIFICATION_INDICATION_EVENT_DATA_SIZE;

                  MAPMEventData.EventData.NotificationConfirmationEventData.RemoteDeviceAddress = _MAPMEntryInfo->RemoteDeviceAddress;
                  MAPMEventData.EventData.NotificationConfirmationEventData.InstanceID          = MAPMEntryInfo->InstanceID;

                  if(!(_MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
                     MAPMEventData.EventData.NotificationConfirmationEventData.ResponseStatusCode = MapResponseCodeToResponseStatusCode(SendEventConfirmationData->ResponseCode);
                  else
                     MAPMEventData.EventData.NotificationConfirmationEventData.ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

                  /* Note the Callback information.                     */
                  EventCallback     = MAPMEntryInfo->CallbackFunction;
                  CallbackParameter = MAPMEntryInfo->CallbackParameter;

                  /* If there is a pending abort, go ahead and issue the*/
                  /* abort.                                             */
                  if(_MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                     IssuePendingAbort(_MAPMEntryInfo);

                  /* Release the Lock so we can dispatch the event.     */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                         (*EventCallback)(&MAPMEventData, CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
               else
               {
                  /* Dispatch the event remotely.                       */

                  /* Format the message.                                */
                  BTPS_MemInitialize(&Message, 0, sizeof(Message));

                  Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_NOTIFICATION_CONFIRMATION;
                  Message.MessageHeader.MessageLength   = MAPM_NOTIFICATION_CONFIRMATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

                  Message.RemoteDeviceAddress           = _MAPMEntryInfo->RemoteDeviceAddress;
                  Message.InstanceID                    = MAPMEntryInfo->InstanceID;

                  if(!(_MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
                     Message.ResponseStatusCode = MapResponseCodeToResponseStatusCode(SendEventConfirmationData->ResponseCode);
                  else
                     Message.ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

                  /* If there is a pending abort, go ahead and issue the*/
                  /* abort.                                             */
                  if(_MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                     IssuePendingAbort(_MAPMEntryInfo);

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&Message);
               }
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find corresponding Message Access Server: %d\n", (int)_MAPMEntryInfo->InstanceID));

            _MAPMEntryInfo->CurrentOperation = coNone;
         }
      }
      else
      {
         if(!_MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", SendEventConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)_MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access notification registration indication event that has been   */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Message Access Manager        */
   /* information held.                                                 */
static void ProcessNotificationRegistrationIndicationEvent(MAP_Notification_Registration_Indication_Data_t *NotificationRegistrationIndicationData)
{
   void                                        *CallbackParameter;
   MAPM_Event_Data_t                            MAPMEventData;
   MAPM_Entry_Info_t                           *MAPMEntryInfo;
   MAPM_Event_Callback_t                        EventCallback;
   MAPM_Enable_Notifications_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(NotificationRegistrationIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, NotificationRegistrationIndicationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coNone))
      {
         /* Flag the new state we are entering.                         */
         MAPMEntryInfo->CurrentOperation = coEnableNotifications;

         /* Flag the pending Notification state.                        */
         if(NotificationRegistrationIndicationData->NotificationStatus)
            MAPMEntryInfo->Flags |= (unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION;
         else
            MAPMEntryInfo->Flags &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION);

         /* Dispatch the event based upon the client registration type. */
         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            /* Format the event data.                                   */
            MAPMEventData.EventType                                                            = metMAPEnableNotificationsIndication;
            MAPMEventData.EventLength                                                          = MAPM_ENABLE_NOTIFICATIONS_INDICATION_EVENT_DATA_SIZE;

            MAPMEventData.EventData.EnableNotificationsIndicationEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.EnableNotificationsIndicationEventData.InstanceID          = MAPMEntryInfo->InstanceID;
            MAPMEventData.EventData.EnableNotificationsIndicationEventData.Enabled             = NotificationRegistrationIndicationData->NotificationStatus;

            /* Note the Callback information.                           */
            EventCallback                                                                      = MAPMEntryInfo->CallbackFunction;
            CallbackParameter                                                                  = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_REQUEST;
            Message.MessageHeader.MessageLength   = MAPM_ENABLE_NOTIFICATIONS_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;
            Message.Enable                        = NotificationRegistrationIndicationData->NotificationStatus;

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", NotificationRegistrationIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Set_Notification_Registration_Response(NotificationRegistrationIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access notification registration confirmation event that has been */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Message Access Manager        */
   /* information held.                                                 */
static void ProcessNotificationRegistrationConfirmationEvent(MAP_Notification_Registration_Confirmation_Data_t *NotificationRegistrationConfirmationData)
{
   void                                         *CallbackParameter;
   Boolean_t                                     NotificationDisconnect;
   MAPM_Event_Data_t                             MAPMEventData;
   MAPM_Entry_Info_t                            *MAPMEntryInfo;
   MAPM_Event_Callback_t                         EventCallback;
   MAPM_Enable_Notifications_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(NotificationRegistrationConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, NotificationRegistrationConfirmationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coEnableNotifications))
      {
         /* Flag that there is no longer an operation in progress.      */
         MAPMEntryInfo->CurrentOperation = coNone;

         /* Flag that we are not disconnecting the local notification   */
         /* server.                                                     */
         NotificationDisconnect          = FALSE;

         /* If successful, go ahead and note the new notification state.*/
         if(NotificationRegistrationConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_OK)
         {
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION)
               MAPMEntryInfo->Flags |= (unsigned long)MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED;
            else
            {
               /* Flag that notifications are not enabled.              */
               MAPMEntryInfo->Flags &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED);

               /* Handle a disconnect.                                  */
               NotificationDisconnect = TRUE;
            }
         }
         else
         {
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION)
               NotificationDisconnect = TRUE;
         }

         /* Go ahead and clear the pending notification flag.           */
         MAPMEntryInfo->Flags &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION);

         /* Handle a Notification Server disconnection if necessary.    */
         if(NotificationDisconnect)
            HandleNotificationDisconnection(MAPMEntryInfo, &NotificationInfo);

         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            MAPMEventData.EventType                                                          = metMAPEnableNotificationsResponse;
            MAPMEventData.EventLength                                                        = MAPM_ENABLE_NOTIFICATIONS_RESPONSE_EVENT_DATA_SIZE;

            MAPMEventData.EventData.EnableNotificationsResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.EnableNotificationsResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
            MAPMEventData.EventData.EnableNotificationsResponseEventData.ResponseStatusCode  = MapResponseCodeToResponseStatusCode(NotificationRegistrationConfirmationData->ResponseCode);

            /* Note the Callback information.                           */
            EventCallback                                                                    = MAPMEntryInfo->CallbackFunction;
            CallbackParameter                                                                = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_RESPONSE;
            Message.MessageHeader.MessageLength   = MAPM_ENABLE_NOTIFICATIONS_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;
            Message.ResponseStatusCode            = MapResponseCodeToResponseStatusCode(NotificationRegistrationConfirmationData->ResponseCode);

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", NotificationRegistrationConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access get folder listing indication event that has been received */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Message Access Manager information   */
   /* held.                                                             */
static void ProcessGetFolderListingIndicationEvent(MAP_Get_Folder_Listing_Indication_Data_t *GetFolderListingIndicationData)
{
   void                                           *CallbackParameter;
   unsigned int                                    DataLength;
   MAPM_Event_Data_t                               MAPMEventData;
   MAPM_Entry_Info_t                              *MAPMEntryInfo;
   MAPM_Event_Callback_t                           EventCallback;
   MAPM_Get_Folder_Listing_Request_Message_t       GetFolderListingMessage;
   MAPM_Get_Folder_Listing_Size_Request_Message_t  GetFolderListingSizeMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(GetFolderListingIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, GetFolderListingIndicationData->MAPID)) != NULL) && ((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coGetFolderListing)))
      {
         /* Determine if this a Get Folder Listing Size or a Get Folder */
         /* Listing.                                                    */
         if((GetFolderListingIndicationData->MaxListCount) || (MAPMEntryInfo->CurrentOperation == coGetFolderListing))
         {
            /* Determine if we need to send more response data.         */
            if((MAPMEntryInfo->CurrentOperation == coGetFolderListing) && (MAPMEntryInfo->DataBufferSize) && (MAPMEntryInfo->DataBuffer))
            {
               /* Calculate the remaining data to send.                 */
               DataLength = MAPMEntryInfo->DataBufferSize - MAPMEntryInfo->DataBufferSent;

               if(_MAP_Get_Folder_Listing_Response(MAPMEntryInfo->MAPID, (Byte_t)(MAPMEntryInfo->DataFinal?MAP_OBEX_RESPONSE_OK:MAP_OBEX_RESPONSE_CONTINUE), NULL, DataLength, &(MAPMEntryInfo->DataBuffer[MAPMEntryInfo->DataBufferSent]), &DataLength) == 0)
               {
                  MAPMEntryInfo->DataBufferSent += DataLength;

                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));

                  /* Error submitting response.  Not sure what we can do*/
                  /* here.                                              */
                  _MAP_Get_Folder_Listing_Response(MAPMEntryInfo->MAPID, (Byte_t)(MAP_OBEX_RESPONSE_SERVER_ERROR), NULL, 0, NULL, NULL);

                  /* Flag that we have sent all the data (so it can be  */
                  /* freed below).                                      */
                  MAPMEntryInfo->DataBufferSent   = MAPMEntryInfo->DataBufferSize;

                  /* Flag that there is no longer an operation in       */
                  /* progress.                                          */
                  MAPMEntryInfo->CurrentOperation = coNone;
               }

               /* Free any memory that was allocated (if we have sent   */
               /* all the data).                                        */
               if((MAPMEntryInfo) && (MAPMEntryInfo->DataBufferSent == MAPMEntryInfo->DataBufferSize))
               {
                  if(MAPMEntryInfo->DataBuffer)
                  {
                     BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                     MAPMEntryInfo->DataBuffer = NULL;
                  }

                  MAPMEntryInfo->CurrentOperation = coNone;
               }
            }
            else
            {
               /* Get Folder Listing Request.                           */
               MAPMEntryInfo->CurrentOperation = coGetFolderListing;

               /* Free any left-over data (just to be safe).            */
               if(MAPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                  MAPMEntryInfo->DataBuffer = NULL;
               }

               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Dispatch the event locally.                        */

                  /* Event needs to be dispatched.  Go ahead and format */
                  /* the event.                                         */
                  BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                  /* Format the event data.                             */
                  MAPMEventData.EventType                                                      = metMAPGetFolderListingRequest;
                  MAPMEventData.EventLength                                                    = MAPM_GET_FOLDER_LISTING_REQUEST_EVENT_DATA_SIZE;

                  MAPMEventData.EventData.GetFolderListingRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
                  MAPMEventData.EventData.GetFolderListingRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;
                  MAPMEventData.EventData.GetFolderListingRequestEventData.MaxListCount        = GetFolderListingIndicationData->MaxListCount;
                  MAPMEventData.EventData.GetFolderListingRequestEventData.ListStartOffset     = GetFolderListingIndicationData->ListStartOffset;

                  /* Note the Callback information.                     */
                  EventCallback                                                                = MAPMEntryInfo->CallbackFunction;
                  CallbackParameter                                                            = MAPMEntryInfo->CallbackParameter;

                  /* Release the Lock so we can dispatch the event.     */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                         (*EventCallback)(&MAPMEventData, CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
               else
               {
                  /* Dispatch the event remotely.                       */

                  /* Format the message.                                */
                  BTPS_MemInitialize(&GetFolderListingMessage, 0, sizeof(GetFolderListingMessage));

                  GetFolderListingMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  GetFolderListingMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  GetFolderListingMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  GetFolderListingMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_REQUEST;
                  GetFolderListingMessage.MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

                  GetFolderListingMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  GetFolderListingMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
                  GetFolderListingMessage.MaxListCount                  = GetFolderListingIndicationData->MaxListCount;
                  GetFolderListingMessage.ListStartOffset               = GetFolderListingIndicationData->ListStartOffset;

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&GetFolderListingMessage);
               }
            }
         }
         else
         {
            /* Get Folder Listing Size Request.                         */
            MAPMEntryInfo->CurrentOperation = coGetFolderListingSize;

            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                          = metMAPGetFolderListingSizeRequest;
               MAPMEventData.EventLength                                                        = MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_EVENT_DATA_SIZE;

               MAPMEventData.EventData.GetFolderListingSizeRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.GetFolderListingSizeRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;

               /* Note the Callback information.                        */
               EventCallback                                                                    = MAPMEntryInfo->CallbackFunction;
               CallbackParameter                                                                = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */
               BTPS_MemInitialize(&GetFolderListingSizeMessage, 0, sizeof(GetFolderListingSizeMessage));

               GetFolderListingSizeMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               GetFolderListingSizeMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               GetFolderListingSizeMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               GetFolderListingSizeMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_REQUEST;
               GetFolderListingSizeMessage.MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

               GetFolderListingSizeMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               GetFolderListingSizeMessage.InstanceID                    = MAPMEntryInfo->InstanceID;

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&GetFolderListingSizeMessage);
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", GetFolderListingIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Get_Folder_Listing_Response(GetFolderListingIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST), NULL, 0, NULL, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access get folder listing confirmation event that has been        */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Message Access Manager        */
   /* information held.                                                 */
static void ProcessGetFolderListingConfirmationEvent(MAP_Get_Folder_Listing_Confirmation_Data_t *GetFolderListingConfirmationData)
{
   void                                            *CallbackParameter;
   unsigned int                                     TrackingID;
   MAPM_Event_Data_t                                MAPMEventData;
   MAPM_Entry_Info_t                               *MAPMEntryInfo;
   MAPM_Event_Callback_t                            EventCallback;
   MAPM_Get_Folder_Listing_Response_Message_t       ErrorGetFolderListingMessage;
   MAPM_Get_Folder_Listing_Response_Message_t      *GetFolderListingMessage = NULL;
   MAPM_Get_Folder_Listing_Size_Response_Message_t  GetFolderListingSizeMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(GetFolderListingConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, GetFolderListingConfirmationData->MAPID)) != NULL) && ((MAPMEntryInfo->CurrentOperation == coGetFolderListing) || (MAPMEntryInfo->CurrentOperation == coGetFolderListingSize)))
      {
         /* Process differently based upon if this was a Get Folder     */
         /* Listing or Get Folder Listing Size request.                 */
         if(MAPMEntryInfo->CurrentOperation == coGetFolderListing)
         {
            /* Get Folder Listing Response.                             */
            if(GetFolderListingConfirmationData->ResponseCode != MAP_OBEX_RESPONSE_CONTINUE)
               MAPMEntryInfo->CurrentOperation = coNone;

            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                       = metMAPGetFolderListingResponse;
               MAPMEventData.EventLength                                                     = MAPM_GET_FOLDER_LISTING_RESPONSE_EVENT_DATA_SIZE;

               MAPMEventData.EventData.GetFolderListingResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.GetFolderListingResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.GetFolderListingResponseEventData.ResponseStatusCode  = MapResponseCodeToResponseStatusCode(GetFolderListingConfirmationData->ResponseCode);
               MAPMEventData.EventData.GetFolderListingResponseEventData.Final               = TRUE;

               if((MAPMEventData.EventData.GetFolderListingResponseEventData.FolderListingLength = GetFolderListingConfirmationData->DataLength) != 0)
                  MAPMEventData.EventData.GetFolderListingResponseEventData.FolderListingData = GetFolderListingConfirmationData->DataBuffer;

               /* Map the response code to special cases.               */
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  MAPMEventData.EventData.GetFolderListingResponseEventData.ResponseStatusCode  = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
               else
               {
                  if(GetFolderListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                     MAPMEventData.EventData.GetFolderListingResponseEventData.Final = FALSE;
               }

               /* Note the Callback information.                        */
               TrackingID        = MAPMEntryInfo->TrackingID;
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();

               /* Make sure that the Entry wasn't deleted during the    */
               /* callback.                                             */
               MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, TrackingID);
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */

               /* First, allocate a message of the correct size.        */
               if((GetFolderListingMessage = (MAPM_Get_Folder_Listing_Response_Message_t *)BTPS_AllocateMemory(MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(GetFolderListingConfirmationData->DataLength))) != NULL)
               {
                  BTPS_MemInitialize(GetFolderListingMessage, 0, MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(GetFolderListingConfirmationData->DataLength));

                  GetFolderListingMessage->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  GetFolderListingMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  GetFolderListingMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  GetFolderListingMessage->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_RESPONSE;
                  GetFolderListingMessage->MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(GetFolderListingConfirmationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE;

                  GetFolderListingMessage->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  GetFolderListingMessage->InstanceID                    = MAPMEntryInfo->InstanceID;
                  GetFolderListingMessage->ResponseStatusCode            = MapResponseCodeToResponseStatusCode(GetFolderListingConfirmationData->ResponseCode);
                  GetFolderListingMessage->Final                         = TRUE;

                  if((GetFolderListingMessage->FolderListingLength = GetFolderListingConfirmationData->DataLength) != 0)
                     BTPS_MemCopy(GetFolderListingMessage->FolderListing, GetFolderListingConfirmationData->DataBuffer, GetFolderListingConfirmationData->DataLength);

                  /* Map the response code to special cases.            */
                  if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                     GetFolderListingMessage->ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
                  else
                  {
                     if(GetFolderListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                        GetFolderListingMessage->Final = FALSE;
                  }

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)GetFolderListingMessage);

                  /* Go ahead and free the memory because we are        */
                  /* finished with it.                                  */
                  BTPS_FreeMemory(GetFolderListingMessage);
               }
               else
               {
                  /* Error allocating the message, go ahead and issue an*/
                  /* error response.                                    */
                  BTPS_MemInitialize(&ErrorGetFolderListingMessage, 0, sizeof(ErrorGetFolderListingMessage));

                  ErrorGetFolderListingMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  ErrorGetFolderListingMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  ErrorGetFolderListingMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  ErrorGetFolderListingMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_RESPONSE;
                  ErrorGetFolderListingMessage.MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

                  ErrorGetFolderListingMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  ErrorGetFolderListingMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
                  ErrorGetFolderListingMessage.ResponseStatusCode            = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED_RESOURCES;
                  ErrorGetFolderListingMessage.Final                         = TRUE;

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&GetFolderListingMessage);
               }
            }

            /* Determine if we have an Abort pending.  If so, we need to*/
            /* issue it.                                                */
            if(MAPMEntryInfo)
            {
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  IssuePendingAbort(MAPMEntryInfo);
               else
               {
                  /* If there was an error, we might need to issue an   */
                  /* Abort.                                             */
                  if((!GetFolderListingMessage) && (GetFolderListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE))
                  {
                     MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                     IssuePendingAbort(MAPMEntryInfo);
                  }
                  else
                  {
                     /* Check to see if we need to re-submit.           */
                     if(GetFolderListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                     {
                        /* Resubmit.                                    */
                        if(_MAP_Get_Folder_Listing_Request(MAPMEntryInfo->MAPID, 0, 0) < 0)
                        {
                           /* Error submitting request.                 */

                           /* Flag that we do not have a current        */
                           /* operation in progress.                    */
                           MAPMEntryInfo->CurrentOperation = coNone;

                           if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
                           {
                              /* Dispatch the event locally.            */

                              /* Note the Tracking ID.                  */
                              TrackingID = MAPMEntryInfo->TrackingID;

                              /* Event needs to be dispatched.  Go ahead*/
                              /* and format the event.                  */
                              BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                              /* Format the event data.                 */
                              MAPMEventData.EventType                                                       = metMAPGetFolderListingResponse;
                              MAPMEventData.EventLength                                                     = MAPM_GET_FOLDER_LISTING_RESPONSE_EVENT_DATA_SIZE;

                              MAPMEventData.EventData.GetFolderListingResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
                              MAPMEventData.EventData.GetFolderListingResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
                              MAPMEventData.EventData.GetFolderListingResponseEventData.ResponseStatusCode  = MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                              MAPMEventData.EventData.GetFolderListingResponseEventData.Final               = TRUE;

                              /* Note the Callback information.         */
                              EventCallback                                                                 = MAPMEntryInfo->CallbackFunction;
                              CallbackParameter                                                             = MAPMEntryInfo->CallbackParameter;

                              /* Release the Lock so we can dispatch the*/
                              /* event.                                 */
                              DEVM_ReleaseLock();

                              __BTPSTRY
                              {
                                 if(EventCallback)
                                     (*EventCallback)(&MAPMEventData, CallbackParameter);
                              }
                              __BTPSEXCEPT(1)
                              {
                                 /* Do Nothing.                         */
                              }

                              /* Re-acquire the Lock.                   */
                              DEVM_AcquireLock();

                              /* Make sure that the Entry wasn't deleted*/
                              /* during the callback.                   */
                              MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, TrackingID);
                           }
                           else
                           {
                              /* Dispatch the event remotely.           */

                              /* Format the message.                    */
                              BTPS_MemInitialize(&ErrorGetFolderListingMessage, 0, sizeof(ErrorGetFolderListingMessage));

                              ErrorGetFolderListingMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                              ErrorGetFolderListingMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                              ErrorGetFolderListingMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                              ErrorGetFolderListingMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_RESPONSE;
                              ErrorGetFolderListingMessage.MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

                              ErrorGetFolderListingMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                              ErrorGetFolderListingMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
                              ErrorGetFolderListingMessage.ResponseStatusCode            = MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                              ErrorGetFolderListingMessage.Final                         = TRUE;

                              /* Message has been formatted, go ahead   */
                              /* and dispatch it.                       */
                              MSG_SendMessage((BTPM_Message_t *)&ErrorGetFolderListingMessage);
                           }

                           /* Go ahead and issue an Abort to clean      */
                           /* things up.                                */
                           if(MAPMEntryInfo)
                           {
                              MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                              IssuePendingAbort(MAPMEntryInfo);
                           }
                        }
                     }
                  }
               }
            }
         }
         else
         {
            /* Get Folder Listing Size Response.                        */
            MAPMEntryInfo->CurrentOperation = coNone;

            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                           = metMAPGetFolderListingSizeResponse;
               MAPMEventData.EventLength                                                         = MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_EVENT_DATA_SIZE;

               MAPMEventData.EventData.GetFolderListingSizeResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.GetFolderListingSizeResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.GetFolderListingSizeResponseEventData.ResponseStatusCode  = MapResponseCodeToResponseStatusCode(GetFolderListingConfirmationData->ResponseCode);

               if(MAPMEventData.EventData.GetFolderListingSizeResponseEventData.ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                  MAPMEventData.EventData.GetFolderListingSizeResponseEventData.NumberOfFolders = GetFolderListingConfirmationData->NumberOfFolders;

               /* Note the Callback information.                        */
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */
               BTPS_MemInitialize(&GetFolderListingSizeMessage, 0, sizeof(GetFolderListingSizeMessage));

               GetFolderListingSizeMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               GetFolderListingSizeMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               GetFolderListingSizeMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               GetFolderListingSizeMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_RESPONSE;
               GetFolderListingSizeMessage.MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

               GetFolderListingSizeMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               GetFolderListingSizeMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
               GetFolderListingSizeMessage.ResponseStatusCode            = MapResponseCodeToResponseStatusCode(GetFolderListingConfirmationData->ResponseCode);

               if(GetFolderListingSizeMessage.ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                  GetFolderListingSizeMessage.NumberOfFolders = GetFolderListingConfirmationData->NumberOfFolders;

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&GetFolderListingSizeMessage);
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", GetFolderListingConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access get message listing indication event that has been received*/
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Message Access Manager information   */
   /* held.                                                             */
static void ProcessGetMessageListingIndicationEvent(MAP_Get_Message_Listing_Indication_Data_t *GetMessageListingIndicationData)
{
   void                                            *CallbackParameter;
   char                                            *FolderName;
   unsigned int                                     DataLength;
   unsigned int                                     FolderLength;
   unsigned int                                     FilterRecipientLength;
   unsigned int                                     FilterOriginatorLength;
   MAPM_Event_Data_t                                MAPMEventData;
   MAPM_Entry_Info_t                               *MAPMEntryInfo;
   MAPM_Event_Callback_t                            EventCallback;
   MAPM_Get_Message_Listing_Request_Message_t      *GetMessageListingMessage;
   MAPM_Get_Message_Listing_Size_Request_Message_t *GetMessageListingSizeMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(GetMessageListingIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, GetMessageListingIndicationData->MAPID)) != NULL) && ((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coGetMessageListing)))
      {
         /* Initialize variables.                                       */
         FolderName   = NULL;
         FolderLength = 0;

         /* Determine if this a Get Message Listing Size or a Get       */
         /* Message Listing.                                            */
         if((GetMessageListingIndicationData->MaxListCount) || (MAPMEntryInfo->CurrentOperation == coGetMessageListing))
         {
            /* Determine if we need to send more response data.         */
            if((MAPMEntryInfo->CurrentOperation == coGetMessageListing) && (MAPMEntryInfo->DataBufferSize) && (MAPMEntryInfo->DataBuffer))
            {
               /* Calculate the remaining data to send.                 */
               DataLength = MAPMEntryInfo->DataBufferSize - MAPMEntryInfo->DataBufferSent;

               if(_MAP_Get_Message_Listing_Response(MAPMEntryInfo->MAPID, (Byte_t)(MAPMEntryInfo->DataFinal?MAP_OBEX_RESPONSE_OK:MAP_OBEX_RESPONSE_CONTINUE), NULL, FALSE, NULL, DataLength, &(MAPMEntryInfo->DataBuffer[MAPMEntryInfo->DataBufferSent]), &DataLength) == 0)
               {
                  MAPMEntryInfo->DataBufferSent += DataLength;

                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));

                  /* Error submitting response.  Not sure what we can do*/
                  /* here.                                              */
                  _MAP_Get_Message_Listing_Response(MAPMEntryInfo->MAPID, (Byte_t)(MAP_OBEX_RESPONSE_SERVER_ERROR), NULL, FALSE, NULL, 0, NULL, NULL);

                  /* Flag that we have sent all the data (so it can be  */
                  /* freed below).                                      */
                  MAPMEntryInfo->DataBufferSent   = MAPMEntryInfo->DataBufferSize;

                  /* Flag that there is no longer an operation in       */
                  /* progress.                                          */
                  MAPMEntryInfo->CurrentOperation = coNone;
               }

               /* Free any memory that was allocated (if we have sent   */
               /* all the data).                                        */
               if(MAPMEntryInfo->DataBufferSent == MAPMEntryInfo->DataBufferSize)
               {
                  if(MAPMEntryInfo->DataBuffer)
                  {
                     BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                     MAPMEntryInfo->DataBuffer = NULL;
                  }

                  MAPMEntryInfo->CurrentOperation = coNone;
               }
            }
            else
            {
               /* Get Message Listing Request.                          */
               MAPMEntryInfo->CurrentOperation = coGetMessageListing;

               /* Free any left-over data (just to be safe).            */
               if(MAPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                  MAPMEntryInfo->DataBuffer = NULL;
               }

               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Dispatch the event locally.                        */

                  /* Event needs to be dispatched.  Go ahead and format */
                  /* the event.                                         */
                  BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                  /* Format the event data.                             */
                  MAPMEventData.EventType                                                       = metMAPGetMessageListingRequest;
                  MAPMEventData.EventLength                                                     = MAPM_GET_MESSAGE_LISTING_REQUEST_EVENT_DATA_SIZE;

                  MAPMEventData.EventData.GetMessageListingRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
                  MAPMEventData.EventData.GetMessageListingRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;
                  MAPMEventData.EventData.GetMessageListingRequestEventData.FolderName          = ConvertUnicodeToUTF8(GetMessageListingIndicationData->FolderName);
                  MAPMEventData.EventData.GetMessageListingRequestEventData.MaxListCount        = GetMessageListingIndicationData->MaxListCount;
                  MAPMEventData.EventData.GetMessageListingRequestEventData.ListStartOffset     = GetMessageListingIndicationData->ListStartOffset;
                  MAPMEventData.EventData.GetMessageListingRequestEventData.ListingInfo         = GetMessageListingIndicationData->ListingInfo;

                  /* Note the Callback information.                     */
                  EventCallback                                                                 = MAPMEntryInfo->CallbackFunction;
                  CallbackParameter                                                             = MAPMEntryInfo->CallbackParameter;

                  /* Release the Lock so we can dispatch the event.     */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                         (*EventCallback)(&MAPMEventData, CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();

                  /* Free any memory that was allocated to hold the     */
                  /* Folder Name.                                       */
                  if(MAPMEventData.EventData.GetMessageListingRequestEventData.FolderName)
                     BTPS_FreeMemory(MAPMEventData.EventData.GetMessageListingRequestEventData.FolderName);
               }
               else
               {
                  /* Dispatch the event remotely.                       */

                  /* First, determine the lengths of all the variable   */
                  /* members.                                           */
                  if(GetMessageListingIndicationData->FolderName)
                  {
                     if((FolderName = ConvertUnicodeToUTF8(GetMessageListingIndicationData->FolderName)) != NULL)
                        FolderLength = BTPS_StringLength(FolderName) + 1;
                     else
                        FolderLength = 0;
                  }
                  else
                     FolderLength = 0;

                  if((GetMessageListingIndicationData->ListingInfo.OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_RECIPIENT_PRESENT) && (GetMessageListingIndicationData->ListingInfo.FilterRecipient))
                     FilterRecipientLength = BTPS_StringLength(GetMessageListingIndicationData->ListingInfo.FilterRecipient) + 1;
                  else
                     FilterRecipientLength = 0;

                  if((GetMessageListingIndicationData->ListingInfo.OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_ORIGINATOR_PRESENT) && (GetMessageListingIndicationData->ListingInfo.FilterOriginator))
                     FilterOriginatorLength = BTPS_StringLength(GetMessageListingIndicationData->ListingInfo.FilterOriginator) + 1;
                  else
                     FilterOriginatorLength = 0;

                  /* Now that we have calculated the sizes, we need to  */
                  /* allocate memory to hold the message.               */
                  if((GetMessageListingMessage = (MAPM_Get_Message_Listing_Request_Message_t *)BTPS_AllocateMemory(MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(FolderLength, FilterRecipientLength, FilterOriginatorLength))) != NULL)
                  {
                     /* Format the message.                             */
                     BTPS_MemInitialize(GetMessageListingMessage, 0, MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(FolderLength, FilterRecipientLength, FilterOriginatorLength));

                     GetMessageListingMessage->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                     GetMessageListingMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                     GetMessageListingMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                     GetMessageListingMessage->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_REQUEST;
                     GetMessageListingMessage->MessageHeader.MessageLength   = MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(FolderLength, FilterRecipientLength, FilterOriginatorLength) - BTPM_MESSAGE_HEADER_SIZE;

                     GetMessageListingMessage->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                     GetMessageListingMessage->InstanceID                    = MAPMEntryInfo->InstanceID;
                     GetMessageListingMessage->MaxListCount                  = GetMessageListingIndicationData->MaxListCount;
                     GetMessageListingMessage->ListStartOffset               = GetMessageListingIndicationData->ListStartOffset;
                     GetMessageListingMessage->ListingInfo                   = GetMessageListingIndicationData->ListingInfo;
                     GetMessageListingMessage->ListingInfo.FilterRecipient   = NULL;
                     GetMessageListingMessage->ListingInfo.FilterOriginator  = NULL;
                     GetMessageListingMessage->FilterRecipientLength         = FilterRecipientLength;
                     GetMessageListingMessage->FilterOriginatorLength        = FilterOriginatorLength;
                     GetMessageListingMessage->FolderNameLength              = FolderLength;

                     /* Finally copy any members to the variable data   */
                     /* member.                                         */
                     DataLength = 0;
                     if(FilterRecipientLength)
                     {
                        BTPS_StringCopy((char *)&(GetMessageListingMessage->VariableData[DataLength]), GetMessageListingIndicationData->ListingInfo.FilterRecipient);

                        DataLength += FilterRecipientLength;
                     }

                     if(FilterOriginatorLength)
                     {
                        BTPS_StringCopy((char *)&(GetMessageListingMessage->VariableData[DataLength]), GetMessageListingIndicationData->ListingInfo.FilterOriginator);

                        DataLength += FilterOriginatorLength;
                     }

                     if((FolderLength) && (FolderName))
                        BTPS_StringCopy((char *)&(GetMessageListingMessage->VariableData[DataLength]), FolderName);

                     /* Message has been formatted, go ahead and        */
                     /* dispatch it.                                    */
                     MSG_SendMessage((BTPM_Message_t *)GetMessageListingMessage);

                     /* Free any memory that was allocated for the      */
                     /* converted Folder Name.                          */
                     if((FolderLength) && (FolderName))
                        BTPS_FreeMemory(FolderName);

                     /* All finished with the message, go ahead and free*/
                     /* the memory.                                     */
                     BTPS_FreeMemory(GetMessageListingMessage);
                  }
                  else
                  {
                     /* Error allocating memory.                        */
                     _MAP_Get_Message_Listing_Response(GetMessageListingIndicationData->MAPID, (Byte_t)(MAP_OBEX_RESPONSE_SERVER_ERROR), NULL, FALSE, NULL, 0, NULL, NULL);

                     MAPMEntryInfo->CurrentOperation = coNone;
                  }
               }
            }
         }
         else
         {
            /* Get Message Listing Size Request.                        */
            MAPMEntryInfo->CurrentOperation = coGetMessageListingSize;

            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                           = metMAPGetMessageListingSizeRequest;
               MAPMEventData.EventLength                                                         = MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_EVENT_DATA_SIZE;

               MAPMEventData.EventData.GetMessageListingSizeRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.GetMessageListingSizeRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.GetMessageListingSizeRequestEventData.FolderName          = ConvertUnicodeToUTF8(GetMessageListingIndicationData->FolderName);
               MAPMEventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo         = GetMessageListingIndicationData->ListingInfo;

               /* Note the Callback information.                        */
               EventCallback                                                                     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter                                                                 = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();

               /* Free any memory that was allocated to hold the        */
               /* converted UTF-8 string.                               */
               if(MAPMEventData.EventData.GetMessageListingSizeRequestEventData.FolderName)
                  BTPS_FreeMemory(MAPMEventData.EventData.GetMessageListingSizeRequestEventData.FolderName);
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* First, determine the lengths of all the variable      */
               /* members.                                              */
               if(GetMessageListingIndicationData->FolderName)
               {
                  if((FolderName = ConvertUnicodeToUTF8(GetMessageListingIndicationData->FolderName)) != NULL)
                     FolderLength = BTPS_StringLength(FolderName) + 1;
                  else
                     FolderLength = 0;
               }
               else
                  FolderLength = 0;

               if((GetMessageListingIndicationData->ListingInfo.OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_RECIPIENT_PRESENT) && (GetMessageListingIndicationData->ListingInfo.FilterRecipient))
                  FilterRecipientLength = BTPS_StringLength(GetMessageListingIndicationData->ListingInfo.FilterRecipient) + 1;
               else
                  FilterRecipientLength = 0;

               if((GetMessageListingIndicationData->ListingInfo.OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_ORIGINATOR_PRESENT) && (GetMessageListingIndicationData->ListingInfo.FilterOriginator))
                  FilterOriginatorLength = BTPS_StringLength(GetMessageListingIndicationData->ListingInfo.FilterOriginator) + 1;
               else
                  FilterOriginatorLength = 0;

               /* Now that we have calculated the sizes, we need to     */
               /* allocate memory to hold the message.                  */
               if((GetMessageListingSizeMessage = (MAPM_Get_Message_Listing_Size_Request_Message_t *)BTPS_AllocateMemory(MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(FolderLength, FilterRecipientLength, FilterOriginatorLength))) != NULL)
               {
                  /* Format the message.                                */
                  BTPS_MemInitialize(&GetMessageListingSizeMessage, 0, MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(FolderLength, FilterRecipientLength, FilterOriginatorLength));

                  GetMessageListingSizeMessage->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  GetMessageListingSizeMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  GetMessageListingSizeMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  GetMessageListingSizeMessage->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE_REQUEST;
                  GetMessageListingSizeMessage->MessageHeader.MessageLength   = MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(FolderLength, FilterRecipientLength, FilterOriginatorLength) - BTPM_MESSAGE_HEADER_SIZE;

                  GetMessageListingSizeMessage->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  GetMessageListingSizeMessage->InstanceID                    = MAPMEntryInfo->InstanceID;
                  GetMessageListingSizeMessage->ListingInfo                   = GetMessageListingIndicationData->ListingInfo;
                  GetMessageListingSizeMessage->ListingInfo.FilterRecipient   = NULL;
                  GetMessageListingSizeMessage->ListingInfo.FilterOriginator  = NULL;
                  GetMessageListingSizeMessage->FilterRecipientLength         = FilterRecipientLength;
                  GetMessageListingSizeMessage->FilterOriginatorLength        = FilterOriginatorLength;
                  GetMessageListingSizeMessage->FolderNameLength              = FolderLength;

                  /* Finally copy any members to the variable data      */
                  /* member.                                            */
                  DataLength = 0;
                  if(FilterRecipientLength)
                  {
                     BTPS_StringCopy((char *)&(GetMessageListingSizeMessage->VariableData[DataLength]), GetMessageListingIndicationData->ListingInfo.FilterRecipient);

                     DataLength += FilterRecipientLength;
                  }

                  if(FilterOriginatorLength)
                  {
                     BTPS_StringCopy((char *)&(GetMessageListingSizeMessage->VariableData[DataLength]), GetMessageListingIndicationData->ListingInfo.FilterOriginator);

                     DataLength += FilterOriginatorLength;
                  }

                  if(FolderLength)
                     BTPS_StringCopy((char *)&(GetMessageListingSizeMessage->VariableData[DataLength]), FolderName);

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&GetMessageListingSizeMessage);

                  /* Free any memory that was allocated for the         */
                  /* converted Folder Name.                             */
                  if((FolderLength) && (FolderName))
                     BTPS_FreeMemory(FolderName);
               }
               else
               {
                  /* Error allocating memory.                           */
                  _MAP_Get_Message_Listing_Response(GetMessageListingIndicationData->MAPID, (Byte_t)(MAP_OBEX_RESPONSE_SERVER_ERROR), NULL, FALSE, NULL, 0, NULL, NULL);

                  MAPMEntryInfo->CurrentOperation = coNone;
               }
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", GetMessageListingIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Get_Message_Listing_Response(GetMessageListingIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST), NULL, FALSE, NULL, 0, NULL, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access get message listing confirmation event that has been       */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Message Access Manager        */
   /* information held.                                                 */
static void ProcessGetMessageListingConfirmationEvent(MAP_Get_Message_Listing_Confirmation_Data_t *GetMessageListingConfirmationData)
{
   void                                        *CallbackParameter;
   unsigned int                                 TrackingID;
   MAPM_Event_Data_t                            MAPMEventData;
   MAPM_Entry_Info_t                           *MAPMEntryInfo;
   MAPM_Event_Callback_t                        EventCallback;
   MAPM_Get_Message_Listing_Response_Message_t  ErrorGetMessageListingMessage;
   MAPM_Get_Message_Listing_Response_Message_t *GetMessageListingMessage;
   MAPM_Get_Message_Listing_Response_Message_t  GetMessageListingSizeMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(GetMessageListingConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, GetMessageListingConfirmationData->MAPID)) != NULL) && ((MAPMEntryInfo->CurrentOperation == coGetMessageListing) || (MAPMEntryInfo->CurrentOperation == coGetMessageListingSize)))
      {
         /* Process differently based upon if this was a Get Message    */
         /* Listing or Get Message Listing Size request.                */
         if(MAPMEntryInfo->CurrentOperation == coGetMessageListing)
         {
            /* Get Message Listing Response.                            */
            if(GetMessageListingConfirmationData->ResponseCode != MAP_OBEX_RESPONSE_CONTINUE)
               MAPMEntryInfo->CurrentOperation = coNone;

            /* Initialize that we have not allocated any memory.        */
            GetMessageListingMessage        = NULL;

            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                        = metMAPGetMessageListingResponse;
               MAPMEventData.EventLength                                                      = MAPM_GET_MESSAGE_LISTING_RESPONSE_EVENT_DATA_SIZE;

               MAPMEventData.EventData.GetMessageListingResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.GetMessageListingResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.GetMessageListingResponseEventData.ResponseStatusCode  = MapResponseCodeToResponseStatusCode(GetMessageListingConfirmationData->ResponseCode);
               MAPMEventData.EventData.GetMessageListingResponseEventData.Final               = TRUE;
               MAPMEventData.EventData.GetMessageListingResponseEventData.NewMessage          = GetMessageListingConfirmationData->NewMessage;
               MAPMEventData.EventData.GetMessageListingResponseEventData.MSETime             = GetMessageListingConfirmationData->MSETime;
               MAPMEventData.EventData.GetMessageListingResponseEventData.NumberOfMessages    = GetMessageListingConfirmationData->NumberOfMessages;

               if((MAPMEventData.EventData.GetMessageListingResponseEventData.MessageListingLength = GetMessageListingConfirmationData->DataLength) != 0)
                  MAPMEventData.EventData.GetMessageListingResponseEventData.MessageListingData = GetMessageListingConfirmationData->DataBuffer;

               /* Map the response code to special cases.               */
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  MAPMEventData.EventData.GetMessageListingResponseEventData.ResponseStatusCode  = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
               else
               {
                  if(GetMessageListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                     MAPMEventData.EventData.GetMessageListingResponseEventData.Final = FALSE;
               }

               /* Note the Callback information.                        */
               TrackingID        = MAPMEntryInfo->TrackingID;
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();

               /* Make sure that the Entry wasn't deleted during the    */
               /* callback.                                             */
               MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, TrackingID);
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */

               /* First, allocate a message of the correct size.        */
               if((GetMessageListingMessage = (MAPM_Get_Message_Listing_Response_Message_t *)BTPS_AllocateMemory(MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(GetMessageListingConfirmationData->DataLength))) != NULL)
               {
                  BTPS_MemInitialize(GetMessageListingMessage, 0, MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(GetMessageListingConfirmationData->DataLength));

                  GetMessageListingMessage->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  GetMessageListingMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  GetMessageListingMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  GetMessageListingMessage->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_RESPONSE;
                  GetMessageListingMessage->MessageHeader.MessageLength   = MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(GetMessageListingConfirmationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE;

                  GetMessageListingMessage->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  GetMessageListingMessage->InstanceID                    = MAPMEntryInfo->InstanceID;
                  GetMessageListingMessage->ResponseStatusCode            = MapResponseCodeToResponseStatusCode(GetMessageListingConfirmationData->ResponseCode);
                  GetMessageListingMessage->Final                         = TRUE;
                  GetMessageListingMessage->MessageCount                  = GetMessageListingConfirmationData->NumberOfMessages;
                  GetMessageListingMessage->NewMessage                    = GetMessageListingConfirmationData->NewMessage;
                  GetMessageListingMessage->MSETime                       = GetMessageListingConfirmationData->MSETime;

                  if((GetMessageListingMessage->MessageListingLength = GetMessageListingConfirmationData->DataLength) != 0)
                     BTPS_MemCopy(GetMessageListingMessage->MessageListing, GetMessageListingConfirmationData->DataBuffer, GetMessageListingConfirmationData->DataLength);

                  /* Map the response code to special cases.            */
                  if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                     GetMessageListingMessage->ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
                  else
                  {
                     if(GetMessageListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                        GetMessageListingMessage->Final = FALSE;
                  }

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)GetMessageListingMessage);

                  /* Go ahead and free the memory because we are        */
                  /* finished with it.                                  */
                  BTPS_FreeMemory(GetMessageListingMessage);
               }
               else
               {
                  /* Error allocating the message, go ahead and issue an*/
                  /* error response.                                    */
                  BTPS_MemInitialize(&ErrorGetMessageListingMessage, 0, sizeof(ErrorGetMessageListingMessage));

                  ErrorGetMessageListingMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  ErrorGetMessageListingMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  ErrorGetMessageListingMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  ErrorGetMessageListingMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_RESPONSE;
                  ErrorGetMessageListingMessage.MessageHeader.MessageLength   = MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

                  ErrorGetMessageListingMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  ErrorGetMessageListingMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
                  ErrorGetMessageListingMessage.ResponseStatusCode            = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED_RESOURCES;
                  ErrorGetMessageListingMessage.Final                         = TRUE;
                  ErrorGetMessageListingMessage.MessageCount                  = GetMessageListingConfirmationData->NumberOfMessages;
                  ErrorGetMessageListingMessage.NewMessage                    = GetMessageListingConfirmationData->NewMessage;
                  ErrorGetMessageListingMessage.MSETime                       = GetMessageListingConfirmationData->MSETime;

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&ErrorGetMessageListingMessage);
               }
            }

            /* Determine if we have an Abort pending.  If so, we need to*/
            /* issue it.                                                */
            if(MAPMEntryInfo)
            {
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  IssuePendingAbort(MAPMEntryInfo);
               else
               {
                  /* If there was an error, we might need to issue an   */
                  /* Abort.                                             */
                  if((((MAPMEntryInfo->ClientID != MSG_GetServerAddressID())) && (!GetMessageListingMessage)) && (GetMessageListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE))
                  {
                     MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                     IssuePendingAbort(MAPMEntryInfo);
                  }
                  else
                  {
                     /* Check to see if we need to re-submit.           */
                     if(GetMessageListingConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                     {
                        /* Resubmit.                                    */
                        if(_MAP_Get_Message_Listing_Request(MAPMEntryInfo->MAPID, NULL, 0, 0, NULL) < 0)
                        {
                           /* Error submitting request.                 */

                           /* Flag that we do not have a current        */
                           /* operation in progress.                    */
                           MAPMEntryInfo->CurrentOperation = coNone;

                           if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
                           {
                              /* Dispatch the event locally.            */

                              /* Note the Tracking ID.                  */
                              TrackingID = MAPMEntryInfo->TrackingID;

                              /* Event needs to be dispatched.  Go ahead*/
                              /* and format the event.                  */
                              BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                              /* Format the event data.                 */
                              MAPMEventData.EventType                                                        = metMAPGetMessageListingResponse;
                              MAPMEventData.EventLength                                                      = MAPM_GET_MESSAGE_LISTING_RESPONSE_EVENT_DATA_SIZE;

                              MAPMEventData.EventData.GetMessageListingResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
                              MAPMEventData.EventData.GetMessageListingResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
                              MAPMEventData.EventData.GetMessageListingResponseEventData.ResponseStatusCode  = MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                              MAPMEventData.EventData.GetMessageListingResponseEventData.Final               = TRUE;
                              MAPMEventData.EventData.GetMessageListingResponseEventData.NewMessage          = GetMessageListingConfirmationData->NewMessage;
                              MAPMEventData.EventData.GetMessageListingResponseEventData.MSETime             = GetMessageListingConfirmationData->MSETime;
                              MAPMEventData.EventData.GetMessageListingResponseEventData.NumberOfMessages    = GetMessageListingConfirmationData->NumberOfMessages;

                              /* Note the Callback information.         */
                              EventCallback                                                                  = MAPMEntryInfo->CallbackFunction;
                              CallbackParameter                                                              = MAPMEntryInfo->CallbackParameter;

                              /* Release the Lock so we can dispatch the*/
                              /* event.                                 */
                              DEVM_ReleaseLock();

                              __BTPSTRY
                              {
                                 if(EventCallback)
                                     (*EventCallback)(&MAPMEventData, CallbackParameter);
                              }
                              __BTPSEXCEPT(1)
                              {
                                 /* Do Nothing.                         */
                              }

                              /* Re-acquire the Lock.                   */
                              DEVM_AcquireLock();

                              /* Make sure that the Entry wasn't deleted*/
                              /* during the callback.                   */
                              MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, TrackingID);
                           }
                           else
                           {
                              /* Dispatch the event remotely.           */

                              /* Format the message.                    */
                              BTPS_MemInitialize(&ErrorGetMessageListingMessage, 0, sizeof(ErrorGetMessageListingMessage));

                              ErrorGetMessageListingMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                              ErrorGetMessageListingMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                              ErrorGetMessageListingMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                              ErrorGetMessageListingMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_RESPONSE;
                              ErrorGetMessageListingMessage.MessageHeader.MessageLength   = MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

                              ErrorGetMessageListingMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                              ErrorGetMessageListingMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
                              ErrorGetMessageListingMessage.ResponseStatusCode            = MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                              ErrorGetMessageListingMessage.Final                         = TRUE;
                              ErrorGetMessageListingMessage.MessageCount                  = GetMessageListingConfirmationData->NumberOfMessages;
                              ErrorGetMessageListingMessage.NewMessage                    = GetMessageListingConfirmationData->NewMessage;
                              ErrorGetMessageListingMessage.MSETime                       = GetMessageListingConfirmationData->MSETime;

                              /* Message has been formatted, go ahead   */
                              /* and dispatch it.                       */
                              MSG_SendMessage((BTPM_Message_t *)&ErrorGetMessageListingMessage);
                           }

                           /* Go ahead and issue an Abort to clean      */
                           /* things up.                                */
                           if(MAPMEntryInfo)
                           {
                              MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                              IssuePendingAbort(MAPMEntryInfo);
                           }
                        }
                     }
                  }
               }
            }
         }
         else
         {
            /* Get Message Listing Size Response.                       */
            MAPMEntryInfo->CurrentOperation = coNone;

            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                            = metMAPGetMessageListingSizeResponse;
               MAPMEventData.EventLength                                                          = MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_EVENT_DATA_SIZE;

               MAPMEventData.EventData.GetMessageListingSizeResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.GetMessageListingSizeResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.GetMessageListingSizeResponseEventData.ResponseStatusCode  = MapResponseCodeToResponseStatusCode(GetMessageListingConfirmationData->ResponseCode);
               MAPMEventData.EventData.GetMessageListingSizeResponseEventData.NewMessage          = GetMessageListingConfirmationData->NewMessage;
               MAPMEventData.EventData.GetMessageListingSizeResponseEventData.MSETime             = GetMessageListingConfirmationData->MSETime;

               if(MAPMEventData.EventData.GetMessageListingSizeResponseEventData.ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                  MAPMEventData.EventData.GetMessageListingSizeResponseEventData.NumberOfMessages = GetMessageListingConfirmationData->NumberOfMessages;

               /* Note the Callback information.                        */
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */
               BTPS_MemInitialize(&GetMessageListingSizeMessage, 0, sizeof(GetMessageListingSizeMessage));

               GetMessageListingSizeMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               GetMessageListingSizeMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               GetMessageListingSizeMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               GetMessageListingSizeMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE_RESPONSE;
               GetMessageListingSizeMessage.MessageHeader.MessageLength   = MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

               GetMessageListingSizeMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               GetMessageListingSizeMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
               GetMessageListingSizeMessage.ResponseStatusCode            = MapResponseCodeToResponseStatusCode(GetMessageListingConfirmationData->ResponseCode);
               GetMessageListingSizeMessage.NewMessage                    = GetMessageListingConfirmationData->NewMessage;
               GetMessageListingSizeMessage.MSETime                       = GetMessageListingConfirmationData->MSETime;

               if(GetMessageListingSizeMessage.ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                  GetMessageListingSizeMessage.MessageCount = GetMessageListingConfirmationData->NumberOfMessages;

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&GetMessageListingSizeMessage);
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", GetMessageListingConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access get message indication event that has been received with   */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessGetMessageIndicationEvent(MAP_Get_Message_Indication_Data_t *GetMessageIndicationData)
{
   void                               *CallbackParameter;
   unsigned int                        DataLength;
   MAPM_Event_Data_t                   MAPMEventData;
   MAPM_Entry_Info_t                  *MAPMEntryInfo;
   MAPM_Event_Callback_t               EventCallback;
   MAPM_Get_Message_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(GetMessageIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, GetMessageIndicationData->MAPID)) != NULL) && ((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coGetMessage)))
      {
         /* Determine if this a continuation to a prior request.        */

         /* Determine if we need to send more response data.            */
         if((MAPMEntryInfo->CurrentOperation == coGetMessage) && (MAPMEntryInfo->DataBufferSize) && (MAPMEntryInfo->DataBuffer))
         {
            /* Calculate the remaining data to send.                    */
            DataLength = MAPMEntryInfo->DataBufferSize - MAPMEntryInfo->DataBufferSent;

            if(_MAP_Get_Message_Response(MAPMEntryInfo->MAPID, (Byte_t)(MAPMEntryInfo->DataFinal?MAP_OBEX_RESPONSE_OK:MAP_OBEX_RESPONSE_CONTINUE), GetMessageIndicationData->FractionalType, DataLength, &(MAPMEntryInfo->DataBuffer[MAPMEntryInfo->DataBufferSent]), &DataLength) == 0)
            {
               MAPMEntryInfo->DataBufferSent += DataLength;

               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));

               /* Error submitting response.  Not sure what we can do   */
               /* here.                                                 */
               _MAP_Get_Message_Response(MAPMEntryInfo->MAPID, (Byte_t)(MAP_OBEX_RESPONSE_SERVER_ERROR), GetMessageIndicationData->FractionalType, 0, NULL, NULL);

               /* Flag that we have sent all the data (so it can be     */
               /* freed below).                                         */
               MAPMEntryInfo->DataBufferSent   = MAPMEntryInfo->DataBufferSize;

               /* Flag that there is no longer an operation in progress.*/
               MAPMEntryInfo->CurrentOperation = coNone;
            }

            /* Free any memory that was allocated (if we have sent all  */
            /* the data).                                               */
            if(MAPMEntryInfo->DataBufferSent == MAPMEntryInfo->DataBufferSize)
            {
               if(MAPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                  MAPMEntryInfo->DataBuffer = NULL;
               }

               MAPMEntryInfo->CurrentOperation = coNone;
            }
         }
         else
         {
            /* Flag the new state we are entering.                      */
            MAPMEntryInfo->CurrentOperation = coGetMessage;

            /* Free any left-over data (just to be safe).               */
            if(MAPMEntryInfo->DataBuffer)
            {
               BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

               MAPMEntryInfo->DataBuffer = NULL;
            }

            /* Dispatch the event based upon the client registration    */
            /* type.                                                    */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                = metMAPGetMessageRequest;
               MAPMEventData.EventLength                                              = MAPM_GET_MESSAGE_REQUEST_EVENT_DATA_SIZE;

               MAPMEventData.EventData.GetMessageRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.GetMessageRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.GetMessageRequestEventData.Attachment          = GetMessageIndicationData->Attachment;
               MAPMEventData.EventData.GetMessageRequestEventData.CharSet             = GetMessageIndicationData->CharSet;
               MAPMEventData.EventData.GetMessageRequestEventData.FractionalType      = GetMessageIndicationData->FractionalType;

               if(GetMessageIndicationData->MessageHandle)
                  BTPS_StringCopy(MAPMEventData.EventData.GetMessageRequestEventData.MessageHandle, GetMessageIndicationData->MessageHandle);

               /* Note the Callback information.                        */
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_REQUEST;
               Message.MessageHeader.MessageLength   = MAPM_GET_MESSAGE_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

               Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               Message.InstanceID                    = MAPMEntryInfo->InstanceID;
               Message.Attachment                    = GetMessageIndicationData->Attachment;
               Message.CharSet                       = GetMessageIndicationData->CharSet;
               Message.FractionalType                = GetMessageIndicationData->FractionalType;

               if(GetMessageIndicationData->MessageHandle)
                  BTPS_StringCopy(Message.MessageHandle, GetMessageIndicationData->MessageHandle);

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&Message);
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", GetMessageIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Get_Message_Response(GetMessageIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST), GetMessageIndicationData->FractionalType, 0, NULL, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access get message confirmation event that has been received with */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessGetMessageConfirmationEvent(MAP_Get_Message_Confirmation_Data_t *GetMessageConfirmationData)
{
   void                                *CallbackParameter;
   unsigned int                         TrackingID;
   MAPM_Event_Data_t                    MAPMEventData;
   MAPM_Entry_Info_t                   *MAPMEntryInfo;
   MAPM_Event_Callback_t                EventCallback;
   MAPM_Get_Message_Response_Message_t  ErrorGetMessageMessage;
   MAPM_Get_Message_Response_Message_t *GetMessageMessage = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(GetMessageConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, GetMessageConfirmationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coGetMessage))
      {
         /* Get Message Response.                                       */
         if(GetMessageConfirmationData->ResponseCode != MAP_OBEX_RESPONSE_CONTINUE)
            MAPMEntryInfo->CurrentOperation = coNone;

         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            /* Format the event data.                                   */
            MAPMEventData.EventType                                                 = metMAPGetMessageResponse;
            MAPMEventData.EventLength                                               = MAPM_GET_MESSAGE_RESPONSE_EVENT_DATA_SIZE;

            MAPMEventData.EventData.GetMessageResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.GetMessageResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
            MAPMEventData.EventData.GetMessageResponseEventData.ResponseStatusCode  = (GetMessageConfirmationData->ResponseCode == 0xFF)?MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST:MapResponseCodeToResponseStatusCode(GetMessageConfirmationData->ResponseCode);
            MAPMEventData.EventData.GetMessageResponseEventData.Final               = TRUE;
            MAPMEventData.EventData.GetMessageResponseEventData.FractionalType      = GetMessageConfirmationData->FractionalType;

            if((MAPMEventData.EventData.GetMessageResponseEventData.MessageDataLength = GetMessageConfirmationData->DataLength) != 0)
               MAPMEventData.EventData.GetMessageResponseEventData.MessageData = GetMessageConfirmationData->DataBuffer;

            /* Map the response code to special cases.                  */
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               MAPMEventData.EventData.GetMessageResponseEventData.ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
            else
            {
               if(GetMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                  MAPMEventData.EventData.GetMessageResponseEventData.Final = FALSE;
            }

            /* Note the Callback information.                           */
            TrackingID        = MAPMEntryInfo->TrackingID;
            EventCallback     = MAPMEntryInfo->CallbackFunction;
            CallbackParameter = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                   (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();

            /* Make sure that the Entry wasn't deleted during the       */
            /* callback.                                                */
            MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, TrackingID);
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */

            /* First, allocate a message of the correct size.           */
            if((GetMessageMessage = (MAPM_Get_Message_Response_Message_t *)BTPS_AllocateMemory(MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(GetMessageConfirmationData->DataLength))) != NULL)
            {
               BTPS_MemInitialize(GetMessageMessage, 0, MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(GetMessageConfirmationData->DataLength));

               GetMessageMessage->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               GetMessageMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
               GetMessageMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               GetMessageMessage->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_RESPONSE;
               GetMessageMessage->MessageHeader.MessageLength   = MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(GetMessageConfirmationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE;

               GetMessageMessage->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               GetMessageMessage->InstanceID                    = MAPMEntryInfo->InstanceID;
               GetMessageMessage->ResponseStatusCode            = (GetMessageConfirmationData->ResponseCode == 0xFF)?MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST:MapResponseCodeToResponseStatusCode(GetMessageConfirmationData->ResponseCode);
               GetMessageMessage->Final                         = TRUE;
               GetMessageMessage->FractionalType                = GetMessageConfirmationData->FractionalType;

               if((GetMessageMessage->MessageDataLength = GetMessageConfirmationData->DataLength) != 0)
                  BTPS_MemCopy(GetMessageMessage->MessageData, GetMessageConfirmationData->DataBuffer, GetMessageConfirmationData->DataLength);

               /* Map the response code to special cases.               */
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  GetMessageMessage->ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
               else
               {
                  if(GetMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                     GetMessageMessage->Final = FALSE;
               }

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)GetMessageMessage);

               /* Go ahead and free the memory because we are finished  */
               /* with it.                                              */
               BTPS_FreeMemory(GetMessageMessage);
            }
            else
            {
               /* Error allocating the message, go ahead and issue an   */
               /* error response.                                       */
               BTPS_MemInitialize(&ErrorGetMessageMessage, 0, sizeof(ErrorGetMessageMessage));

               ErrorGetMessageMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               ErrorGetMessageMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               ErrorGetMessageMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               ErrorGetMessageMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_RESPONSE;
               ErrorGetMessageMessage.MessageHeader.MessageLength   = MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

               ErrorGetMessageMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               ErrorGetMessageMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
               ErrorGetMessageMessage.ResponseStatusCode            = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED_RESOURCES;
               ErrorGetMessageMessage.Final                         = TRUE;
               ErrorGetMessageMessage.FractionalType                = GetMessageConfirmationData->FractionalType;

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&ErrorGetMessageMessage);
            }
         }

         /* Determine if we have an Abort pending.  If so, we need to   */
         /* issue it.                                                   */
         if(MAPMEntryInfo)
         {
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               IssuePendingAbort(MAPMEntryInfo);
            else
            {
               /* If there was an error, we might need to issue an      */
               /* Abort.                                                */
               if((!GetMessageMessage) && (GetMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE))
               {
                  MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                  IssuePendingAbort(MAPMEntryInfo);
               }
               else
               {
                  /* Check to see if we need to re-submit.              */
                  if(GetMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
                  {
                     /* Resubmit.                                       */
                     if(_MAP_Get_Message_Request(MAPMEntryInfo->MAPID, NULL, FALSE, csUTF8, GetMessageConfirmationData->FractionalType) < 0)
                     {
                        /* Error submitting request.                    */

                        /* Flag that we do not have a current operation */
                        /* in progress.                                 */
                        MAPMEntryInfo->CurrentOperation = coNone;

                        if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
                        {
                           /* Dispatch the event locally.               */

                           /* Note the Tracking ID.                     */
                           TrackingID = MAPMEntryInfo->TrackingID;

                           /* Event needs to be dispatched.  Go ahead   */
                           /* and format the event.                     */
                           BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

                           /* Format the event data.                    */
                           MAPMEventData.EventType                                                 = metMAPGetMessageResponse;
                           MAPMEventData.EventLength                                               = MAPM_GET_MESSAGE_RESPONSE_EVENT_DATA_SIZE;

                           MAPMEventData.EventData.GetMessageResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
                           MAPMEventData.EventData.GetMessageResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
                           MAPMEventData.EventData.GetMessageResponseEventData.ResponseStatusCode  = MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                           MAPMEventData.EventData.GetMessageResponseEventData.Final               = TRUE;
                           MAPMEventData.EventData.GetMessageResponseEventData.FractionalType      = GetMessageConfirmationData->FractionalType;

                           /* Note the Callback information.            */
                           EventCallback                                                           = MAPMEntryInfo->CallbackFunction;
                           CallbackParameter                                                       = MAPMEntryInfo->CallbackParameter;

                           /* Release the Lock so we can dispatch the   */
                           /* event.                                    */
                           DEVM_ReleaseLock();

                           __BTPSTRY
                           {
                              if(EventCallback)
                                  (*EventCallback)(&MAPMEventData, CallbackParameter);
                           }
                           __BTPSEXCEPT(1)
                           {
                              /* Do Nothing.                            */
                           }

                           /* Re-acquire the Lock.                      */
                           DEVM_AcquireLock();

                           /* Make sure that the Entry wasn't deleted   */
                           /* during the callback.                      */
                           MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, TrackingID);
                        }
                        else
                        {
                           /* Dispatch the event remotely.              */

                           /* Format the message.                       */
                           BTPS_MemInitialize(&ErrorGetMessageMessage, 0, sizeof(ErrorGetMessageMessage));

                           ErrorGetMessageMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                           ErrorGetMessageMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                           ErrorGetMessageMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                           ErrorGetMessageMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_RESPONSE;
                           ErrorGetMessageMessage.MessageHeader.MessageLength   = MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

                           ErrorGetMessageMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                           ErrorGetMessageMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
                           ErrorGetMessageMessage.ResponseStatusCode            = MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                           ErrorGetMessageMessage.Final                         = TRUE;
                           ErrorGetMessageMessage.FractionalType                = GetMessageConfirmationData->FractionalType;

                           /* Message has been formatted, go ahead and  */
                           /* dispatch it.                              */
                           MSG_SendMessage((BTPM_Message_t *)&GetMessageMessage);
                        }

                        /* Go ahead and issue an Abort to clean things  */
                        /* up.                                          */
                        if(MAPMEntryInfo)
                        {
                           MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                           IssuePendingAbort(MAPMEntryInfo);
                        }
                     }
                  }
               }
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", GetMessageConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access set message status indication event that has been received */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Message Access Manager information   */
   /* held.                                                             */
static void ProcessSetMessageStatusIndicationEvent(MAP_Set_Message_Status_Indication_Data_t *SetMessageStatusIndicationData)
{
   void                                      *CallbackParameter;
   MAPM_Event_Data_t                          MAPMEventData;
   MAPM_Entry_Info_t                         *MAPMEntryInfo;
   MAPM_Event_Callback_t                      EventCallback;
   MAPM_Set_Message_Status_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(SetMessageStatusIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, SetMessageStatusIndicationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coNone))
      {
         /* Flag the new state we are entering.                         */
         MAPMEntryInfo->CurrentOperation = coSetMessageStatus;

         /* Dispatch the event based upon the client registration type. */
         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            /* Format the event data.                                   */
            MAPMEventData.EventType                                                      = metMAPSetMessageStatusRequest;
            MAPMEventData.EventLength                                                    = MAPM_SET_MESSAGE_STATUS_REQUEST_EVENT_DATA_SIZE;

            MAPMEventData.EventData.SetMessageStatusRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.SetMessageStatusRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;
            MAPMEventData.EventData.SetMessageStatusRequestEventData.StatusIndicator     = SetMessageStatusIndicationData->StatusIndicator;
            MAPMEventData.EventData.SetMessageStatusRequestEventData.StatusValue         = SetMessageStatusIndicationData->StatusValue;

            if(SetMessageStatusIndicationData->MessageHandle)
               BTPS_StringCopy(MAPMEventData.EventData.SetMessageStatusRequestEventData.MessageHandle, SetMessageStatusIndicationData->MessageHandle);

            /* Note the Callback information.                           */
            EventCallback     = MAPMEntryInfo->CallbackFunction;
            CallbackParameter = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_REQUEST;
            Message.MessageHeader.MessageLength   = MAPM_SET_MESSAGE_STATUS_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;
            Message.StatusIndicator               = SetMessageStatusIndicationData->StatusIndicator;
            Message.StatusValue                   = SetMessageStatusIndicationData->StatusValue;

            if(SetMessageStatusIndicationData->MessageHandle)
               BTPS_StringCopy(Message.MessageHandle, SetMessageStatusIndicationData->MessageHandle);

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", SetMessageStatusIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Set_Message_Status_Response(SetMessageStatusIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access set message status confimation event that has been received*/
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Message Access Manager information   */
   /* held.                                                             */
static void ProcessSetMessageStatusConfirmationEvent(MAP_Set_Message_Status_Confirmation_Data_t *SetMessageStatusConfirmationData)
{
   void                                       *CallbackParameter;
   MAPM_Event_Data_t                           MAPMEventData;
   MAPM_Entry_Info_t                          *MAPMEntryInfo;
   MAPM_Event_Callback_t                       EventCallback;
   MAPM_Set_Message_Status_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(SetMessageStatusConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, SetMessageStatusConfirmationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coSetMessageStatus))
      {
         /* Flag that there is no longer an operation in progress.      */
         MAPMEntryInfo->CurrentOperation = coNone;

         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            MAPMEventData.EventType                                                       = metMAPSetMessageStatusResponse;
            MAPMEventData.EventLength                                                     = MAPM_SET_MESSAGE_STATUS_RESPONSE_EVENT_DATA_SIZE;

            MAPMEventData.EventData.SetMessageStatusResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.SetMessageStatusResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
            MAPMEventData.EventData.SetMessageStatusResponseEventData.ResponseStatusCode  = MapResponseCodeToResponseStatusCode(SetMessageStatusConfirmationData->ResponseCode);

            /* Note the Callback information.                           */
            EventCallback                                                                 = MAPMEntryInfo->CallbackFunction;
            CallbackParameter                                                             = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_RESPONSE;
            Message.MessageHeader.MessageLength   = MAPM_SET_MESSAGE_STATUS_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;
            Message.ResponseStatusCode            = MapResponseCodeToResponseStatusCode(SetMessageStatusConfirmationData->ResponseCode);

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", SetMessageStatusConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access push message indication event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessPushMessageIndicationEvent(MAP_Push_Message_Indication_Data_t *PushMessageIndicationData)
{
   void                                *CallbackParameter;
   char                                *FolderName;
   Byte_t                               ResponseCode;
   unsigned int                         FolderLength;
   unsigned int                         TrackingID;
   MAPM_Event_Data_t                    MAPMEventData;
   MAPM_Entry_Info_t                   *MAPMEntryInfo;
   MAPM_Event_Callback_t                EventCallback;
   MAPM_Push_Message_Request_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(PushMessageIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, PushMessageIndicationData->MAPID)) != NULL) && ((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coPushMessage)))
      {
         /* Go ahead and initialize the response (either OK or a        */
         /* continue).                                                  */
         if(PushMessageIndicationData->Final)
         {
            ResponseCode                    = MAP_OBEX_RESPONSE_OK;
            MAPMEntryInfo->CurrentOperation = coPushMessage;
         }
         else
         {
            ResponseCode                    = MAP_OBEX_RESPONSE_CONTINUE;
            MAPMEntryInfo->CurrentOperation = coNone;
         }

         /* Initialize some defaults.                                   */
         FolderName   = NULL;
         FolderLength = 0;

         /* Process differently based upon the client registration type.*/
         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            /* Format the event data.                                   */
            MAPMEventData.EventType                                                 = metMAPPushMessageRequest;
            MAPMEventData.EventLength                                               = MAPM_PUSH_MESSAGE_REQUEST_EVENT_DATA_SIZE;

            MAPMEventData.EventData.PushMessageRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.PushMessageRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;
            MAPMEventData.EventData.PushMessageRequestEventData.Final               = PushMessageIndicationData->Final;
            MAPMEventData.EventData.PushMessageRequestEventData.Transparent         = PushMessageIndicationData->Transparent;
            MAPMEventData.EventData.PushMessageRequestEventData.Retry               = PushMessageIndicationData->Retry;
            MAPMEventData.EventData.PushMessageRequestEventData.CharSet             = PushMessageIndicationData->CharSet;
            MAPMEventData.EventData.PushMessageRequestEventData.FolderName          = ConvertUnicodeToUTF8(PushMessageIndicationData->FolderName);

            if((MAPMEventData.EventData.PushMessageRequestEventData.MessageDataLength = PushMessageIndicationData->DataLength) != 0)
               MAPMEventData.EventData.PushMessageRequestEventData.MessageData = PushMessageIndicationData->DataBuffer;

            /* Note the Callback information.                           */
            TrackingID        = MAPMEntryInfo->TrackingID;
            EventCallback     = MAPMEntryInfo->CallbackFunction;
            CallbackParameter = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                   (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();

            /* Free any memory that was allocated for the Folder Name   */
            /* mapping.                                                 */
            if(MAPMEventData.EventData.PushMessageRequestEventData.FolderName)
               BTPS_FreeMemory(MAPMEventData.EventData.PushMessageRequestEventData.FolderName);

            /* Make sure that the Entry wasn't deleted during the       */
            /* callback.                                                */
            MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, TrackingID);
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */

            /* We need to determine how much space is required for the  */
            /* Folder Name (this is a Unicode string, so we need to     */
            /* convert it to ASCII).                                    */
            if(PushMessageIndicationData->FolderName)
            {
               if((FolderName = ConvertUnicodeToUTF8(PushMessageIndicationData->FolderName)) != NULL)
                  FolderLength = BTPS_StringLength(FolderName) + 1;
               else
                  FolderLength = 0;
            }
            else
               FolderLength = 0;

            /* Next, allocate a message of the correct size.            */
            if((Message = (MAPM_Push_Message_Request_Message_t *)BTPS_AllocateMemory(MAPM_PUSH_MESSAGE_REQUEST_MESSAGE_SIZE(FolderLength, PushMessageIndicationData->DataLength))) != NULL)
            {
               BTPS_MemInitialize(Message, 0, MAPM_PUSH_MESSAGE_REQUEST_MESSAGE_SIZE(FolderLength, PushMessageIndicationData->DataLength));

               Message->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               Message->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_REQUEST;
               Message->MessageHeader.MessageLength   = MAPM_PUSH_MESSAGE_REQUEST_MESSAGE_SIZE(FolderLength, PushMessageIndicationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE;

               Message->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               Message->InstanceID                    = MAPMEntryInfo->InstanceID;
               Message->Final                         = PushMessageIndicationData->Final;
               Message->Transparent                   = PushMessageIndicationData->Transparent;
               Message->Retry                         = PushMessageIndicationData->Retry;
               Message->CharSet                       = PushMessageIndicationData->CharSet;
               Message->FolderNameLength              = FolderLength;

               if(FolderLength)
                  BTPS_StringCopy((char *)(Message->VariableData), FolderName);

               if((Message->MessageLength = PushMessageIndicationData->DataLength) != 0)
                  BTPS_MemCopy(&(Message->VariableData[FolderLength]), PushMessageIndicationData->DataBuffer, PushMessageIndicationData->DataLength);

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)Message);

               /* Free any memory that was allocated to hold the        */
               /* converted Folder Name.                                */
               if((FolderLength) && (FolderName))
                  BTPS_FreeMemory(FolderName);

               /* Go ahead and free the memory because we are finished  */
               /* with it.                                              */
               BTPS_FreeMemory(Message);
            }
            else
            {
               /* Error allocating memory, flag an error.               */
               ResponseCode                     = MAP_OBEX_RESPONSE_SERVER_ERROR;

               MAPMEntryInfo->CurrentOperation = coNone;
            }
         }

         /* Respond to the request.                                     */
         /* * NOTE * We do not respond to the last request because the  */
         /*          server will need to respond with the Message       */
         /*          Handle.                                            */
         if(MAPMEntryInfo)
         {
            if(PushMessageIndicationData->Final == FALSE)
            {
               if(_MAP_Push_Message_Response(PushMessageIndicationData->MAPID, ResponseCode, NULL))
                  MAPMEntryInfo->CurrentOperation = coNone;
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", PushMessageIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Push_Message_Response(PushMessageIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST), NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access push message confimation event that has been received with */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessPushMessageConfirmationEvent(MAP_Push_Message_Confirmation_Data_t *PushMessageConfirmationData)
{
   void                                 *CallbackParameter;
   Boolean_t                             DispatchEvent;
   Boolean_t                             Error;
   unsigned int                          DataLength;
   MAPM_Event_Data_t                     MAPMEventData;
   MAPM_Entry_Info_t                    *MAPMEntryInfo;
   MAPM_Event_Callback_t                 EventCallback;
   MAPM_Push_Message_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(PushMessageConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, PushMessageConfirmationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coPushMessage))
      {
         /* Flag that we do not need to dispatch the event confirmation */
         /* (this might change below).                                  */
         DispatchEvent = FALSE;

         /* Flag that no error occurred.                                */
         Error         = FALSE;

         /* If this is the first confirmation, and it is successful, we */
         /* need to note the Message Handle so that we can send it to   */
         /* the user in later event notifications.                      */
         if(((PushMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_OK) || (PushMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)) && (PushMessageConfirmationData->MessageHandle))
         {
            if((!BTPS_StringLength(MAPMEntryInfo->DataMessageHandle)))
               BTPS_StringCopy(MAPMEntryInfo->DataMessageHandle, PushMessageConfirmationData->MessageHandle);
         }
         else
            MAPMEntryInfo->DataMessageHandle[0] = '\0';

         /* Determine if we need to send more data.                     */
         if(((PushMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_OK) || (PushMessageConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)) && (MAPMEntryInfo->DataBufferSize) && (MAPMEntryInfo->DataBuffer) && (!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)))
         {
            /* Calculate the remaining data to send.                    */
            DataLength = MAPMEntryInfo->DataBufferSize - MAPMEntryInfo->DataBufferSent;

            if(_MAP_Push_Message_Request(MAPMEntryInfo->MAPID, NULL, FALSE, FALSE, csUTF8, DataLength, &(MAPMEntryInfo->DataBuffer[MAPMEntryInfo->DataBufferSent]), &DataLength, MAPMEntryInfo->DataFinal) > 0)
            {
               MAPMEntryInfo->DataBufferSent += DataLength;

               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)MAPMEntryInfo->MAPID, MAPMEntryInfo->DataBufferSent));

               /* Flag that we have sent all the data (so it can be     */
               /* freed below).                                         */
               MAPMEntryInfo->DataBufferSent = MAPMEntryInfo->DataBufferSize;

               DispatchEvent                 = TRUE;

               Error                         = TRUE;
            }

            /* Free any memory that was allocated (if we have sent all  */
            /* the data).                                               */
            if((MAPMEntryInfo) && (MAPMEntryInfo->DataBufferSent == MAPMEntryInfo->DataBufferSize))
            {
               if(MAPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                  MAPMEntryInfo->DataBuffer = NULL;
               }
            }
         }
         else
         {
            DispatchEvent = TRUE;

            /* If we have a pending abort, let's go ahead and clean up  */
            /* everything.                                              */
            if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
            {
               if(MAPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                  MAPMEntryInfo->DataBuffer = NULL;
               }
            }
         }

         if(DispatchEvent)
         {
            /* Determine if we need to clear the state.                 */
            if((MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT) || (PushMessageConfirmationData->ResponseCode != MAP_OBEX_RESPONSE_CONTINUE) || (Error))
               MAPMEntryInfo->CurrentOperation = coNone;

            /* Process differently based upon the client registration   */
            /* type.                                                    */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                  = metMAPPushMessageResponse;
               MAPMEventData.EventLength                                                = MAPM_PUSH_MESSAGE_RESPONSE_EVENT_DATA_SIZE;

               MAPMEventData.EventData.PushMessageResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.PushMessageResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;

               /* Copy the message Handle.                              */
               BTPS_StringCopy(MAPMEventData.EventData.PushMessageResponseEventData.MessageHandle, MAPMEntryInfo->DataMessageHandle);

               if(!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
                  MAPMEventData.EventData.PushMessageResponseEventData.ResponseStatusCode = MapResponseCodeToResponseStatusCode(PushMessageConfirmationData->ResponseCode);
               else
                  MAPMEventData.EventData.PushMessageResponseEventData.ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

               /* Note the Callback information.                        */
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* If there is a pending abort, go ahead and issue the   */
               /* abort.                                                */
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  IssuePendingAbort(MAPMEntryInfo);

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
               Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_RESPONSE;
               Message.MessageHeader.MessageLength   = MAPM_PUSH_MESSAGE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

               Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
               Message.InstanceID                    = MAPMEntryInfo->InstanceID;

               /* Copy the message Handle.                              */
               BTPS_StringCopy(Message.MessageHandle, MAPMEntryInfo->DataMessageHandle);

               if(!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
                  Message.ResponseStatusCode = MapResponseCodeToResponseStatusCode(PushMessageConfirmationData->ResponseCode);
               else
                  Message.ResponseStatusCode = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

               /* If there is a pending abort, go ahead and issue the   */
               /* abort.                                                */
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  IssuePendingAbort(MAPMEntryInfo);

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&Message);
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", PushMessageConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access update inbox indication event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessUpdateInboxIndicationEvent(MAP_Update_Inbox_Indication_Data_t *UpdateInboxIndicationData)
{
   void                                *CallbackParameter;
   MAPM_Event_Data_t                    MAPMEventData;
   MAPM_Entry_Info_t                   *MAPMEntryInfo;
   MAPM_Event_Callback_t                EventCallback;
   MAPM_Update_Inbox_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(UpdateInboxIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, UpdateInboxIndicationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coNone))
      {
         /* Flag the new state we are entering.                         */
         MAPMEntryInfo->CurrentOperation = coUpdateInbox;

         /* Dispatch the event based upon the client registration type. */
         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            /* Format the event data.                                   */
            MAPMEventData.EventType                                                 = metMAPUpdateInboxRequest;
            MAPMEventData.EventLength                                               = MAPM_UPDATE_INBOX_REQUEST_EVENT_DATA_SIZE;

            MAPMEventData.EventData.UpdateInboxRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.UpdateInboxRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;

            /* Note the Callback information.                           */
            EventCallback                                                           = MAPMEntryInfo->CallbackFunction;
            CallbackParameter                                                       = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_REQUEST;
            Message.MessageHeader.MessageLength   = MAPM_UPDATE_INBOX_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", UpdateInboxIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Update_Inbox_Response(UpdateInboxIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access update inbox confirmation event that has been received with*/
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessUpdateInboxConfirmationEvent(MAP_Update_Inbox_Confirmation_Data_t *UpdateInboxConfirmationData)
{
   void                                 *CallbackParameter;
   MAPM_Event_Data_t                     MAPMEventData;
   MAPM_Entry_Info_t                    *MAPMEntryInfo;
   MAPM_Event_Callback_t                 EventCallback;
   MAPM_Update_Inbox_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(UpdateInboxConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, UpdateInboxConfirmationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coUpdateInbox))
      {
         /* Flag that there is no longer an operation in progress.      */
         MAPMEntryInfo->CurrentOperation = coNone;

         if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

            MAPMEventData.EventType                                                  = metMAPUpdateInboxResponse;
            MAPMEventData.EventLength                                                = MAPM_UPDATE_INBOX_RESPONSE_EVENT_DATA_SIZE;

            MAPMEventData.EventData.UpdateInboxResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
            MAPMEventData.EventData.UpdateInboxResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
            MAPMEventData.EventData.UpdateInboxResponseEventData.ResponseStatusCode  = MapResponseCodeToResponseStatusCode(UpdateInboxConfirmationData->ResponseCode);

            /* Note the Callback information.                           */
            EventCallback                                                            = MAPMEntryInfo->CallbackFunction;
            CallbackParameter                                                        = MAPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&MAPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_RESPONSE;
            Message.MessageHeader.MessageLength   = MAPM_UPDATE_INBOX_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
            Message.InstanceID                    = MAPMEntryInfo->InstanceID;
            Message.ResponseStatusCode            = MapResponseCodeToResponseStatusCode(UpdateInboxConfirmationData->ResponseCode);

            /* Message has been formatted, go ahead and dispatch it.    */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", UpdateInboxConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access set folder indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Message Access Manager information held.      */
static void ProcessSetFolderIndicationEvent(MAP_Set_Folder_Indication_Data_t *SetFolderIndicationData)
{
   void                              *CallbackParameter;
   char                              *FolderName;
   char                              *PendingPath;
   Word_t                            *UnicodeFolderName;
   MAPM_Event_Data_t                  MAPMEventData;
   MAPM_Entry_Info_t                 *MAPMEntryInfo;
   MAPM_Event_Callback_t              EventCallback;
   MAPM_Set_Folder_Request_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(SetFolderIndicationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, SetFolderIndicationData->MAPID)) != NULL) && (MAPMEntryInfo->CurrentOperation == coNone))
      {
         /* Check if we have a folder to append.                        */
         if((SetFolderIndicationData) && (SetFolderIndicationData->FolderName[0]))
            UnicodeFolderName = SetFolderIndicationData->FolderName;
         else
            UnicodeFolderName = NULL;

         /* Note the Pending Folder.                                    */
         /* * NOTE * Here are the options to test for PathOption:       */
         /*             - sfRoot - FolderName ignored                   */
         /*             - sfDown - FolderName required                  */
         /*             - sfUp   - FolderName optional                  */
         if(BuildPendingFolder(SetFolderIndicationData->PathOption, UnicodeFolderName, MAPMEntryInfo->CurrentPath, &PendingPath))
         {
            /* Flag the new state we are entering.                      */
            MAPMEntryInfo->CurrentOperation = coSetFolder;

            /* Note the Pending Path.                                   */
            if(MAPMEntryInfo->PendingPath)
            {
               BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

               MAPMEntryInfo->PendingPath = NULL;
            }

            /* Note the Pending Path.                                   */
            MAPMEntryInfo->PendingPath = PendingPath;

            /* Dispatch the event based upon the client registration    */
            /* type.                                                    */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                               = metMAPSetFolderRequest;
               MAPMEventData.EventLength                                             = MAPM_SET_FOLDER_REQUEST_EVENT_DATA_SIZE;

               MAPMEventData.EventData.SetFolderRequestEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.SetFolderRequestEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.SetFolderRequestEventData.PathOption          = SetFolderIndicationData->PathOption;
               MAPMEventData.EventData.SetFolderRequestEventData.FolderName          = ConvertUnicodeToUTF8(SetFolderIndicationData->FolderName);
               MAPMEventData.EventData.SetFolderRequestEventData.NewPath             = PendingPath;

               /* Note the Callback information.                        */
               EventCallback                                                         = MAPMEntryInfo->CallbackFunction;
               CallbackParameter                                                     = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();

               /* If memory was allocated to hold the converted Path    */
               /* Name, we need to free it.                             */
               if(MAPMEventData.EventData.SetFolderRequestEventData.FolderName)
                  BTPS_FreeMemory(MAPMEventData.EventData.SetFolderRequestEventData.FolderName);
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* First convert the Folder Name from Unicode to UTF-8.  */
               FolderName = ConvertUnicodeToUTF8(SetFolderIndicationData->FolderName);

               /* Next, allocate a message of the correct size.         */
               if((Message = (MAPM_Set_Folder_Request_Message_t *)BTPS_AllocateMemory(MAPM_SET_FOLDER_REQUEST_MESSAGE_SIZE((FolderName?(BTPS_StringLength(FolderName)+1):0), (PendingPath?(BTPS_StringLength(PendingPath)+1):0)))) != NULL)
               {
                  /* Format the message.                                */
                  BTPS_MemInitialize(Message, 0, sizeof(MAPM_Set_Folder_Request_Message_t));

                  Message->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  Message->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_SET_FOLDER_REQUEST;
                  Message->MessageHeader.MessageLength   = MAPM_SET_FOLDER_REQUEST_MESSAGE_SIZE((FolderName?(BTPS_StringLength(FolderName)+1):0), (PendingPath?(BTPS_StringLength(PendingPath)+1):0)) - BTPM_MESSAGE_HEADER_SIZE;

                  Message->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  Message->InstanceID                    = MAPMEntryInfo->InstanceID;
                  Message->PathOption                    = SetFolderIndicationData->PathOption;
                  Message->FolderNameLength              = (FolderName?(BTPS_StringLength(FolderName)+1):0);
                  Message->NewPathLength                 = (PendingPath?(BTPS_StringLength(PendingPath)+1):0);

                  if(FolderName)
                  {
                     BTPS_StringCopy((char *)(Message->VariableData), FolderName);

                     /* All finished with the memory, so go ahead and   */
                     /* free it.                                        */
                     BTPS_FreeMemory(FolderName);
                  }

                  if(PendingPath)
                     BTPS_StringCopy((char *)&(Message->VariableData[Message->FolderNameLength]), PendingPath);


                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)Message);

                  /* All finished with the message, so go ahead and free*/
                  /* the memory.                                        */
                  BTPS_FreeMemory(Message);
               }
               else
               {
                  /* All finished with the memory, so go ahead and free */
                  /* it.                                                */
                  if(FolderName)
                     BTPS_FreeMemory(FolderName);

                  _MAP_Set_Folder_Response(SetFolderIndicationData->MAPID, (Byte_t)MAP_OBEX_RESPONSE_SERVER_ERROR);
               }
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameters specified: %d\n", (int)SetFolderIndicationData->MAPID));

            _MAP_Set_Folder_Response(SetFolderIndicationData->MAPID, (Byte_t)MAP_OBEX_RESPONSE_BAD_REQUEST);
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", SetFolderIndicationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));

         _MAP_Set_Folder_Response(SetFolderIndicationData->MAPID, (Byte_t)(MAPMEntryInfo?MAP_OBEX_RESPONSE_NOT_ACCEPTABLE:MAP_OBEX_RESPONSE_BAD_REQUEST));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access set folder confirmation event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Message Access Manager information held.  */
static void ProcessSetFolderConfirmationEvent(MAP_Set_Folder_Confirmation_Data_t *SetFolderConfirmationData)
{
   int                                 Index;
   void                               *CallbackParameter;
   char                                ReplaceChar;
   Word_t                             *_FolderName;
   Boolean_t                           Dispatch;
   Boolean_t                           Error;
   MAPM_Event_Data_t                   MAPMEventData;
   MAPM_Entry_Info_t                  *MAPMEntryInfo;
   MAPM_Event_Callback_t               EventCallback;
   MAPM_Set_Folder_Response_Message_t  ErrorSetFolderResponseMessage;
   MAPM_Set_Folder_Response_Message_t *SetFolderResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(SetFolderConfirmationData)
   {
      /* Search for the MAPID in the entry list.                        */
      if(((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, SetFolderConfirmationData->MAPID)) != NULL) && ((MAPMEntryInfo->CurrentOperation == coSetFolder) || (MAPMEntryInfo->CurrentOperation == coSetFolderAbsolute)))
      {
         /* Initialize that we need to dispatch the event.              */
         Dispatch = TRUE;
         Error    = FALSE;

         /* First, let's see if the response was success.               */
         if(SetFolderConfirmationData->ResponseCode == MAP_OBEX_RESPONSE_OK)
         {
            /* Check to see if this was a non absolute set or if we are */
            /* finished with an absolute path set operation.            */
            if((MAPMEntryInfo->CurrentOperation == coSetFolder) || ((!MAPMEntryInfo->PendingPath) || (MAPMEntryInfo->PendingPathOffset == (int)BTPS_StringLength(MAPMEntryInfo->PendingPath))))
            {
               /* Set the Current Operation to coNone.                  */
               MAPMEntryInfo->CurrentOperation = coNone;

               /* Go ahead and note the new current directory.          */
               if(MAPMEntryInfo->CurrentPath)
                  BTPS_FreeMemory(MAPMEntryInfo->CurrentPath);

               MAPMEntryInfo->CurrentPath = MAPMEntryInfo->PendingPath;

               MAPMEntryInfo->PendingPath = NULL;
            }
            else
            {
               /* Absolute Path - not completed.                        */

               Index = ++MAPMEntryInfo->PendingPathOffset;
               while((MAPMEntryInfo->PendingPath[MAPMEntryInfo->PendingPathOffset]) && (MAPMEntryInfo->PendingPath[MAPMEntryInfo->PendingPathOffset] != '/'))
                  MAPMEntryInfo->PendingPathOffset++;

               /* We need to change the directory which has the name    */
               /* between Index and PendingPathOffset.                  */

               /* Note the current character that is either a NULL or a */
               /* delimiter.                                            */
               ReplaceChar                                                  = MAPMEntryInfo->PendingPath[MAPMEntryInfo->PendingPathOffset];

               /* NULL terminate the string.                            */
               MAPMEntryInfo->PendingPath[MAPMEntryInfo->PendingPathOffset] = '\0';

               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("PendingPath: %s\n", MAPMEntryInfo->PendingPath));

               if(!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
               {
                  if(((_FolderName = ConvertUTF8ToUnicode(&(MAPMEntryInfo->PendingPath[Index]))) == NULL) || (_MAP_Set_Folder_Request(MAPMEntryInfo->MAPID, sfDown, _FolderName)))
                  {
                     /* Set the Current Operation to coNone.            */
                     MAPMEntryInfo->CurrentOperation = coNone;

                     /* Error, go ahead and clean up.                   */
                     if(MAPMEntryInfo->CurrentPath)
                        BTPS_FreeMemory(MAPMEntryInfo->CurrentPath);

                     MAPMEntryInfo->CurrentPath = MAPMEntryInfo->PendingPath;

                     MAPMEntryInfo->PendingPath = NULL;

                     Error                      = TRUE;
                  }
                  else
                  {
                     /* Replace the Delimiter character (if it was      */
                     /* trashed).                                       */
                     MAPMEntryInfo->PendingPath[MAPMEntryInfo->PendingPathOffset] = ReplaceChar;

                     Dispatch = FALSE;
                  }

                  if(_FolderName)
                     BTPS_FreeMemory(_FolderName);
               }
            }
         }

         if((SetFolderConfirmationData->ResponseCode != MAP_OBEX_RESPONSE_OK) || (MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
         {
            if(MAPMEntryInfo->CurrentOperation == coSetFolder)
            {
               /* Set the Current Operation to coNone.                  */
               MAPMEntryInfo->CurrentOperation = coNone;

               /* Error, go ahead delete the Pending Path.              */
               if(MAPMEntryInfo->PendingPath)
               {
                  BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                  MAPMEntryInfo->PendingPath = NULL;
               }
            }
            else
            {
               /* Set the Current Operation to coNone.                  */
               MAPMEntryInfo->CurrentOperation = coNone;

               /* Failure for Set Folder Absolute.  We need to determine*/
               /* the last place we ended up at so we can set the       */
               /* current path.                                         */
               if(MAPMEntryInfo->PendingPath)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("PendingPath: %d, %s\n", MAPMEntryInfo->PendingPathOffset, MAPMEntryInfo->PendingPath));

                  /* Go ahead and clear out the Current Path (we will   */
                  /* overwrite it below).                               */
                  if(MAPMEntryInfo->CurrentPath)
                  {
                     BTPS_FreeMemory(MAPMEntryInfo->CurrentPath);

                     MAPMEntryInfo->CurrentPath = NULL;
                  }

                  /* We are either sitting at Root (-1), or a '/'       */
                  /* character, or the NULL terminator character.  If we*/
                  /* are sitting at a '/' character or the NULL         */
                  /* terminator, then we need to back up past it.       */
                  Index = MAPMEntryInfo->PendingPathOffset;
                  if((Index > 0) && ((MAPMEntryInfo->PendingPath[Index] == '/') || (!MAPMEntryInfo->PendingPath[Index])))
                     Index--;

                  while(Index >= 0)
                  {
                     if(MAPMEntryInfo->PendingPath[Index] == '/')
                        break;
                     else
                        Index--;
                  }

                  /* Check to see if we ended up at the root.           */
                  if(Index > 0)
                  {
                     MAPMEntryInfo->PendingPath[Index] = '\0';

                     MAPMEntryInfo->CurrentPath        = MAPMEntryInfo->PendingPath;

                     MAPMEntryInfo->PendingPath        = NULL;
                  }
                  else
                  {
                     /* We are now at the root.  Clear both paths.      */
                     if(MAPMEntryInfo->PendingPath)
                     {
                        BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                        MAPMEntryInfo->PendingPath = NULL;
                     }
                  }
               }
               else
               {
                  /* Setting the path to root failed, go ahead and      */
                  /* simply delete the pending path (and leave the      */
                  /* current directory where it's at) - aka do nothing. */
               }
            }
         }

         /* Dispatch any event that is required (note that we already   */
         /* have fixed up the Current/Pending Path information).        */
         if(Dispatch)
         {
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               BTPS_MemInitialize(&MAPMEventData, 0, sizeof(MAPM_Event_Data_t));

               /* Format the event data.                                */
               MAPMEventData.EventType                                                = metMAPSetFolderResponse;
               MAPMEventData.EventLength                                              = MAPM_SET_FOLDER_RESPONSE_EVENT_DATA_SIZE;

               MAPMEventData.EventData.SetFolderResponseEventData.RemoteDeviceAddress = MAPMEntryInfo->RemoteDeviceAddress;
               MAPMEventData.EventData.SetFolderResponseEventData.InstanceID          = MAPMEntryInfo->InstanceID;
               MAPMEventData.EventData.SetFolderResponseEventData.ResponseStatusCode  = Error?MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST:MapResponseCodeToResponseStatusCode(SetFolderConfirmationData->ResponseCode);
               MAPMEventData.EventData.SetFolderResponseEventData.CurrentPath         = MAPMEntryInfo->CurrentPath;

               /* Map the response code to special cases.               */
               if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               {
                  MAPMEventData.EventData.SetFolderResponseEventData.ResponseStatusCode  = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

                  MAPMEntryInfo->Flags                                                  &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT);
               }

               /* Note the Callback information.                        */
               EventCallback     = MAPMEntryInfo->CallbackFunction;
               CallbackParameter = MAPMEntryInfo->CallbackParameter;

               /* Release the Lock so we can dispatch the event.        */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                      (*EventCallback)(&MAPMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
            else
            {
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */

               /* First, allocate a message of the correct size.        */
               if((SetFolderResponseMessage = (MAPM_Set_Folder_Response_Message_t *)BTPS_AllocateMemory(MAPM_SET_FOLDER_RESPONSE_MESSAGE_SIZE((MAPMEntryInfo->CurrentPath?(BTPS_StringLength(MAPMEntryInfo->CurrentPath)+1):0)))) != NULL)
               {
                  BTPS_MemInitialize(SetFolderResponseMessage, 0, MAPM_SET_FOLDER_RESPONSE_MESSAGE_SIZE((MAPMEntryInfo->CurrentPath?(BTPS_StringLength(MAPMEntryInfo->CurrentPath)+1):0)));

                  SetFolderResponseMessage->MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  SetFolderResponseMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  SetFolderResponseMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  SetFolderResponseMessage->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_SET_FOLDER_RESPONSE;
                  SetFolderResponseMessage->MessageHeader.MessageLength   = MAPM_SET_FOLDER_RESPONSE_MESSAGE_SIZE((MAPMEntryInfo->CurrentPath?(BTPS_StringLength(MAPMEntryInfo->CurrentPath)+1):0)) - BTPM_MESSAGE_HEADER_SIZE;

                  SetFolderResponseMessage->RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  SetFolderResponseMessage->InstanceID                    = MAPMEntryInfo->InstanceID;
                  SetFolderResponseMessage->ResponseStatusCode            = Error?MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST:MapResponseCodeToResponseStatusCode(SetFolderConfirmationData->ResponseCode);
                  SetFolderResponseMessage->CurrentPathLength             = MAPMEntryInfo->CurrentPath?BTPS_StringLength(MAPMEntryInfo->CurrentPath):0;

                  /* Map the response code to special cases.            */
                  if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  {
                     SetFolderResponseMessage->ResponseStatusCode  = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

                     MAPMEntryInfo->Flags                         &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT);
                  }

                  if(MAPMEntryInfo->CurrentPath)
                     BTPS_StringCopy(SetFolderResponseMessage->CurrentPath, MAPMEntryInfo->CurrentPath);

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)SetFolderResponseMessage);

                  /* Go ahead and free the memory because we are        */
                  /* finished with it.                                  */
                  BTPS_FreeMemory(SetFolderResponseMessage);
               }
               else
               {
                  /* Error allocating the message, go ahead and issue an*/
                  /* error response.                                    */
                  BTPS_MemInitialize(&ErrorSetFolderResponseMessage, 0, sizeof(ErrorSetFolderResponseMessage));

                  ErrorSetFolderResponseMessage.MessageHeader.AddressID       = MAPMEntryInfo->ClientID;
                  ErrorSetFolderResponseMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  ErrorSetFolderResponseMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
                  ErrorSetFolderResponseMessage.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_SET_FOLDER_RESPONSE;
                  ErrorSetFolderResponseMessage.MessageHeader.MessageLength   = MAPM_SET_FOLDER_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

                  ErrorSetFolderResponseMessage.RemoteDeviceAddress           = MAPMEntryInfo->RemoteDeviceAddress;
                  ErrorSetFolderResponseMessage.InstanceID                    = MAPMEntryInfo->InstanceID;
                  ErrorSetFolderResponseMessage.ResponseStatusCode            = MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED_RESOURCES;
                  ErrorSetFolderResponseMessage.CurrentPathLength             = 0;

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&ErrorSetFolderResponseMessage);
               }
            }
         }
      }
      else
      {
         if(!MAPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find MAPID: %d\n", SetFolderConfirmationData->MAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)MAPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access abort indication event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Message Access Manager information held.      */
static void ProcessAbortIndicationEvent(MAP_Abort_Indication_Data_t *AbortIndicationData)
{
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(AbortIndicationData)
   {
      /* Find the list entry by the MAPID.                              */
      /* * NOTE * This entry could be either a MAP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, AbortIndicationData->MAPID)) == NULL)
         MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, AbortIndicationData->MAPID);

      if(MAPMEntryInfo)
      {
         /* Flag that there is no longer an operation in progress.      */
         MAPMEntryInfo->CurrentOperation = coNone;

         /* Free any temporary resources.                               */
         if(MAPMEntryInfo->PendingPath)
         {
            BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

            MAPMEntryInfo->PendingPath = NULL;
         }

         if(MAPMEntryInfo->DataBuffer)
         {
            BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

            MAPMEntryInfo->DataBuffer = NULL;
         }

//xxx  Not sure if we should dispatch this or not.
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Message    */
   /* Access abort confirmation event that has been received with the   */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Message Access Manager information held.      */
static void ProcessAbortConfirmationEvent(MAP_Abort_Confirmation_Data_t *AbortConfirmationData)
{
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(AbortConfirmationData)
   {
      /* Find the list entry by the MAPID.                              */
      /* * NOTE * This entry could be either a MAP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if((MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList, AbortConfirmationData->MAPID)) == NULL)
         MAPMEntryInfo = SearchMAPMEntryInfoByMAPID(&MAPMEntryInfoList_Notification, AbortConfirmationData->MAPID);

      if(MAPMEntryInfo)
      {
         /* Nothing really to do other than to clear the Pending Abort  */
         /* flag.                                                       */
         MAPMEntryInfo->Flags &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT);

//xxx  Not sure if we should dispatch this or not.
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing MAP Events that have been received.  This function     */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessMAPEvent(MAPM_MAP_Event_Data_t *MAPEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be valid.     */
   if(MAPEventData)
   {
      /* Process the event based on the event type.                     */
      switch(MAPEventData->EventType)
      {
         case etMAP_Open_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication\n"));

            ProcessOpenRequestIndicationEvent(&(MAPEventData->EventData.OpenRequestIndicationData));
            break;
         case etMAP_Open_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Port Indication\n"));

            ProcessOpenPortIndicationEvent(&(MAPEventData->EventData.OpenPortIndicationData));
            break;
         case etMAP_Open_Port_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Port Confirmation\n"));

            ProcessOpenPortConfirmationEvent(&(MAPEventData->EventData.OpenPortConfirmationData));
            break;
         case etMAP_Close_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Port Indication\n"));

            ProcessClosePortIndicationEvent(&(MAPEventData->EventData.ClosePortIndicationData));
            break;
         case etMAP_Send_Event_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Event Indication\n"));

            ProcessSendEventIndicationEvent(&(MAPEventData->EventData.SendEventIndicationData));
            break;
         case etMAP_Send_Event_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Event Confirmation\n"));

            ProcessSendEventConfirmationEvent(&(MAPEventData->EventData.SendEventConfirmationData));
            break;
         case etMAP_Notification_Registration_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Registration Indication\n"));

            ProcessNotificationRegistrationIndicationEvent(&(MAPEventData->EventData.NotificationRegistrationIndicationData));
            break;
         case etMAP_Notification_Registration_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Registration Confirmation\n"));

            ProcessNotificationRegistrationConfirmationEvent(&(MAPEventData->EventData.NotificationRegistrationConfirmationData));
            break;
         case etMAP_Get_Folder_Listing_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Indication\n"));

            ProcessGetFolderListingIndicationEvent(&(MAPEventData->EventData.GetFolderListingIndicationData));
            break;
         case etMAP_Get_Folder_Listing_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Confirmation\n"));

            ProcessGetFolderListingConfirmationEvent(&(MAPEventData->EventData.GetFolderListingConfirmationData));
            break;
         case etMAP_Get_Message_Listing_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Indication\n"));

            ProcessGetMessageListingIndicationEvent(&(MAPEventData->EventData.GetMessageListingIndicationData));
            break;
         case etMAP_Get_Message_Listing_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Confirmation\n"));

            ProcessGetMessageListingConfirmationEvent(&(MAPEventData->EventData.GetMessageListingConfirmationData));
            break;
         case etMAP_Get_Message_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Indication\n"));

            ProcessGetMessageIndicationEvent(&(MAPEventData->EventData.GetMessageIndicationData));
            break;
         case etMAP_Get_Message_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Confirmation\n"));

            ProcessGetMessageConfirmationEvent(&(MAPEventData->EventData.GetMessageConfirmationData));
            break;
         case etMAP_Set_Message_Status_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Message Status Indication\n"));

            ProcessSetMessageStatusIndicationEvent(&(MAPEventData->EventData.SetMessageStatusIndicationData));
            break;
         case etMAP_Set_Message_Status_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Message Status Confirmation\n"));

            ProcessSetMessageStatusConfirmationEvent(&(MAPEventData->EventData.SetMessageStatusConfirmationData));
            break;
         case etMAP_Push_Message_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Message Indication\n"));

            ProcessPushMessageIndicationEvent(&(MAPEventData->EventData.PushMessageIndicationData));
            break;
         case etMAP_Push_Message_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Message Confirmation\n"));

            ProcessPushMessageConfirmationEvent(&(MAPEventData->EventData.PushMessageConfirmationData));
            break;
         case etMAP_Update_Inbox_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Inbox Indication\n"));

            ProcessUpdateInboxIndicationEvent(&(MAPEventData->EventData.UpdateInboxIndicationData));
            break;
         case etMAP_Update_Inbox_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Inbox Confirmation\n"));

            ProcessUpdateInboxConfirmationEvent(&(MAPEventData->EventData.UpdateInboxConfirmationData));
            break;
         case etMAP_Set_Folder_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Folder Indication\n"));

            ProcessSetFolderIndicationEvent(&(MAPEventData->EventData.SetFolderIndicationData));
            break;
         case etMAP_Set_Folder_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Folder Confirmation\n"));

            ProcessSetFolderConfirmationEvent(&(MAPEventData->EventData.SetFolderConfirmationData));
            break;
         case etMAP_Abort_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Indication\n"));

            ProcessAbortIndicationEvent(&(MAPEventData->EventData.AbortIndicationData));
            break;
         case etMAP_Abort_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Confirmation\n"));

            ProcessAbortConfirmationEvent(&(MAPEventData->EventData.AbortConfirmationData));
            break;
      }
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid MAP Event Data\n"));

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* process Device Manager (DEVM) Status Events (for out-going        */
   /* connection management).                                           */
static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status)
{
   int                                Result;
   Boolean_t                          Notification;
   MAPM_Entry_Info_t                 *MAPMEntryInfo;
   MAPM_Entry_Info_t                 *NextMAPMEntryInfo;
   MAP_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (MAPM): 0x%08X, %d\n", StatusType, Status));

   /* Flag that this is NOT a notification connection (at least not     */
   /* until we figure out that it is).                                  */
   Notification = FALSE;


   /* We need to handle all connections waiting on this status.         */
   MAPMEntryInfo = MAPMEntryInfoList;

   while(MAPMEntryInfo != NULL)
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Loop: %d\n",Notification));

      /* Go ahead and grab the next entry, because some code paths may  */
      /* delete the current entry.                                      */
      NextMAPMEntryInfo = MAPMEntryInfo->NextMAPMEntryInfoPtr;

      /* Check if this connection is one we are waiting on, and it is in*/
      /* a valid state.                                                 */
      if(COMPARE_BD_ADDR(BD_ADDR, MAPMEntryInfo->RemoteDeviceAddress) && (MAPMEntryInfo->ConnectionState != csConnected) && (!MAPMEntryInfo->MAPID))
      {
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Matching Connection Entry Found\n"));

         /* Check whether this is incoming or outgoing.                 */
         if(!(MAPMEntryInfo->ConnectionFlags & MAPM_ENTRY_INFO_FLAGS_SERVER))
         {
            /* Outgoing, so handle properly.                            */

            /* Initialize common connection event members.              */
            BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(MAP_Open_Port_Confirmation_Data_t));

            /*************************** NOTE ***************************/
            /* * We do not have a MAP ID because we were unable to    * */
            /* * make a connection.  To allow re-use of the           * */
            /* * disconnect event dispatching we will use the         * */
            /* * Tracking ID AND logical OR the Most Significant bit  * */
            /* * (this is an ID that cannot occur as a MAP ID).       * */
            /* * There is special code added to process Open Port     * */
            /* * Confirmation function to handle this case.           * */
            /*************************** NOTE ***************************/
            if(!MAPMEntryInfo->MAPID)
               OpenPortConfirmationData.MAPID = MAPMEntryInfo->TrackingID | 0x80000000;
            else
               OpenPortConfirmationData.MAPID = MAPMEntryInfo->MAPID;

            if(Status)
            {
               /* Disconnect the device.                                */
               DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

               /* Connection Failed.                                    */

               /* Map the status to a known status.                     */
               switch(Status)
               {
                  case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
                  case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                     OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_CONNECTION_REFUSED;
                     break;
                  case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
                  case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                     OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_CONNECTION_TIMEOUT;
                     break;
                  default:
                     OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_UNKNOWN_ERROR;
                     break;
               }

               /* * NOTE * This function will delete the MAP entry from */
               /*          the list.                                    */
               ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
            }
            else
            {
               /* Connection succeeded.                                 */

               /* Move the state to the connecting state.               */
               MAPMEntryInfo->ConnectionState = csConnecting;

               /* We need to handle the connection differently based    */
               /* upon if the user is requesting to connect to a        */
               /* notification server.                                  */
               if(Notification)
               {
                  /* Notification server. We need to determine the MAP  */
                  /* ID that has registered for Notifications.          */
                  Result = _MAP_Open_Remote_Message_Notification_Server_Port(MAPMEntryInfo->InstanceID, MAPMEntryInfo->PortNumber);
               }
               else
                  Result = _MAP_Open_Remote_Message_Access_Server_Port(BD_ADDR, MAPMEntryInfo->PortNumber);

               if((Result <= 0) && (Result != BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_ALREADY_CONNECTED))
               {
                  /* Error opening device.                              */
                  DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

                  OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_UNKNOWN_ERROR;

                  /* * NOTE * This function will delete the Message     */
                  /*          Access entry from the list.               */
                  ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
               }
               else
               {
                  /* If the device is already connected, we will        */
                  /* dispatch the the Status only (note this case       */
                  /* shouldn't really occur, but just to be safe we will*/
                  /* clean up our state machine).                       */
                  if(Result == BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_ALREADY_CONNECTED)
                  {
                     MAPMEntryInfo->ConnectionState      = csConnected;

                     OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_SUCCESS;

                     ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
                  }
                  else
                     MAPMEntryInfo->MAPID = (unsigned int)Result;
               }
            }
         }
         else
         {
            /* Incoming, so handle properly.                            */
            if((MAPMEntryInfo->ConnectionState == csAuthenticating) || (MAPMEntryInfo->ConnectionState == csEncrypting))
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Entry found\n"));

               /* Status references an outgoing connection.             */
               if(!Status)
               {
                  /* Success, accept the connection.                    */
                  _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, TRUE);
               }
               else
               {
                  /* Failure, reject the connection.                    */
                  _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);

                  /* Set the state back to Idle.                        */
                  MAPMEntryInfo->ConnectionState = csIdle;
               }
            }
         }
      }

      /* Move to the next connection entry.                             */
      MAPMEntryInfo = NextMAPMEntryInfo;

      /* If this is the end of the MAP Connections, we need to switch to*/
      /* Notification Connections.                                      */
      if((!MAPMEntryInfo) && (!Notification))
      {
         MAPMEntryInfo = MAPMEntryInfoList_Notification;
         Notification  = TRUE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (MAPM)\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Message Access Manager asynchronous events. */
static void BTPSAPI BTPMDispatchCallback_MAPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Message Access state      */
         /* information.                                                */
         if(DEVM_AcquireLock())
         {
            /* Process the Message.                                     */
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Message Access Manager update events.       */
static void BTPSAPI BTPMDispatchCallback_MAP(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to ensure the function parameter is not NULL.               */
   if(CallbackParameter)
   {
      /* Check to make sure the module is initialized.                  */
      if(Initialized)
      {
         /* Wait for access to the Message Access state information.    */
         if(DEVM_AcquireLock())
         {
            if(((MAPM_Update_Data_t *)CallbackParameter)->UpdateType == utMAPEvent)
            {
               /* Process the Message.                                  */
               ProcessMAPEvent(&(((MAPM_Update_Data_t *)CallbackParameter)->UpdateData.MAPEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Process the Client Un-Register Message.                  */
            ProcessClientUnRegister((unsigned int)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Message Access Profile Manager Timer Events.*/
static void BTPSAPI BTPMDispatchCallback_TMR(void *CallbackParameter)
{
   int                                Result;
   Boolean_t                          Notification;
   MAPM_Entry_Info_t                 *MAPMEntryInfo;
   MAP_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (MAPM)\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            if((MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList, (unsigned int)CallbackParameter)) == NULL)
            {
               if((MAPMEntryInfo = SearchMAPMEntryInfoByTrackingID(&MAPMEntryInfoList_Notification, (unsigned int)CallbackParameter)) != NULL)
                  Notification = TRUE;
               else
                  Notification = FALSE;
            }
            else
               Notification = FALSE;

            if(MAPMEntryInfo)
            {
               /* Check to see if the Timer is still active.            */
               if(MAPMEntryInfo->ConnectionTimerID)
               {
                  /* Flag that the Timer is no longer valid (it has been*/
                  /* processed).                                        */
                  MAPMEntryInfo->ConnectionTimerID = 0;

                  /* Finally make sure that we are still in the correct */
                  /* state.                                             */
                  if(MAPMEntryInfo->ConnectionState == csConnectingWaiting)
                  {
                     /* Everything appears to be valid, go ahead and    */
                     /* attempt to check to see if a connection is      */
                     /* possible (if so, attempt it).                   */
                     if(!SPPM_WaitForPortDisconnection(MAPMEntryInfo->PortNumber, FALSE, MAPMEntryInfo->RemoteDeviceAddress, MAXIMUM_MAP_PORT_DELAY_TIMEOUT_MS))
                     {
                        /* Port is disconnected, let's attempt to make  */
                        /* the connection.                              */
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device      */
                        if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION)
                           MAPMEntryInfo->ConnectionState = csEncrypting;
                        else
                        {
                           if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION)
                              MAPMEntryInfo->ConnectionState = csAuthenticating;
                           else
                              MAPMEntryInfo->ConnectionState = csConnectingDevice;
                        }

                        Result = DEVM_ConnectWithRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, (MAPMEntryInfo->ConnectionState == csConnectingDevice)?0:((MAPMEntryInfo->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                        if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                        {
                           /* Check to see if we need to actually issue */
                           /* the Remote connection.                    */
                           if(Result < 0)
                           {
                              /* Set the state to connecting remote     */
                              /* device.                                */
                              MAPMEntryInfo->ConnectionState = csConnecting;


                              if(Notification)
                              {
                                 /* Notification server.  We need to    */
                                 /* determine the MAP ID that has       */
                                 /* registered for Notifications.       */
                                 Result = _MAP_Open_Remote_Message_Notification_Server_Port(MAPMEntryInfo->InstanceID, MAPMEntryInfo->PortNumber);
                              }
                              else
                                 Result = _MAP_Open_Remote_Message_Access_Server_Port(MAPMEntryInfo->RemoteDeviceAddress, MAPMEntryInfo->PortNumber);

                              if(Result < 0)
                                 Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
                              else
                              {
                                 /* Note the MAP ID.                    */
                                 MAPMEntryInfo->MAPID = (unsigned int)Result;

                                 /* Flag success.                       */
                                 Result               = 0;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Port is not disconnected, check to see if the*/
                        /* count exceedes the maximum count.            */
                        MAPMEntryInfo->ConnectionTimerCount++;

                        if(MAPMEntryInfo->ConnectionTimerCount >= MAXIMUM_MAP_PORT_OPEN_DELAY_RETRY)
                           Result = BTPM_ERROR_CODE_MESSAGE_ACCESS_CONNECTION_RETRIES_EXCEEDED;
                        else
                        {
                           /* Port is NOT disconnected, go ahead and    */
                           /* start a timer so that we can continue to  */
                           /* check for the Port Disconnection.         */
                           Result = TMR_StartTimer((void *)MAPMEntryInfo->TrackingID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                           /* If the timer was started, go ahead and    */
                           /* note the Timer ID.                        */
                           if(Result > 0)
                           {
                              MAPMEntryInfo->ConnectionTimerID = (unsigned int)Result;

                              Result                           = 0;
                           }
                           else
                              Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
                        }
                     }

                     /* Error occurred, go ahead and dispatch an error  */
                     /* (as well as to delete the connection entry).    */
                     if(Result < 0)
                     {
                        /* Initialize common connection event members.  */
                        BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(MAP_Open_Port_Confirmation_Data_t));

                        /********************* NOTE *********************/
                        /* * We do not have a MAP ID because we were  * */
                        /* * unable to make a connection.  To allow   * */
                        /* * re-use of the disconnect event           * */
                        /* * dispatching we will use the Tracking ID  * */
                        /* * AND logical OR the Most Significant bit  * */
                        /* * (this is an ID that cannot occur as a    * */
                        /* * MAP ID).  There is special code added to * */
                        /* * process Open Port Confirmation function  * */
                        /* * to handle this case.                     * */
                        /********************* NOTE *********************/
                        OpenPortConfirmationData.MAPID = MAPMEntryInfo->TrackingID | 0x80000000;

                        if(Result)
                        {
                           /* Error, go ahead and disconnect the device.*/
                           DEVM_DisconnectRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, 0);

                           /* Connection Failed.                        */

                           /* Map the status to a known status.         */
                           switch(Result)
                           {
                              case BTPM_ERROR_CODE_MESSAGE_ACCESS_CONNECTION_RETRIES_EXCEEDED:
                              case BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER:
                                 OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_CONNECTION_TIMEOUT;
                                 break;
                              case BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE:
                                 OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_UNKNOWN_ERROR;
                                 break;
                              case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
                              case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                              default:
                                 OpenPortConfirmationData.OpenStatus = MAP_OPEN_STATUS_CONNECTION_REFUSED;
                                 break;
                           }

                           /* * NOTE * This function will delete the    */
                           /*          Message Access entry from the    */
                           /*          list.                            */
                           ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
                        }
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MAP Connection is no longer in the correct state: 0x%08X (%d)\n", (unsigned int)CallbackParameter, MAPMEntryInfo->ConnectionState));
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MAP Close Timer is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MAP Connection is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (MAPM)\n"));
}

   /* The following function is the Timer Callback function that is     */
   /* registered to process Serial Port Disconnection Events (to        */
   /* determine when it is safe to connect to a remote device).         */
static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (MAPM)\n"));

   /* Simply queue a Timer Callback Event to process.                   */
   if(BTPM_QueueMailboxCallback(BTPMDispatchCallback_TMR, CallbackParameter))
      ret_val = FALSE;
   else
      ret_val = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (MAPM): %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all MAP Manager Messages.   */
static void BTPSAPI MessageAccessManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MAP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a MAP Manager defined    */
            /* Message.  If it is it will be within the range:          */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Still processing, go ahead and post the message to the*/
               /* MAP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_MAPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue MAP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

                  MSG_FreeReceivedMessageGroupHandlerMessage(Message);
               }
            }
            else
            {
               /* Dispatch to the main handler that a client has        */
               /* un-registered.                                        */
               if(Message->MessageHeader.MessageFunction == BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION)
               {
                  if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE) && (!(((BTPM_Client_Registration_Message_t *)Message)->Registered)))
                  {
                     if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_MSG, (void *)(((BTPM_Client_Registration_Message_t *)Message)->AddressID)))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue MAP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an MAP Manager defined   */
            /* Message.  If it is it will be within the range:          */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
               MSG_FreeReceivedMessageGroupHandlerMessage(Message);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Non MAP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Serial Port Profile Manager dispatches an event (and the client   */
   /* has registered for events).  This function passes to the caller   */
   /* the Serial Port Profile Manager Event and the Callback Parameter  */
   /* that was specified when this Callback was installed.  The caller  */
   /* is free to use the contents of the Event Data ONLY in the context */
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e. this function DOES NOT have be reentrant).         */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* Message will not be processed while this function call is         */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
static void BTPSAPI SPPM_Event_Callback(SPPM_Event_Data_t *EventData, void *CallbackParameter)
{
   MAPM_Notification_Info_t *NotificationInfo;

   if((EventData) && ((NotificationInfo = (MAPM_Notification_Info_t *)CallbackParameter) != NULL))
   {
      /* Reject and/or close anybody who attempts to connect to the     */
      /* placeholder SPP Port.                                          */
      switch(EventData->EventType)
      {
         case setServerPortOpenRequest:
            /* Reject the Port Open request on the Fake SPP Port.       */
            if(EventData->EventData.ServerPortOpenRequestEventData.PortHandle == NotificationInfo->FakeSPPPortID)
               SPPM_OpenServerPortRequestResponse(NotificationInfo->FakeSPPPortID, FALSE);
            break;
         default:
            /* Nothing to do.                                           */
            break;
      }
   }
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager MAP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI MAPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                         Result;
   MAPM_Initialization_Data_t *MAPMInitializationData;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing MAP Manager\n"));

         /* Initialize success.                                         */
         Result = 0;

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process MAP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER, MessageAccessManagerGroupHandler, NULL))
         {
            /* Note the default MAP Initialization parameters.          */
            BTPS_MemInitialize(&NotificationInfo, 0, MAPM_NOTIFICATION_INFO_DATA_SIZE);

            NotificationInfo.NotificationServiceName = "MAP Notificiation Server";

            /* Check to see if any Initialization data was specified.   */
            if((MAPMInitializationData = (MAPM_Initialization_Data_t *)InitializationData) != NULL)
            {
               /* Initialization data specified, now let's verify that  */
               /* the data is valid.                                    */
               if(MAPMInitializationData->NotificationServerPort <= MAP_PORT_NUMBER_MAXIMUM)
               {
                  /* Note the configuration parameters.                 */
                  NotificationInfo.NotificationServerPort  = MAPMInitializationData->NotificationServerPort;
                  NotificationInfo.NotificationServiceName = MAPMInitializationData->NotificationServiceName;
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Notification Server Port: %d\n", MAPMInitializationData->NotificationServerPort));

                  Result = 1;
               }
            }

            /* Initialize the actual MAP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the MAP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _MAPM_Initialize()))
            {
               /* Go ahead and flag that this module is initialized.    */
               Initialized = TRUE;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _MAPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MAP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the MAP Manager Implementation that  */
            /* we are shutting down.                                    */
            _MAPM_Cleanup();

            /* Make sure that the MAP Entry Information List is empty.  */
            FreeMAPMEntryInfoList(&MAPMEntryInfoList);
            FreeMAPMEntryInfoList(&MAPMEntryInfoList_Notification);

            /* Free up information stored for the local Notification    */
            /* Server.                                                  */
            CleanupLocalNotificationServer(&NotificationInfo);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI MAPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                Result;
   Boolean_t          Present;
   Boolean_t          Done;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the MAP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  /* Set the Bluetooth Stack ID in the MAPMGR.          */
                  _MAPM_SetBluetoothStackID((unsigned int)Result);

                  /* Clear the connection count for the local           */
                  /* notification server.                               */
                  NotificationInfo.NumberOfConnections            = 0;

                  /* Clear the SDP Record Handle for the local          */
                  /* notification server.                               */
                  NotificationInfo.MAPNotificationServerSDPRecord = 0;

                  /* Register an SPP Port holder if the server port is  */
                  /* valid.                                             */
                  if(NotificationInfo.NotificationServerPort)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Server Port Number: %u.\n", NotificationInfo.NotificationServerPort));

                     /* Verify that no other servers are open on this   */
                     /* port.                                           */
                     Result = SPPM_QueryServerPresent(NotificationInfo.NotificationServerPort, &Present);
                     if((!Result) && (!Present))
                     {
                        /* Register a fake Notification Server SPP Port.*/
                        Result = SPPM_RegisterServerPort(NotificationInfo.NotificationServerPort, SPPM_REGISTER_SERVER_PORT_FLAGS_REQUIRE_AUTHORIZATION, SPPM_Event_Callback, (void *)&NotificationInfo);
                        if(Result > 0)
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Server Port ID: %u.\n", (unsigned int)Result));

                           NotificationInfo.FakeSPPPortID = (unsigned int)Result;
                        }
                        else
                           DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Failed to get SPP Port for Notification Server %d.\n", Result));
                     }
                     else
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Failed to open MAP Notification Server on Port # %u (server already present).\n", NotificationInfo.NotificationServerPort));

                        NotificationInfo.NotificationServerPort = 0;
                     }
                  }
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Loop through all outgoing connections to determine if */
               /* there are any synchronous connections outstanding.    */
               MAPMEntryInfo = MAPMEntryInfoList;

               Done          = FALSE;

               while(MAPMEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(MAPMEntryInfo->ConnectionEvent)
                  {
                     MAPMEntryInfo->ConnectionStatus = MAPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(MAPMEntryInfo->ConnectionEvent);
                  }

                  /* Next, determine if we need to close down anything. */
                  if(MAPMEntryInfo->MAPID)
                  {
                     if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_SERVER)
                     {
                        if(MAPMEntryInfo->ConnectionState == csAuthorizing)
                           _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);

                        _MAP_Close_Server(MAPMEntryInfo->MAPID);

                        if(MAPMEntryInfo->ServiceRecordHandle)
                           _MAP_Un_Register_SDP_Record(MAPMEntryInfo->MAPID, MAPMEntryInfo->ServiceRecordHandle);
                     }
                     else
                        _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                  }

                  MAPMEntryInfo = MAPMEntryInfo->NextMAPMEntryInfoPtr;

                  /* Loop back around and handle the Notification       */
                  /* servers.                                           */
                  if((!MAPMEntryInfo) && (!Done))
                  {
                     MAPMEntryInfo = MAPMEntryInfoList_Notification;

                     Done          = TRUE;
                  }
               }

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeMAPMEntryInfoList(&MAPMEntryInfoList);
               FreeMAPMEntryInfoList(&MAPMEntryInfoList_Notification);

               /* Free up information stored for the local Notification */
               /* Server.                                               */
               CleanupLocalNotificationServer(&NotificationInfo);

               /* Inform the MAP Manager that the Stack has been closed.*/
               _MAPM_SetBluetoothStackID(0);
               break;
            case detRemoteDeviceAuthenticationStatus:
               /* Authentication Status, process the Status Event.      */
               ProcessDEVMStatusEvent(dstAuthentication, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status);
               break;
            case detRemoteDeviceEncryptionStatus:
               /* Encryption Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstEncryption, EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status);
               break;
            case detRemoteDeviceConnectionStatus:
               /* Connection Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstConnection, EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the MAP Manager of a specific Update Event.  The MAP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t MAPM_NotifyUpdate(MAPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utMAPEvent:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing MAP Event: %d\n", UpdateData->UpdateData.MAPEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_MAP, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Message Access Profile (MAP) Manager (MAPM) Common Functions.     */

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming MAP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A MAP Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI MAPM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Accept)
{
   int                ret_val;
   Boolean_t          Authenticate;
   Boolean_t          Encrypt;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, determine if the Port is in the correct state.        */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
         {
            /* Verify that the server process is the 'owner' of this    */
            /* server/client connection.                                */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               if(MAPMEntryInfo->ConnectionState == csAuthorizing)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: 0x%08X (%d), %d\n", InstanceID, InstanceID, Accept));

                  /* If the caller has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(Accept)
                  {
                     /* Determine if Authentication and/or Encryption is*/
                     /* required for this link.                         */
                     if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION)
                        Authenticate = TRUE;
                     else
                        Authenticate = FALSE;

                     if(MAPMEntryInfo->ConnectionFlags & MAPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION)
                        Encrypt = TRUE;
                     else
                        Encrypt = FALSE;

                     if((Authenticate) || (Encrypt))
                     {
                        if(Encrypt)
                           ret_val = DEVM_EncryptRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, 0);
                        else
                           ret_val = DEVM_AuthenticateRemoteDevice(MAPMEntryInfo->RemoteDeviceAddress, 0);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

                     if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                     {
                        /* Authorization not required, and we are       */
                        /* already in the correct state.                */
                        if((ret_val = _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, TRUE)) != 0)
                        {
                           /* Failure, go ahead and try to disconnect it*/
                           /* (will probably fail as well).             */
                           MAPMEntryInfo->ConnectionState = csIdle;

                           _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                        }
                     }
                     else
                     {
                        /* If we were successfully able to Authenticate */
                        /* and/or Encrypt, then we need to set the      */
                        /* correct state.                               */
                        if(!ret_val)
                        {
                           if(Encrypt)
                              MAPMEntryInfo->ConnectionState = csEncrypting;
                           else
                              MAPMEntryInfo->ConnectionState = csAuthenticating;

                           /* Flag success to the caller.               */
                           ret_val = 0;
                        }
                        else
                        {
                           /* Error, reject the request.                */
                           if(_MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE))
                           {
                              /* Failure, go ahead and try to disconnect*/
                              /* it (will probably fail as well).       */
                              MAPMEntryInfo->ConnectionState = csIdle;

                              _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                           }
                        }
                     }
                  }
                  else
                  {
                     /* Rejection - Simply respond to the request.      */
                     _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);

                     MAPMEntryInfo->ConnectionState = csIdle;

                     _MAP_Close_Connection(MAPMEntryInfo->MAPID);

                     /* Flag success.                                   */
                     ret_val = 0;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_NOT_CONNECTED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Register_Server(unsigned int ServerPort, unsigned long ServerFlags, unsigned int InstanceID, unsigned long SupportedMessageTypes, MAPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   MAPM_Entry_Info_t  MAPMEntryInfo;
   MAPM_Entry_Info_t *MAPMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(CallbackFunction)
      {
         /* Verify that the specified Instance ID appears to be         */
         /* semi-valid.                                                 */
         if((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE))
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Next, check to see if we are powered up.              */
               if(CurrentPowerState)
               {
                  /* Device Powered up, go ahead and store the          */
                  /* information into our list.                         */

                  /* First, let's make sure there is NO server that is  */
                  /* registered for the same Instance ID.               */
                  if(!SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, InstanceID))
                  {
                     BTPS_MemInitialize(&MAPMEntryInfo, 0, sizeof(MAPM_Entry_Info_t));

                     MAPMEntryInfo.TrackingID            = GetNextTrackingID();
                     MAPMEntryInfo.ClientID              = MSG_GetServerAddressID();
                     MAPMEntryInfo.Flags                 = MAPM_ENTRY_INFO_FLAGS_SERVER;
                     MAPMEntryInfo.ConnectionState       = csIdle;
                     MAPMEntryInfo.ConnectionFlags       = ServerFlags;
                     MAPMEntryInfo.CurrentOperation      = coNone;
                     MAPMEntryInfo.InstanceID            = InstanceID;
                     MAPMEntryInfo.PortNumber            = ServerPort;
                     MAPMEntryInfo.SupportedMessageTypes = SupportedMessageTypes;
                     MAPMEntryInfo.CallbackFunction      = CallbackFunction;
                     MAPMEntryInfo.CallbackParameter     = CallbackParameter;

                     if((MAPMEntryInfoPtr = AddMAPMEntryInfoEntry(&MAPMEntryInfoList, &MAPMEntryInfo)) != NULL)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Register Server Port: %u, Instance: %u, Flags: 0x%08lX\n", ServerPort, InstanceID, ServerFlags));

                        /* Next, attempt to register the Server.        */
                        if((ret_val = _MAP_Open_Message_Access_Server(ServerPort)) > 0)
                        {
                           /* Note the returned MAP ID.                 */
                           MAPMEntryInfoPtr->MAPID = (unsigned int)ret_val;

                           /* Flag success to the caller.               */
                           ret_val                 = 0;
                        }
                        else
                        {
                           /* Error opening port, go ahead and delete   */
                           /* the entry that was added.                 */
                           if((MAPMEntryInfoPtr = DeleteMAPMEntryInfoEntry(&MAPMEntryInfoList, MAPMEntryInfoPtr->TrackingID)) != NULL)
                           {
                              CleanupMAPMEntryInfo(MAPMEntryInfoPtr);

                              FreeMAPMEntryInfoEntryMemory(MAPMEntryInfoPtr);
                           }
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_DUPLICATE_INSTANCE_ID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

               /* Release the Lock because we are finished with it.     */
               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered MAP server (registered via a  */
   /* successful call the the MAP_Register_Server() function).  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI MAPM_Un_Register_Server(unsigned int InstanceID)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, attempt to find the MAP Server Entry from the MAP     */
         /* Entry List.                                                 */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, InstanceID)) != NULL)
         {
            /* Verify that the server process is the 'owner' of this    */
            /* server/client connection.                                */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Next, attempt to delete the MAP Server Entry from the */
               /* MAP Entry List.                                       */
               if((MAPMEntryInfo = DeleteMAPMEntryInfoEntry(&MAPMEntryInfoList, MAPMEntryInfo->MAPID)) != NULL)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Delete Server: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Reject any incoming connection that might be in    */
                  /* progress.                                          */
                  if((MAPMEntryInfo->ConnectionState == csAuthenticating) || (MAPMEntryInfo->ConnectionState == csAuthorizing) || (MAPMEntryInfo->ConnectionState == csEncrypting))
                     _MAP_Open_Request_Response(MAPMEntryInfo->MAPID, FALSE);

                  /* If there was a Service Record Registered, go ahead */
                  /* and make sure it is freed.                         */
                  if(MAPMEntryInfo->ServiceRecordHandle)
                     _MAP_Un_Register_SDP_Record(MAPMEntryInfo->MAPID, MAPMEntryInfo->ServiceRecordHandle);

                  /* Next, go ahead and Un-Register the Server.         */
                  _MAP_Close_Server(MAPMEntryInfo->MAPID);

                  /* Clean up any resources that were allocated for this*/
                  /* entry.                                             */
                  CleanupMAPMEntryInfo(MAPMEntryInfo);

                  /* All finished, free any memory that was allocated   */
                  /* for the server.                                    */
                  FreeMAPMEntryInfoEntryMemory(MAPMEntryInfo);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to Register an SDP Service Record for a previously        */
   /* registered MAP Server.  This function returns a positive,         */
   /* non-zero, value if successful, or a negative return error code if */
   /* there was an error.  If this function is successful, the value    */
   /* that is returned represents the SDP Service Record Handle of the  */
   /* Service Record that was added to the SDP Database.  The           */
   /* ServiceName parameter is a pointer to a NULL terminated UTF-8     */
   /* encoded string.                                                   */
long BTPSAPI MAPM_Register_Service_Record(unsigned int InstanceID, char *ServiceName)
{
   long               ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that the Service Name is semi-valid.                    */
      if((ServiceName) && (BTPS_StringLength(ServiceName)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to delete the MAP Server Entry from the MAP*/
            /* Entry List.                                              */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, InstanceID)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Register SDP Record : 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Verify that there is no Service Record already     */
                  /* registered.                                        */
                  if(!MAPMEntryInfo->ServiceRecordHandle)
                  {
                     if((ret_val = _MAP_Register_Message_Access_Server_SDP_Record(MAPMEntryInfo->MAPID, ServiceName, (Byte_t)MAPMEntryInfo->InstanceID, (Byte_t)MAPMEntryInfo->SupportedMessageTypes)) > 0)
                     {
                        /* Success.  Note the handle that was returned. */
                        MAPMEntryInfo->ServiceRecordHandle = (unsigned long)ret_val;
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ALREADY_REGISTERED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %ld\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered SDP Service Record.*/
   /* This function accepts the Instance ID of the MAP Server that is to*/
   /* have the Service Record Removed.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int BTPSAPI MAPM_Un_Register_Service_Record(unsigned int InstanceID)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, attempt to locate the MAP Server Entry in the MAP     */
         /* Entry List.                                                 */
         if((MAPMEntryInfo = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, InstanceID)) != NULL)
         {
            /* Verify that the server process is the 'owner' of this    */
            /* server/client connection.                                */
            if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Un-Register SDP Record : 0x%08X (%d)\n", InstanceID, InstanceID));

               /* Verify that there is a Service Record registered.     */
               if(MAPMEntryInfo->ServiceRecordHandle)
               {
                  if((ret_val = _MAP_Un_Register_SDP_Record(MAPMEntryInfo->MAPID, MAPMEntryInfo->ServiceRecordHandle)) == 0)
                  {
                     /* Success.  Note the handle that was returned.    */
                     MAPMEntryInfo->ServiceRecordHandle = 0;
                  }
               }
               else
                  ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
   /*          MAPM_Free_Parsed_Message_Access_Service_Info() in order  */
   /*          to release any resources that were allocated during the  */
   /*          query process.                                           */
int BTPSAPI MAPM_Parse_Remote_Message_Access_Services(BD_ADDR_t RemoteDeviceAddress, MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo)
{
   int                                    ret_val;
   UUID_128_t                             MASServiceClass;
   UUID_128_t                             RFCOMMProtocolUUID;
   UUID_128_t                             RecordProtocolUUID;
   unsigned int                           ProtocolIndex;
   unsigned int                           ServiceRecordIndex;
   unsigned int                           ServiceDetailIndex;
   unsigned int                           RawServiceDataLength;
   unsigned int                           ReservedBufferLength;
   unsigned int                           NumberMessageAccessRecords;
   unsigned char                         *ReservedBuffer;
   unsigned char                         *RawServiceDataBuffer;
   unsigned char                         *ReservedBufferPosition;
   SDP_Data_Element_t                    *Attribute;
   DEVM_Parsed_SDP_Data_t                 ParsedSDPData;
   MAPM_MAS_Service_Details_t            *ServiceDetailBuffer;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   SDP_ASSIGN_MESSAGE_ACCESS_SERVER_UUID_128(MASServiceClass);
   SDP_ASSIGN_RFCOMM_UUID_128(RFCOMMProtocolUUID);

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ServiceInfo))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            if((ret_val = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, FALSE, 0, NULL, &RawServiceDataLength)) >= 0)
            {
               if(RawServiceDataLength)
               {
                  if((RawServiceDataBuffer = (unsigned char *)BTPS_AllocateMemory(RawServiceDataLength)) != NULL)
                  {
                     if((ret_val = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, FALSE, RawServiceDataLength, RawServiceDataBuffer, NULL)) == (int)RawServiceDataLength)
                     {
                        /* Release the lock because we are finished with*/
                        /* it.                                          */
                        DEVM_ReleaseLock();

                        RawServiceDataLength = (unsigned int)ret_val;
                        ret_val              = 0;

                        if((ret_val = DEVM_ConvertRawSDPStreamToParsedSDPData(RawServiceDataLength, RawServiceDataBuffer, &ParsedSDPData)) == 0)
                        {
                           NumberMessageAccessRecords = 0;
                           ReservedBufferLength       = 0;

                           /* Determine the number of Message Access    */
                           /* records and the amount of extra storage   */
                           /* required for the service details.         */
                           for(ServiceRecordIndex = 0; ServiceRecordIndex < ParsedSDPData.NumberServiceRecords; ServiceRecordIndex++)
                           {
                              if(ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], MASServiceClass))
                              {
                                 NumberMessageAccessRecords++;

                                 /* Determine the size of variable data */
                                 /* within the record.                  */
                                 if((Attribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], (SDP_DEFAULT_LANGUAGE_BASE_ATTRIBUTE_ID + SDP_ATTRIBUTE_OFFSET_ID_SERVICE_NAME))) != NULL)
                                 {
                                    if(Attribute->SDP_Data_Element_Type == deTextString)
                                       ReservedBufferLength += (Attribute->SDP_Data_Element_Length + 1);
                                 }
                              }
                           }

                           if(NumberMessageAccessRecords)
                           {
                              if((ServiceDetailBuffer = (MAPM_MAS_Service_Details_t *)BTPS_AllocateMemory(NumberMessageAccessRecords * sizeof(MAPM_MAS_Service_Details_t))) != NULL)
                              {
                                 ret_val = 0;

                                 if(ReservedBufferLength)
                                 {
                                    if((ReservedBuffer = (unsigned char *)BTPS_AllocateMemory(ReservedBufferLength)) != NULL)
                                    {
                                       ReservedBufferPosition = ReservedBuffer;
                                    }
                                    else
                                    {
                                       ret_val                = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                                       ReservedBufferPosition = NULL;

                                       BTPS_FreeMemory(ServiceDetailBuffer);
                                    }
                                 }
                                 else
                                 {
                                    ReservedBuffer         = NULL;
                                    ReservedBufferPosition = NULL;
                                 }

                                 if(!ret_val)
                                 {
                                    ServiceDetailIndex = 0;

                                    /* Populate the buffers with the    */
                                    /* parsed service record data.      */
                                    for(ServiceRecordIndex = 0; ((ServiceRecordIndex < ParsedSDPData.NumberServiceRecords) && (ServiceDetailIndex < NumberMessageAccessRecords)); ServiceRecordIndex++)
                                    {
                                       if(ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], MASServiceClass))
                                       {
                                          /* ServerPort                 */
                                          if((Attribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], SDP_ATTRIBUTE_ID_PROTOCOL_DESCRIPTOR_LIST)) != NULL)
                                          {
                                             //XXX Replace with a generic "Get RFCOMM Port" routine once implemented in SPPM
                                             if(Attribute->SDP_Data_Element_Type == deSequence)
                                             {
                                                for(ProtocolIndex = 0; ProtocolIndex < Attribute->SDP_Data_Element_Length; ProtocolIndex++)
                                                {
                                                   if((Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element_Type == deSequence) && (Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element_Length >= 2))
                                                   {
                                                      /* Check if we    */
                                                      /* have the RFCOMM*/
                                                      /* protocol       */
                                                      /* descriptor.    */
                                                      if((Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUUID_16) || (Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUUID_32) || (Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUUID_128))
                                                      {
                                                         switch(Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type)
                                                         {
                                                            case deUUID_16:
                                                               SDP_ASSIGN_BASE_UUID(RecordProtocolUUID);
                                                               ASSIGN_SDP_UUID_16_TO_SDP_UUID_128(RecordProtocolUUID, Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UUID_16);
                                                               break;
                                                            case deUUID_32:
                                                               SDP_ASSIGN_BASE_UUID(RecordProtocolUUID);
                                                               ASSIGN_SDP_UUID_32_TO_SDP_UUID_128(RecordProtocolUUID, Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UUID_32);
                                                               break;
                                                            case deUUID_128:
                                                               RecordProtocolUUID = Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UUID_128;
                                                               break;
                                                            default:
                                                               BTPS_MemInitialize(&RecordProtocolUUID, 0, sizeof(UUID_128_t));
                                                         }

                                                         if(COMPARE_UUID_128(RecordProtocolUUID, RFCOMMProtocolUUID))
                                                         {
                                                            if(Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Type == deUnsignedInteger1Byte)
                                                               ServiceDetailBuffer[ServiceDetailIndex].ServerPort = Attribute->SDP_Data_Element.SDP_Data_Element_Sequence[ProtocolIndex].SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element.UnsignedInteger1Byte;
                                                         }
                                                      }
                                                   }
                                                }
                                             }
                                          }

                                          /* InstanceID                 */
                                          if((Attribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], SDP_ATTRIBUTE_ID_MAS_INSTANCE_ID)) != NULL)
                                          {
                                             if(Attribute->SDP_Data_Element_Type == deUnsignedInteger1Byte)
                                                ServiceDetailBuffer[ServiceDetailIndex].InstanceID = Attribute->SDP_Data_Element.UnsignedInteger1Byte;
                                          }

                                          /* SupportedMessageTypes      */
                                          if((Attribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], SDP_ATTRIBUTE_ID_SUPPORTED_MESSAGE_TYPES)) != NULL)
                                          {
                                             if(Attribute->SDP_Data_Element_Type == deUnsignedInteger1Byte)
                                                ServiceDetailBuffer[ServiceDetailIndex].SupportedMessageTypes = Attribute->SDP_Data_Element.UnsignedInteger1Byte;
                                          }

                                          if(ReservedBuffer)
                                          {
                                             /* ServiceName             */
                                             if((Attribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], (SDP_DEFAULT_LANGUAGE_BASE_ATTRIBUTE_ID + SDP_ATTRIBUTE_OFFSET_ID_SERVICE_NAME))) != NULL)
                                             {
                                                if((Attribute->SDP_Data_Element_Type == deTextString) && (Attribute->SDP_Data_Element_Length > 0))
                                                {
                                                   BTPS_MemCopy(ReservedBufferPosition, Attribute->SDP_Data_Element.TextString, Attribute->SDP_Data_Element_Length);
                                                   ReservedBufferPosition[Attribute->SDP_Data_Element_Length] = '\0';

                                                   ServiceDetailBuffer[ServiceDetailIndex].ServiceName = (char *)ReservedBufferPosition;
                                                   ReservedBufferPosition                             += (Attribute->SDP_Data_Element_Length + 1);
                                                }
                                             }
                                          }

                                          ServiceDetailIndex++;
                                       }
                                    }


                                    /* If we parsed as many service     */
                                    /* detail entries as we expected,   */
                                    /* then provide the parsed          */
                                    /* information to the caller and    */
                                    /* return success.                  */
                                    if(ServiceDetailIndex == NumberMessageAccessRecords)
                                    {
                                       ServiceInfo->NumberServices = NumberMessageAccessRecords;
                                       ServiceInfo->ServiceDetails = ServiceDetailBuffer;
                                       ServiceInfo->RESERVED       = ReservedBuffer;

                                       ret_val                     = 0;
                                    }
                                    else
                                       ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                                 }

                                 /* If an error occured, clean up       */
                                 /* allocated resources.                */
                                 if(ret_val)
                                 {
                                    BTPS_FreeMemory(ServiceDetailBuffer);

                                    if(ReservedBuffer)
                                       BTPS_FreeMemory(ReservedBuffer);
                                 }
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_SERVICE_NOT_ADVERTISED;

                           DEVM_FreeParsedSDPData(&ParsedSDPData);
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                     }
                     else
                     {
                        /* Release the lock because we are finished with*/
                        /* it.                                          */
                        DEVM_ReleaseLock();
                     }

                     BTPS_FreeMemory(RawServiceDataBuffer);
                  }
                  else
                  {
                     /* Release the lock because we are finished with   */
                     /* it.                                             */
                     DEVM_ReleaseLock();

                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
               }
               else
               {
                  /* Release the lock because we are finished with it.  */
                  DEVM_ReleaseLock();

                  ret_val = BTPM_ERROR_CODE_DEVICE_SERVICES_NOT_KNOWN;
               }
            }
            else
            {
               /* Release the lock because we are finished with it.     */
               DEVM_ReleaseLock();
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to free all resources that were allocated to query the    */
   /* service details of a remote Message Access Server device. See the */
   /* MAPM_Query_Remote_Message_Access_Services() function for more     */
   /* information.                                                      */
void BTPSAPI MAPM_Free_Parsed_Message_Access_Service_Info(MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo)
{
   /* Verify that the input parameters appear to be semi-valid.         */
   if(ServiceInfo)
   {
      /* Free any allocated data buffers.                               */
      if(ServiceInfo->ServiceDetails)
         BTPS_FreeMemory(ServiceInfo->ServiceDetails);

      if(ServiceInfo->RESERVED)
         BTPS_FreeMemory(ServiceInfo->RESERVED);

      BTPS_MemInitialize(ServiceInfo, 0, sizeof(MAPM_Parsed_Message_Access_Service_Info_t));
   }
}

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
int BTPSAPI MAPM_Connect_Remote_Device(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned int InstanceID, unsigned long ConnectionFlags, MAPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                ret_val;
   Event_t            ConnectionEvent;
   Boolean_t          Notification;
   unsigned int       TrackingID;
   unsigned int       MAPID;
   MAPM_Entry_Info_t  MAPMEntryInfo;
   MAPM_Entry_Info_t *MAPMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort >= MAP_PORT_NUMBER_MINIMUM) && (RemoteServerPort <= MAP_PORT_NUMBER_MAXIMUM) && (InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE))
      {
         /* Verify that the Event Callback appears to be semi-valid.    */
         if(CallbackFunction)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Next, check to see if we are powered up.              */
               if(CurrentPowerState)
               {
                  /* Initialize success.                                */
                  ret_val = 0;

                  /* Flag that there is no MAP ID that was found.       */
                  MAPID   = 0;

                  /* Determine if this a Client connection or a         */
                  /* Notification connection.                           */
                  if((ConnectionType == mctNotificationServer) || (ConnectionType == mctNotificationClient))
                  {
                     if((MAPMEntryInfoPtr = SearchMAPMEntryInfoByServerInstanceID(&MAPMEntryInfoList, InstanceID)) != NULL)
                     {
                        /* Verify that the server process is the 'owner'*/
                        /* of this server/client connection.            */
                        if(MAPMEntryInfoPtr->ClientID == MSG_GetServerAddressID())
                        {
                           /* Instance ID found.  Verify that           */
                           /* Notifications have been enabled.          */
                           if(MAPMEntryInfoPtr->Flags & MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED)
                           {
                              /* We need to connect to the Notification */
                              /* Server based on the existing MAP ID, so*/
                              /* let's go ahead and note it.            */
                              MAPID        = MAPMEntryInfoPtr->MAPID;

                              Notification = TRUE;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOTIFICATIONS_NOT_ENABLED;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;
                  }
                  else
                     Notification = FALSE;

                  if(!ret_val)
                  {
                     /* Next, make sure that we do not already have a   */
                     /* connection to the specified device.             */
                     if((MAPMEntryInfoPtr = SearchMAPMEntryInfoByConnection(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), RemoteDeviceAddress, RemoteServerPort, FALSE)) == NULL)
                     {
                        /* Entry is not present, go ahead and create a  */
                        /* new entry.                                   */
                        BTPS_MemInitialize(&MAPMEntryInfo, 0, sizeof(MAPM_Entry_Info_t));

                        MAPMEntryInfo.TrackingID          = GetNextTrackingID();
                        MAPMEntryInfo.ClientID            = MSG_GetServerAddressID();
                        MAPMEntryInfo.RemoteDeviceAddress = RemoteDeviceAddress;
                        MAPMEntryInfo.PortNumber          = RemoteServerPort;
                        MAPMEntryInfo.InstanceID          = InstanceID;
                        MAPMEntryInfo.ConnectionState     = csIdle;
                        MAPMEntryInfo.ConnectionFlags     = ConnectionFlags;
                        MAPMEntryInfo.CurrentOperation    = coNone;
                        MAPMEntryInfo.CallbackFunction    = CallbackFunction;
                        MAPMEntryInfo.CallbackParameter   = CallbackParameter;

                        if((MAPMEntryInfoPtr = AddMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), &MAPMEntryInfo)) != NULL)
                        {
                           if(ConnectionStatus)
                              MAPMEntryInfoPtr->ConnectionEvent = BTPS_CreateEvent(FALSE);

                           if((!ConnectionStatus) || ((ConnectionStatus) && (MAPMEntryInfoPtr->ConnectionEvent)))
                           {
                              /* First, let's wait for the Port to      */
                              /* disconnect.                            */
                              if(!SPPM_WaitForPortDisconnection(RemoteServerPort, FALSE, RemoteDeviceAddress, MAXIMUM_MAP_PORT_DELAY_TIMEOUT_MS))
                              {
                                 DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                                 /* Next, attempt to open the remote    */
                                 /* device.                             */
                                 if(ConnectionFlags & MAPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                                    MAPMEntryInfoPtr->ConnectionState = csEncrypting;
                                 else
                                 {
                                    if(ConnectionFlags & MAPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                                       MAPMEntryInfoPtr->ConnectionState = csAuthenticating;
                                    else
                                       MAPMEntryInfoPtr->ConnectionState = csConnectingDevice;
                                 }

                                 DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Device\n"));

                                 ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, (MAPMEntryInfoPtr->ConnectionState == csConnectingDevice)?0:((MAPMEntryInfoPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                                 if((ret_val >= 0) || (ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                                 {
                                    /* Check to see if we need to       */
                                    /* actually issue the Remote        */
                                    /* connection.                      */
                                    if(ret_val < 0)
                                    {
                                       /* Set the state to connecting   */
                                       /* remote device.                */
                                       MAPMEntryInfoPtr->ConnectionState = csConnecting;

                                       /* We need to handle the         */
                                       /* connection differently based  */
                                       /* upon if the user is requesting*/
                                       /* to connect to a notification  */
                                       /* server.                       */
                                       if(Notification)
                                       {
                                          /* Notification server.  We   */
                                          /* need to determine the MAP  */
                                          /* ID that has registered for */
                                          /* Notifications.             */
                                          ret_val = _MAP_Open_Remote_Message_Notification_Server_Port(MAPID, RemoteServerPort);
                                       }
                                       else
                                          ret_val = _MAP_Open_Remote_Message_Access_Server_Port(RemoteDeviceAddress, RemoteServerPort);

                                       if(ret_val < 0)
                                       {
                                          ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;

                                          /* Error opening device, go   */
                                          /* ahead and delete the entry */
                                          /* that was added.            */
                                          if((MAPMEntryInfoPtr = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfoPtr->TrackingID)) != NULL)
                                          {
                                             CleanupMAPMEntryInfo(MAPMEntryInfoPtr);

                                             FreeMAPMEntryInfoEntryMemory(MAPMEntryInfoPtr);
                                          }
                                       }
                                       else
                                       {
                                          /* Note the MAP ID.           */
                                          MAPMEntryInfoPtr->MAPID = (unsigned int)ret_val;

                                          /* Flag success.              */
                                          ret_val                = 0;
                                       }
                                    }
                                 }
                              }
                              else
                              {
                                 /* Move the state to the connecting    */
                                 /* Waiting state.                      */
                                 MAPMEntryInfoPtr->ConnectionState = csConnectingWaiting;

                                 if((BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS) && (MAXIMUM_MAP_PORT_OPEN_DELAY_RETRY))
                                 {
                                    /* Port is NOT disconnected, go     */
                                    /* ahead and start a timer so that  */
                                    /* we can continue to check for the */
                                    /* Port Disconnection.              */
                                    ret_val = TMR_StartTimer((void *)MAPMEntryInfoPtr->TrackingID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                                    /* If the timer was started, go     */
                                    /* ahead and note the Timer ID.     */
                                    if(ret_val > 0)
                                    {
                                       MAPMEntryInfoPtr->ConnectionTimerID = (unsigned int)ret_val;

                                       ret_val                             = 0;
                                    }
                                    else
                                       ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
                                 }
                                 else
                                    ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_ALREADY_CONNECTED;
                              }

                              /* Next, determine if the caller has      */
                              /* requested a blocking open.             */
                              if((!ret_val) && (ConnectionStatus))
                              {
                                 /* Blocking open, go ahead and wait for*/
                                 /* the event.                          */

                                 /* Note the Tracking ID.               */
                                 TrackingID      = MAPMEntryInfoPtr->TrackingID;

                                 /* Note the Open Event.                */
                                 ConnectionEvent = MAPMEntryInfoPtr->ConnectionEvent;

                                 /* Release the lock because we are     */
                                 /* finished with it.                   */
                                 DEVM_ReleaseLock();

                                 BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                                 /* Re-acquire the Lock.                */
                                 if(DEVM_AcquireLock())
                                 {
                                    if((MAPMEntryInfoPtr = SearchMAPMEntryInfoByTrackingID(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), TrackingID)) != FALSE)
                                    {
                                       /* Note the connection status.   */
                                       *ConnectionStatus = MAPMEntryInfoPtr->ConnectionStatus;

                                       BTPS_CloseEvent(MAPMEntryInfoPtr->ConnectionEvent);

                                       /* Flag that the Connection Event*/
                                       /* is no longer valid.           */
                                       MAPMEntryInfoPtr->ConnectionEvent = NULL;

                                       /* Flag success to the caller.   */
                                       ret_val = 0;
                                    }
                                    else
                                       ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
                                 }
                                 else
                                    ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                              }
                              else
                              {
                                 /* If there was an error, go ahead and */
                                 /* delete the entry that was added.    */
                                 if(ret_val)
                                 {
                                    if((MAPMEntryInfoPtr = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfo.TrackingID)) != NULL)
                                    {
                                       CleanupMAPMEntryInfo(MAPMEntryInfoPtr);

                                       FreeMAPMEntryInfoEntryMemory(MAPMEntryInfoPtr);
                                    }
                                 }
                              }
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;

                           /* If an error occurred, go ahead and delete */
                           /* the Connection Information that was added.*/
                           if(ret_val)
                           {
                              if((MAPMEntryInfoPtr = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfo.TrackingID)) != NULL)
                              {
                                 CleanupMAPMEntryInfo(MAPMEntryInfoPtr);

                                 FreeMAPMEntryInfoEntryMemory(MAPMEntryInfoPtr);
                              }
                           }
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                     }
                     else
                     {
                        if(MAPMEntryInfoPtr->ConnectionState == csConnected)
                           ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_ALREADY_CONNECTED;
                        else
                           ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_CONNECTION_IN_PROGRESS;
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

               /* Release the Lock because we are finished with it.     */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
                  DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Disconnect(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                ret_val;
   Boolean_t          Notification;
   Boolean_t          Server;
   Boolean_t          PerformDisconnect;
   unsigned int       ServerPort;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Go ahead and note if this is a notification abort or a   */
            /* normal MAP abort.                                        */
            if((ConnectionType == mctNotificationServer) || (ConnectionType == mctNotificationClient))
               Notification = TRUE;
            else
               Notification = FALSE;

            if((ConnectionType == mctNotificationServer) || (ConnectionType == mctMessageAccessServer))
               Server = TRUE;
            else
               Server = FALSE;

            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), RemoteDeviceAddress, InstanceID, Server)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Connection: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Next, go ahead and close the connection.           */
                  if((Server) && (MAPMEntryInfo->ConnectionState == csIdle))
                     ret_val = 0;
                  else
                  {
                     if(Server)
                     {
                        CleanupMAPMEntryInfo(MAPMEntryInfo);

                        MAPMEntryInfo->ConnectionState = csIdle;

                        ret_val = _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                     }
                     else
                     {
                        /* Not if this is a Message Access Client and we*/
                        /* need to check to see if we should cleanup the*/
                        /* local Notification Server state.             */
                        if(Notification == FALSE)
                        {
                           /* If we have enabled (or are pending an     */
                           /* enable for notifications) on this MCE     */
                           /* connection handle the disconnect.         */
                           if(MAPMEntryInfo->Flags & (MAPM_ENTRY_INFO_FLAGS_NOTIFICATIONS_ENABLED | MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION))
                              HandleNotificationDisconnection(MAPMEntryInfo, &NotificationInfo);
                        }

                        switch(MAPMEntryInfo->ConnectionState)
                        {
                           case csAuthorizing:
                           case csAuthenticating:
                           case csEncrypting:
                              /* Should not occur.                      */
                              PerformDisconnect = FALSE;
                              break;
                           case csConnectingWaiting:
                              if(MAPMEntryInfo->ConnectionTimerID)
                                 TMR_StopTimer(MAPMEntryInfo->ConnectionTimerID);

                              PerformDisconnect = FALSE;
                              break;
                           case csConnectingDevice:
                              PerformDisconnect = FALSE;
                              break;
                           default:
                           case csConnecting:
                           case csConnected:
                              PerformDisconnect = TRUE;
                              break;
                        }

                        if(PerformDisconnect)
                        {
                           /* Nothing really to do other than to        */
                           /* disconnect the device.                    */
                           ret_val = _MAP_Close_Connection(MAPMEntryInfo->MAPID);
                        }
                        else
                           ret_val = 0;

                        /* If this is a client, we need to go ahead and */
                        /* delete the entry.                            */
                        if(!ret_val)
                        {
                           /* Note the Port Number before we delete the */
                           /* MAP Entry (we will use it below after we  */
                           /* free the entry).                          */
                           ServerPort = MAPMEntryInfo->PortNumber;

                           if((MAPMEntryInfo = DeleteMAPMEntryInfoEntry(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), MAPMEntryInfo->TrackingID)) != NULL)
                           {
                              /* All finished, free any memory that was */
                              /* allocated for the server.              */
                              CleanupMAPMEntryInfo(MAPMEntryInfo);

                              FreeMAPMEntryInfoEntryMemory(MAPMEntryInfo);
                           }

                           /* Go ahead and give the port some time to   */
                           /* disconnect (since it was initiated        */
                           /* locally).                                 */
                           SPPM_WaitForPortDisconnection(ServerPort, FALSE, RemoteDeviceAddress, MAXIMUM_MAP_PORT_DELAY_TIMEOUT_MS);
                        }
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Abort(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                ret_val;
   Boolean_t          Notification;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Go ahead and note if this is a notification abort or a   */
            /* normal MAP abort.                                        */
            if((ConnectionType == mctNotificationServer) || (ConnectionType == mctNotificationClient))
               Notification = TRUE;
            else
               Notification = FALSE;

            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(Notification?(&MAPMEntryInfoList_Notification):(&MAPMEntryInfoList), RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Abort: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is an on-going   */
                  /* operation.                                         */
                  if((MAPMEntryInfo->CurrentOperation != coNone) && (!(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)))
                  {
                     /* Operation in progress, go ahead and send the    */
                     /* Abort.                                          */
                     if((ret_val = _MAP_Abort_Request(MAPMEntryInfo->MAPID)) == 0)
                        MAPMEntryInfo->Flags |= MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT;
                  }
                  else
                  {
                     if(MAPMEntryInfo->Flags & MAPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                        ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_ABORT_OPERATION_IN_PROGRESS;
                     else
                        ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NO_OPERATION_IN_PROGRESS;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Query_Current_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int BufferSize, char *Buffer)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!BufferSize) || ((BufferSize) && (Buffer))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Folder: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* If a buffer was specified, go ahead and initialize */
                  /* it to a known value.                               */
                  if(BufferSize)
                     Buffer[0] = '\0';

                  if(MAPMEntryInfo->CurrentPath)
                  {
                     /* Current Folder is present, go ahead and copy as */
                     /* much as we have space for.                      */
                     ret_val = BTPS_StringLength(MAPMEntryInfo->CurrentPath);

                     if(BufferSize)
                     {
                        /* Buffer specified, see how much data we can   */
                        /* copy into the buffer.                        */
                        if((unsigned int)ret_val > BufferSize)
                           ret_val = (int)BufferSize;

                        /* Make sure the return buffer is NULL          */
                        /* terminated.                                  */
                        Buffer[BufferSize - 1] = '\0';

                        if(BufferSize - 1)
                           BTPS_MemCopy(Buffer, MAPMEntryInfo->CurrentPath, (BufferSize - 1));
                     }
                  }
                  else
                     ret_val = 0;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Enable_Notifications(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Enabled)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Enable Notifications: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* Handle the Notification Enable/Disable Request. */
                     if((Enabled == FALSE) || (!(ret_val = HandleNotificationEnableRequest(MAPMEntryInfo, &NotificationInfo))))
                     {
                        /* No operation in progress, go ahead and       */
                        /* enable/disable notifications.                */
                        if((ret_val = _MAP_Set_Notification_Registration_Request(MAPMEntryInfo->MAPID, Enabled)) == 0)
                        {
                           if(Enabled)
                              MAPMEntryInfo->Flags |= (unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION;
                           else
                              MAPMEntryInfo->Flags &= ~((unsigned long)MAPM_ENTRY_INFO_FLAGS_PENDING_ENABLE_NOTIFICATION);

                           MAPMEntryInfo->CurrentOperation = coEnableNotifications;
                        }
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Set_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Set_Folder_Option_t PathOption, char *FolderName)
{
   int                ret_val;
   char              *PendingPath;
   Word_t            *_FolderName;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      /* * NOTE * Here are the options to test for PathOption:          */
      /*             - sfRoot - FolderName ignored                      */
      /*             - sfDown - FolderName required                     */
      /*             - sfUp   - FolderName optional                     */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((PathOption == sfRoot) || ((PathOption == sfDown) && (FolderName) && (BTPS_StringLength(FolderName))) || ((PathOption == sfUp) && ((!FolderName) || ((FolderName) && BTPS_StringLength(FolderName))))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Path: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     /* Go ahead and convert the folder name from UTF-8 */
                     /* to Unicode.                                     */
                     if(PathOption == sfRoot)
                        _FolderName = NULL;
                     else
                     {
                        if(BTPS_StringLength(FolderName))
                           _FolderName = ConvertUTF8ToUnicode(FolderName);
                        else
                           _FolderName = NULL;
                     }

                     /* Go ahead and attempt to build the Pending Path. */
                     if(BuildPendingFolder(PathOption, _FolderName, MAPMEntryInfo->CurrentPath, &PendingPath))
                     {
                        /* Free any current Pending Set Path Information*/
                        /* (should already be clear).                   */
                        if(MAPMEntryInfo->PendingPath)
                        {
                           BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                           MAPMEntryInfo->PendingPath = NULL;
                        }

                        /* Set the new Pending Path.                    */
                        MAPMEntryInfo->PendingPath = PendingPath;

                        if((ret_val = _MAP_Set_Folder_Request(MAPMEntryInfo->MAPID, PathOption, _FolderName)) == 0)
                           MAPMEntryInfo->CurrentOperation = coSetFolder;
                        else
                        {
                           /* Error free any resources that were        */
                           /* allocated.                                */
                           if(MAPMEntryInfo->PendingPath)
                           {
                              BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                              MAPMEntryInfo->PendingPath = NULL;
                           }
                        }

                        /* Free any memory that was allocated to hold   */
                        /* the Unicode Folder Name.                     */
                        if(_FolderName)
                           BTPS_FreeMemory(_FolderName);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_PATH;

                     /* Free memory that was allocated for the converted*/
                     /* Folder Name.                                    */
                     if(_FolderName)
                        BTPS_FreeMemory(_FolderName);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Set_Folder_Absolute(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Absolute Path: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* Check to see if we can simply set the path to   */
                     /* the root.                                       */
                     if((!FolderName) || ((FolderName) && (!BTPS_StringLength(FolderName))))
                        ret_val = MAPM_Set_Folder(RemoteDeviceAddress, InstanceID, sfRoot, NULL);
                     else
                     {
                        /* We need to set a non-NULL absolute path.     */

                        /* Free any current Pending Set Path Information*/
                        /* (should already be clear).                   */
                        if(MAPMEntryInfo->PendingPath)
                        {
                           BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                           MAPMEntryInfo->PendingPath = NULL;
                        }

                        /* Next, attempt to store the path we are trying*/
                        /* to set to the context.                       */
                        /* * NOTE * We will strip any potential leading */
                        /*          delimiter character ('/').          */
                        if((FolderName) && (BTPS_StringLength(FolderName)) && ((FolderName[0] != '/') || ((FolderName[0] == '/') && (FolderName[1] != '\0'))))
                        {
                           if((MAPMEntryInfo->PendingPath = BTPS_AllocateMemory(BTPS_StringLength(FolderName) + 1)) != NULL)
                           {
                              if(FolderName[0] == '/')
                                 BTPS_StringCopy(MAPMEntryInfo->PendingPath, &(FolderName[1]));
                              else
                                 BTPS_StringCopy(MAPMEntryInfo->PendingPath, FolderName);

                              ret_val = 0;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }
                        else
                           ret_val = 0;

                        /* Pending Path Folder Name stored, go ahead and*/
                        /* attempt to set the current folder to root (to*/
                        /* start the process).                          */
                        if(!ret_val)
                        {
                           if((ret_val = _MAP_Set_Folder_Request(MAPMEntryInfo->MAPID, sfRoot, NULL)) == 0)
                           {
                              /* Flag that we are setting an absolute   */
                              /* path (as opposed to simply setting the */
                              /* path).                                 */
                              MAPMEntryInfo->CurrentOperation  = coSetFolderAbsolute;

                              /* Flag that we are currently setting the */
                              /* Root Path.                             */
                              MAPMEntryInfo->PendingPathOffset = -1;
                           }
                           else
                           {
                              /* Error free any resources that were     */
                              /* allocated.                             */
                              if(MAPMEntryInfo->PendingPath)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->PendingPath);

                                 MAPMEntryInfo->PendingPath = NULL;
                              }
                           }
                        }
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Get_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Folder Listing: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Get Folder Listing command.       */
                     if((ret_val = _MAP_Get_Folder_Listing_Request(MAPMEntryInfo->MAPID, MaxListCount, ListStartOffset)) == 0)
                        MAPMEntryInfo->CurrentOperation = coGetFolderListing;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Get_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Folder Listing Size: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Get Folder Listing Size command.  */
                     if((ret_val = _MAP_Get_Folder_Listing_Request(MAPMEntryInfo->MAPID, 0, 0)) == 0)
                        MAPMEntryInfo->CurrentOperation = coGetFolderListingSize;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Get_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo)
{
   int                ret_val;
   Word_t            *_FolderName;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Message Listing: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     /* Initialize success.                             */
                     ret_val = 0;
                     if((!FolderName) || ((FolderName) && (!BTPS_StringLength(FolderName))))
                        _FolderName = NULL;
                     else
                     {
                        if((_FolderName = ConvertUTF8ToUnicode(FolderName)) == NULL)
                           ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_FOLDER_NAME;
                     }

                     /* Folder Name converted, go ahead and attempt to  */
                     /* Get the Message Listing.                        */
                     if(!ret_val)
                     {
                        if((ret_val = _MAP_Get_Message_Listing_Request(MAPMEntryInfo->MAPID, _FolderName, MaxListCount, ListStartOffset, ListingInfo)) == 0)
                           MAPMEntryInfo->CurrentOperation = coGetMessageListing;
                     }

                     /* Free any memory that was allocated for the      */
                     /* Folder Name.                                    */
                     if(_FolderName)
                        BTPS_FreeMemory(_FolderName);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Get_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, MAP_Message_Listing_Info_t *ListingInfo)
{
   int                ret_val;
   Word_t            *_FolderName;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Message Listing Size: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     /* Initialize success.                             */
                     ret_val = 0;
                     if((!FolderName) || ((FolderName) && (!BTPS_StringLength(FolderName))))
                        _FolderName = NULL;
                     else
                     {
                        if((_FolderName = ConvertUTF8ToUnicode(FolderName)) == NULL)
                           ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_FOLDER_NAME;
                     }

                     /* Folder Name converted, go ahead and attempt to  */
                     /* Get the Message Listing Size.                   */
                     if(!ret_val)
                     {
                        if((ret_val = _MAP_Get_Message_Listing_Request(MAPMEntryInfo->MAPID, _FolderName, 0, 0, ListingInfo)) == 0)
                           MAPMEntryInfo->CurrentOperation = coGetMessageListingSize;
                     }

                     /* Free any memory that was allocated for the      */
                     /* Folder Name.                                    */
                     if(_FolderName)
                        BTPS_FreeMemory(_FolderName);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Get Message Request to the */
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  The           */
   /* MessageHandle is a 16 byte NULL terminated string containing      */
   /* Unicode hexadecimal characters which identifies the message.  The */
   /* Attachment parameter indicates whether any attachments to the     */
   /* message should be included in the response.  The CharSet and      */
   /* FractionalType parameters specify the format of the response.     */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
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
int BTPSAPI MAPM_Get_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (MessageHandle) && (BTPS_StringLength(MessageHandle) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Get Message: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Get Message command.              */
                     if((ret_val = _MAP_Get_Message_Request(MAPMEntryInfo->MAPID, MessageHandle, Attachment, CharSet, FractionalType)) == 0)
                        MAPMEntryInfo->CurrentOperation = coGetMessage;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if((!MessageHandle) || ((MessageHandle) && ((!BTPS_StringLength(MessageHandle) || (BTPS_StringLength(MessageHandle) > MAP_MESSAGE_HANDLE_LENGTH)))))
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_MESSAGE_HANDLE;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Set_Message_Status(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (MessageHandle) && (BTPS_StringLength(MessageHandle)) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Message Status: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Set Message Status command.       */
                     if((ret_val = _MAP_Set_Message_Status_Request(MAPMEntryInfo->MAPID, MessageHandle, StatusIndicator, StatusValue)) == 0)
                        MAPMEntryInfo->CurrentOperation = coSetMessageStatus;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if((!MessageHandle) || ((MessageHandle) && ((!BTPS_StringLength(MessageHandle)) || (BTPS_StringLength(MessageHandle) > MAP_MESSAGE_HANDLE_LENGTH))))
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_MESSAGE_HANDLE;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Push_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                ret_val;
   Word_t            *_FolderName;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!DataLength) || ((DataLength) && (DataBuffer))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Push Message: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coPushMessage))
                  {
                     /* No operation in progress, go ahead and convert  */
                     /* the Folder Name from UTF-8 to Unicode.          */

                     /* Initialize success.                             */
                     ret_val = 0;
                     if((!FolderName) || ((FolderName) && (!BTPS_StringLength(FolderName))))
                        _FolderName = NULL;
                     else
                     {
                        if((_FolderName = ConvertUTF8ToUnicode(FolderName)) == NULL)
                           ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_FOLDER_NAME;
                     }

                     /* Folder Name converted, go ahead and attempt to  */
                     /* Push the Message.                               */
                     if(!ret_val)
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        if((MAPMEntryInfo->DataBufferSize = DataLength) != 0)
                        {
                           /* Free any current data we have buffered    */
                           /* (should be none).                         */
                           if(MAPMEntryInfo->DataBuffer)
                           {
                              BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                              MAPMEntryInfo->DataBuffer = NULL;
                           }

                           /* Go ahead and allocate the buffer (we will */
                           /* not copy it yet, but we will allocate it  */
                           /* so that we don't get an error *AFTER* we  */
                           /* send the first part of the data.          */
                           if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(DataLength)) == NULL)
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }

                        /* Flag that we have not sent any data at this  */
                        /* point.                                       */
                        MAPMEntryInfo->DataBufferSent = 0;

                        if(!ret_val)
                        {
                           if((ret_val = _MAP_Push_Message_Request(MAPMEntryInfo->MAPID, _FolderName, Transparent, Retry, CharSet, DataLength, DataBuffer, &(MAPMEntryInfo->DataBufferSent), Final)) == 0)
                           {
                              /* Go ahead and clear out the current     */
                              /* Message Handle (it needs to be cached  */
                              /* for responses).                        */
                              if(MAPMEntryInfo->CurrentOperation == coNone)
                                 BTPS_MemInitialize(MAPMEntryInfo->DataMessageHandle, 0, sizeof(MAPMEntryInfo->DataMessageHandle));

                              /* Flag that a Push Message Operation is  */
                              /* in progress.                           */
                              MAPMEntryInfo->CurrentOperation = coPushMessage;

                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if(MAPMEntryInfo->DataBufferSent != DataLength)
                              {
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, DataBuffer, DataLength);

                                 /* Note the Final status.              */
                                 MAPMEntryInfo->DataFinal = Final;
                              }
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((ret_val) || (MAPMEntryInfo->DataBufferSent == DataLength))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }

                     /* Free any memory that was allocated for the      */
                     /* Folder Name.                                    */
                     if(_FolderName)
                        BTPS_FreeMemory(_FolderName);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Update_Inbox(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Update Inbox: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if(MAPMEntryInfo->CurrentOperation == coNone)
                  {
                     /* No operation in progress, go ahead and attempt  */
                     /* to submit the Update Inbox command.             */
                     if((ret_val = _MAP_Update_Inbox_Request(MAPMEntryInfo->MAPID)) == 0)
                        MAPMEntryInfo->CurrentOperation = coUpdateInbox;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Message Access Server (MSE) Functions.                            */

   /* The following function generates a MAP Set Notification           */
   /* Registration Response to the specified remote MAP Client.  The    */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode Parameter is the OBEX Response   */
   /* Code to send with the response.  This function returns zero if    */
   /* successful and a negative return error code if there was an error.*/
int BTPSAPI MAPM_Enable_Notifications_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Enable Confirmation: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coEnableNotifications)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Set Notification Confirmation  */
                        /* response.                                    */
                        if((ret_val = _MAP_Set_Notification_Registration_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Set Folder Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
int BTPSAPI MAPM_Set_Folder_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Folder Confirmation: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coSetFolder)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Set Folder Response command.   */
                        if((ret_val = _MAP_Set_Folder_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                        {
                           /* If the Set Folder confirmation was        */
                           /* successful, we need to go ahead and note  */
                           /* the new Folder (this is stored in the     */
                           /* PendingPath member.                       */
                           if(ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                           {
                              if(MAPMEntryInfo->CurrentPath)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->CurrentPath);

                                 MAPMEntryInfo->CurrentPath = NULL;
                              }

                              MAPMEntryInfo->CurrentPath = MAPMEntryInfo->PendingPath;

                              MAPMEntryInfo->PendingPath = NULL;
                           }

                           MAPMEntryInfo->CurrentOperation = coNone;
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Send_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, unsigned int FolderListingLength, Byte_t *FolderListing, Boolean_t Final)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!FolderListingLength) || ((FolderListingLength) && (FolderListing))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Folder Listing: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that the correct on-going   */
                  /* operation is in progress.                          */
                  if(MAPMEntryInfo->CurrentOperation == coGetFolderListing)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        /* * NOTE * There is no reason to worry about   */
                        /*          sending any data (or backing any    */
                        /*          data up if this is not a successful */
                        /*          response).                          */
                        if(ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                        {
                           /* Check to see if we need to map the final  */
                           /* bit into a continue (the lack of it).     */
                           /* * NOTE * This is required because there is*/
                           /*          No Final flag for responses (it  */
                           /*          is inherant with either an OK or */
                           /*          or CONTINUE being sent as the    */
                           /*          code).                           */
                           if(!Final)
                              ResponseCode = MAP_OBEX_RESPONSE_CONTINUE;

                           if((MAPMEntryInfo->DataBufferSize = FolderListingLength) != 0)
                           {
                              /* Free any current data we have buffered */
                              /* (should be none).                      */
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }

                              /* Go ahead and allocate the buffer (we   */
                              /* will not copy it yet, but we will      */
                              /* allocate it so that we don't get an    */
                              /* error *AFTER* we send the first part of*/
                              /* the data.                              */
                              if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(FolderListingLength)) == NULL)
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                              else
                                 ret_val = 0;
                           }
                           else
                              ret_val = 0;

                           /* Flag that we have not sent any data at    */
                           /* this point.                               */
                           MAPMEntryInfo->DataBufferSent = 0;
                        }
                        else
                        {
                           /* There is no reason to send any data       */
                           /* because this is an error response.        */
                           FolderListingLength = 0;
                           FolderListing       = NULL;

                           ret_val             = 0;
                        }

                        if(!ret_val)
                        {
                           if((ret_val = _MAP_Get_Folder_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, NULL, FolderListingLength, FolderListing, &(MAPMEntryInfo->DataBufferSent))) == 0)
                           {
                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if((ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent != FolderListingLength))
                              {
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, FolderListing, FolderListingLength);

                                 /* Note the Final status.              */
                                 MAPMEntryInfo->DataFinal = Final;
                              }
                              else
                                 MAPMEntryInfo->CurrentOperation = coNone;
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((ret_val) || ((ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent == FolderListingLength)))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a Folder Listing Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The Size of the size of the listing to return.  This   */
   /* function returns zero if successful and a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI MAPM_Send_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t FolderCount)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Folder Listing Size: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coGetFolderListingSize)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Get Folder Listing Response    */
                        /* command.                                     */
                        if((ret_val = _MAP_Get_Folder_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, &FolderCount, 0, NULL, NULL)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Send_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime, unsigned int MessageListingLength, Byte_t *MessageListing, Boolean_t Final)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (CurrentTime) && ((!MessageListingLength) || ((MessageListingLength) && (MessageListing))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Message Listing: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that the correct on-going   */
                  /* operation is in progress.                          */
                  if(MAPMEntryInfo->CurrentOperation == coGetMessageListing)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        /* * NOTE * There is no reason to worry about   */
                        /*          sending any data (or backing any    */
                        /*          data up if this is not a successful */
                        /*          response).                          */
                        if(ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                        {
                           /* Check to see if we need to map the final  */
                           /* bit into a continue (the lack of it).     */
                           /* * NOTE * This is required because there is*/
                           /*          No Final flag for responses (it  */
                           /*          is inherant with either an OK or */
                           /*          or CONTINUE being sent as the    */
                           /*          code).                           */
                           if(!Final)
                              ResponseCode = MAP_OBEX_RESPONSE_CONTINUE;

                           if((MAPMEntryInfo->DataBufferSize = MessageListingLength) != 0)
                           {
                              /* Free any current data we have buffered */
                              /* (should be none).                      */
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }

                              /* Go ahead and allocate the buffer (we   */
                              /* will not copy it yet, but we will      */
                              /* allocate it so that we don't get an    */
                              /* error *AFTER* we send the first part of*/
                              /* the data.                              */
                              if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(MessageListingLength)) == NULL)
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                              else
                                 ret_val = 0;
                           }
                           else
                              ret_val = 0;

                           /* Flag that we have not sent any data at    */
                           /* this point.                               */
                           MAPMEntryInfo->DataBufferSent = 0;
                        }
                        else
                        {
                           /* There is no reason to send any data       */
                           /* because this is an error response.        */
                           MessageListingLength = 0;
                           MessageListing       = NULL;

                           ret_val              = 0;
                        }

                        if(!ret_val)
                        {
                           if((ret_val = _MAP_Get_Message_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, &MessageCount, NewMessage, CurrentTime, MessageListingLength, MessageListing, &(MAPMEntryInfo->DataBufferSent))) == 0)
                           {
                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if((ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent != MessageListingLength))
                              {
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, MessageListing, MessageListingLength);

                                 /* Note the Final status.              */
                                 MAPMEntryInfo->DataFinal = Final;
                              }
                              else
                                 MAPMEntryInfo->CurrentOperation = coNone;
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((ret_val) || ((ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent == MessageListingLength)))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Message Listing Size       */
   /* Response to the specified remote MAP Client.  The                 */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode parameter is the OBEX Response   */
   /* Code to send with the response.  The Size parameter is the size of*/
   /* the message listing to return.  This function returns zero if     */
   /* successful and a negative return error code if there was an error.*/
int BTPSAPI MAPM_Send_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (CurrentTime))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Message Listing Size: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coGetMessageListingSize)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Get Message Listing Response   */
                        /* command.                                     */
                        if((ret_val = _MAP_Get_Message_Listing_Response(MAPMEntryInfo->MAPID, ResponseCode, &MessageCount, NewMessage, CurrentTime, 0, NULL, NULL)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Send_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, MAP_Fractional_Type_t FractionalType, unsigned int MessageLength, Byte_t *Message, Boolean_t Final)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!MessageLength) || ((MessageLength) && (Message))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Message: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that the correct on-going   */
                  /* operation is in progress.                          */
                  if(MAPMEntryInfo->CurrentOperation == coGetMessage)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Determine if we need to back up the data we  */
                        /* are sending.                                 */
                        /* * NOTE * There is no reason to worry about   */
                        /*          sending any data (or backing any    */
                        /*          data up if this is not a successful */
                        /*          response).                          */
                        if(ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
                        {
                           /* Check to see if we need to map the final  */
                           /* bit into a continue (the lack of it).     */
                           /* * NOTE * This is required because there is*/
                           /*          No Final flag for responses (it  */
                           /*          is inherant with either an OK or */
                           /*          or CONTINUE being sent as the    */
                           /*          code).                           */
                           if(!Final)
                              ResponseCode = MAP_OBEX_RESPONSE_CONTINUE;

                           if((MAPMEntryInfo->DataBufferSize = MessageLength) != 0)
                           {
                              /* Free any current data we have buffered */
                              /* (should be none).                      */
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }

                              /* Go ahead and allocate the buffer (we   */
                              /* will not copy it yet, but we will      */
                              /* allocate it so that we don't get an    */
                              /* error *AFTER* we send the first part of*/
                              /* the data.                              */
                              if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(MessageLength)) == NULL)
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                              else
                                 ret_val = 0;
                           }
                           else
                              ret_val = 0;

                           /* Flag that we have not sent any data at    */
                           /* this point.                               */
                           MAPMEntryInfo->DataBufferSent = 0;
                        }
                        else
                        {
                           /* There is no reason to send any data       */
                           /* because this is an error response.        */
                           MessageLength = 0;
                           Message       = NULL;

                           ret_val       = 0;
                        }

                        if(!ret_val)
                        {
                           if((ret_val = _MAP_Get_Message_Response(MAPMEntryInfo->MAPID, ResponseCode, FractionalType, MessageLength, Message, &(MAPMEntryInfo->DataBufferSent))) == 0)
                           {
                              /* Copy any remaining data into the buffer*/
                              /* for future operations.                 */
                              if((ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent != MessageLength))
                              {
                                 BTPS_MemCopy(MAPMEntryInfo->DataBuffer, Message, MessageLength);

                                 /* Note the Final status.              */
                                 MAPMEntryInfo->DataFinal = Final;
                              }
                              else
                                 MAPMEntryInfo->CurrentOperation = coNone;
                           }

                           /* If there was an error or we sent all of   */
                           /* the data, then we need to free any buffer */
                           /* that was allocated.                       */
                           if((ret_val) || ((ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MAPMEntryInfo->DataBufferSent == MessageLength)))
                           {
                              if(MAPMEntryInfo->DataBuffer)
                              {
                                 BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                                 MAPMEntryInfo->DataBuffer = NULL;
                              }
                           }
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Set Message Status Response*/
   /* to the specified remote MAP Client.  The RemoteDeviceAddress is   */
   /* the address of the remote client.  The InstanceID parameter       */
   /* specifies which server instance on the local device to use.  The  */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
int BTPSAPI MAPM_Set_Message_Status_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Set Message Status Confirmation: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coSetMessageStatus)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Set Message Status Response    */
                        /* command.                                     */
                        if((ret_val = _MAP_Set_Message_Status_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Push Message Response to   */
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The message handle is the handle for the client to     */
   /* refer to the message.  This function returns zero if successful   */
   /* and a negative return error code if there was an error.           */
int BTPSAPI MAPM_Push_Message_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, char *MessageHandle)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((ResponseStatusCode != MAPM_RESPONSE_STATUS_CODE_SUCCESS) || ((ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS) && (MessageHandle) && (BTPS_StringLength(MessageHandle)) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Push Message Confirmation: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coPushMessage)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Send Push Message Response     */
                        /* command.                                     */
                        if((ret_val = _MAP_Push_Message_Response(MAPMEntryInfo->MAPID, ResponseCode, MessageHandle)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(ResponseStatusCode == MAPM_RESPONSE_STATUS_CODE_SUCCESS)
         {
            if((!MessageHandle) || ((MessageHandle) && ((BTPS_StringLength(MessageHandle)) || (BTPS_StringLength(MessageHandle) > MAP_MESSAGE_HANDLE_LENGTH))))
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_MESSAGE_HANDLE;
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Update Inbox Response to   */
   /* the specified remote MAP Client. The RemoteDeviceAddress is the   */
   /* address of the remote client. The InstanceID parameter specifies  */
   /* which server instance on the local device to use. The ResponseCode*/
   /* parameter is the OBEX Response Code to send with the response.    */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
int BTPSAPI MAPM_Update_Inbox_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                ret_val;
   Byte_t             ResponseCode;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList, RemoteDeviceAddress, InstanceID, TRUE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Update Inbox Confirmation: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is the correct   */
                  /* on-going operation.                                */
                  if(MAPMEntryInfo->CurrentOperation == coUpdateInbox)
                  {
                     if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &ResponseCode))
                     {
                        /* Operation in progress, go ahead and attempt  */
                        /* to submit the Update Inbox Response command. */
                        if((ret_val = _MAP_Update_Inbox_Response(MAPMEntryInfo->MAPID, ResponseCode)) == 0)
                           MAPMEntryInfo->CurrentOperation = coNone;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Send_Notification(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int DataLength, Byte_t *EventData, Boolean_t Final)
{
   int                ret_val;
   MAPM_Entry_Info_t *MAPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Message Access Manager has been     */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify the input parameters appear to be semi-valid.           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!DataLength) || ((DataLength) && (EventData))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, attempt to determine if the Instance ID specified  */
            /* (with connection type and remote device address) is      */
            /* valid.                                                   */
            if((MAPMEntryInfo = SearchMAPMEntryInfoByConnection(&MAPMEntryInfoList_Notification, RemoteDeviceAddress, InstanceID, FALSE)) != NULL)
            {
               /* Verify that the server process is the 'owner' of this */
               /* server/client connection.                             */
               if(MAPMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Send Notification: 0x%08X (%d)\n", InstanceID, InstanceID));

                  /* Entry found, make sure that there is NO on-going   */
                  /* operation.                                         */
                  if((MAPMEntryInfo->CurrentOperation == coNone) || (MAPMEntryInfo->CurrentOperation == coSendEvent))
                  {
                     /* Initialize the return value to indicate success.*/
                     ret_val = 0;

                     /* Determine if we need to back up the data we are */
                     /* sending.                                        */
                     if((MAPMEntryInfo->DataBufferSize = DataLength) != 0)
                     {
                        /* Free any current data we have buffered       */
                        /* (should be none).                            */
                        if(MAPMEntryInfo->DataBuffer)
                        {
                           BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                           MAPMEntryInfo->DataBuffer = NULL;
                        }

                        /* Go ahead and allocate the buffer (we will not*/
                        /* copy it yet, but we will allocate it so that */
                        /* we don't get an error *AFTER* we send the    */
                        /* first part of the data.                      */
                        if((MAPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(DataLength)) == NULL)
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                     }

                     /* Flag that we have not sent any data at this     */
                     /* point.                                          */
                     MAPMEntryInfo->DataBufferSent = 0;

                     if(!ret_val)
                     {
                        if((ret_val = _MAP_Send_Event_Request(MAPMEntryInfo->MAPID, DataLength, EventData, &(MAPMEntryInfo->DataBufferSent), Final)) == 0)
                        {
                           /* Flag that a Push Message Operation is in  */
                           /* progress.                                 */
                           MAPMEntryInfo->CurrentOperation = coSendEvent;

                           /* Copy any remaining data into the buffer   */
                           /* for future operations.                    */
                           if(MAPMEntryInfo->DataBufferSent != DataLength)
                           {
                              BTPS_MemCopy(MAPMEntryInfo->DataBuffer, EventData, DataLength);

                              /* Note the Final status.                 */
                              MAPMEntryInfo->DataFinal = Final;
                           }
                        }

                        /* If there was an error or we sent all of the  */
                        /* data, then we need to free any buffer that   */
                        /* was allocated.                               */
                        if((ret_val) || (MAPMEntryInfo->DataBufferSent == DataLength))
                        {
                           if(MAPMEntryInfo->DataBuffer)
                           {
                              BTPS_FreeMemory(MAPMEntryInfo->DataBuffer);

                              MAPMEntryInfo->DataBuffer = NULL;
                           }
                        }
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
               }
               else
                  ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

