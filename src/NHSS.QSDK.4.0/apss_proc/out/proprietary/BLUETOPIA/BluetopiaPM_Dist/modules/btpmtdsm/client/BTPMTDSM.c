/*****< btpmtdsm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMTDSM - 3D Sync Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMTDSM.h"            /* BTPM 3D Sync Manager Prototypes/Constants.*/
#include "TDSMAPI.h"             /* 3D Sync Manager Prototypes/Constants.     */
#include "TDSMMSG.h"             /* BTPM 3D Sync Manager Message Formats.     */
#include "TDSMGR.h"              /* 3D Sync Manager Imp. Prototypes/Constants.*/

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagTDSM_Callback_Info_t
{
   unsigned int                     CallbackID;
   TDSM_Event_Callback_t            EventCallback;
   void                            *CallbackParameter;
   struct _tagTDSM_Callback_Info_t *NextCallbackInfoPtr;
} TDSM_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   TDSM_Event_Callback_t  EventCallback;
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
static Mutex_t TDSManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the callback ID registered with the remote PM*/
   /* server.                                                           */
static unsigned int TDSEventsCallbackID;

   /* Variable which holds the callback ID registered with the remote PM*/
   /* server when registered for Control.                               */
static unsigned int TDSControlEventsCallbackID;

   /* Variables which hold pointers to the first element in the callback*/
   /* information lists.                                                */
static TDSM_Callback_Info_t *CallbackInfoListHead;
static TDSM_Callback_Info_t *ControlCallbackInfoListHead;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static TDSM_Callback_Info_t *AddCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, TDSM_Callback_Info_t *EntryToAdd);
static TDSM_Callback_Info_t *SearchCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID);
static TDSM_Callback_Info_t *DeleteCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID);
static void FreeCallbackInfoEntryMemory(TDSM_Callback_Info_t *EntryToFree);
static void FreeCallbackInfoList(TDSM_Callback_Info_t **ListHead);

static void DispatchTDSEvent(Boolean_t ControlOnly, TDSM_Event_Data_t *EventData);

static void ProcessDisplayConnectionAnnouncementEvent(TDSM_Display_Connection_Announcement_Message_t *Message);
static void ProcessDisplaySynchronizationTrainCompleteEvent(TDSM_Display_Synchronization_Train_Complete_Message_t *Message);
static void ProcessDisplayCSBSupervisionTimeoutEvent(TDSM_Display_CSB_Supervision_Timeout_Message_t *Message);
static void ProcessDisplayChannelMapChangeEvent(TDSM_Display_Channel_Map_Change_Message_t *Message);
static void ProcessDisplaySlavePageResponseTimeoutEvent(TDSM_Display_Slave_Page_Response_Timeout_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_TDSM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique callback ID that can be used to add an entry    */
   /* into the 3D Sync callback information list.                       */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

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
   /*            callbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static TDSM_Callback_Info_t *AddCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, TDSM_Callback_Info_t *EntryToAdd)
{
   TDSM_Callback_Info_t *AddedEntry = NULL;
   TDSM_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (TDSM_Callback_Info_t *)BTPS_AllocateMemory(sizeof(TDSM_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextCallbackInfoPtr = NULL;

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
                     FreeCallbackInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextCallbackInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified callback ID.  This function returns NULL if either the  */
   /* list head is invalid, the callback ID is invalid, or the specified*/
   /* callback ID was NOT found.                                        */
static TDSM_Callback_Info_t *SearchCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID)
{
   TDSM_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified 3D Sync entry       */
   /* information list for the specified callback ID and removes it from*/
   /* the List.  This function returns NULL if either the 3D Sync entry */
   /* information list head is invalid, the callback ID is invalid, or  */
   /* the specified callback ID was NOT present in the list.  The entry */
   /* returned will have the next entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeCallbackInfoEntryMemory().                   */
static TDSM_Callback_Info_t *DeleteCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID)
{
   TDSM_Callback_Info_t *FoundEntry = NULL;
   TDSM_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextCallbackInfoPtr = FoundEntry->NextCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextCallbackInfoPtr;

         FoundEntry->NextCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified 3D Sync entry information       */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeCallbackInfoEntryMemory(TDSM_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified 3D Sync entry information list.  Upon    */
   /* return of this function, the head pointer is set to NULL.         */
static void FreeCallbackInfoList(TDSM_Callback_Info_t **ListHead)
{
   TDSM_Callback_Info_t *EntryToFree;
   TDSM_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextCallbackInfoPtr;

         FreeCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified 3D Sync event to every registered 3D Sync  */
   /* Event Callback.                                                   */
   /* * NOTE * This function should be called with the 3D Sync Manager  */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the 3D Sync Manager Mutex.                               */
static void DispatchTDSEvent(Boolean_t ControlOnly, TDSM_Event_Data_t *EventData)
{
   unsigned int       Index;
   unsigned int       NumberCallbacks;
   CallbackInfo_t     CallbackInfoArray[16];
   CallbackInfo_t    *CallbackInfoArrayPtr;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((ControlCallbackInfoListHead) || ((!ControlOnly) && (CallbackInfoListHead))) && (EventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      if(!ControlOnly)
      {
         CallbackInfo = CallbackInfoListHead;

         while(CallbackInfo)
         {
            if(CallbackInfo->EventCallback)
               NumberCallbacks++;

            CallbackInfo = CallbackInfo->NextCallbackInfoPtr;
         }
      }

      /* Next, add the control handlers.                                */
      CallbackInfo = ControlCallbackInfoListHead;

      while(CallbackInfo)
      {
         if(CallbackInfo->EventCallback)
            NumberCallbacks++;

         CallbackInfo = CallbackInfo->NextCallbackInfoPtr;
      }

      if(NumberCallbacks)
      {
         if(NumberCallbacks <= (sizeof(CallbackInfoArray)/sizeof(CallbackInfo_t)))
            CallbackInfoArrayPtr = CallbackInfoArray;
         else
            CallbackInfoArrayPtr = BTPS_AllocateMemory((NumberCallbacks*sizeof(CallbackInfo_t)));

         /* Make sure that we have memory to copy the Callback List     */
         /* into.                                                       */
         if(CallbackInfoArrayPtr)
         {
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            if(!ControlOnly)
            {
               CallbackInfo = CallbackInfoListHead;

               while(CallbackInfo)
               {
                  if(CallbackInfo->EventCallback)
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  CallbackInfo = CallbackInfo->NextCallbackInfoPtr;
               }
            }

            /* Next, add the control handlers.                          */
            CallbackInfo = ControlCallbackInfoListHead;

            while(CallbackInfo)
            {
               if(CallbackInfo->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfo = CallbackInfo->NextCallbackInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(TDSManagerMutex);

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* Go ahead and make the callback.                       */
               /* * NOTE * If the callback was deleted (or new ones were*/
               /*          added, they will not be caught for this      */
               /*          message dispatch).  Under normal operating   */
               /*          circumstances this case shouldn't matter     */
               /*          because these groups aren't really dynamic   */
               /*          and are only registered at initialization    */
               /*          time.                                        */
               __BTPSTRY
               {
                  if(CallbackInfoArrayPtr[Index].EventCallback)
                  {
                     (*CallbackInfoArrayPtr[Index].EventCallback)(EventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                  }
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               Index++;
            }

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(TDSManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TDSManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(TDSManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisplayConnectionAnnouncementEvent(TDSM_Display_Connection_Announcement_Message_t *Message)
{
   TDSM_Event_Data_t TDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

   TDSMEventData.EventType                                                       = tetTDSM_Display_Connection_Announcement;
   TDSMEventData.EventDataSize                                                   = TDSM_DISPLAY_CONNECTION_ANNOUNCEMENT_DATA_SIZE;

   TDSMEventData.EventData.DisplayConnectionAnnouncementData.RemoteDeviceAddress = Message->BD_ADDR;
   TDSMEventData.EventData.DisplayConnectionAnnouncementData.BatteryLevel        = Message->BatteryLevel;
   TDSMEventData.EventData.DisplayConnectionAnnouncementData.Flags               = Message->Flags;

   /* Now dispatch the event.                                           */
   DispatchTDSEvent(FALSE, &TDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisplaySynchronizationTrainCompleteEvent(TDSM_Display_Synchronization_Train_Complete_Message_t *Message)
{
   TDSM_Event_Data_t TDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

   TDSMEventData.EventType                                                = tetTDSM_Display_Synchronization_Train_Complete;
   TDSMEventData.EventDataSize                                            = TDSM_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_DATA_SIZE;

   TDSMEventData.EventData.DisplaySynchronizationTrainCompleteData.Status = Message->Status;

   /* Now dispatch the event.                                           */
   DispatchTDSEvent(FALSE, &TDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisplayCSBSupervisionTimeoutEvent(TDSM_Display_CSB_Supervision_Timeout_Message_t *Message)
{
   TDSM_Event_Data_t TDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

   TDSMEventData.EventType     = tetTDSM_Display_CSB_Supervision_Timeout;
   TDSMEventData.EventDataSize = 0;

   /* Now dispatch the event.                                           */
   DispatchTDSEvent(FALSE, &TDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisplayChannelMapChangeEvent(TDSM_Display_Channel_Map_Change_Message_t *Message)
{
   TDSM_Event_Data_t TDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

   TDSMEventData.EventType                                        = tetTDSM_Display_Channel_Map_Change;
   TDSMEventData.EventDataSize                                    = TDSM_DISPLAY_CHANNEL_MAP_CHANGE_DATA_SIZE;

   TDSMEventData.EventData.DisplayChannelMapChangeData.ChannelMap = Message->ChannelMap;

   /* Now dispatch the event.                                           */
   DispatchTDSEvent(FALSE, &TDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisplaySlavePageResponseTimeoutEvent(TDSM_Display_Slave_Page_Response_Timeout_Message_t *Message)
{
   TDSM_Event_Data_t TDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

   TDSMEventData.EventType     = tetTDSM_Display_Slave_Page_Response_Timeout;
   TDSMEventData.EventDataSize = 0;

   /* Now dispatch the event.                                           */
   DispatchTDSEvent(FALSE, &TDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case TDSM_MESSAGE_FUNCTION_DISPLAY_CONNECTION_ANNOUNCEMENT:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display Connection Announcement Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_DISPLAY_CONNECTION_ANNOUNCEMENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisplayConnectionAnnouncementEvent((TDSM_Display_Connection_Announcement_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Dispaly Synchronization Train Complete Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisplaySynchronizationTrainCompleteEvent((TDSM_Display_Synchronization_Train_Complete_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_DISPLAY_CSB_SUPERVISION_TIMEOUT:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display CSB Supervision Timeout Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_DISPLAY_CSB_SUPERVISION_TIMEOUT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisplayCSBSupervisionTimeoutEvent((TDSM_Display_CSB_Supervision_Timeout_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_DISPLAY_CHANNEL_MAP_CHANGE:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display Channel Map Change Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_DISPLAY_CHANNEL_MAP_CHANGE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisplayChannelMapChangeEvent((TDSM_Display_Channel_Map_Change_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_DISPLAY_SLAVE_PAGE_RESPONSE_TIMEOUT:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display Slave Page Response Timeout Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_DISPLAY_SLAVE_PAGE_RESPONSE_TIMEOUT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisplaySlavePageResponseTimeoutEvent((TDSM_Display_Slave_Page_Response_Timeout_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(TDSManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process 3D Sync Manager Asynchronous Events.        */
static void BTPSAPI BTPMDispatchCallback_TDSM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the 3D Sync state information.*/
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the 3D Sync state information.*/
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Dump the callback list, because we are no longer         */
            /* registered with the remote server.                       */
            FreeCallbackInfoList(&CallbackInfoListHead);
            FreeCallbackInfoList(&ControlCallbackInfoListHead);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(TDSManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all 3D Sync Manager         */
   /* Messages.                                                         */
static void BTPSAPI TDSManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("3D Sync Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a 3D Sync Manager        */
            /* defined Message.  If it is it will be within the range:  */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Still processing, go ahead and post the message to the*/
               /* 3D Sync Manager thread.                               */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_TDSM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue 3D Sync Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue 3D Sync Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an 3D Sync Manager       */
            /* defined Message.  If it is it will be within the range:  */
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
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Non 3D Sync Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager 3D Sync Manager module.  This      */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI TDSM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing 3D Sync Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((TDSManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process 3D Sync Manager messages.                     */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER, TDSManagerGroupHandler, NULL))
            {
               /* Initialize the actual 3D Sync Manager Implementation  */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the HDSET       */
               /* Manager functionality - this module is just the       */
               /* framework shell).                                     */
               if(!(Result = _TDSM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting 3D Sync Callback ID. */
                  NextCallbackID = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized    = TRUE;
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
            if(TDSManagerMutex)
               BTPS_CloseMutex(TDSManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER);

            /* Flag that none of the resources are allocated.           */
            TDSManagerMutex        = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("3D Sync Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for 3D Sync Events.                          */
            if(TDSEventsCallbackID)
               _TDSM_Un_Register_Event_Callback(TDSEventsCallbackID);

            if(TDSControlEventsCallbackID)
               _TDSM_Un_Register_Event_Callback(TDSControlEventsCallbackID);

            /* Make sure we inform the 3D Sync Manager Implementation   */
            /* that we are shutting down.                               */
            _TDSM_Cleanup();

            BTPS_CloseMutex(TDSManagerMutex);

            /* Make sure that the 3D Sync callback information list is  */
            /* empty.                                                   */
            FreeCallbackInfoList(&CallbackInfoListHead);
            FreeCallbackInfoList(&ControlCallbackInfoListHead);

            /* Flag that the resources are no longer allocated.         */
            TDSManagerMutex            = NULL;
            CurrentPowerState          = FALSE;
            TDSEventsCallbackID        = 0;
            TDSControlEventsCallbackID = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI TDSM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(TDSManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* 3D Synchronization Profile API Functions.                         */

   /* The following function will configure the Synchronization Train   */
   /* parameters on the Bluetooth controller.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The SyncTrainParams parameter is a pointer to the       */
   /* Synchronization Train parameters to be set.  The IntervalResult   */
   /* parameter is a pointer to the variable that will be populated with*/
   /* the Synchronization Interval chosen by the Bluetooth controller.  */
   /* This function will return zero on success; otherwise, a negative  */
   /* error value will be returned.                                     */
   /* * NOTE * The Timout value in the                                  */
   /*          TDS_Synchronization_Train_Parameters_t structure must be */
   /*          a value of at least 120 seconds, or the function call    */
   /*          will fail.                                               */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Write_Synchronization_Train_Parameters(unsigned int TDSMControlCallbackID, TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((TDSMControlCallbackID) && (SyncTrainParams) && (IntervalResult))
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check if the callback is registered as control.          */
            if((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoListHead, TDSMControlCallbackID)) != NULL)
            {
               /* Simply submit the command to the server.              */
               ret_val = _TDSM_Write_Synchronization_Train_Parameters(TDSControlEventsCallbackID, SyncTrainParams, IntervalResult);
            }
            else
               ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function will enable the Synchronization Train. The */
   /* TDSMControlCallbackID parameter is an ID returned from a succesfull*/
   /* call to TDSM_Register_Event_Callback() with the Control parameter */
   /* set to TRUE.  This function will return zero on success;          */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The TDSM_Write_Synchronization_Train_Parameters function */
   /*          should be called at least once after initializing the    */
   /*          stack and before calling this function.                  */
   /* * NOTE * The tetTDSM_Display_Synchronization_Train_Complete event */
   /*          will be triggered when the Synchronization Train         */
   /*          completes.  This function can be called again at this    */
   /*          time to restart the Synchronization Train.               */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Start_Synchronization_Train(unsigned int TDSMControlCallbackID)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(TDSMControlCallbackID)
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check if the callback is registered as control.          */
            if((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoListHead, TDSMControlCallbackID)) != NULL)
            {
               /* Simply submit the command to the server.              */
               ret_val = _TDSM_Start_Synchronization_Train(TDSControlEventsCallbackID);
            }
            else
               ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function will configure and enable                  */
   /* the Connectionless Slave Broadcast channel on the                 */
   /* Bluetooth controller.  The TDSMControlCallbackID                   */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The ConnectionlessSlaveBroadcastParams parameter is a   */
   /* pointer to the Connectionless Slave Broadcast parameters to be    */
   /* set.  The IntervalResult parameter is a pointer to the variable   */
   /* that will be populated with the Broadcast Interval chosen by the  */
   /* Bluetooth controller.  This function will return zero on success; */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The MinInterval value should be greater than or equal to */
   /*          50 milliseconds, and the MaxInterval value should be less*/
   /*          than or equal to 100 milliseconds; otherwise, the        */
   /*          function will fail.                                      */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Enable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID, TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((TDSMControlCallbackID) && (ConnectionlessSlaveBroadcastParams) && (IntervalResult))
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check if the callback is registered as control.          */
            if((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoListHead, TDSMControlCallbackID)) != NULL)
            {
               /* Simply submit the command to the server.              */
               ret_val = _TDSM_Enable_Connectionless_Slave_Broadcast(TDSControlEventsCallbackID, ConnectionlessSlaveBroadcastParams, IntervalResult);
            }
            else
               ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.                                                             */
   /* * NOTE * Calling this function will terminate the Synchronization */
   /*          Train (if it is currently enabled).                      */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Disable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((TDSMControlCallbackID))
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check if the callback is registered as control.          */
            if((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoListHead, TDSMControlCallbackID)) != NULL)
            {
               /* Simply submit the command to the server.              */
               ret_val = _TDSM_Disable_Connectionless_Slave_Broadcast(TDSControlEventsCallbackID);
            }
            else
               ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is used to get the current information     */
   /* being used int the synchronization broadcasts.  The               */
   /* CurrentBroadcastInformation parameter to a structure in which the */
   /* current information will be placed.  This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Get_Current_Broadcast_Information(TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((CurrentBroadcastInformation))
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the command to the server.                 */
            ret_val = _TDSM_Get_Current_Broadcast_Information(CurrentBroadcastInformation);

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is used to update the information being    */
   /* sent in the synchronization broadcasts.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.  The BroadcastInformationUpdate parameter is a pointer to a */
   /* structure which contains the information to update.  This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Update_Broadcast_Information(unsigned int TDSMControlCallbackID, TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((TDSMControlCallbackID))
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check if the callback is registered as control.          */
            if((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoListHead, TDSMControlCallbackID)) != NULL)
            {
               /* Simply submit the command to the server.              */
               ret_val = _TDSM_Update_Broadcast_Information(TDSControlEventsCallbackID, BroadcastInformationUpdate);
            }
            else
               ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the 3D Sync Profile  */
   /* Manager Service.  This Callback will be dispatched by the 3D Sync */
   /* Manager when various 3D Sync Manager events occur.  This function */
   /* accepts the callback function and callback parameter              */
   /* (respectively) to call when a 3D Sync Manager event needs to be   */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TDSM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
   /* * NOTE * Any registered TDSM Callback will get all TDSM events,   */
   /*          but only a callback registered with the Control parameter*/
   /*          set to TRUE can manage the Sync Train and Broadcast      */
   /*          information. There can only be one of these Control      */
   /*          callbacks registered.                                    */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Register_Event_Callback(Boolean_t Control, TDSM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                   ret_val;
   unsigned int         *ServerCallbackID;
   TDSM_Callback_Info_t  CallbackInfo;
   TDSM_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            ServerCallbackID = Control?&TDSControlEventsCallbackID:&TDSEventsCallbackID;

            if((!Control) || (!ControlCallbackInfoListHead))
            {
               /* Attempt to add the entry.                             */
               BTPS_MemInitialize(&CallbackInfo, 0, sizeof(CallbackInfo));

               CallbackInfo.CallbackID        = GetNextCallbackID();
               CallbackInfo.EventCallback     = CallbackFunction;
               CallbackInfo.CallbackParameter = CallbackParameter;

               if((CallbackInfoPtr = AddCallbackInfoEntry(Control?&ControlCallbackInfoListHead:&CallbackInfoListHead, &CallbackInfo)) != NULL)
               {
                  /* If we have not registered with the server, we need */
                  /* to do so.                                          */
                  if(*ServerCallbackID == 0)
                  {
                     /* Attempt to register with the server.            */
                     if((ret_val = _TDSM_Register_Event_Callback(Control)) > 0)
                     {
                        /* Succesfully registered, so note the Server   */
                        /* Callback ID and the Callback ID to return.   */
                        *ServerCallbackID = (unsigned int)ret_val;
                        ret_val           = CallbackInfoPtr->CallbackID;
                     }
                     else
                     {
                        /* Registration failed, so delete the entry.    */
                        if((CallbackInfoPtr = DeleteCallbackInfoEntry(Control?&ControlCallbackInfoListHead:&CallbackInfoListHead, CallbackInfoPtr->CallbackID)) != NULL)
                           FreeCallbackInfoEntryMemory(CallbackInfoPtr);
                     }
                  }
                  else
                  {
                     /* We are registered, so note the Callback ID to   */
                     /* return.                                         */
                     ret_val = CallbackInfoPtr->CallbackID;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_3D_SYNC_CONTROL_ALREADY_REGISTERED;

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered 3D Sync Manager event Callback*/
   /* (registered via a successful call to the                          */
   /* TDSM_Register_Event_Callback() function.  This function accepts as*/
   /* input the 3D Sync Manager event callback ID (return value from the*/
   /* TDSM_Register_Event_Callback() function).                         */
BTPSAPI_DECLARATION void BTPSAPI TDSM_Un_Register_Event_Callback(unsigned int TDSManagerEventCallbackID)
{
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* 3D Sync Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(TDSManagerEventCallbackID)
      {
         /* Attempt to wait for access to the 3D Sync Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(TDSManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the entry in the list.                            */
            if((CallbackInfo = DeleteCallbackInfoEntry(&CallbackInfoListHead, TDSManagerEventCallbackID)) != NULL)
            {
               /* Check if we need to un-register the basic handler.    */
               if(TDSEventsCallbackID)
               {
                  _TDSM_Un_Register_Event_Callback(TDSEventsCallbackID);
                  TDSEventsCallbackID = 0;
               }
            }
            else
            {
               if((CallbackInfo = DeleteCallbackInfoEntry(&ControlCallbackInfoListHead, TDSManagerEventCallbackID)) != NULL)
               {
                  /* Un-register the control.                           */
                  _TDSM_Un_Register_Event_Callback(TDSControlEventsCallbackID);
                  TDSControlEventsCallbackID = 0;
               }
            }

            BTPS_ReleaseMutex(TDSManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}
