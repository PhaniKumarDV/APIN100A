/*****< btpmtipm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMTIPM - TIP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMTIPM.h"            /* BTPM TIP Manager Prototypes/Constants.    */
#include "TIPMAPI.h"             /* TIP Manager Prototypes/Constants.         */
#include "TIPMMSG.h"             /* BTPM TIP Manager Message Formats.         */
#include "TIPMGR.h"              /* TIP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagTIP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 EventHandlerID;
   unsigned long                Flags;
   BD_ADDR_t                    BD_ADDR;
   TIPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagTIP_Entry_Info_t *NextTIPEntryInfoPtr;
} TIP_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   TIPM_Event_Callback_t  EventCallback;
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
static Mutex_t TIPManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the TIP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static TIP_Entry_Info_t *TIPServerEntryInfoList;

   /* Variable which holds a pointer to the first element in the TIP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static TIP_Entry_Info_t *TIPClientEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static TIP_Entry_Info_t *AddTIPEntryInfoEntry(TIP_Entry_Info_t **ListHead, TIP_Entry_Info_t *EntryToAdd);
static TIP_Entry_Info_t *SearchTIPEntryInfoEntryByHandlerID(TIP_Entry_Info_t **ListHead, unsigned int EventHandlerID);
static TIP_Entry_Info_t *SearchTIPEntryInfoEntry(TIP_Entry_Info_t **ListHead, unsigned int CallbackID);
static TIP_Entry_Info_t *DeleteTIPEntryInfoEntry(TIP_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeTIPEntryInfoEntryMemory(TIP_Entry_Info_t *EntryToFree);
static void FreeTIPServerEntryInfoList(TIP_Entry_Info_t **ListHead);

static void ProcessTIPConnectedMessage(TIPM_Connected_Message_t *Message);
static void ProcessTIPDisconnectedMessage(TIPM_Disconnected_Message_t *Message);
static void ProcessTIPReferenceTimeInformationRequestMessage(TIPM_Reference_Time_Request_Message_t *Message);

static void ProcessGetCurrentTimeResponseMessage(TIPM_Get_Current_Time_Response_Message_t *Message);
static void ProcessCurrentTimeNotificationMessage(TIPM_Current_Time_Notification_Message_t *Message);
static void ProcessLocalTimeInformationResponseMessage(TIPM_Local_Time_Information_Response_Message_t *Message);
static void ProcessTimeAccuracyResponseMessage(TIPM_Time_Accuracy_Response_Message_t *Message);
static void ProcessNextDSTChangeResponseMessage(TIPM_Next_DST_Change_Response_Message_t *Message);
static void ProcessTimeUpdateStateResponseMessage(TIPM_Time_Update_State_Response_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_TIPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI TIPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the TIP Entry Information List.                              */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

   return(ret_val);
}

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
static TIP_Entry_Info_t *AddTIPEntryInfoEntry(TIP_Entry_Info_t **ListHead, TIP_Entry_Info_t *EntryToAdd)
{
   TIP_Entry_Info_t *AddedEntry = NULL;
   TIP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (TIP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(TIP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextTIPEntryInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->CallbackID == AddedEntry->CallbackID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeTIPEntryInfoEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextTIPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextTIPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextTIPEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static TIP_Entry_Info_t *SearchTIPEntryInfoEntryByHandlerID(TIP_Entry_Info_t **ListHead, unsigned int EventHandlerID)
{
   TIP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventHandlerID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventHandlerID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventHandlerID != EventHandlerID))
         FoundEntry = FoundEntry->NextTIPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static TIP_Entry_Info_t *SearchTIPEntryInfoEntry(TIP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   TIP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextTIPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified TIP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the TIP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTIPEntryInfoEntryMemory().                   */
static TIP_Entry_Info_t *DeleteTIPEntryInfoEntry(TIP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   TIP_Entry_Info_t *FoundEntry = NULL;
   TIP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextTIPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextTIPEntryInfoPtr = FoundEntry->NextTIPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextTIPEntryInfoPtr;

         FoundEntry->NextTIPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified TIP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTIPEntryInfoEntryMemory(TIP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified TIP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeTIPServerEntryInfoList(TIP_Entry_Info_t **ListHead)
{
   TIP_Entry_Info_t *EntryToFree;
   TIP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextTIPEntryInfoPtr;

         FreeTIPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the TIP      */
   /* Connected asynchronous message.                                   */
static void ProcessTIPConnectedMessage(TIPM_Connected_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIP_Entry_Info_t  *TIPEntryList;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType                                          = aetTIPConnected;
   TIPMEventData.EventLength                                        = TIPM_CONNECTED_EVENT_DATA_SIZE;

   TIPMEventData.EventData.ConnectedEventData.ConnectionType        = Message->ConnectionType;
   TIPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress   = Message->RemoteDeviceAddress;
   TIPMEventData.EventData.ConnectedEventData.SupportedServicesMask = Message->SupportedServicesMask;

   /* Detemine which list to search.                                    */
   if(Message->ConnectionType == tctServer)
      TIPEntryList = TIPServerEntryInfoList;
   else
      TIPEntryList = TIPClientEntryInfoList;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPEntryList, Message->EventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.ConnectedEventData.CallbackID = TIPEntryInfo->CallbackID;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the TIP      */
   /* Disconnected asynchronous message.                                */
static void ProcessTIPDisconnectedMessage(TIPM_Disconnected_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIP_Entry_Info_t  *TIPEntryList;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType                                           = aetTIPDisconnected;
   TIPMEventData.EventLength                                         = TIPM_DISCONNECTED_EVENT_DATA_SIZE;

   TIPMEventData.EventData.DisconnectedEventData.ConnectionType      = Message->ConnectionType;
   TIPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

   /* Detemine which list to search.                                    */
   if(Message->ConnectionType == tctServer)
      TIPEntryList = TIPServerEntryInfoList;
   else
      TIPEntryList = TIPClientEntryInfoList;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPEntryList, Message->EventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.DisconnectedEventData.CallbackID = TIPEntryInfo->CallbackID;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the TIP      */
   /* Reference Time Information Request asynchronous message.          */
static void ProcessTIPReferenceTimeInformationRequestMessage(TIPM_Reference_Time_Request_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType   = aetTIPGetReferenceTimeRequest;
   TIPMEventData.EventLength = TIPM_GET_REFERENCE_TIME_REQUEST_EVENT_DATA_SIZE;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPServerEntryInfoList, Message->ServerEventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.GetReferenceTimeRequestEventData.ServerCallbackID = TIPEntryInfo->CallbackID;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for handling a Get Current  */
   /* Time Response message.                                            */
static void ProcessGetCurrentTimeResponseMessage(TIPM_Get_Current_Time_Response_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType   = aetTIPGetCurrentTimeResponse;
   TIPMEventData.EventLength = TIPM_GET_CURRENT_TIME_RESPONSE_EVENT_DATA_SIZE;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPClientEntryInfoList, Message->ClientEventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.GetCurrentTimeResponseEventData.ClientCallbackID    = TIPEntryInfo->CallbackID;
      TIPMEventData.EventData.GetCurrentTimeResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      TIPMEventData.EventData.GetCurrentTimeResponseEventData.Status              = Message->Status;
      TIPMEventData.EventData.GetCurrentTimeResponseEventData.CurrentTimeData     = Message->CurrentTimeData;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for handling a Current Time */
   /* Notification message.                                             */
static void ProcessCurrentTimeNotificationMessage(TIPM_Current_Time_Notification_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType   = aetTIPCurrentTimeNotification;
   TIPMEventData.EventLength = TIPM_CURRENT_TIME_NOTIFICATION_EVENT_DATA_SIZE;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPClientEntryInfoList, Message->ClientEventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.CurrentTimeNotificationEventData.ClientCallbackID    = TIPEntryInfo->CallbackID;
      TIPMEventData.EventData.CurrentTimeNotificationEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      TIPMEventData.EventData.CurrentTimeNotificationEventData.CurrentTimeData     = Message->CurrentTimeData;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for handling a Local Time   */
   /* Information Response message.                                     */
static void ProcessLocalTimeInformationResponseMessage(TIPM_Local_Time_Information_Response_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType   = aetTIPLocalTimeInformationResponse;
   TIPMEventData.EventLength = TIPM_LOCAL_TIME_INFORMATION_RESPONSE_EVENT_DATA_SIZE;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPClientEntryInfoList, Message->ClientEventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.LocalTimeInformationResponseEventData.ClientCallbackID     = TIPEntryInfo->CallbackID;
      TIPMEventData.EventData.LocalTimeInformationResponseEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
      TIPMEventData.EventData.LocalTimeInformationResponseEventData.Status               = Message->Status;
      TIPMEventData.EventData.LocalTimeInformationResponseEventData.LocalTimeInformation = Message->LocalTimeInformation;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for handling a Time Accuracy*/
   /* Response message.                                                 */
static void ProcessTimeAccuracyResponseMessage(TIPM_Time_Accuracy_Response_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType   = aetTIPTimeAccuracyResponse;
   TIPMEventData.EventLength = TIPM_TIME_ACCURACY_RESPONSE_EVENT_DATA_SIZE;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPClientEntryInfoList, Message->ClientEventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.TimeAccuracyResponseEventData.ClientCallbackID         = TIPEntryInfo->CallbackID;
      TIPMEventData.EventData.TimeAccuracyResponseEventData.RemoteDeviceAddress      = Message->RemoteDeviceAddress;
      TIPMEventData.EventData.TimeAccuracyResponseEventData.Status                   = Message->Status;
      TIPMEventData.EventData.TimeAccuracyResponseEventData.ReferenceTimeInformation = Message->ReferenceTimeInformation;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for handling a Next DST     */
   /* Change Response message.                                          */
static void ProcessNextDSTChangeResponseMessage(TIPM_Next_DST_Change_Response_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType   = aetTIPNextDSTChangeResponse;
   TIPMEventData.EventLength = TIPM_NEXT_DST_CHANGE_RESPONSE_EVENT_DATA_SIZE;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPClientEntryInfoList, Message->ClientEventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.NextDSTChangeResponseEventData.ClientCallbackID    = TIPEntryInfo->CallbackID;
      TIPMEventData.EventData.NextDSTChangeResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      TIPMEventData.EventData.NextDSTChangeResponseEventData.Status              = Message->Status;
      TIPMEventData.EventData.NextDSTChangeResponseEventData.TimeWithDST         = Message->TimeWithDST;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for handling a Time Update  */
   /* State Response message.                                           */
static void ProcessTimeUpdateStateResponseMessage(TIPM_Time_Update_State_Response_Message_t *Message)
{
   TIP_Entry_Info_t  *TIPEntryInfo;
   TIPM_Event_Data_t  TIPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   TIPMEventData.EventType   = aetTIPTimeUpdateStateResponse;
   TIPMEventData.EventLength = TIPM_TIME_UPDATE_STATE_RESPONSE_EVENT_DATA_SIZE;

   /* Search for the Event Callback Entry for the device that we will   */
   /* dispatch this to.                                                 */
   if((TIPEntryInfo = SearchTIPEntryInfoEntryByHandlerID(&TIPClientEntryInfoList, Message->ClientEventHandlerID)) != NULL)
   {
      /* Save the Callback ID for the event.                            */
      TIPMEventData.EventData.TimeUpdateStateResponseEventData.ClientCallbackID    = TIPEntryInfo->CallbackID;
      TIPMEventData.EventData.TimeUpdateStateResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      TIPMEventData.EventData.TimeUpdateStateResponseEventData.Status              = Message->Status;
      TIPMEventData.EventData.TimeUpdateStateResponseEventData.TimeUpdateStateData = Message->TimeUpdateStateData;

      /* Release the Mutex before we dispatch the event.                */
      BTPS_ReleaseMutex(TIPManagerMutex);

      /* Go ahead and make the callback.                                */
      __BTPSTRY
      {
         if(TIPEntryInfo->EventCallback)
            (*TIPEntryInfo->EventCallback)(&TIPMEventData, TIPEntryInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the manager mutex.                             */
      BTPS_ReleaseMutex(TIPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the TIP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case TIPM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Connection Established\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connected Event.                                      */
               ProcessTIPConnectedMessage((TIPM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Disconnect\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnected Event.                                   */
               ProcessTIPDisconnectedMessage((TIPM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Reference Time Request\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_REFERENCE_TIME_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Reference Time Request Event.                         */
               ProcessTIPReferenceTimeInformationRequestMessage((TIPM_Reference_Time_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Get Current Time Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_GET_CURRENT_TIME_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Current Time Response Event.                          */
               ProcessGetCurrentTimeResponseMessage((TIPM_Get_Current_Time_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_CURRENT_TIME_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Current Time Notification\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_CURRENT_TIME_NOTIFICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Current Time Notification Event.                      */
               ProcessCurrentTimeNotificationMessage((TIPM_Current_Time_Notification_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_LOCAL_TIME_INFORMATION_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Local Time Information Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_LOCAL_TIME_INFORMATION_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Local Time Information Response Event.                */
               ProcessLocalTimeInformationResponseMessage((TIPM_Local_Time_Information_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_TIME_ACCURACY_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Time Accuracy Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_TIME_ACCURACY_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Time Accuracy Response Event.                         */
               ProcessTimeAccuracyResponseMessage((TIPM_Time_Accuracy_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_NEXT_DST_CHANGE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Next DST Change Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_NEXT_DST_CHANGE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Next*/
               /* DST Change Response Event.                            */
               ProcessNextDSTChangeResponseMessage((TIPM_Next_DST_Change_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_TIME_UPDATE_STATE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Time Update State Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_TIME_UPDATE_STATE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Time*/
               /* Update State Response Event.                          */
               ProcessTimeUpdateStateResponseMessage((TIPM_Time_Update_State_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(TIPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process TIP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_TIPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the TIP state information.    */
         if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the TIP state information.    */
         if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the Event Callback List.                            */
            FreeTIPServerEntryInfoList(&TIPServerEntryInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(TIPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all TIP Manager Messages.   */
static void BTPSAPI TIPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_TIME_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a TIP Manager defined    */
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
               /* TIP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_TIPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue TIP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue TIP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an TIP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Non TIP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager TIP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI TIPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing TIP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((TIPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process TIP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_TIME_MANAGER, TIPManagerGroupHandler, NULL))
            {
               /* Initialize the actual TIP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the TIP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _TIPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique starting Callback ID.          */
                  NextCallbackID      = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized         = TRUE;
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
            if(TIPManagerMutex)
               BTPS_CloseMutex(TIPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_TIME_MANAGER);

            /* Flag that none of the resources are allocated.           */
            TIPManagerMutex = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_TIME_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we inform the TIP Manager Implementation that  */
            /* we are shutting down.                                    */
            _TIPM_Cleanup();

            BTPS_CloseMutex(TIPManagerMutex);

            /* Make sure that the TIP Entry Information List is empty.  */
            FreeTIPServerEntryInfoList(&TIPServerEntryInfoList);

            /* Flag that the resources are no longer allocated.         */
            TIPManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI TIPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Server Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Server Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI TIPM_Register_Server_Event_Callback(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   TIP_Entry_Info_t TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the TIP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the TIP Entry list.         */
            BTPS_MemInitialize(&TIPEntryInfo, 0, sizeof(TIP_Entry_Info_t));

            TIPEntryInfo.CallbackID        = GetNextCallbackID();
            TIPEntryInfo.EventCallback     = CallbackFunction;
            TIPEntryInfo.CallbackParameter = CallbackParameter;

            /* Attempt to register with the server.                     */
            if((ret_val = _TIPM_Register_Server_Events()) > 0)
            {
               /* Save the Event Handler ID for the callback.           */
               TIPEntryInfo.EventHandlerID = (unsigned int)ret_val;

               /* Attempt to add the callback to the callback list.     */
               if(AddTIPEntryInfoEntry(&TIPServerEntryInfoList, &TIPEntryInfo))
                  ret_val = TIPEntryInfo.CallbackID;
               else
               {
                  /* Un-register the event handler from the server.     */
                  _TIPM_Un_Register_Server_Events(TIPEntryInfo.EventHandlerID);

                  /* Return an error to the caller.                     */
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(TIPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Server Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Server_Event_Callback() function).  This function   */
   /* accepts as input the Server Event Callback ID (return value from  */
   /* TIPM_Register_Server_Event_Callback() function).                  */
void BTPSAPI TIPM_Un_Register_Server_Event_Callback(unsigned int ServerCallbackID)
{
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ServerCallbackID)
      {
         /* Attempt to wait for access to the TIP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to delete the callback from the list.            */
            if((TIPEntryInfo = DeleteTIPEntryInfoEntry(&TIPServerEntryInfoList, ServerCallbackID)) != NULL)
            {
               /* Un-register the event handler from the server.        */
               _TIPM_Un_Register_Server_Events(TIPEntryInfo->EventHandlerID);

               /* Free the memory because we are finished with it.      */
               FreeTIPEntryInfoEntryMemory(TIPEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(TIPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to set  */
   /* the current Local Time Information.  This function accepts the    */
   /* Server Callback ID (return value from                             */
   /* TIPM_Register_Server_Event_Callback() function) and a pointer to  */
   /* the Local Time Information to set.  This function returns ZERO if */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI TIPM_Set_Local_Time_Information(unsigned int ServerCallbackID, TIPM_Local_Time_Information_Data_t *LocalTimeInformation)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPServerEntryInfoList, ServerCallbackID)) != NULL)
         {
            /* Simply call the Implementation Manager to send the       */
            /* correct message to the PM Server to Set the Local Time   */
            /* Information.                                             */
            ret_val = _TIPM_Set_Local_Time_Information(TIPEntryInfo->EventHandlerID, LocalTimeInformation);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to force*/
   /* an update of the Current Time.  This function accepts the Server  */
   /* Callback ID (return value from                                    */
   /* TIPM_Register_Server_Event_Callback() function) and a bit mask    */
   /* that contains the reason for the Current Time Update.  This       */
   /* function returns ZERO if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI TIPM_Update_Current_Time(unsigned int ServerCallbackID, unsigned long AdjustReasonMask)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPServerEntryInfoList, ServerCallbackID)) != NULL)
         {
            /* Simply call the Implementation Manager to send the       */
            /* correct message to the PM Server to Update the Local Time*/
            /* Information.                                             */
            ret_val = _TIPM_Update_Current_Time(TIPEntryInfo->EventHandlerID, AdjustReasonMask);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* respond to a request for the Reference Time Information.  This    */
   /* function accepts the Server Callback ID (return value from        */
   /* TIPM_Register_Server_Event_Callback() function) and a pointer to  */
   /* the Reference Time Information to respond to the request with.    */
   /* This function returns ZERO if successful, or a negative return    */
   /* error code if there was an error.                                 */
int BTPSAPI TIPM_Reference_Time_Response(unsigned int ServerCallbackID, TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPServerEntryInfoList, ServerCallbackID)) != NULL)
         {
            /* Simply call the Implementation Manager to send the       */
            /* correct message to the PM Server to Respond to the       */
            /* Request for the Reference Time Information.              */
            ret_val = _TIPM_Reference_Time_Response(TIPEntryInfo->EventHandlerID, ReferenceTimeInformation);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Client callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Client Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Client Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI TIPM_Register_Client_Event_Callback(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   TIP_Entry_Info_t TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the TIP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the TIP Entry list.         */
            BTPS_MemInitialize(&TIPEntryInfo, 0, sizeof(TIP_Entry_Info_t));

            TIPEntryInfo.CallbackID        = GetNextCallbackID();
            TIPEntryInfo.EventCallback     = CallbackFunction;
            TIPEntryInfo.CallbackParameter = CallbackParameter;

            /* Attempt to register with the server.                     */
            if((ret_val = _TIPM_Register_Client_Events()) > 0)
            {
               /* Save the Event Handler ID for the callback.           */
               TIPEntryInfo.EventHandlerID = (unsigned int)ret_val;

               /* Attempt to add the callback to the callback list.     */
               if(AddTIPEntryInfoEntry(&TIPClientEntryInfoList, &TIPEntryInfo))
                  ret_val = TIPEntryInfo.CallbackID;
               else
               {
                  /* Un-register the event handler from the server.     */
                  _TIPM_Un_Register_Client_Events(TIPEntryInfo.EventHandlerID);

                  /* Return an error to the caller.                     */
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(TIPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the Client Event Callback ID (return value from  */
   /* TIPM_Register_Client_Event_Callback() function).                  */
void BTPSAPI TIPM_Un_Register_Client_Event_Callback(unsigned int ClientCallbackID)
{
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ClientCallbackID)
      {
         /* Attempt to wait for access to the TIP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to delete the callback from the list.            */
            if((TIPEntryInfo = DeleteTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Un-register the event handler from the server.        */
               _TIPM_Un_Register_Client_Events(TIPEntryInfo->EventHandlerID);

               /* Free the memory because we are finished with it.      */
               FreeTIPEntryInfoEntryMemory(TIPEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(TIPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is provided to allow a mechanism for       */
   /* modules to get the current time from a remote TIP server.  The    */
   /* first parameter is the CallbackID returned from a successful call */
   /* to TIPM_Register_Client_Events.  The second parameter is the      */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetGetCurrentTimeResponse  */
   /*          event.                                                   */
int BTPSAPI TIPM_Get_Current_Time(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
         {
            /* Simply send the request to the server.                   */
            ret_val = _TIPM_Get_Current_Time(TIPEntryInfo->EventHandlerID, RemoteDeviceAddress);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to enable current time notifications from a remote TIP    */
   /* Server.  The first parameter is the CallbackID returned from      */
   /* a successful call to TIPM_Register_Client_Events.  The second     */
   /* parameter is the Bluetooth Address of the remote TIP Server.  This*/
   /* function returns zero if successful and a negative return error   */
   /* code if there is an error.                                        */
int BTPSAPI TIPM_Enable_Time_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Enable)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
         {
            /* Simply send the request to the server.                   */
            ret_val = _TIPM_Enable_Time_Notifications(TIPEntryInfo->EventHandlerID, RemoteDeviceAddress, Enable);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the local time information from a remote TIP       */
   /* server.  The first parameter is the CallbackID returned from      */
   /* a successful call to TIPM_Register_Client_Events.  The second     */
   /* parameter is the Bluetooth Address of the remote TIP Server.      */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The                  */
   /*          result of the request will be returned in a              */
   /*          aetLocalTimeInformationResponse event.                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Local_Time_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
         {
            /* Simply send the request to the server.                   */
            ret_val = _TIPM_Get_Local_Time_Information(TIPEntryInfo->EventHandlerID, RemoteDeviceAddress);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the time accuracy and the reference time           */
   /* information of a remote TIP Server.  The first parameter          */
   /* is the CallbackID returned from a successful call to              */
   /* TIPM_Register_Client_Events.  The second parameter is the         */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetTimeAccuracyResponse    */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Time_Accuracy(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
         {
            /* Simply send the request to the server.                   */
            ret_val = _TIPM_Get_Time_Accuracy(TIPEntryInfo->EventHandlerID, RemoteDeviceAddress);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the next Daylight Savings Time change information  */
   /* from a remote TIP Server.  The first parameter is the CallbackID  */
   /* returned from a successful call to TIPM_Register_Client_Events.   */
   /* The second parameter is the Bluetooth Address of the remote TIP   */
   /* Server.  This function returns a positive number representing the */
   /* Transaction ID of this request if successful and a negative return*/
   /* error code if there was an error.                                 */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetNextDSTChangeResponse   */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Next_DST_Change_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
         {
            /* Simply send the request to the server.                   */
            ret_val = _TIPM_Get_Next_DST_Change_Information(TIPEntryInfo->EventHandlerID, RemoteDeviceAddress);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the time update state of a remote TIP Server.  The */
   /* first parameter is the CallbackID returned from a successful call */
   /* to TIPM_Register_Client_Events.  The second parameter is the      */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetTimeUpdateStateResponse */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Reference_Time_Update_State(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
         {
            /* Simply send the request to the server.                   */
            ret_val = _TIPM_Get_Reference_Time_Update_State(TIPEntryInfo->EventHandlerID, RemoteDeviceAddress);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to request a reference time update on a remote TIP Server */
   /* The first parameter is the CallbackID returned from a successful  */
   /* call to TIPM_Register_Client_Events.  The second parameter is the */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* zero if successful and a negative return error code if there is an*/
   /* error.                                                            */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Request_Reference_Time_Update(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   TIP_Entry_Info_t *TIPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the TIP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(TIPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Search for the callback entry for the specified device.     */
         if((TIPEntryInfo = SearchTIPEntryInfoEntry(&TIPClientEntryInfoList, ClientCallbackID)) != NULL)
         {
            /* Simply send the request to the server.                   */
            ret_val = _TIPM_Request_Reference_Time_Update(TIPEntryInfo->EventHandlerID, RemoteDeviceAddress);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TIPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated        */
   /* with the total number of connected devices if the function is     */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns    */
   /* a non-negative value if successful which represents the number    */
   /* of connected devices that were copied into the specified input    */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI TIPM_Query_Connected_Devices(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, TIPM_Remote_Device_t *RemoteDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Simply submit the query.                                       */
      ret_val = _TIPM_Query_Connected_Devices(ConnectionType, MaximumRemoteDeviceListEntries, RemoteDeviceList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

