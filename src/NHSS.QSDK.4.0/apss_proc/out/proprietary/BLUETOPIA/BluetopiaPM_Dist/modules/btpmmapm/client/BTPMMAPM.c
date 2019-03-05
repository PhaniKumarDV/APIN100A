/*****< btpmmapm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMAPM - MAP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/24/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMMAPM.h"            /* BTPM MAP Manager Prototypes/Constants.    */
#include "MAPMAPI.h"             /* MAP Manager Prototypes/Constants.         */
#include "MAPMMSG.h"             /* BTPM MAP Manager Message Formats.         */
#include "MAPMGR.h"              /* MAP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagMAP_Entry_Info_t
{
   unsigned int                 InstanceID;
   BD_ADDR_t                    BD_ADDR;
   MAPM_Connection_Type_t       ConnectionType;
   unsigned int                 ConnectionStatus;
   Event_t                      ConnectionEvent;
   unsigned long                Flags;
   MAPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagMAP_Entry_Info_t *NextMAPEntryInfoPtr;
} MAP_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* MAP_Entry_Info_t structure to denote various state information.   */
#define MAP_ENTRY_INFO_FLAGS_CONNECTION_OPENING             0x80000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   MAPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} CallbackInfo_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t MAPManagerMutex;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the MAP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static MAP_Entry_Info_t *MAPEntryInfoList;

   /* Variable which holds a pointer to the first element of the MAP    */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static MAP_Entry_Info_t *MAPEntryInfoDataList;

static MAP_Entry_Info_t *AddMAPEntryInfoEntry(MAP_Entry_Info_t **ListHead, MAP_Entry_Info_t *EntryToAdd);
static MAP_Entry_Info_t *SearchMAPEntryInfo(MAP_Entry_Info_t **ListHead, MAPM_Connection_Type_t ConnectionType, BD_ADDR_t *RemoteDeviceAddress, unsigned int InstanceID);
static MAP_Entry_Info_t *DeleteMAPEntryInfoEntry(MAP_Entry_Info_t **ListHead, MAPM_Connection_Type_t ConnectionType, BD_ADDR_t *RemoteDeviceAddress, unsigned int InstanceID);
static void FreeMAPEntryInfoEntryMemory(MAP_Entry_Info_t *EntryToFree);
static void FreeMAPEntryInfoList(MAP_Entry_Info_t **ListHead);

static void ProcessConnectionRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
static void ProcessDeviceConnectedEvent(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
static void ProcessDeviceDisconnectedEvent(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
static void ProcessConnectionStatusEvent(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ConnectionStatus);
static void ProcessEnableNotificationsResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);
static void ProcessGetFolderListingResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Boolean_t Final, unsigned int FolderListingLength, Byte_t *FolderListing);
static void ProcessGetFolderListingSizeResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t NumberOfFolders);
static void ProcessGetMessageListingResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *MSETime, Boolean_t Final, unsigned int MessageListingLength, Byte_t *MessageListing);
static void ProcessGetMessageListingSizeResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime);
static void ProcessGetMessageResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, MAP_Fractional_Type_t FractionalType, Boolean_t Final, unsigned int MessageDataLength, Byte_t *MessageData);
static void ProcessSetMessageStatusResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);
static void ProcessPushMessageResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, char *MessageHandle);
static void ProcessUpdateInboxResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);
static void ProcessSetFolderResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, unsigned int CurrentPathLength, char *CurrentPath);
static void ProcessNotificationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Final, unsigned int EventDataLength, Byte_t *EventDataBuffer);
static void ProcessEnableNotificationsIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Enabled);
static void ProcessGetFolderListingRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset);
static void ProcessGetFolderListingSizeRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
static void ProcessGetMessageListingRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo, unsigned int FilterRecipientLength, unsigned int FilterOriginatorLength, unsigned int FolderNameLength, Byte_t *VariableData);
static void ProcessGetMessageListingSizeRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Message_Listing_Info_t *ListingInfo, unsigned int FilterRecipientLength, unsigned int FilterOriginatorLength, unsigned int FolderNameLength, Byte_t *VariableData);
static void ProcessGetMessageRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType, char *MessageHandle);
static void ProcessSetMessageStatusRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue);
static void ProcessPushMessageRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int FolderNameLength, unsigned int MessageLength, Boolean_t Final, Byte_t *VariableData);
static void ProcessUpdateInboxRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);
static void ProcessSetFolderRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Set_Folder_Option_t PathOption, unsigned int FolderNameLength, unsigned int NewPathLength, char *VariableData);
static void ProcessNotificationConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_MAPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI MAPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            CallbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static MAP_Entry_Info_t *AddMAPEntryInfoEntry(MAP_Entry_Info_t **ListHead, MAP_Entry_Info_t *EntryToAdd)
{
   MAP_Entry_Info_t *AddedEntry = NULL;
   MAP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* OK, data seems semi-valid, let's allocate a new data structure */
      /* to add to the list.                                            */
      AddedEntry = (MAP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(MAP_Entry_Info_t));

      if(AddedEntry)
      {
         /* Copy All Data over.                                         */
         *AddedEntry                     = *EntryToAdd;

         /* Now Add it to the end of the list.                          */
         AddedEntry->NextMAPEntryInfoPtr = NULL;

         /* First, let's check to see if there are any elements already */
         /* present in the List that was passed in.                     */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Head Pointer was not NULL, so we will traverse the list  */
            /* until we reach the last element.                         */
            while(tmpEntry)
            {
               /* Check whether the entry was already added. If so, free*/
               /* the memory and abort the search.                      */
               switch(AddedEntry->ConnectionType)
               {
                  case mctNotificationServer:
                     if(tmpEntry->ConnectionType == AddedEntry->ConnectionType)
                     {
                        FreeMAPEntryInfoEntryMemory(AddedEntry);
                        AddedEntry = NULL;
                        tmpEntry   = NULL;
                     }
                     break;
                  case mctNotificationClient:
                     if((tmpEntry->ConnectionType == AddedEntry->ConnectionType) && (COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR)))
                     {
                        FreeMAPEntryInfoEntryMemory(AddedEntry);
                        AddedEntry = NULL;
                        tmpEntry   = NULL;
                     }
                     break;
                  case mctMessageAccessServer:
                     if((tmpEntry->ConnectionType == AddedEntry->ConnectionType) && (tmpEntry->InstanceID == AddedEntry->InstanceID))
                     {
                        FreeMAPEntryInfoEntryMemory(AddedEntry);
                        AddedEntry = NULL;
                        tmpEntry   = NULL;
                     }
                     break;
                  case mctMessageAccessClient:
                     if((tmpEntry->ConnectionType == AddedEntry->ConnectionType) && (tmpEntry->InstanceID == AddedEntry->InstanceID) && (COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR)))
                     {
                        FreeMAPEntryInfoEntryMemory(AddedEntry);
                        AddedEntry = NULL;
                        tmpEntry   = NULL;
                     }
                     break;
               }

               if((AddedEntry) && (tmpEntry))
               {
                  /* OK, we need to see if we are at the last element   */
                  /* of the List. If we are, we simply break out of     */
                  /* the list traversal because we know there are NO    */
                  /* duplicates AND we are at the end of the list.      */
                  if(tmpEntry->NextMAPEntryInfoPtr)
                     tmpEntry = tmpEntry->NextMAPEntryInfoPtr;
                  else
                     break;
               }
            }

            if((AddedEntry) && (tmpEntry))
            {
               /* Last element found, simply Add the entry.             */
               tmpEntry->NextMAPEntryInfoPtr = AddedEntry;
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
   /* specified connection. The way the search is performed is dependent*/
   /* upon the connection type:                                         */
   /*   - Access Server: InstanceID                                     */
   /*   - Access Client: RemoteDeviceAddress, InstanceID                */
   /*   - Notification Server: none (there can be only one)             */
   /*   - Notification Client: RemoteDeviceAddress                      */
   /* This function returns NULL if either the list head is invalid, the*/
   /* connection-specific search criteria is invalid, or the specified  */
   /* entry was NOT found.                                              */
static MAP_Entry_Info_t *SearchMAPEntryInfo(MAP_Entry_Info_t **ListHead, MAPM_Connection_Type_t ConnectionType, BD_ADDR_t *RemoteDeviceAddress, unsigned int InstanceID)
{
   MAP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", InstanceID));

   /* Check if ListHead appears valid.                                  */
   if(ListHead)
   {
      /* Search the list for the specified entry.                       */
      FoundEntry  = *ListHead;

      switch(ConnectionType)
      {
         case mctNotificationServer:
            while((FoundEntry) && (FoundEntry->ConnectionType != ConnectionType))
               FoundEntry = FoundEntry->NextMAPEntryInfoPtr;

            break;
         case mctNotificationClient:
            if((RemoteDeviceAddress) && (!COMPARE_NULL_BD_ADDR(*RemoteDeviceAddress)))
            {
               while((FoundEntry) && ((FoundEntry->ConnectionType != ConnectionType) || (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, *RemoteDeviceAddress))))
                  FoundEntry = FoundEntry->NextMAPEntryInfoPtr;
            }
            else
               FoundEntry = NULL;
            break;
         case mctMessageAccessServer:
            while((FoundEntry) && ((FoundEntry->ConnectionType != ConnectionType) || (FoundEntry->InstanceID != InstanceID)))
               FoundEntry = FoundEntry->NextMAPEntryInfoPtr;

            break;
         case mctMessageAccessClient:
            if((RemoteDeviceAddress) && (!COMPARE_NULL_BD_ADDR(*RemoteDeviceAddress)))
            {
               while((FoundEntry) && ((FoundEntry->ConnectionType != ConnectionType) || (FoundEntry->InstanceID != InstanceID) || (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, *RemoteDeviceAddress))))
                  FoundEntry = FoundEntry->NextMAPEntryInfoPtr;
            }
            else
               FoundEntry = NULL;
            break;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified MAP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the MAP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeMAPEntryInfoEntryMemory().                   */
static MAP_Entry_Info_t *DeleteMAPEntryInfoEntry(MAP_Entry_Info_t **ListHead, MAPM_Connection_Type_t ConnectionType, BD_ADDR_t *RemoteDeviceAddress, unsigned int InstanceID)
{
   MAP_Entry_Info_t *FoundEntry = NULL;
   MAP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      switch(ConnectionType)
      {
         case mctNotificationServer:
            while((FoundEntry) && (FoundEntry->ConnectionType != ConnectionType))
            {
               LastEntry  = FoundEntry;
               FoundEntry = FoundEntry->NextMAPEntryInfoPtr;
            }

            break;
         case mctNotificationClient:
            if((RemoteDeviceAddress) && (!COMPARE_NULL_BD_ADDR(*RemoteDeviceAddress)))
            {
               while((FoundEntry) && ((FoundEntry->ConnectionType != ConnectionType) || (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, *RemoteDeviceAddress))))
               {
                  LastEntry  = FoundEntry;
                  FoundEntry = FoundEntry->NextMAPEntryInfoPtr;
               }
            }
            else
               FoundEntry = NULL;
            break;
         case mctMessageAccessServer:
            while((FoundEntry) && ((FoundEntry->ConnectionType != ConnectionType) || (FoundEntry->InstanceID != InstanceID)))
            {
               LastEntry  = FoundEntry;
               FoundEntry = FoundEntry->NextMAPEntryInfoPtr;
            }

            break;
         case mctMessageAccessClient:
            if((RemoteDeviceAddress) && (!COMPARE_NULL_BD_ADDR(*RemoteDeviceAddress)))
            {
               while((FoundEntry) && ((FoundEntry->ConnectionType != ConnectionType) || (FoundEntry->InstanceID != InstanceID) || (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, *RemoteDeviceAddress))))
               {
                  LastEntry  = FoundEntry;
                  FoundEntry = FoundEntry->NextMAPEntryInfoPtr;
               }
            }
            else
               FoundEntry = NULL;
            break;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextMAPEntryInfoPtr = FoundEntry->NextMAPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextMAPEntryInfoPtr;

         FoundEntry->NextMAPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified MAP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeMAPEntryInfoEntryMemory(MAP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified MAP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeMAPEntryInfoList(MAP_Entry_Info_t **ListHead)
{
   MAP_Entry_Info_t *EntryToFree;
   MAP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextMAPEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeMAPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Connection Request asynchronous message.                          */
static void ProcessConnectionRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified Port.                        */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                        = metMAPIncomingConnectionRequest;
      EventData.EventLength                                                      = MAPM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;
      EventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.IncomingConnectionRequestEventData.InstanceID          = InstanceID;

      /* Note the Callback information.                                 */
      EventCallback                                                              = MAPEntryInfo->EventCallback;
      CallbackParameter                                                          = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Instance: %u\n", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Device   */
   /* Connected asynchronous message.                                   */
static void ProcessDeviceConnectedEvent(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified Port.                        */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                        = metMAPConnected;
      EventData.EventLength                                      = MAPM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.ConnectionType      = ConnectionType;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.ConnectedEventData.InstanceID          = InstanceID;

      /* Note the Callback information.                                 */
      EventCallback                                              = MAPEntryInfo->EventCallback;
      CallbackParameter                                          = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Instance: %u\n", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Device   */
   /* Disconnected asynchronous message.                                */
static void ProcessDeviceDisconnectedEvent(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

//XXX Need to check how Notification Server disconnections are
//XXX reported. Should they be matched to a single client connection
//XXX according to InstanceID or broadcast to all clients connected to
//XXX the same remote device?

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                           = metMAPDisconnected;
      EventData.EventLength                                         = MAPM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionType;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.DisconnectedEventData.InstanceID          = InstanceID;

      /* Note the Callback information.                                 */
      EventCallback                                                 = MAPEntryInfo->EventCallback;
      CallbackParameter                                             = MAPEntryInfo->CallbackParameter;

      /* Now process differently based upon client or server.           */
      if((ConnectionType == mctMessageAccessClient) || (ConnectionType == mctNotificationClient))
      {
         /* Client, go ahead and delete the Info Entry and free all     */
         /* resources.                                                  */
         if((MAPEntryInfo = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
         {
            /* Close any events that were allocated.                    */
            if(MAPEntryInfo->ConnectionEvent)
               BTPS_CloseEvent(MAPEntryInfo->ConnectionEvent);

            /* All finished with the memory so free the entry.          */
            FreeMAPEntryInfoEntryMemory(MAPEntryInfo);
         }
      }

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Connection Status asynchronous message.                           */
static void ProcessConnectionStatusEvent(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ConnectionStatus)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Check to see if we need to dispatch the event or set an        */
      /* internal event.                                                */
      if(MAPEntryInfo->Flags & MAP_ENTRY_INFO_FLAGS_CONNECTION_OPENING)
      {
         /* Note the connection status and notify any threads waiting on*/
         /* this connection.                                            */
         MAPEntryInfo->ConnectionStatus = ConnectionStatus;
         BTPS_SetEvent(MAPEntryInfo->ConnectionEvent);

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(MAPManagerMutex);
      }
      else
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                               = metMAPConnectionStatus;
         EventData.EventLength                                             = MAPM_CONNECTION_STATUS_EVENT_DATA_SIZE;
         EventData.EventData.ConnectionStatusEventData.ConnectionType      = ConnectionType;
         EventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData.EventData.ConnectionStatusEventData.InstanceID          = InstanceID;
         EventData.EventData.ConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

         /* Note the Callback information.                              */
         EventCallback                                                     = MAPEntryInfo->EventCallback;
         CallbackParameter                                                 = MAPEntryInfo->CallbackParameter;

         /* If the connection request failed then delete the connection */
         /* from the list.                                              */
         if(ConnectionStatus != MAPM_CONNECTION_STATUS_SUCCESS)
         {
            if((MAPEntryInfo = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
            {
               if(MAPEntryInfo->ConnectionEvent)
                  BTPS_CloseEvent(MAPEntryInfo->ConnectionEvent);

               FreeMAPEntryInfoEntryMemory(MAPEntryInfo);
            }
         }

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Enable   */
   /* Notifications Response asynchronous message.                      */
static void ProcessEnableNotificationsResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                          = metMAPEnableNotificationsResponse;
      EventData.EventLength                                                        = MAPM_ENABLE_NOTIFICATIONS_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.EnableNotificationsResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.EnableNotificationsResponseEventData.InstanceID          = InstanceID;
      EventData.EventData.EnableNotificationsResponseEventData.ResponseStatusCode  = ResponseStatusCode;

      /* Note the Callback information.                                 */
      EventCallback                                                                = MAPEntryInfo->EventCallback;
      CallbackParameter                                                            = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Folder Listing Response asynchronous message.                     */
static void ProcessGetFolderListingResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Boolean_t Final, unsigned int FolderListingLength, Byte_t *FolderListing)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                       = metMAPGetFolderListingResponse;
      EventData.EventLength                                                     = MAPM_GET_FOLDER_LISTING_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.GetFolderListingResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.GetFolderListingResponseEventData.InstanceID          = InstanceID;
      EventData.EventData.GetFolderListingResponseEventData.ResponseStatusCode  = ResponseStatusCode;
      EventData.EventData.GetFolderListingResponseEventData.Final               = Final;
      EventData.EventData.GetFolderListingResponseEventData.FolderListingLength = FolderListingLength;
      EventData.EventData.GetFolderListingResponseEventData.FolderListingData   = (FolderListingLength ? FolderListing : NULL);

      /* Note the Callback information.                                 */
      EventCallback                                                             = MAPEntryInfo->EventCallback;
      CallbackParameter                                                         = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Folder Listing Size Response asynchronous message.                */
static void ProcessGetFolderListingSizeResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t NumberOfFolders)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                           = metMAPGetFolderListingSizeResponse;
      EventData.EventLength                                                         = MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.GetFolderListingSizeResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.GetFolderListingSizeResponseEventData.InstanceID          = InstanceID;
      EventData.EventData.GetFolderListingSizeResponseEventData.ResponseStatusCode  = ResponseStatusCode;
      EventData.EventData.GetFolderListingSizeResponseEventData.NumberOfFolders     = NumberOfFolders;

      /* Note the Callback information.                                 */
      EventCallback                                                                 = MAPEntryInfo->EventCallback;
      CallbackParameter                                                             = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Message Listing Response asynchronous message.                    */
static void ProcessGetMessageListingResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *MSETime, Boolean_t Final, unsigned int MessageListingLength, Byte_t *MessageListing)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(MSETime)
   {
      /* First, attempt to find the specified connection entry.         */
      if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                                         = metMAPGetMessageListingResponse;
         EventData.EventLength                                                       = MAPM_GET_MESSAGE_LISTING_RESPONSE_EVENT_DATA_SIZE;
         EventData.EventData.GetMessageListingResponseEventData.RemoteDeviceAddress  = RemoteDeviceAddress;
         EventData.EventData.GetMessageListingResponseEventData.InstanceID           = InstanceID;
         EventData.EventData.GetMessageListingResponseEventData.ResponseStatusCode   = ResponseStatusCode;
         EventData.EventData.GetMessageListingResponseEventData.NewMessage           = NewMessage;
         EventData.EventData.GetMessageListingResponseEventData.MSETime              = *MSETime;
         EventData.EventData.GetMessageListingResponseEventData.NumberOfMessages     = MessageCount;
         EventData.EventData.GetMessageListingResponseEventData.Final                = Final;
         EventData.EventData.GetMessageListingResponseEventData.MessageListingLength = MessageListingLength;
         EventData.EventData.GetMessageListingResponseEventData.MessageListingData   = (MessageListingLength ? MessageListing : NULL);

         /* Note the Callback information.                              */
         EventCallback                                                               = MAPEntryInfo->EventCallback;
         CallbackParameter                                                           = MAPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(MAPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameter\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Message Listing Size Response asynchronous message.               */
static void ProcessGetMessageListingSizeResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(CurrentTime)
   {
      /* First, attempt to find the specified connection entry.         */
      if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                                            = metMAPGetMessageListingSizeResponse;
         EventData.EventLength                                                          = MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_EVENT_DATA_SIZE;
         EventData.EventData.GetMessageListingSizeResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData.EventData.GetMessageListingSizeResponseEventData.InstanceID          = InstanceID;
         EventData.EventData.GetMessageListingSizeResponseEventData.ResponseStatusCode  = ResponseStatusCode;
         EventData.EventData.GetMessageListingSizeResponseEventData.NewMessage          = NewMessage;
         EventData.EventData.GetMessageListingSizeResponseEventData.MSETime             = *CurrentTime;
         EventData.EventData.GetMessageListingSizeResponseEventData.NumberOfMessages    = MessageCount;

         /* Note the Callback information.                              */
         EventCallback                                                                  = MAPEntryInfo->EventCallback;
         CallbackParameter                                                              = MAPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(MAPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameter\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Message Response asynchronous message.                            */
static void ProcessGetMessageResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, MAP_Fractional_Type_t FractionalType, Boolean_t Final, unsigned int MessageDataLength, Byte_t *MessageData)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                 = metMAPGetMessageResponse;
      EventData.EventLength                                               = MAPM_GET_MESSAGE_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.GetMessageResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.GetMessageResponseEventData.InstanceID          = InstanceID;
      EventData.EventData.GetMessageResponseEventData.ResponseStatusCode  = ResponseStatusCode;
      EventData.EventData.GetMessageResponseEventData.FractionalType      = FractionalType;
      EventData.EventData.GetMessageResponseEventData.Final               = Final;
      EventData.EventData.GetMessageResponseEventData.MessageDataLength   = MessageDataLength;
      EventData.EventData.GetMessageResponseEventData.MessageData         = (MessageDataLength ? MessageData : NULL);

      /* Note the Callback information.                                 */
      EventCallback                                                       = MAPEntryInfo->EventCallback;
      CallbackParameter                                                   = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Set      */
   /* Message Status Response asynchronous message.                     */
static void ProcessSetMessageStatusResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                       = metMAPSetMessageStatusResponse;
      EventData.EventLength                                                     = MAPM_SET_MESSAGE_STATUS_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.SetMessageStatusResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.SetMessageStatusResponseEventData.InstanceID          = InstanceID;
      EventData.EventData.SetMessageStatusResponseEventData.ResponseStatusCode  = ResponseStatusCode;

      /* Note the Callback information.                                 */
      EventCallback                                                             = MAPEntryInfo->EventCallback;
      CallbackParameter                                                         = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Push     */
   /* Message Response asynchronous message.                            */
static void ProcessPushMessageResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, char *MessageHandle)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((MessageHandle) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
   {
      /* First, attempt to find the specified connection entry.         */
      if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                                  = metMAPPushMessageResponse;
         EventData.EventLength                                                = MAPM_PUSH_MESSAGE_RESPONSE_EVENT_DATA_SIZE;
         EventData.EventData.PushMessageResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData.EventData.PushMessageResponseEventData.InstanceID          = InstanceID;
         EventData.EventData.PushMessageResponseEventData.ResponseStatusCode  = ResponseStatusCode;

         BTPS_StringCopy(EventData.EventData.PushMessageResponseEventData.MessageHandle, MessageHandle);

         /* Note the Callback information.                              */
         EventCallback                                                        = MAPEntryInfo->EventCallback;
         CallbackParameter                                                    = MAPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(MAPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameter\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Update   */
   /* Inbox Response asynchronous message.                              */
static void ProcessUpdateInboxResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                  = metMAPUpdateInboxResponse;
      EventData.EventLength                                                = MAPM_UPDATE_INBOX_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.UpdateInboxResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.UpdateInboxResponseEventData.InstanceID          = InstanceID;
      EventData.EventData.UpdateInboxResponseEventData.ResponseStatusCode  = ResponseStatusCode;

      /* Note the Callback information.                                 */
      EventCallback                                                        = MAPEntryInfo->EventCallback;
      CallbackParameter                                                    = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Set      */
   /* Folder Response asynchronous message.                             */
static void ProcessSetFolderResponseEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, unsigned int CurrentPathLength, char *CurrentPath)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                = metMAPSetFolderResponse;
      EventData.EventLength                                              = MAPM_SET_FOLDER_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.SetFolderResponseEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.SetFolderResponseEventData.InstanceID          = InstanceID;
      EventData.EventData.SetFolderResponseEventData.ResponseStatusCode  = ResponseStatusCode;
      EventData.EventData.SetFolderResponseEventData.CurrentPath         = (CurrentPathLength ? CurrentPath : NULL);

      /* Note the Callback information.                                 */
      EventCallback                                                      = MAPEntryInfo->EventCallback;
      CallbackParameter                                                  = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Notification Indication asynchronous message.                     */
static void ProcessNotificationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Final, unsigned int EventDataLength, Byte_t *EventDataBuffer)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessClient, &RemoteDeviceAddress, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                     = metMAPNotificationIndication;
      EventData.EventLength                                                   = MAPM_NOTIFICATION_INDICATION_EVENT_DATA_SIZE;
      EventData.EventData.NotificationIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.NotificationIndicationEventData.InstanceID          = InstanceID;
      EventData.EventData.NotificationIndicationEventData.Final               = Final;
      EventData.EventData.NotificationIndicationEventData.EventReportLength   = EventDataLength;
      EventData.EventData.NotificationIndicationEventData.EventReportData     = (EventDataLength ? EventDataBuffer : NULL);

      /* Note the Callback information.                                 */
      EventCallback                                                           = MAPEntryInfo->EventCallback;
      CallbackParameter                                                       = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate connection for %02X%02X%02X%02X%02X%02X, Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Enable   */
   /* Notifications Indication asynchronous message.                    */
static void ProcessEnableNotificationsIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Enabled)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                            = metMAPEnableNotificationsIndication;
      EventData.EventLength                                                          = MAPM_ENABLE_NOTIFICATIONS_INDICATION_EVENT_DATA_SIZE;
      EventData.EventData.EnableNotificationsIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.EnableNotificationsIndicationEventData.InstanceID          = InstanceID;
      EventData.EventData.EnableNotificationsIndicationEventData.Enabled             = Enabled;

      /* Note the Callback information.                                 */
      EventCallback                                                                  = MAPEntryInfo->EventCallback;
      CallbackParameter                                                              = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Folder Listing Request asynchronous message.                      */
static void ProcessGetFolderListingRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                      = metMAPGetFolderListingRequest;
      EventData.EventLength                                                    = MAPM_GET_FOLDER_LISTING_REQUEST_EVENT_DATA_SIZE;
      EventData.EventData.GetFolderListingRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.GetFolderListingRequestEventData.InstanceID          = InstanceID;
      EventData.EventData.GetFolderListingRequestEventData.MaxListCount        = MaxListCount;
      EventData.EventData.GetFolderListingRequestEventData.ListStartOffset     = ListStartOffset;

      /* Note the Callback information.                                 */
      EventCallback                                                            = MAPEntryInfo->EventCallback;
      CallbackParameter                                                        = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Folder Listing Size Request asynchronous message.                 */
static void ProcessGetFolderListingSizeRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                          = metMAPGetFolderListingSizeRequest;
      EventData.EventLength                                                        = MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_EVENT_DATA_SIZE;
      EventData.EventData.GetFolderListingSizeRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.GetFolderListingSizeRequestEventData.InstanceID          = InstanceID;

      /* Note the Callback information.                                 */
      EventCallback                                                                = MAPEntryInfo->EventCallback;
      CallbackParameter                                                            = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Message Listing Request asynchronous message.                     */
static void ProcessGetMessageListingRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo, unsigned int FilterRecipientLength, unsigned int FilterOriginatorLength, unsigned int FolderNameLength, Byte_t *VariableData)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(ListingInfo)
   {
      /* First, attempt to find the specified connection entry.         */
      if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                                       = metMAPGetMessageListingRequest;
         EventData.EventLength                                                     = MAPM_GET_MESSAGE_LISTING_REQUEST_EVENT_DATA_SIZE;
         EventData.EventData.GetMessageListingRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData.EventData.GetMessageListingRequestEventData.InstanceID          = InstanceID;
         EventData.EventData.GetMessageListingRequestEventData.MaxListCount        = MaxListCount;
         EventData.EventData.GetMessageListingRequestEventData.ListStartOffset     = ListStartOffset;
         EventData.EventData.GetMessageListingRequestEventData.ListingInfo         = *ListingInfo;

         if((FilterRecipientLength) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_RECIPIENT_PRESENT))
         {
            EventData.EventData.GetMessageListingRequestEventData.ListingInfo.FilterRecipient                          = (char *)&VariableData[0];
            EventData.EventData.GetMessageListingRequestEventData.ListingInfo.FilterRecipient[FilterRecipientLength-1] = '\0';
         }
         else
            EventData.EventData.GetMessageListingRequestEventData.ListingInfo.FilterRecipient = NULL;

         if((FilterOriginatorLength) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_ORIGINATOR_PRESENT))
         {
            EventData.EventData.GetMessageListingRequestEventData.ListingInfo.FilterOriginator                           = (char *)&VariableData[FilterRecipientLength];
            EventData.EventData.GetMessageListingRequestEventData.ListingInfo.FilterOriginator[FilterOriginatorLength-1] = '\0';
         }
         else
            EventData.EventData.GetMessageListingRequestEventData.ListingInfo.FilterOriginator = NULL;

         if(FolderNameLength)
         {
            EventData.EventData.GetMessageListingRequestEventData.FolderName                     = (char *)&VariableData[FilterRecipientLength + FilterOriginatorLength];
            EventData.EventData.GetMessageListingRequestEventData.FolderName[FolderNameLength-1] = '\0';
         }
         else
            EventData.EventData.GetMessageListingRequestEventData.FolderName = NULL;

         /* Note the Callback information.                              */
         EventCallback     = MAPEntryInfo->EventCallback;
         CallbackParameter = MAPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(MAPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameter\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Message Listing Size Request asynchronous message.                */
static void ProcessGetMessageListingSizeRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Message_Listing_Info_t *ListingInfo, unsigned int FilterRecipientLength, unsigned int FilterOriginatorLength, unsigned int FolderNameLength, Byte_t *VariableData)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(ListingInfo)
   {
      /* First, attempt to find the specified connection entry.         */
      if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                                           = metMAPGetMessageListingSizeRequest;
         EventData.EventLength                                                         = MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_EVENT_DATA_SIZE;
         EventData.EventData.GetMessageListingSizeRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData.EventData.GetMessageListingSizeRequestEventData.InstanceID          = InstanceID;
         EventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo         = *ListingInfo;

         if((FilterRecipientLength) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_RECIPIENT_PRESENT))
         {
            EventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo.FilterRecipient                          = (char *)&VariableData[0];
            EventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo.FilterRecipient[FilterRecipientLength-1] = '\0';
         }
         else
            EventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo.FilterRecipient = NULL;

         if((FilterOriginatorLength) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_ORIGINATOR_PRESENT))
         {
            EventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo.FilterOriginator                           = (char *)&VariableData[FilterRecipientLength];
            EventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo.FilterOriginator[FilterOriginatorLength-1] = '\0';
         }
         else
            EventData.EventData.GetMessageListingSizeRequestEventData.ListingInfo.FilterOriginator = NULL;

         if(FolderNameLength)
         {
            EventData.EventData.GetMessageListingSizeRequestEventData.FolderName                     = (char *)&VariableData[FilterRecipientLength + FilterOriginatorLength];
            EventData.EventData.GetMessageListingSizeRequestEventData.FolderName[FolderNameLength-1] = '\0';
         }
         else
            EventData.EventData.GetMessageListingSizeRequestEventData.FolderName = NULL;

         /* Note the Callback information.                              */
         EventCallback     = MAPEntryInfo->EventCallback;
         CallbackParameter = MAPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(MAPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameter\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Message Request asynchronous message.                             */
static void ProcessGetMessageRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType, char *MessageHandle)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((MessageHandle) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
   {
      /* First, attempt to find the specified connection entry.         */
      if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                                = metMAPGetMessageRequest;
         EventData.EventLength                                              = MAPM_GET_MESSAGE_REQUEST_EVENT_DATA_SIZE;
         EventData.EventData.GetMessageRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData.EventData.GetMessageRequestEventData.InstanceID          = InstanceID;
         EventData.EventData.GetMessageRequestEventData.Attachment          = Attachment;
         EventData.EventData.GetMessageRequestEventData.CharSet             = CharSet;
         EventData.EventData.GetMessageRequestEventData.FractionalType      = FractionalType;

         BTPS_StringCopy(EventData.EventData.GetMessageRequestEventData.MessageHandle, MessageHandle);

         /* Note the Callback information.                              */
         EventCallback                                                      = MAPEntryInfo->EventCallback;
         CallbackParameter                                                  = MAPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(MAPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameter\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Set      */
   /* Message Status Request asynchronous message.                      */
static void ProcessSetMessageStatusRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((MessageHandle) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
   {
      /* First, attempt to find the specified connection entry.         */
      if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

         EventData.EventType                                                      = metMAPSetMessageStatusRequest;
         EventData.EventLength                                                    = MAPM_SET_MESSAGE_STATUS_REQUEST_EVENT_DATA_SIZE;
         EventData.EventData.SetMessageStatusRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData.EventData.SetMessageStatusRequestEventData.InstanceID          = InstanceID;
         EventData.EventData.SetMessageStatusRequestEventData.StatusIndicator     = StatusIndicator;
         EventData.EventData.SetMessageStatusRequestEventData.StatusValue         = StatusValue;

         BTPS_StringCopy(EventData.EventData.SetMessageStatusRequestEventData.MessageHandle, MessageHandle);

         /* Note the Callback information.                              */
         EventCallback                                                            = MAPEntryInfo->EventCallback;
         CallbackParameter                                                        = MAPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(MAPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(MAPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid parameter\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Push     */
   /* Message Request asynchronous message.                             */
static void ProcessPushMessageRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int FolderNameLength, unsigned int MessageLength, Boolean_t Final, Byte_t *VariableData)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                 = metMAPPushMessageRequest;
      EventData.EventLength                                               = MAPM_PUSH_MESSAGE_REQUEST_EVENT_DATA_SIZE;
      EventData.EventData.PushMessageRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.PushMessageRequestEventData.InstanceID          = InstanceID;
      EventData.EventData.PushMessageRequestEventData.Transparent         = Transparent;
      EventData.EventData.PushMessageRequestEventData.Retry               = Retry;
      EventData.EventData.PushMessageRequestEventData.CharSet             = CharSet;
      EventData.EventData.PushMessageRequestEventData.Final               = Final;
      EventData.EventData.PushMessageRequestEventData.MessageDataLength   = MessageLength;
      EventData.EventData.PushMessageRequestEventData.MessageData         = (MessageLength ? &VariableData[FolderNameLength] : NULL);

      if(FolderNameLength)
      {
         EventData.EventData.PushMessageRequestEventData.FolderName                     = (char *)&VariableData[0];
         EventData.EventData.PushMessageRequestEventData.FolderName[FolderNameLength-1] = '\0';
      }
      else
         EventData.EventData.PushMessageRequestEventData.FolderName = NULL;

      /* Note the Callback information.                                 */
      EventCallback                                                       = MAPEntryInfo->EventCallback;
      CallbackParameter                                                   = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Update   */
   /* Inbox Request asynchronous message.                               */
static void ProcessUpdateInboxRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                 = metMAPUpdateInboxRequest;
      EventData.EventLength                                               = MAPM_UPDATE_INBOX_REQUEST_EVENT_DATA_SIZE;
      EventData.EventData.UpdateInboxRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.UpdateInboxRequestEventData.InstanceID          = InstanceID;

      /* Note the Callback information.                                 */
      EventCallback                                                       = MAPEntryInfo->EventCallback;
      CallbackParameter                                                   = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Set      */
   /* Folder Request asynchronous message.                              */
static void ProcessSetFolderRequestEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Set_Folder_Option_t PathOption, unsigned int FolderNameLength, unsigned int NewPathLength, char *VariableData)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                               = metMAPSetFolderRequest;
      EventData.EventLength                                             = MAPM_SET_FOLDER_REQUEST_EVENT_DATA_SIZE;
      EventData.EventData.SetFolderRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.SetFolderRequestEventData.InstanceID          = InstanceID;
      EventData.EventData.SetFolderRequestEventData.PathOption          = PathOption;

      if(FolderNameLength)
      {
         EventData.EventData.SetFolderRequestEventData.FolderName                     = &VariableData[0];
         EventData.EventData.SetFolderRequestEventData.FolderName[FolderNameLength-1] = '\0';
      }
      else
         EventData.EventData.SetFolderRequestEventData.FolderName = NULL;

      if(NewPathLength)
      {
         EventData.EventData.SetFolderRequestEventData.NewPath                  = &VariableData[FolderNameLength];
         EventData.EventData.SetFolderRequestEventData.NewPath[NewPathLength-1] = '\0';
      }
      else
         EventData.EventData.SetFolderRequestEventData.NewPath = NULL;

      /* Note the Callback information.                                 */
      EventCallback                                                     = MAPEntryInfo->EventCallback;
      CallbackParameter                                                 = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Notification Confirmation asynchronous message.                   */
static void ProcessNotificationConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   void                  *CallbackParameter;
   MAPM_Event_Data_t      EventData;
   MAP_Entry_Info_t      *MAPEntryInfo;
   MAPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, attempt to find the specified connection entry.            */
   if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
   {
      /* Event needs to be dispatched. Go ahead and format the event.   */
      BTPS_MemInitialize(&EventData, 0, sizeof(MAPM_Event_Data_t));

      EventData.EventType                                                       = metMAPNotificationConfirmation;
      EventData.EventLength                                                     = MAPM_NOTIFICATION_CONFIRMATION_EVENT_DATA_SIZE;
      EventData.EventData.NotificationConfirmationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.NotificationConfirmationEventData.InstanceID          = InstanceID;
      EventData.EventData.NotificationConfirmationEventData.ResponseStatusCode  = ResponseStatusCode;

      /* Note the Callback information.                                 */
      EventCallback                                                             = MAPEntryInfo->EventCallback;
      CallbackParameter                                                         = MAPEntryInfo->CallbackParameter;

      /* Release the Mutex so we can dispatch the event.                */
      BTPS_ReleaseMutex(MAPManagerMutex);

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(MAPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate server for Instance %u", InstanceID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the MAP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessConnectionRequestEvent(((MAPM_Connection_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Connection_Request_Message_t *)Message)->InstanceID);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_DEVICE_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_DEVICE_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connected Event.                               */
               ProcessDeviceConnectedEvent(((MAPM_Device_Connected_Message_t *)Message)->ConnectionType, ((MAPM_Device_Connected_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Device_Connected_Message_t *)Message)->InstanceID);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_DEVICE_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Disconnected Event.                            */
               ProcessDeviceDisconnectedEvent(((MAPM_Device_Disconnected_Message_t *)Message)->ConnectionType, ((MAPM_Device_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Device_Disconnected_Message_t *)Message)->InstanceID);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connection Status Event.                              */
               ProcessConnectionStatusEvent(((MAPM_Connection_Status_Message_t *)Message)->ConnectionType, ((MAPM_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Connection_Status_Message_t *)Message)->InstanceID, ((MAPM_Connection_Status_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Notifications Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_ENABLE_NOTIFICATIONS_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Enable Notifications Response Event.                  */
               ProcessEnableNotificationsResponseEvent(((MAPM_Enable_Notifications_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Enable_Notifications_Response_Message_t *)Message)->InstanceID, ((MAPM_Enable_Notifications_Response_Message_t *)Message)->ResponseStatusCode);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_RESPONSE_MESSAGE_SIZE(((MAPM_Get_Folder_Listing_Response_Message_t *)Message)->FolderListingLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Folder Listing Response Event.                        */
               ProcessGetFolderListingResponseEvent(((MAPM_Get_Folder_Listing_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Folder_Listing_Response_Message_t *)Message)->InstanceID, ((MAPM_Get_Folder_Listing_Response_Message_t *)Message)->ResponseStatusCode, ((MAPM_Get_Folder_Listing_Response_Message_t *)Message)->Final, ((MAPM_Get_Folder_Listing_Response_Message_t *)Message)->FolderListingLength, ((MAPM_Get_Folder_Listing_Response_Message_t *)Message)->FolderListing);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Size Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Folder Listing Size Response Event.                   */
               ProcessGetFolderListingSizeResponseEvent(((MAPM_Get_Folder_Listing_Size_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Folder_Listing_Size_Response_Message_t *)Message)->InstanceID, ((MAPM_Get_Folder_Listing_Size_Response_Message_t *)Message)->ResponseStatusCode, ((MAPM_Get_Folder_Listing_Size_Response_Message_t *)Message)->NumberOfFolders);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_RESPONSE_MESSAGE_SIZE(((MAPM_Get_Message_Listing_Response_Message_t *)Message)->MessageListingLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Message Listing Response Event.                       */
               ProcessGetMessageListingResponseEvent(((MAPM_Get_Message_Listing_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Message_Listing_Response_Message_t *)Message)->InstanceID, ((MAPM_Get_Message_Listing_Response_Message_t *)Message)->ResponseStatusCode, ((MAPM_Get_Message_Listing_Response_Message_t *)Message)->MessageCount, ((MAPM_Get_Message_Listing_Response_Message_t *)Message)->NewMessage, &(((MAPM_Get_Message_Listing_Response_Message_t *)Message)->MSETime), ((MAPM_Get_Message_Listing_Response_Message_t *)Message)->Final, ((MAPM_Get_Message_Listing_Response_Message_t *)Message)->MessageListingLength, ((MAPM_Get_Message_Listing_Response_Message_t *)Message)->MessageListing);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Size Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Message Listing Size Response Event.                  */
               ProcessGetMessageListingSizeResponseEvent(((MAPM_Get_Message_Listing_Size_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Message_Listing_Size_Response_Message_t *)Message)->InstanceID, ((MAPM_Get_Message_Listing_Size_Response_Message_t *)Message)->ResponseStatusCode, ((MAPM_Get_Message_Listing_Size_Response_Message_t *)Message)->MessageCount, ((MAPM_Get_Message_Listing_Size_Response_Message_t *)Message)->NewMessage, &(((MAPM_Get_Message_Listing_Size_Response_Message_t *)Message)->CurrentTime));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_RESPONSE_MESSAGE_SIZE(((MAPM_Get_Message_Response_Message_t *)Message)->MessageDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Message Response Event.                               */
               ProcessGetMessageResponseEvent(((MAPM_Get_Message_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Message_Response_Message_t *)Message)->InstanceID, ((MAPM_Get_Message_Response_Message_t *)Message)->ResponseStatusCode, ((MAPM_Get_Message_Response_Message_t *)Message)->FractionalType, ((MAPM_Get_Message_Response_Message_t *)Message)->Final, ((MAPM_Get_Message_Response_Message_t *)Message)->MessageDataLength, ((MAPM_Get_Message_Response_Message_t *)Message)->MessageData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Message Status Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_MESSAGE_STATUS_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Message Status Response Event.                        */
               ProcessSetMessageStatusResponseEvent(((MAPM_Set_Message_Status_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Set_Message_Status_Response_Message_t *)Message)->InstanceID, ((MAPM_Set_Message_Status_Response_Message_t *)Message)->ResponseStatusCode);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Message Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Push*/
               /* Message Response Event.                               */
               ProcessPushMessageResponseEvent(((MAPM_Push_Message_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Push_Message_Response_Message_t *)Message)->InstanceID, ((MAPM_Push_Message_Response_Message_t *)Message)->ResponseStatusCode, ((MAPM_Push_Message_Response_Message_t *)Message)->MessageHandle);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Inbox Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_UPDATE_INBOX_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Update Inbox Response Event.                          */
               ProcessUpdateInboxResponseEvent(((MAPM_Update_Inbox_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Update_Inbox_Response_Message_t *)Message)->InstanceID, ((MAPM_Update_Inbox_Response_Message_t *)Message)->ResponseStatusCode);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_FOLDER_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Folder Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_RESPONSE_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_RESPONSE_MESSAGE_SIZE(((MAPM_Set_Folder_Response_Message_t *)Message)->CurrentPathLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Folder Response Event.                                */
               ProcessSetFolderResponseEvent(((MAPM_Set_Folder_Response_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Set_Folder_Response_Message_t *)Message)->InstanceID, ((MAPM_Set_Folder_Response_Message_t *)Message)->ResponseStatusCode, ((MAPM_Set_Folder_Response_Message_t *)Message)->CurrentPathLength, ((MAPM_Set_Folder_Response_Message_t *)Message)->CurrentPath);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_NOTIFICATION_INDICATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_NOTIFICATION_INDICATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_NOTIFICATION_INDICATION_MESSAGE_SIZE(((MAPM_Notification_Indication_Message_t *)Message)->EventDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Notification Indication Event.                        */
               ProcessNotificationIndicationEvent(((MAPM_Notification_Indication_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Notification_Indication_Message_t *)Message)->InstanceID, ((MAPM_Notification_Indication_Message_t *)Message)->Final, ((MAPM_Notification_Indication_Message_t *)Message)->EventDataLength, ((MAPM_Notification_Indication_Message_t *)Message)->EventData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Notifications Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_ENABLE_NOTIFICATIONS_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Enable Notifications Indication Event.                */
               ProcessEnableNotificationsIndicationEvent(((MAPM_Enable_Notifications_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Enable_Notifications_Request_Message_t *)Message)->InstanceID, ((MAPM_Enable_Notifications_Request_Message_t *)Message)->Enable);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Folder Listing Request Event.                         */
               ProcessGetFolderListingRequestEvent(((MAPM_Get_Folder_Listing_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Folder_Listing_Request_Message_t *)Message)->InstanceID, ((MAPM_Get_Folder_Listing_Request_Message_t *)Message)->MaxListCount, ((MAPM_Get_Folder_Listing_Request_Message_t *)Message)->ListStartOffset);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Folder Listing Size Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Folder Listing Size Request Event.                    */
               ProcessGetFolderListingSizeRequestEvent(((MAPM_Get_Folder_Listing_Size_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Folder_Listing_Size_Request_Message_t *)Message)->InstanceID);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Request Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_REQUEST_MESSAGE_SIZE(0,0,0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_REQUEST_MESSAGE_SIZE(((MAPM_Get_Message_Listing_Request_Message_t *)Message)->FilterRecipientLength, ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->FilterOriginatorLength, ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->FolderNameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Message Listing Request Event.                        */
               ProcessGetMessageListingRequestEvent(((MAPM_Get_Message_Listing_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->InstanceID, ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->MaxListCount,((MAPM_Get_Message_Listing_Request_Message_t *)Message)->ListStartOffset, &(((MAPM_Get_Message_Listing_Request_Message_t *)Message)->ListingInfo), ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->FilterRecipientLength, ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->FilterOriginatorLength, ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->FolderNameLength, ((MAPM_Get_Message_Listing_Request_Message_t *)Message)->VariableData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Listing Size Request Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_MESSAGE_SIZE(0,0,0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_MESSAGE_SIZE(((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->FilterRecipientLength, ((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->FilterOriginatorLength, ((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->FolderNameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Message Listing Size Request Event.                   */
               ProcessGetMessageListingSizeRequestEvent(((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->InstanceID, &(((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->ListingInfo), ((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->FilterRecipientLength, ((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->FilterOriginatorLength, ((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->FolderNameLength, ((MAPM_Get_Message_Listing_Size_Request_Message_t *)Message)->VariableData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_GET_MESSAGE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Message Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Message Request Event.                                */
               ProcessGetMessageRequestEvent(((MAPM_Get_Message_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Get_Message_Request_Message_t *)Message)->InstanceID, ((MAPM_Get_Message_Request_Message_t *)Message)->Attachment, ((MAPM_Get_Message_Request_Message_t *)Message)->CharSet, ((MAPM_Get_Message_Request_Message_t *)Message)->FractionalType, ((MAPM_Get_Message_Request_Message_t *)Message)->MessageHandle);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Message Status Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_MESSAGE_STATUS_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Message Status Request Event.                         */
               ProcessSetMessageStatusRequestEvent(((MAPM_Set_Message_Status_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Set_Message_Status_Request_Message_t *)Message)->InstanceID, ((MAPM_Set_Message_Status_Request_Message_t *)Message)->MessageHandle, ((MAPM_Set_Message_Status_Request_Message_t *)Message)->StatusIndicator, ((MAPM_Set_Message_Status_Request_Message_t *)Message)->StatusValue);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Message Request Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_REQUEST_MESSAGE_SIZE(0,0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_REQUEST_MESSAGE_SIZE(((MAPM_Push_Message_Request_Message_t *)Message)->FolderNameLength, ((MAPM_Push_Message_Request_Message_t *)Message)->MessageLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Push*/
               /* Message Request Event.                                */
               ProcessPushMessageRequestEvent(((MAPM_Push_Message_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Push_Message_Request_Message_t *)Message)->InstanceID, ((MAPM_Push_Message_Request_Message_t *)Message)->Transparent, ((MAPM_Push_Message_Request_Message_t *)Message)->Retry, ((MAPM_Push_Message_Request_Message_t *)Message)->CharSet, ((MAPM_Push_Message_Request_Message_t *)Message)->FolderNameLength, ((MAPM_Push_Message_Request_Message_t *)Message)->MessageLength, ((MAPM_Push_Message_Request_Message_t *)Message)->Final, ((MAPM_Push_Message_Request_Message_t *)Message)->VariableData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Inbox Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_UPDATE_INBOX_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Update Inbox Request Event.                           */
               ProcessUpdateInboxRequestEvent(((MAPM_Update_Inbox_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Update_Inbox_Request_Message_t *)Message)->InstanceID);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_SET_FOLDER_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Folder Request Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_REQUEST_MESSAGE_SIZE(0,0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_REQUEST_MESSAGE_SIZE(((MAPM_Set_Folder_Request_Message_t *)Message)->FolderNameLength, ((MAPM_Set_Folder_Request_Message_t *)Message)->NewPathLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Folder Request Event.                                 */
               ProcessSetFolderRequestEvent(((MAPM_Set_Folder_Request_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Set_Folder_Request_Message_t *)Message)->InstanceID, ((MAPM_Set_Folder_Request_Message_t *)Message)->PathOption, ((MAPM_Set_Folder_Request_Message_t *)Message)->FolderNameLength, ((MAPM_Set_Folder_Request_Message_t *)Message)->NewPathLength, ((MAPM_Set_Folder_Request_Message_t *)Message)->VariableData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case MAPM_MESSAGE_FUNCTION_NOTIFICATION_CONFIRMATION:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= MAPM_NOTIFICATION_CONFIRMATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Notification Confirmation Event.                      */
               ProcessNotificationConfirmationEvent(((MAPM_Notification_Confirmation_Message_t *)Message)->RemoteDeviceAddress, ((MAPM_Notification_Confirmation_Message_t *)Message)->InstanceID, ((MAPM_Notification_Confirmation_Message_t *)Message)->ResponseStatusCode);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
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

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(MAPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process MAP Manager Asynchronous Events.            */
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
         /* Attempt to wait for access to the MAP state information.    */
         if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Process the Message.                                     */
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Note we do not have to release the Mutex because         */
            /* ProcessReceivedMessage() is documented that it will be   */
            /* called with the Mutex being held and it will release the */
            /* Mutex when it is finished with it.                       */
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   MAP_Entry_Info_t *MAPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the MAP state information.    */
         if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            MAPEntryInfo = MAPEntryInfoList;

            while(MAPEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if((MAPEntryInfo->Flags & MAP_ENTRY_INFO_FLAGS_CONNECTION_OPENING) && (MAPEntryInfo->ConnectionEvent))
               {
                  MAPEntryInfo->ConnectionStatus = MAPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(MAPEntryInfo->ConnectionEvent);
               }

               MAPEntryInfo = MAPEntryInfo->NextMAPEntryInfoPtr;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(MAPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all MAP Manager Messages.   */
static void BTPSAPI MAPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
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
               /* Dispatch to the main handler that the server has      */
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

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager MAP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI MAPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing MAP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((MAPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process MAP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER, MAPManagerGroupHandler, NULL))
            {
               /* Initialize the actual MAP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the MAP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _MAPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and flag that this module is initialized. */
                  Initialized       = TRUE;
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result < 0)
         {
            if(MAPManagerMutex)
               BTPS_CloseMutex(MAPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER);

            /* Flag that none of the resources are allocated.           */
            MAPManagerMutex = NULL;
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

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we inform the MAP Manager Implementation that  */
            /* we are shutting down.                                    */
            _MAPM_Cleanup();

            BTPS_CloseMutex(MAPManagerMutex);

            /* Make sure that the MAP Entry Information List is empty.  */
            FreeMAPEntryInfoList(&MAPEntryInfoList);

            /* Make sure that the MAP Entry Data Information List is    */
            /* empty.                                                   */
            FreeMAPEntryInfoList(&MAPEntryInfoDataList);

            /* Flag that the resources are no longer allocated.         */
            MAPManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
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
   MAP_Entry_Info_t *MAPEntryInfo;
   MAP_Entry_Info_t *tmpMAPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* MAP Entries and set any events that have synchronous  */
               /* operations pending.                                   */
               MAPEntryInfo = MAPEntryInfoList;

               while(MAPEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((MAPEntryInfo->Flags & MAP_ENTRY_INFO_FLAGS_CONNECTION_OPENING) && (MAPEntryInfo->ConnectionEvent))
                  {
                     MAPEntryInfo->ConnectionStatus = MAPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(MAPEntryInfo->ConnectionEvent);

                     MAPEntryInfo = MAPEntryInfo->NextMAPEntryInfoPtr;
                  }
                  else
                  {
                     tmpMAPEntryInfo = MAPEntryInfo;
                     MAPEntryInfo    = MAPEntryInfo->NextMAPEntryInfoPtr;

                     /* This is a server callback.                      */
                     if((tmpMAPEntryInfo = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, tmpMAPEntryInfo->ConnectionType, NULL, tmpMAPEntryInfo->InstanceID)) != NULL)
                        FreeMAPEntryInfoEntryMemory(tmpMAPEntryInfo);
                  }
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(MAPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Message Access Profile (MAP) Manager (MAPM) Common/Connection     */
   /* Functions.                                                        */

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming MAP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A MAP Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI MAPM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Connection_Request_Response(RemoteDeviceAddress, InstanceID, Accept);
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
   int               ret_val;
   MAP_Entry_Info_t  MAPEntryInfo;
   MAP_Entry_Info_t *MAPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the MAP state information.    */
         if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device Powered up, go ahead and store the information */
               /* into our list.                                        */
               BTPS_MemInitialize(&MAPEntryInfo, 0, sizeof(MAP_Entry_Info_t));

               MAPEntryInfo.InstanceID        = InstanceID;
               MAPEntryInfo.ConnectionType    = mctMessageAccessServer;
               MAPEntryInfo.ConnectionEvent   = BTPS_CreateEvent(FALSE);
               MAPEntryInfo.EventCallback     = CallbackFunction;
               MAPEntryInfo.CallbackParameter = CallbackParameter;

               /* Make sure that all events were able to be created.    */
               if(MAPEntryInfo.ConnectionEvent)
               {
                  if((MAPEntryInfoPtr = AddMAPEntryInfoEntry(&MAPEntryInfoList, &MAPEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Register Server Instance: %u, Port: %u, Flags: 0x%08lX\n", InstanceID, ServerPort, ServerFlags));

                     /* Next, attempt to register the Server.           */
                     if((ret_val = _MAPM_Register_Server(ServerPort, ServerFlags, InstanceID, SupportedMessageTypes)) < 0)
                     {
                        /* Error opening port, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((MAPEntryInfoPtr = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
                        {
                           BTPS_CloseEvent(MAPEntryInfoPtr->ConnectionEvent);

                           FreeMAPEntryInfoEntryMemory(MAPEntryInfoPtr);
                        }
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(MAPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
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
   int               ret_val;
   MAP_Entry_Info_t *MAPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the MAP state information.       */
      if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Next, attempt to delete the MAP Entry from the List.        */
         if((MAPEntryInfo = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Delete Instance: %u\n", InstanceID));

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(MAPManagerMutex);

            /* Next, go ahead and Un-Register the Server Port.          */
            _MAPM_Un_Register_Server(InstanceID);

            /* All finished, free any memory that was allocated for the */
            /* port.                                                    */
            FreeMAPEntryInfoEntryMemory(MAPEntryInfo);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(MAPManagerMutex);
         }
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
   long ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the MAP state information.       */
      if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Next, retrieve the MAP Entry from the MAP Entry List.       */
         if(SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID))
         {
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Register Service Record for Instance: %u\n", InstanceID));

            /* The only thing left to do is to submit the request.      */
            ret_val = _MAPM_Register_Service_Record(InstanceID, ServiceName);
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(MAPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
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
   long ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the MAP state information.       */
      if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Next, retrieve the MAP Entry from the MAP Entry List.       */
         if(SearchMAPEntryInfo(&MAPEntryInfoList, mctMessageAccessServer, NULL, InstanceID))
         {
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to UnRegister Service Record for Instance: %u\n", InstanceID));

            /* The only thing left to do is to submit the request.      */
            ret_val = _MAPM_Un_Register_Service_Record(InstanceID);
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(MAPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %ld\n", ret_val));

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
   long ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the MAP state information.       */
      if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* The only thing left to do is to submit the request.         */
         ret_val = _MAPM_Parse_Remote_Message_Access_Services(RemoteDeviceAddress, ServiceInfo);

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(MAPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %ld\n", ret_val));

   /* Finally return the result to the caller.                          */
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
   int               ret_val;
   Event_t           ConnectionEvent;
   MAP_Entry_Info_t  MAPEntryInfo;
   MAP_Entry_Info_t *MAPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(((ConnectionType == mctMessageAccessClient) || (ConnectionType == mctNotificationClient)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (((CallbackFunction) && (!ConnectionStatus)) || ((!CallbackFunction) && (ConnectionStatus))))
      {
         /* Attempt to wait for access to the MAP state information.    */
         if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* MAP Entry Info list.                                  */
               BTPS_MemInitialize(&MAPEntryInfo, 0, sizeof(MAP_Entry_Info_t));

//XXX If connecting to a notification server, we should search for the local Access Server entry associated
//XXX with the InstanceID so we have something concrete to which we can issue the connection status event.
               MAPEntryInfo.InstanceID        = InstanceID;
               MAPEntryInfo.BD_ADDR           = RemoteDeviceAddress;
               MAPEntryInfo.ConnectionType    = ConnectionType;
               MAPEntryInfo.ConnectionEvent   = BTPS_CreateEvent(FALSE);
               MAPEntryInfo.EventCallback     = CallbackFunction;
               MAPEntryInfo.CallbackParameter = CallbackParameter;

               if(MAPEntryInfo.ConnectionEvent)
               {
                  if((MAPEntryInfoPtr = AddMAPEntryInfoEntry(&MAPEntryInfoList, &MAPEntryInfo)) != NULL)
                  {
                     if(ConnectionType == mctMessageAccessClient)
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote MAP Server Instance %u on Port %u, 0x%08lX\n", InstanceID, RemoteServerPort, ConnectionFlags));
                     else
                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Notification Server on Port %u for local MAP Server Instance %u, 0x%08lX\n", RemoteServerPort, InstanceID, ConnectionFlags));

                     /* Next, attempt to open the remote port.          */
                     if((ret_val = _MAPM_Connect_Remote_Device(ConnectionType, RemoteDeviceAddress, RemoteServerPort, InstanceID, ConnectionFlags)) == 0)
                     {
                        /* Connection request submitted successfully.   */
                        /* If the caller has requested a blocking open, */
                        /* note that we are waiting for the result.     */
                        if(ConnectionStatus)
                           MAPEntryInfoPtr->Flags |= MAP_ENTRY_INFO_FLAGS_CONNECTION_OPENING;
                     }
                     else
                     {
                        /* Error opening port, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((MAPEntryInfoPtr = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
                        {
                           BTPS_CloseEvent(MAPEntryInfoPtr->ConnectionEvent);

                           FreeMAPEntryInfoEntryMemory(MAPEntryInfoPtr);
                        }

                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((ret_val == 0) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Connection Event.                   */
                        ConnectionEvent = MAPEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(MAPManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((MAPEntryInfoPtr = SearchMAPEntryInfo(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
                           {
                              /* Flag that we are no longer opening the */
                              /* connection.                            */
                              MAPEntryInfoPtr->Flags &= ~((unsigned long)MAP_ENTRY_INFO_FLAGS_CONNECTION_OPENING);

                              *ConnectionStatus       = MAPEntryInfoPtr->ConnectionStatus;

                              if(MAPEntryInfoPtr->ConnectionStatus)
                              {
                                 if((MAPEntryInfoPtr = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
                                 {
                                    BTPS_CloseEvent(MAPEntryInfoPtr->ConnectionEvent);

                                    FreeMAPEntryInfoEntryMemory(MAPEntryInfoPtr);
                                 }

                                 ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
                              }
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Mutex because we are finished with it.       */
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               BTPS_ReleaseMutex(MAPManagerMutex);
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
   int               ret_val;
   MAP_Entry_Info_t *MAPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the MAP state information.       */
      if(BTPS_WaitMutex(MAPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Next, retrieve the MAP Entry from the Info List.            */
         if((MAPEntryInfo = SearchMAPEntryInfo(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
         {
            switch(ConnectionType)
            {
               case mctNotificationServer:
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Notification Server Connection from %02X%02X%02X%02X%02X%02X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0));
                  break;
               case mctNotificationClient:
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Notification Client Connection to %02X%02X%02X%02X%02X%02X for local Server Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
                  break;
               case mctMessageAccessServer:
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Message Access Server Connection from %02X%02X%02X%02X%02X%02X for local Server Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
                  break;
               case mctMessageAccessClient:
                  DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Message Access Client Connection to %02X%02X%02X%02X%02X%02X for Remote Instance %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));
                  break;
            }

            /* If the connection is not disconnected, go ahead and do it*/
            /* at this time.                                            */
            ret_val = _MAPM_Disconnect(ConnectionType, RemoteDeviceAddress, InstanceID);

            /* Now process differently based upon client or server.     */
            if((!ret_val) && ((ConnectionType == mctMessageAccessClient) || (ConnectionType == mctNotificationClient)))
            {
               /* Client, go ahead and delete the Info Entry and free   */
               /* all resources.                                        */
               if((MAPEntryInfo = DeleteMAPEntryInfoEntry(&MAPEntryInfoList, ConnectionType, &RemoteDeviceAddress, InstanceID)) != NULL)
               {
                  /* Close any events that were allocated.              */
                  if(MAPEntryInfo->ConnectionEvent)
                     BTPS_CloseEvent(MAPEntryInfo->ConnectionEvent);

                  /* All finished with the memory so free the entry.    */
                  FreeMAPEntryInfoEntryMemory(MAPEntryInfo);
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(MAPManagerMutex);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Abort(ConnectionType, RemoteDeviceAddress, InstanceID);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Query_Current_Folder(RemoteDeviceAddress, InstanceID, BufferSize, Buffer);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Enable_Notifications(RemoteDeviceAddress, InstanceID, Enabled);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Set_Folder(RemoteDeviceAddress, InstanceID, PathOption, FolderName);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Set_Folder_Absolute(RemoteDeviceAddress, InstanceID, FolderName);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Get_Folder_Listing(RemoteDeviceAddress, InstanceID, MaxListCount, ListStartOffset);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Get_Folder_Listing_Size(RemoteDeviceAddress, InstanceID);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Get_Message_Listing(RemoteDeviceAddress, InstanceID, FolderName, MaxListCount, ListStartOffset, ListingInfo);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Get_Message_Listing_Size(RemoteDeviceAddress, InstanceID, FolderName, ListingInfo);
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI MAPM_Get_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Get_Message(RemoteDeviceAddress, InstanceID, MessageHandle, Attachment, CharSet, FractionalType);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Set_Message_Status(RemoteDeviceAddress, InstanceID, MessageHandle, StatusIndicator, StatusValue);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Push_Message(RemoteDeviceAddress, InstanceID, FolderName, Transparent, Retry, CharSet, DataLength, DataBuffer, Final);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Update_Inbox(RemoteDeviceAddress, InstanceID);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Enable_Notifications_Confirmation(RemoteDeviceAddress, InstanceID, ResponseStatusCode);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Set_Folder_Confirmation(RemoteDeviceAddress, InstanceID, ResponseStatusCode);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Send_Folder_Listing(RemoteDeviceAddress, InstanceID, ResponseStatusCode, FolderListingLength, FolderListing, Final);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Send_Folder_Listing_Size(RemoteDeviceAddress, InstanceID, ResponseStatusCode, FolderCount);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Send_Message_Listing(RemoteDeviceAddress, InstanceID, ResponseStatusCode, MessageCount, NewMessage, CurrentTime, MessageListingLength, MessageListing, Final);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Send_Message_Listing_Size(RemoteDeviceAddress, InstanceID, ResponseStatusCode, MessageCount, NewMessage, CurrentTime);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Send_Message(RemoteDeviceAddress, InstanceID, ResponseStatusCode, FractionalType, MessageLength, Message, Final);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Set_Message_Status_Confirmation(RemoteDeviceAddress, InstanceID, ResponseStatusCode);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Push_Message_Confirmation(RemoteDeviceAddress, InstanceID, ResponseStatusCode, MessageHandle);
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
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Update_Inbox_Confirmation(RemoteDeviceAddress, InstanceID, ResponseStatusCode);
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
   /* Parameter specifies the length of the data.  The EventData        */
   /* contains the data to be sent.  This function returns zero if      */
   /* successful and a negative return error code if there was an       */
   /* error.                                                            */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int BTPSAPI MAPM_Send_Notification(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int DataLength, Byte_t *EventData, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the MAP Manager has been initialized.   */
   if(Initialized)
      ret_val = _MAPM_Send_Notification(RemoteDeviceAddress, InstanceID, DataLength, EventData, Final);
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

