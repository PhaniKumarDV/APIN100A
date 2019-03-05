/*****< btpmaudm.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMAUDM - Audio Manager for Stonestreet One Bluetooth Protocol Stack     */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMAUDM.h"            /* BTPM Audio Manager Prototypes/Constants.  */
#include "AUDMMSG.h"             /* BTPM Audio Manager Message Formats.       */
#include "AUDMGR.h"              /* Audio Manager Impl. Prototypes/Constants. */
#include "AUDMUTIL.h"            /* Audio Manager Util. Prototypes/Constants. */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagAudio_Entry_Info_t
{
   unsigned int                   CallbackID;
   unsigned int                   ConnectionStatus;
   Event_t                        ConnectionEvent;
   Boolean_t                      EventCallbackEntry;
   BD_ADDR_t                      RemoteDeviceAddress;
   AUD_Stream_Type_t              StreamType;
   AUDM_Event_Callback_t          EventCallback;
   void                          *CallbackParameter;
   struct _tagAudio_Entry_Info_t *NextAudioEntryInfoPtr;
} Audio_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   AUDM_Event_Callback_t  EventCallback;
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
static Mutex_t AudioManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the current Audio Events Callback ID         */
   /* (registered with the Server to receive events).                   */
static unsigned int AudioEventsCallbackID;

   /* Variable which holds a pointer to the first element in the Audio  */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static Audio_Entry_Info_t *AudioEntryInfoList;

   /* Variable which holds a pointer to the first element of the Audio  */
   /* Entry Information List for Remote Control Event Callbacks         */
   /* registered for outgoing connections (which holds all Remote       */
   /* Control Event Callbacks tracked by this module).                  */
static Audio_Entry_Info_t *OutgoingRemoteControlList;

   /* Variable which holds a pointer to the first element of the Audio  */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static Audio_Entry_Info_t *AudioEntryInfoDataList;

   /* Variable which holds a pointer to the first element of the Audio  */
   /* Entry Information List for Remote Control Event Callbacks (which  */
   /* holds all Remote Control Event Callbacks tracked by this module). */
static Audio_Entry_Info_t *AudioEntryInfoRemoteControlList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static Audio_Entry_Info_t *AddAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, Audio_Entry_Info_t *EntryToAdd);
static Audio_Entry_Info_t *SearchAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID);
static Audio_Entry_Info_t *DeleteAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeAudioEntryInfoEntryMemory(Audio_Entry_Info_t *EntryToFree);
static void FreeAudioEntryInfoList(Audio_Entry_Info_t **ListHead);

static void DispatchAudioEvent(AUDM_Event_Data_t *AUDMEventData);
static void DispatchRemoteControlEvent(AUDM_Event_Data_t *AUDMEventData, unsigned int ServiceType);

static void ProcessIncomingConnectionRequestEvent(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessAudioStreamConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned int ConnectionStatus, unsigned int MediaMTU, AUD_Stream_Format_t *StreamFormat);
static void ProcessAudioStreamDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType);
static void ProcessAudioStreamStateChangedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState);
static void ProcessChangeAudioStreamStateStatusEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState);
static void ProcessAudioStreamFormatChangedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);
static void ProcessChangeAudioStreamFormatStatusEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);
static void ProcessEncodedAudioDataReceivedEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int StreamDataEventsHandlerID, AUD_RTP_Header_Info_t *RTPHeaderInfo, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame);
static void ProcessRemoteControlCommandStatusEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int TransactionID, AVRCP_Message_Type_t MessageType, unsigned int MessageDataLength, unsigned char *MessageData);
static void ProcessRemoteControlCommandReceivedEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AVRCP_Message_Type_t MessageType, unsigned int MessageDataLength, unsigned char *MessageData);
static void ProcessRemoteControlConnectedEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessRemoteControlConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus);
static void ProcessRemoteControlDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Disconnect_Reason_t DisconnectReason);
static void ProcessRemoteControlBrowsingConnectedEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessRemoteControlBrowsingConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus);
static void ProcessRemoteControlBrowsingDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI AudioManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_DEVM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_AUDM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the Audio Entry Information List.                            */
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
static Audio_Entry_Info_t *AddAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, Audio_Entry_Info_t *EntryToAdd)
{
   Audio_Entry_Info_t *AddedEntry = NULL;
   Audio_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Audio_Entry_Info_t *)BTPS_AllocateMemory(sizeof(Audio_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                       = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextAudioEntryInfoPtr = NULL;

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
                     FreeAudioEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextAudioEntryInfoPtr)
                        tmpEntry = tmpEntry->NextAudioEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextAudioEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static Audio_Entry_Info_t *SearchAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   Audio_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextAudioEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Audio Entry         */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the Audio Entry   */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeAudioEntryInfoEntryMemory().                 */
static Audio_Entry_Info_t *DeleteAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   Audio_Entry_Info_t *FoundEntry = NULL;
   Audio_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextAudioEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextAudioEntryInfoPtr = FoundEntry->NextAudioEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextAudioEntryInfoPtr;

         FoundEntry->NextAudioEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Audio Entry Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeAudioEntryInfoEntryMemory(Audio_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Audio Entry Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeAudioEntryInfoList(Audio_Entry_Info_t **ListHead)
{
   Audio_Entry_Info_t *EntryToFree;
   Audio_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Callback ID: 0x%08X\n", EntryToFree->CallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextAudioEntryInfoPtr;

         if((!tmpEntry->EventCallbackEntry) && (tmpEntry->ConnectionEvent))
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeAudioEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Audio event to every registered Audio Event*/
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the Audio Manager    */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Audio Manager Mutex.                                 */
static void DispatchAudioEvent(AUDM_Event_Data_t *AUDMEventData)
{
   unsigned int        Index;
   unsigned int        NumberCallbacks;
   CallbackInfo_t      CallbackInfoArray[16];
   CallbackInfo_t     *CallbackInfoArrayPtr;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((AudioEntryInfoList) || (AudioEntryInfoDataList) || (OutgoingRemoteControlList) || (AudioEntryInfoRemoteControlList)) && (AUDMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      AudioEntryInfo  = AudioEntryInfoList;
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(AudioEntryInfo)
      {
         if(AudioEntryInfo->EventCallback)
            NumberCallbacks++;

         AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* We need to add the Audio Data Entry Information List as well.  */
      AudioEntryInfo  = AudioEntryInfoDataList;
      while(AudioEntryInfo)
      {
         if(AudioEntryInfo->EventCallback)
            NumberCallbacks++;

         AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
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
            AudioEntryInfo  = AudioEntryInfoList;
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(AudioEntryInfo)
            {
               if(AudioEntryInfo->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = AudioEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = AudioEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            /* We need to add the Audio Data Entry Information List as  */
            /* well.                                                    */
            AudioEntryInfo  = AudioEntryInfoDataList;
            while(AudioEntryInfo)
            {
               if(AudioEntryInfo->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = AudioEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = AudioEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(AudioManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(AUDMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(AudioManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Audio event to every registered Audio Event*/
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the Audio Manager    */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Audio Manager Mutex.                                 */
static void DispatchRemoteControlEvent(AUDM_Event_Data_t *AUDMEventData, unsigned int ServiceType)
{
   unsigned int        Index;
   unsigned int        Index1;
   unsigned int        NumberCallbacks;
   CallbackInfo_t      CallbackInfoArray[16];
   CallbackInfo_t     *CallbackInfoArrayPtr;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((AudioEntryInfoList) || (AudioEntryInfoDataList) || (OutgoingRemoteControlList) || (AudioEntryInfoRemoteControlList)) && (AUDMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      /* We need to add the Remote Control Entry Information List as    */
      /* well.                                                          */
      AudioEntryInfo = AudioEntryInfoRemoteControlList;
      while(AudioEntryInfo)
      {
         if(((unsigned int)AudioEntryInfo->ConnectionEvent) & ServiceType)
            NumberCallbacks++;

         AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
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

            /* We need to add the Remote Control Entry Information List */
            /* as well.                                                 */
            AudioEntryInfo = AudioEntryInfoRemoteControlList;
            while(AudioEntryInfo)
            {
               if(((unsigned int)AudioEntryInfo->ConnectionEvent) & ServiceType)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = AudioEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = AudioEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(AudioManagerMutex);

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* Make sure we have not already dispatched this event   */
               /* to this callback.                                     */
               for(Index1=0;Index1<Index;Index1++)
               {
                  if(CallbackInfoArrayPtr[Index1].EventCallback == CallbackInfoArrayPtr[Index].EventCallback)
                     break;
               }

               /* Go ahead and make the callback.                       */
               /* * NOTE * If the callback was deleted (or new ones were*/
               /*          added, they will not be caught for this      */
               /*          message dispatch).  Under normal operating   */
               /*          circumstances this case shouldn't matter     */
               /*          because these groups aren't really dynamic   */
               /*          and are only registered at initialization    */
               /*          time.                                        */

               if(Index1 == Index)
               {
                  __BTPSTRY
                  {
                     if(CallbackInfoArrayPtr[Index].EventCallback)
                     {
                        (*CallbackInfoArrayPtr[Index].EventCallback)(AUDMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
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
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(AudioManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Audio    */
   /* Stream Connected asynchronous message.                            */
static void ProcessAudioStreamConnectedEvent(AUD_Stream_Type_t StreamType, BD_ADDR_t RemoteDeviceAddress, unsigned int MediaMTU, AUD_Stream_Format_t *StreamFormat)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(StreamFormat)
   {
      /* Format up the Event.                                           */
      AUDMEventData.EventType                                                   = aetAudioStreamConnected;
      AUDMEventData.EventLength                                                 = AUDM_AUDIO_STREAM_CONNECTED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.AudioStreamConnectedEventData.StreamType          = StreamType;
      AUDMEventData.EventData.AudioStreamConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      AUDMEventData.EventData.AudioStreamConnectedEventData.MediaMTU            = MediaMTU;
      AUDMEventData.EventData.AudioStreamConnectedEventData.StreamFormat        = *StreamFormat;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchAudioEvent(&AUDMEventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Incoming */
   /* Connection Request asynchronous message.                          */
static void ProcessIncomingConnectionRequestEvent(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                        = aetIncomingConnectionRequest;
   AUDMEventData.EventLength                                                      = AUDM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

   AUDMEventData.EventData.IncomingConnectionRequestEventData.RequestType         = RequestType;
   AUDMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.  If the connection  */
   /* request is for a Remote Control connection, issue the request to  */
   /* registered Remote Control callbacks, first.                       */
   if(RequestType == acrRemoteControl)
      DispatchRemoteControlEvent(&AUDMEventData, (AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER));

   /* For legacy reasons, always dispatch connection requests (of any   */
   /* type) to registered Audio event callbacks.                        */
   DispatchAudioEvent(&AUDMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Audio    */
   /* Stream Connection Status asynchronous message.                    */
static void ProcessAudioStreamConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned int ConnectionStatus, unsigned int MediaMTU, AUD_Stream_Format_t *StreamFormat)
{
   void                  *CallbackParameter;
   Boolean_t              ReleaseMutex;
   AUDM_Event_Data_t      AUDMEventData;
   Audio_Entry_Info_t    *AudioEntryInfo;
   AUDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if((AudioEntryInfoList) && (StreamFormat))
   {
      ReleaseMutex   = TRUE;
      AudioEntryInfo = AudioEntryInfoList;
      while(AudioEntryInfo)
      {
         if((!AudioEntryInfo->EventCallbackEntry) && (AudioEntryInfo->StreamType == StreamType))
         {
            /* Callback registered, now see if the callback is          */
            /* synchronous or asynchronous.                             */
            if(AudioEntryInfo->ConnectionEvent)
            {
               /* Synchronous.                                          */

               /* Note the Status.                                      */
               AudioEntryInfo->ConnectionStatus = ConnectionStatus;

               /* Set the Event.                                        */
               BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);

               /* Break out of the list.                                */
               AudioEntryInfo = NULL;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(AudioManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;
            }
            else
            {
               /* Asynchronous Entry, go ahead dispatch the result.     */

               /* Format up the Event.                                  */
               AUDMEventData.EventType                                                          = aetAudioStreamConnectionStatus;
               AUDMEventData.EventLength                                                        = AUDM_AUDIO_STREAM_CONNECTION_STATUS_EVENT_DATA_SIZE;

               AUDMEventData.EventData.AudioStreamConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
               AUDMEventData.EventData.AudioStreamConnectionStatusEventData.StreamType          = StreamType;
               AUDMEventData.EventData.AudioStreamConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;
               AUDMEventData.EventData.AudioStreamConnectionStatusEventData.MediaMTU            = MediaMTU;
               AUDMEventData.EventData.AudioStreamConnectionStatusEventData.StreamFormat        = *StreamFormat;

               /* Note the Callback information.                        */
               EventCallback     = AudioEntryInfo->EventCallback;
               CallbackParameter = AudioEntryInfo->CallbackParameter;

               if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfo->CallbackID)) != NULL)
                  FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

               /* Release the Mutex so we can dispatch the event.       */
               BTPS_ReleaseMutex(AudioManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&AUDMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Break out of the list.                                */
               AudioEntryInfo = NULL;
            }
         }
         else
            AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* If the Mutex was not released, then we need to make sure we    */
      /* release it.                                                    */
      if(ReleaseMutex)
         BTPS_ReleaseMutex(AudioManagerMutex);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Audio    */
   /* Stream Disconnected asynchronous message.                         */
static void ProcessAudioStreamDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                      = aetAudioStreamDisconnected;
   AUDMEventData.EventLength                                                    = AUDM_AUDIO_STREAM_DISCONNECTED_EVENT_DATA_SIZE;

   AUDMEventData.EventData.AudioStreamDisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   AUDMEventData.EventData.AudioStreamDisconnectedEventData.StreamType          = StreamType;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchAudioEvent(&AUDMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Audio    */
   /* Stream State Changed asynchronous message.                        */
static void ProcessAudioStreamStateChangedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                      = aetAudioStreamStateChanged;
   AUDMEventData.EventLength                                                    = AUDM_AUDIO_STREAM_STATE_CHANGED_EVENT_DATA_SIZE;

   AUDMEventData.EventData.AudioStreamStateChangedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   AUDMEventData.EventData.AudioStreamStateChangedEventData.StreamType          = StreamType;
   AUDMEventData.EventData.AudioStreamStateChangedEventData.StreamState         = StreamState;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchAudioEvent(&AUDMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Change   */
   /* Audio Stream State Status asynchronous message.                   */
static void ProcessChangeAudioStreamStateStatusEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                           = aetChangeAudioStreamStateStatus;
   AUDMEventData.EventLength                                                         = AUDM_CHANGE_AUDIO_STREAM_STATE_STATUS_EVENT_DATA_SIZE;

   AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.Successful          = Successful;
   AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.StreamType          = StreamType;
   AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.StreamState         = StreamState;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchAudioEvent(&AUDMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Audio    */
   /* Stream Format Changed asynchronous message.                       */
static void ProcessAudioStreamFormatChangedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   if(StreamFormat)
   {
      AUDMEventData.EventType                                                       = aetAudioStreamFormatChanged;
      AUDMEventData.EventLength                                                     = AUDM_AUDIO_STREAM_FORMAT_CHANGED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.AudioStreamFormatChangedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      AUDMEventData.EventData.AudioStreamFormatChangedEventData.StreamType          = StreamType;
      AUDMEventData.EventData.AudioStreamFormatChangedEventData.StreamFormat        = *StreamFormat;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchAudioEvent(&AUDMEventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Change   */
   /* Audio Stream Format Status asynchronous message.                  */
static void ProcessChangeAudioStreamFormatStatusEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   if(StreamFormat)
   {
      AUDMEventData.EventType                                                            = aetChangeAudioStreamFormatStatus;
      AUDMEventData.EventLength                                                          = AUDM_CHANGE_AUDIO_STREAM_FORMAT_STATUS_EVENT_DATA_SIZE;

      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.Successful          = Successful;
      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.StreamType          = StreamType;
      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.StreamFormat        = *StreamFormat;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchAudioEvent(&AUDMEventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Encoded  */
   /* Audio Data Received asynchronous message.                         */
static void ProcessEncodedAudioDataReceivedEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int StreamDataEventsHandlerID, AUD_RTP_Header_Info_t *RTPHeaderInfo, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame)
{
   void                  *CallbackParameter;
   AUDM_Event_Data_t      AUDMEventData;
   Audio_Entry_Info_t    *AudioEntryInfo;
   AUDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   if((RawAudioDataFrameLength) && (RawAudioDataFrame))
   {
      AUDMEventData.EventType                                                           = aetEncodedAudioStreamData;
      AUDMEventData.EventLength                                                         = AUDM_ENCODED_AUDIO_STREAM_DATA_EVENT_DATA_SIZE;

      AUDMEventData.EventData.EncodedAudioStreamDataEventData.RemoteDeviceAddress       = RemoteDeviceAddress;
      AUDMEventData.EventData.EncodedAudioStreamDataEventData.StreamDataEventsHandlerID = 0;
      AUDMEventData.EventData.EncodedAudioStreamDataEventData.RawAudioDataFrameLength   = RawAudioDataFrameLength;
      AUDMEventData.EventData.EncodedAudioStreamDataEventData.RawAudioDataFrame         = RawAudioDataFrame;

      /* Check if the caller specified RTP Header Information.          */
      if(RTPHeaderInfo)
      {
         /* RTP Header Information was specified, note the RTP Header   */
         /* Information.                                                */
         AUDMEventData.EventData.EncodedAudioStreamDataEventData.RTPHeaderInfo = RTPHeaderInfo;
      }
      else
      {
         /* RTP Header Information was specified, set the RTP Header    */
         /* Info pointer to NULL.                                       */
         AUDMEventData.EventData.EncodedAudioStreamDataEventData.RTPHeaderInfo = NULL;
      }

      /* Now that the event is formatted, dispatch it.                  */

      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      AudioEntryInfo = AudioEntryInfoDataList;
      while(AudioEntryInfo)
      {
         if((AudioEntryInfo->ConnectionStatus == StreamDataEventsHandlerID) && (AudioEntryInfo->StreamType == astSNK))
            break;
         else
            AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* Determine if we found an Audio Entry Event Handler.            */
      if(AudioEntryInfo)
      {
         /* Note the Callback Information.                              */
         EventCallback                                                                     = AudioEntryInfo->EventCallback;
         CallbackParameter                                                                 = AudioEntryInfo->CallbackParameter;

         /* We need to map the Stream Events Handler ID from the Server */
         /* ID to the Client ID.                                        */
         AUDMEventData.EventData.EncodedAudioStreamDataEventData.StreamDataEventsHandlerID = AudioEntryInfo->CallbackID;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(AudioManagerMutex);

         __BTPSTRY
         {
            (*EventCallback)(&AUDMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(AudioManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Remote   */
   /* Control Command Status asynchronous message.                      */
static void ProcessRemoteControlCommandStatusEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int TransactionID, AVRCP_Message_Type_t MessageType, unsigned int MessageDataLength, unsigned char *MessageData)
{
   AUDM_Event_Data_t                AUDMEventData;
   RemoteControlDecodeInformation_t DecodeInformation;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(((MessageDataLength) && (MessageData)) || (Status))
   {
      /* Check if this is an error message, and if it's not convert the */
      /* stream to a decoded response event structure.                  */
      if((Status) || (!ConvertStreamAVRCPResponseToDecoded(MessageType, MessageDataLength, MessageData, &DecodeInformation, &(AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.RemoteControlResponseData))))
      {
         /* Either this is an error command confirmation response or the*/
         /* message has been converted, either way build the event and  */
         /* dispatch it.                                                */
         AUDMEventData.EventType                                                               = aetRemoteControlCommandConfirmation;
         AUDMEventData.EventLength                                                             = AUDM_REMOTE_CONTROL_COMMAND_CONFIRMATION_EVENT_DATA_SIZE;

         AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.TransactionID       = TransactionID;
         AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.Status              = Status;

         /* Now that the event is formatted, dispatch it.               */
         DispatchRemoteControlEvent(&AUDMEventData, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER);

         /* Check if this was a successful command confirmation         */
         /* response.                                                   */
         if(!Status)
         {
            /* This was a successful command confirmation response and  */
            /* we need to free any resource allocated when the stream   */
            /* was converted to a decoded structure.                    */
            FreeAVRCPDecodedResponse(&DecodeInformation);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(AudioManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Remote   */
   /* Control Command Received asynchronous message.                    */
static void ProcessRemoteControlCommandReceivedEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AVRCP_Message_Type_t MessageType, unsigned int MessageDataLength, unsigned char *MessageData)
{
   AUDM_Event_Data_t                AUDMEventData;
   RemoteControlDecodeInformation_t DecodeInformation;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if((MessageDataLength) && (MessageData))
   {
      /* Convert the message from it's stream format to the format      */
      /* required for the real Audio Manager (AVRCP Parsed).            */
      if(!ConvertStreamAVRCPCommandToDecoded(MessageType, MessageDataLength, MessageData, &DecodeInformation, &(AUDMEventData.EventData.RemoteControlCommandIndicationEventData.RemoteControlCommandData)))
      {
         /* Message has been converted, now build the Event and dispatch*/
         /* it.                                                         */
         AUDMEventData.EventType                                                             = aetRemoteControlCommandIndication;
         AUDMEventData.EventLength                                                           = AUDM_REMOTE_CONTROL_COMMAND_INDICATION_EVENT_DATA_SIZE;

         AUDMEventData.EventData.RemoteControlCommandIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         AUDMEventData.EventData.RemoteControlCommandIndicationEventData.TransactionID       = TransactionID;

         /* Now that the event is formatted, dispatch it.               */
         DispatchRemoteControlEvent(&AUDMEventData, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);

         // todo: comments.
         FreeAVRCPDecodedCommand(&DecodeInformation);
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(AudioManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Remote   */
   /* Control Connected asynchronous message.                           */
static void ProcessRemoteControlConnectedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                     = aetRemoteControlConnected;
   AUDMEventData.EventLength                                                   = AUDM_REMOTE_CONTROL_CONNECTED_EVENT_DATA_SIZE;

   AUDMEventData.EventData.RemoteControlConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchRemoteControlEvent(&AUDMEventData, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Remote   */
   /* Control Connection Status asynchronous message.                   */
static void ProcessRemoteControlConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus)
{
   void                  *CallbackParameter;
   Boolean_t              ReleaseMutex;
   AUDM_Event_Data_t      AUDMEventData;
   Audio_Entry_Info_t    *AudioEntryInfo;
   AUDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if(OutgoingRemoteControlList)
   {
      ReleaseMutex   = TRUE;
      AudioEntryInfo = OutgoingRemoteControlList;
      while(AudioEntryInfo)
      {
         if((!AudioEntryInfo->EventCallbackEntry) && (COMPARE_BD_ADDR(AudioEntryInfo->RemoteDeviceAddress, RemoteDeviceAddress)))
         {
            /* Callback registered, now see if the callback is          */
            /* synchronous or asynchronous.                             */
            if(AudioEntryInfo->ConnectionEvent)
            {
               /* Synchronous.                                          */

               /* Note the Status.                                      */
               AudioEntryInfo->ConnectionStatus = ConnectionStatus;

               /* Set the Event.                                        */
               BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);

               /* Break out of the list.                                */
               AudioEntryInfo = NULL;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(AudioManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;
            }
            else
            {
               /* Asynchronous Entry, go ahead dispatch the result.     */

               /* Format up the Event.                                  */
               AUDMEventData.EventType                                                            = aetRemoteControlConnectionStatus;
               AUDMEventData.EventLength                                                          = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_EVENT_DATA_SIZE;

               AUDMEventData.EventData.RemoteControlConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
               AUDMEventData.EventData.RemoteControlConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

               /* Note the Callback information.                        */
               EventCallback     = AudioEntryInfo->EventCallback;
               CallbackParameter = AudioEntryInfo->CallbackParameter;

               if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo->CallbackID)) != NULL)
                  FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

               /* Release the Mutex so we can dispatch the event.       */
               BTPS_ReleaseMutex(AudioManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&AUDMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Break out of the list.                                */
               AudioEntryInfo = NULL;
            }
         }
         else
            AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* If the Mutex was not released, then we need to make sure we    */
      /* release it.                                                    */
      if(ReleaseMutex)
         BTPS_ReleaseMutex(AudioManagerMutex);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Remote   */
   /* Control Disconnected asynchronous message.                        */
static void ProcessRemoteControlDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Disconnect_Reason_t DisconnectReason)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                        = aetRemoteControlDisconnected;
   AUDMEventData.EventLength                                                      = AUDM_REMOTE_CONTROL_DISCONNECTED_EVENT_DATA_SIZE;

   AUDMEventData.EventData.RemoteControlDisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   AUDMEventData.EventData.RemoteControlDisconnectedEventData.DisconnectReason    = DisconnectReason;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchRemoteControlEvent(&AUDMEventData, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessRemoteControlBrowsingConnectedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                             = aetRemoteControlBrowsingConnected;
   AUDMEventData.EventLength                                                           = AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_EVENT_DATA_SIZE;

   AUDMEventData.EventData.RemoteControlBrowsingConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchRemoteControlEvent(&AUDMEventData, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessRemoteControlBrowsingConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus)
{
   void                  *CallbackParameter;
   Boolean_t              ReleaseMutex;
   AUDM_Event_Data_t      AUDMEventData;
   Audio_Entry_Info_t    *AudioEntryInfo;
   AUDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if(OutgoingRemoteControlList)
   {
      ReleaseMutex   = TRUE;
      AudioEntryInfo = OutgoingRemoteControlList;
      while(AudioEntryInfo)
      {
         if((!AudioEntryInfo->EventCallbackEntry) && (COMPARE_BD_ADDR(AudioEntryInfo->RemoteDeviceAddress, RemoteDeviceAddress)))
         {
            /* Callback registered, now see if the callback is          */
            /* synchronous or asynchronous.                             */
            if(AudioEntryInfo->ConnectionEvent)
            {
               /* Synchronous.                                          */

               /* Note the Status.                                      */
               AudioEntryInfo->ConnectionStatus = ConnectionStatus;

               /* Set the Event.                                        */
               BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);

               /* Break out of the list.                                */
               AudioEntryInfo = NULL;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(AudioManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;
            }
            else
            {
               /* Asynchronous Entry, go ahead dispatch the result.     */

               /* Format up the Event.                                  */
               AUDMEventData.EventType                                                                    = aetRemoteControlBrowsingConnectionStatus;
               AUDMEventData.EventLength                                                                  = AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_EVENT_DATA_SIZE;

               AUDMEventData.EventData.RemoteControlBrowsingConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
               AUDMEventData.EventData.RemoteControlBrowsingConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

               /* Note the Callback information.                        */
               EventCallback     = AudioEntryInfo->EventCallback;
               CallbackParameter = AudioEntryInfo->CallbackParameter;

               if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo->CallbackID)) != NULL)
                  FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

               /* Release the Mutex so we can dispatch the event.       */
               BTPS_ReleaseMutex(AudioManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&AUDMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Break out of the list.                                */
               AudioEntryInfo = NULL;
            }
         }
         else
            AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* If the Mutex was not released, then we need to make sure we    */
      /* release it.                                                    */
      if(ReleaseMutex)
         BTPS_ReleaseMutex(AudioManagerMutex);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(AudioManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessRemoteControlBrowsingDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   AUDM_Event_Data_t AUDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   AUDMEventData.EventType                                                                = aetRemoteControlBrowsingDisconnected;
   AUDMEventData.EventLength                                                              = AUDM_REMOTE_CONTROL_BROWSING_DISCONNECTED_EVENT_DATA_SIZE;

   AUDMEventData.EventData.RemoteControlBrowsingDisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchRemoteControlEvent(&AUDMEventData, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Audio Manager    */
   /*          Mutex held.  This function will release the Mutex before */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX). */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessIncomingConnectionRequestEvent(((AUDM_Connection_Request_Message_t *)Message)->RequestType, ((AUDM_Connection_Request_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Stream Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_AUDIO_STREAM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Audio Stream Connected Event.                         */
               ProcessAudioStreamConnectedEvent(((AUDM_Audio_Stream_Connected_Message_t *)Message)->StreamType, ((AUDM_Audio_Stream_Connected_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Audio_Stream_Connected_Message_t *)Message)->MediaMTU, &(((AUDM_Audio_Stream_Connected_Message_t *)Message)->StreamFormat));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Stream Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_AUDIO_STREAM_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Audio Stream Connection Status Event.                 */
               ProcessAudioStreamConnectionStatusEvent(((AUDM_Audio_Stream_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Audio_Stream_Connection_Status_Message_t *)Message)->StreamType, ((AUDM_Audio_Stream_Connection_Status_Message_t *)Message)->ConnectionStatus, ((AUDM_Audio_Stream_Connection_Status_Message_t *)Message)->MediaMTU, &(((AUDM_Audio_Stream_Connection_Status_Message_t *)Message)->StreamFormat));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Stream Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_AUDIO_STREAM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Audio Stream Disconnected Event.                      */
               ProcessAudioStreamDisconnectedEvent(((AUDM_Audio_Stream_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Audio_Stream_Disconnected_Message_t *)Message)->StreamType);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_STATE_CHANGED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Stream State Changed Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_AUDIO_STREAM_STATE_CHANGED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Audio Stream State Changed Event.                     */
               ProcessAudioStreamStateChangedEvent(((AUDM_Audio_Stream_State_Changed_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Audio_Stream_State_Changed_Message_t *)Message)->StreamType, ((AUDM_Audio_Stream_State_Changed_Message_t *)Message)->StreamState);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Audio Stream State Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CHANGE_AUDIO_STREAM_STATE_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Change Audio Stream State Status Event.               */
               ProcessChangeAudioStreamStateStatusEvent(((AUDM_Change_Audio_Stream_State_Status_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Change_Audio_Stream_State_Status_Message_t *)Message)->Successful, ((AUDM_Change_Audio_Stream_State_Status_Message_t *)Message)->StreamType, ((AUDM_Change_Audio_Stream_State_Status_Message_t *)Message)->StreamState);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_FORMAT_CHANGED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Stream Format Changed Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_AUDIO_STREAM_FORMAT_CHANGED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Audio Stream Format Changed Event.                    */
               ProcessAudioStreamFormatChangedEvent(((AUDM_Audio_Stream_Format_Changed_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Audio_Stream_Format_Changed_Message_t *)Message)->StreamType, &(((AUDM_Audio_Stream_Format_Changed_Message_t *)Message)->StreamFormat));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT_STAT:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Audio Stream Format Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CHANGE_AUDIO_STREAM_FORMAT_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Change Audio Stream Format Status Event.              */
               ProcessChangeAudioStreamFormatStatusEvent(((AUDM_Change_Audio_Stream_Format_Status_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Change_Audio_Stream_Format_Status_Message_t *)Message)->Successful, ((AUDM_Change_Audio_Stream_Format_Status_Message_t *)Message)->StreamType, &(((AUDM_Change_Audio_Stream_Format_Status_Message_t *)Message)->StreamFormat));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_ENCODED_AUDIO_DATA_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Encoded Audio Data Received Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(((AUDM_Encoded_Audio_Data_Received_Message_t *)Message)->RawAudioDataFrameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Encoded Audio Data Received Event.                    */
               ProcessEncodedAudioDataReceivedEvent(((AUDM_Encoded_Audio_Data_Received_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Encoded_Audio_Data_Received_Message_t *)Message)->StreamDataEventsHandlerID, NULL, ((AUDM_Encoded_Audio_Data_Received_Message_t *)Message)->RawAudioDataFrameLength, ((AUDM_Encoded_Audio_Data_Received_Message_t *)Message)->RawAudioDataFrame);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_RTP_ENCODED_AUDIO_DATA_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("RTP Encoded Audio Data Received Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_RTP_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_RTP_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(((AUDM_RTP_Encoded_Audio_Data_Received_Message_t *)Message)->DataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Encoded Audio Data Received Event.                    */
               ProcessEncodedAudioDataReceivedEvent(((AUDM_RTP_Encoded_Audio_Data_Received_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_RTP_Encoded_Audio_Data_Received_Message_t *)Message)->StreamDataEventsHandlerID, &(((AUDM_RTP_Encoded_Audio_Data_Received_Message_t *)Message)->RTPHeaderInfo), ((AUDM_RTP_Encoded_Audio_Data_Received_Message_t *)Message)->DataLength, ((AUDM_RTP_Encoded_Audio_Data_Received_Message_t *)Message)->Data);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Command Status Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_COMMAND_STATUS_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_COMMAND_STATUS_MESSAGE_SIZE(((AUDM_Remote_Control_Command_Status_Message_t *)Message)->MessageDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Command Status Event.                  */
               ProcessRemoteControlCommandStatusEvent(((AUDM_Remote_Control_Command_Status_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Remote_Control_Command_Status_Message_t *)Message)->Status, ((AUDM_Remote_Control_Command_Status_Message_t *)Message)->TransactionID, ((AUDM_Remote_Control_Command_Status_Message_t *)Message)->MessageType, ((AUDM_Remote_Control_Command_Status_Message_t *)Message)->MessageDataLength, ((AUDM_Remote_Control_Command_Status_Message_t *)Message)->MessageData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Command Received Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_COMMAND_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_COMMAND_RECEIVED_MESSAGE_SIZE(((AUDM_Remote_Control_Command_Received_Message_t *)Message)->MessageDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Command Received Event.                */
               ProcessRemoteControlCommandReceivedEvent(((AUDM_Remote_Control_Command_Received_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Remote_Control_Command_Received_Message_t *)Message)->TransactionID, ((AUDM_Remote_Control_Command_Received_Message_t *)Message)->MessageType, ((AUDM_Remote_Control_Command_Received_Message_t *)Message)->MessageDataLength, ((AUDM_Remote_Control_Command_Received_Message_t *)Message)->MessageData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Connected Event.                       */
               ProcessRemoteControlConnectedEvent(((AUDM_Remote_Control_Connected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Connection Status Event.               */
               ProcessRemoteControlConnectionStatusEvent(((AUDM_Remote_Control_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Remote_Control_Connection_Status_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Disconnected Event.                    */
               ProcessRemoteControlDisconnectedEvent(((AUDM_Remote_Control_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Remote_Control_Disconnected_Message_t *)Message)->DisconnectReason);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Browsing Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Browsing Connected Event.              */
               ProcessRemoteControlBrowsingConnectedEvent(((AUDM_Remote_Control_Browsing_Connected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Browsing Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Browsing Connection Status Event.      */
               ProcessRemoteControlBrowsingConnectionStatusEvent(((AUDM_Remote_Control_Browsing_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((AUDM_Remote_Control_Browsing_Connection_Status_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Browsing Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REMOTE_CONTROL_BROWSING_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Remote Control Browsing Disconnected Event.           */
               ProcessRemoteControlBrowsingDisconnectedEvent(((AUDM_Remote_Control_Browsing_Disconnected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(AudioManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Local Device Manager Asynchronous Events.   */
static void BTPSAPI BTPMDispatchCallback_DEVM(void *CallbackParameter)
{
   unsigned int        Index;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Power Dispatch Callback.                                          */

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Attempt to wait for access to the Audio state information.     */
      if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Verify that this is flagging a power off event.             */
         if(!CallbackParameter)
         {
            /* Power off event, let's loop through ALL the registered   */
            /* Audio Entries and set any events that have synchronous   */
            /* operations pending.                                      */
            Index          = 2;
            AudioEntryInfo = AudioEntryInfoList;
            while(Index)
            {
               while(AudioEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((!AudioEntryInfo->EventCallbackEntry) && (AudioEntryInfo->ConnectionEvent))
                  {
                     AudioEntryInfo->ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);
                  }

                  AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
               }

               --Index;

               if(Index == 1)
                  AudioEntryInfo = OutgoingRemoteControlList;
            }
         }
         else
         {
            /* Power on Event.                                          */

            /* There is nothing really to do here.                      */
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(AudioManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Audio Manager Asynchronous Events.          */
static void BTPSAPI BTPMDispatchCallback_AUDM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Audio state information.  */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   unsigned int        Index;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Audio Manager state       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            Index          = 2;
            AudioEntryInfo = AudioEntryInfoList;
            while(Index)
            {
               while(AudioEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((!AudioEntryInfo->EventCallbackEntry) && (AudioEntryInfo->ConnectionEvent))
                  {
                     AudioEntryInfo->ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);
                  }

                  AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
               }

               --Index;

               if(Index == 1)
                  AudioEntryInfo = OutgoingRemoteControlList;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Audio Manager Messages. */
static void BTPSAPI AudioManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_AUDIO_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is an Audio Manager defined */
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
               /* Audio Manager Thread.                                 */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_AUDM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Audio Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Audio Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Audio Manager defined */
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
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Audio Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Audio Manager Module.  This        */
   /* function should be registered with the Bluetopia Platform Manager */
   /* Module Handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI AUDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                 Result;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Audio Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((AudioManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Audio Manager messages.                       */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_AUDIO_MANAGER, AudioManagerGroupHandler, NULL))
            {
               /* Initialize the actual Audio Manager Implementation    */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the Audio       */
               /* Manager functionality - this module is just the       */
               /* framework shell).                                     */
               if(!(Result = _AUDM_Initialize()))
               {
                  /* Finally determine the current Device Power State.  */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the Audio Manager       */
                  /* Server.                                            */
                  Result = _AUDM_Register_Stream_Events();

                  if(Result > 0)
                  {
                     if(Result > 0)
                        AudioEventsCallbackID = (unsigned int)Result;

                     Result         = 0;

                     /* Initialize a unique, starting Audio Callback ID.*/
                     NextCallbackID = 0x000000001;

                     /* Go ahead and flag that this module is           */
                     /* initialized.                                    */
                     Initialized    = TRUE;
                  }
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _AUDM_Cleanup();

            if(AudioEventsCallbackID)
               _AUDM_Un_Register_Stream_Events(AudioEventsCallbackID);

            if(AudioManagerMutex)
               BTPS_CloseMutex(AudioManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_AUDIO_MANAGER);

            /* Flag that none of the resources are allocated.           */
            AudioManagerMutex     = NULL;
            AudioEventsCallbackID = 0;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_AUDIO_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            if(AudioEventsCallbackID)
               _AUDM_Un_Register_Stream_Events(AudioEventsCallbackID);

            /* We now need to make sure that we Un-Register any data    */
            /* events.                                                  */
            AudioEntryInfo = AudioEntryInfoDataList;
            while(AudioEntryInfo)
            {
               /* Entry found, go ahead and Un-Register.                */
               _AUDM_Un_Register_Stream_Data_Events(AudioEntryInfo->ConnectionStatus);

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            AudioEntryInfo = AudioEntryInfoRemoteControlList;
            while(AudioEntryInfo)
            {
               /* Entry found, go ahead and Un-Register.                */
               _AUDM_Un_Register_Stream_Data_Events(AudioEntryInfo->ConnectionStatus);

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            /* Make sure we inform the Audio Manager Implementation that*/
            /* we are shutting down.                                    */
            _AUDM_Cleanup();

            BTPS_CloseMutex(AudioManagerMutex);

            /* Make sure that the Audio Entry Information List is empty.*/
            FreeAudioEntryInfoList(&AudioEntryInfoList);

            /* Make sure that the Audio Entry Data Information List is  */
            /* empty.                                                   */
            FreeAudioEntryInfoList(&AudioEntryInfoDataList);

            /* Make sure that the list for outgoing Remote Control      */
            /* connections is empty.                                    */
            FreeAudioEntryInfoList(&OutgoingRemoteControlList);

            /* Make sure that the Audio Entry Remote Control Information*/
            /* List is empty.                                           */
            FreeAudioEntryInfoList(&AudioEntryInfoRemoteControlList);

            /* Flag that the resources are no longer allocated.         */
            AudioManagerMutex     = NULL;
            CurrentPowerState     = FALSE;
            AudioEventsCallbackID = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized           = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI AUDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      switch(EventData->EventType)
      {
         case detDevicePoweredOn:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

            /* Note the new Power State.                                */
            CurrentPowerState = TRUE;

            BTPM_QueueMailboxCallback(BTPMDispatchCallback_DEVM, (void *)TRUE);
            break;
         case detDevicePoweringOff:
         case detDevicePoweredOff:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

            /* Note the new Power State.                                */
            CurrentPowerState = FALSE;

            BTPM_QueueMailboxCallback(BTPMDispatchCallback_DEVM, (void *)FALSE);
            break;
         default:
            /* Do nothing.                                              */
            break;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Audio Stream or*/
   /* Remote Control connection.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  An Audio Stream   */
   /*          Connected event will be dispatched to signify the actual */
   /*          result.                                                  */
int BTPSAPI AUDM_Connection_Request_Response(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Respond to the Connection Request.                             */
      ret_val = _AUDM_Connection_Request_Response(RequestType, RemoteDeviceAddress, Accept);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI AUDM_Connect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned long StreamFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   BD_ADDR_t           NULL_BD_ADDR;
   unsigned int        CallbackID;
   Audio_Entry_Info_t  AudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfoPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* Audio Entry list.                                     */
               BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

               AudioEntryInfo.CallbackID        = GetNextCallbackID();
               AudioEntryInfo.EventCallback     = CallbackFunction;
               AudioEntryInfo.CallbackParameter = CallbackParameter;
               AudioEntryInfo.StreamType        = StreamType;

               if(ConnectionStatus)
                  AudioEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

               if((!ConnectionStatus) || ((ConnectionStatus) && (AudioEntryInfo.ConnectionEvent)))
               {
                  if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&AudioEntryInfoList, &AudioEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Stream %d, 0x%08lX\n", (unsigned int)StreamType, StreamFlags));

                     /* Next, attempt to open the remote stream.        */
                     if((ret_val = _AUDM_Connect_Audio_Stream(RemoteDeviceAddress, StreamType, StreamFlags)) != 0)
                     {
                        /* Error opening stream, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfoPtr->CallbackID)) != NULL)
                        {
                           if(AudioEntryInfoPtr->ConnectionEvent)
                              BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                           FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                        }
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((!ret_val) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Callback ID.                        */
                        CallbackID      = AudioEntryInfoPtr->CallbackID;

                        /* Note the Open Event.                         */
                        ConnectionEvent = AudioEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(AudioManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, CallbackID)) != NULL)
                           {
                              /* Note the connection status.            */
                              *ConnectionStatus = AudioEntryInfoPtr->ConnectionStatus;

                              BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);

                              /* Flag success to the caller.            */
                              ret_val = 0;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_AUDIO_STREAM;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                     }
                     else
                     {
                        /* If we are not tracking this connection OR    */
                        /* there was an error, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((!CallbackFunction) || (ret_val))
                        {
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfo.CallbackID)) != NULL)
                           {
                              if(AudioEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                           }
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
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Audio Stream.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI AUDM_Disconnect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Disconnect the Audio Stream.                                   */
      ret_val = _AUDM_Disconnect_Audio_Stream(RemoteDeviceAddress, StreamType);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI AUDM_Query_Audio_Connected_Devices(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Query the connected audio devices.                             */
      ret_val = _AUDM_Query_Audio_Connected_Devices(StreamType, MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream state of the     */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream State of the Audio Stream (if*/
   /* this function is successful).                                     */
int BTPSAPI AUDM_Query_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t *StreamState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if(StreamState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Query the Audio Stream State.                               */
         ret_val = _AUDM_Query_Audio_Stream_State(RemoteDeviceAddress, StreamType, StreamState);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream format of the    */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream Format of the Audio Stream   */
   /* (if this function is successful).                                 */
int BTPSAPI AUDM_Query_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if(StreamFormat)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Query the Audio Stream Format.                              */
         ret_val = _AUDM_Query_Audio_Stream_Format(RemoteDeviceAddress, StreamType, StreamFormat);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to start/suspend the specified Audio Stream.  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI AUDM_Change_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Start/Suspend the Audio Stream.                                */
      ret_val = _AUDM_Change_Audio_Stream_State(RemoteDeviceAddress, StreamType, StreamState);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the stream format of a currently connected (but */
   /* suspended) Audio Stream.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The stream format can ONLY be changed when the stream    */
   /*          state is stopped.                                        */
int BTPSAPI AUDM_Change_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* change the Audio Stream Format.                                */
      ret_val = _AUDM_Change_Audio_Stream_Format(RemoteDeviceAddress, StreamType, StreamFormat);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream configuration of */
   /* the specified Audio Stream.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* The final parameter will hold the Audio Stream Configuration of   */
   /* the Audio Stream (if this function is successful).                */
int BTPSAPI AUDM_Query_Audio_Stream_Configuration(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Configuration_t *StreamConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if(StreamConfiguration)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Query the Audio Stream Configuration.                       */
         ret_val = _AUDM_Query_Audio_Stream_Configuration(RemoteDeviceAddress, StreamType, StreamConfiguration);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for Audio Manager */
   /* Connections (Audio Streams and Remote Control).  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI AUDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* update the Incoming Connection Flags.                          */
      ret_val = _AUDM_Change_Incoming_Connection_Flags(ConnectionFlags);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI AUDM_Send_Encoded_Audio_Data(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerDataEventCallbackID)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoDataList, AudioManagerDataEventCallbackID)) != NULL)
            {
               /* Double check that the type is an Audio Source.        */
               if(AudioEntryInfo->StreamType == astSRC)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Encoded Audio Data.           */
                  ret_val = _AUDM_Send_Encoded_Audio_Data(RemoteDeviceAddress, AudioEntryInfo->ConnectionStatus, RawAudioDataFrameLength, RawAudioDataFrame);
               }
               else
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

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
int BTPSAPI AUDM_Send_RTP_Encoded_Audio_Data(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerDataEventCallbackID)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoDataList, AudioManagerDataEventCallbackID)) != NULL)
            {
               /* Double check that the type is an Audio Source.        */
               if(AudioEntryInfo->StreamType == astSRC)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Encoded Audio Data.           */
                  ret_val = _AUDM_Send_RTP_Encoded_Audio_Data(RemoteDeviceAddress, AudioEntryInfo->ConnectionStatus, RawAudioDataFrameLength, RawAudioDataFrame, Flags, RTPHeaderInfo);
               }
               else
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

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
int BTPSAPI AUDM_Connect_Remote_Control(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   BD_ADDR_t           NULL_BD_ADDR;
   unsigned int        CallbackID;
   Audio_Entry_Info_t  AudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfoPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* Audio Entry list.                                     */
               BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

               AudioEntryInfo.CallbackID          = GetNextCallbackID();
               AudioEntryInfo.EventCallback       = CallbackFunction;
               AudioEntryInfo.CallbackParameter   = CallbackParameter;
               AudioEntryInfo.RemoteDeviceAddress = RemoteDeviceAddress;

               if(ConnectionStatus)
                  AudioEntryInfo.ConnectionEvent  = BTPS_CreateEvent(FALSE);

               if((!ConnectionStatus) || ((ConnectionStatus) && (AudioEntryInfo.ConnectionEvent)))
               {
                  if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&OutgoingRemoteControlList, &AudioEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Control 0x%08lX\n", ConnectionFlags));

                     /* Next, attempt to open the remote stream.        */
                     ret_val = _AUDM_Connect_Remote_Control(RemoteDeviceAddress, ConnectionFlags);

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((!ret_val) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Callback ID.                        */
                        CallbackID      = AudioEntryInfoPtr->CallbackID;

                        /* Note the Open Event.                         */
                        ConnectionEvent = AudioEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(AudioManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, CallbackID)) != NULL)
                           {
                              /* Note the connection status.            */
                              *ConnectionStatus = AudioEntryInfoPtr->ConnectionStatus;

                              BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);

                              /* Flag success to the caller.            */
                              ret_val = 0;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_CONTROL;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                     }
                     else
                     {
                        /* If we are not tracking this connection OR    */
                        /* there was an error, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((!CallbackFunction) || (ret_val))
                        {
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo.CallbackID)) != NULL)
                           {
                              if(AudioEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                           }
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
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Remote Control        */
   /* session.  This function returns zero if successful, or a negative */
   /* return error code if there was an error.                          */
int BTPSAPI AUDM_Disconnect_Remote_Control(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Disconnect the Audio Stream.                                   */
      ret_val = _AUDM_Disconnect_Remote_Control(RemoteDeviceAddress);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Remote  */
   /* Control Target or Controller sessions (specified by the first     */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer). The next parameter is       */
   /* optional and, if specified, will be populated with the total      */
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
int BTPSAPI AUDM_Query_Remote_Control_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Query the connected remote control devices.                    */
      ret_val = _AUDM_Query_Remote_Control_Connected_Devices(MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI AUDM_Send_Remote_Control_Command(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerRemoteControlEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, AudioManagerRemoteControlEventCallbackID)) != NULL)
            {
               /* Double check that the type is supported.              */
               if(((unsigned int)(AudioEntryInfo->ConnectionEvent)) & AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Remote Control Data.          */
                  ret_val = _AUDM_Send_Remote_Control_Command(AudioEntryInfo->ConnectionStatus, RemoteDeviceAddress, ResponseTimeout, CommandData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

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
int BTPSAPI AUDM_Send_Remote_Control_Response(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerRemoteControlEventCallbackID)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, AudioManagerRemoteControlEventCallbackID)) != NULL)
            {
               /* Double check that the type is supported.              */
               if(((unsigned int)(AudioEntryInfo->ConnectionEvent)) & AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Remote Control Data.          */
                  ret_val = _AUDM_Send_Remote_Control_Response(AudioEntryInfo->ConnectionStatus, RemoteDeviceAddress, TransactionID, ResponseData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_ROLE_IS_NOT_REGISTERED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

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
int BTPSAPI AUDM_Register_Event_Callback(AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   Audio_Entry_Info_t AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the Audio Entry list.       */
            BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

            AudioEntryInfo.CallbackID         = GetNextCallbackID();
            AudioEntryInfo.EventCallback      = CallbackFunction;
            AudioEntryInfo.CallbackParameter  = CallbackParameter;
            AudioEntryInfo.EventCallbackEntry = TRUE;

            if(AddAudioEntryInfoEntry(&AudioEntryInfoList, &AudioEntryInfo))
               ret_val = AudioEntryInfo.CallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Callback  */
   /* (registered via a successful call to the                          */
   /* AUDM_Register_Event_Callback() function).  This function accepts  */
   /* as input the Audio Manager Event Callback ID (return value from   */
   /* AUDM_Register_Event_Callback() function).                         */
void BTPSAPI AUDM_Un_Register_Event_Callback(unsigned int AudioManagerCallbackID)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(AudioManagerCallbackID)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeAudioEntryInfoEntryMemory(AudioEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

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
int BTPSAPI AUDM_Register_Data_Event_Callback(AUD_Stream_Type_t StreamType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                 ret_val;
   Audio_Entry_Info_t  AudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, Register the handler locally.                     */
            /* * NOTE * We will use the ConnectionStatus member to hold */
            /*          the real Event Callback ID returned from the    */
            /*          server.                                         */
            BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

            AudioEntryInfo.CallbackID         = GetNextCallbackID();
            AudioEntryInfo.StreamType         = StreamType;
            AudioEntryInfo.EventCallback      = CallbackFunction;
            AudioEntryInfo.CallbackParameter  = CallbackParameter;
            AudioEntryInfo.EventCallbackEntry = TRUE;

            if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&AudioEntryInfoDataList, &AudioEntryInfo)) != NULL)
            {
               /* Attempt to register it with the system.               */
               if((ret_val = _AUDM_Register_Stream_Data_Events(StreamType)) > 0)
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  AudioEntryInfoPtr->ConnectionStatus = ret_val;

                  ret_val                             = AudioEntryInfoPtr->CallbackID;
               }
               else
               {
                  /* Error, go ahead and delete the entry we added      */
                  /* locally.                                           */
                  if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoDataList, AudioEntryInfoPtr->CallbackID)) != NULL)
                     FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Callback  */
   /* (registered via a successful call to the                          */
   /* AUDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Audio Manager Data Event Callback ID (return */
   /* value from AUDM_Register_Data_Event_Callback() function).         */
void BTPSAPI AUDM_Un_Register_Data_Event_Callback(unsigned int AudioManagerDataCallbackID)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerDataCallbackID)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, delete the local handler.                         */
            if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoDataList, AudioManagerDataCallbackID)) != NULL)
            {
               /* Handler found, go ahead and delete it from the server.*/
               _AUDM_Un_Register_Stream_Data_Events(AudioEntryInfo->ConnectionStatus);

               /* All finished with the entry, delete it.               */
               FreeAudioEntryInfoEntryMemory(AudioEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

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
int BTPSAPI AUDM_Register_Remote_Control_Event_Callback(unsigned int ServiceType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                 ret_val;
   Audio_Entry_Info_t  AudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if((Initialized) && (ServiceType & (AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET)))
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Before proceeding any further, make sure that there is   */
            /* not already a Remote Control Event Handler for the       */
            /* specified Service Type.                                  */
            /* * NOTE * We are using the ConnectionStatus member to     */
            /*          denote the Service Type.                        */
            AudioEntryInfoPtr = AudioEntryInfoRemoteControlList;

            while(AudioEntryInfoPtr)
            {
               if((AudioEntryInfoPtr->EventCallbackEntry) && (((unsigned int)AudioEntryInfoPtr->ConnectionEvent) & ServiceType))
                  break;
               else
                  AudioEntryInfoPtr = AudioEntryInfoPtr->NextAudioEntryInfoPtr;
            }

            if(!AudioEntryInfoPtr)
            {
               /* First, Register the handler locally.                  */
               BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

               /* We will use the ConnectionStatus member to keep track */
               /* of the Service Type.                                  */
               AudioEntryInfo.CallbackID         = GetNextCallbackID();
               AudioEntryInfo.ConnectionEvent    = (void *)ServiceType;
               AudioEntryInfo.EventCallback      = CallbackFunction;
               AudioEntryInfo.CallbackParameter  = CallbackParameter;
               AudioEntryInfo.EventCallbackEntry = TRUE;

               if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, &AudioEntryInfo)) != NULL)
               {
                  /* Attempt to register it with the system.            */
                  if((ret_val = _AUDM_Register_Remote_Control_Event_Callback(ServiceType)) > 0)
                  {
                     /* Remote Control Handler registered, go ahead and */
                     /* flag success to the caller.                     */
                     AudioEntryInfoPtr->ConnectionStatus = ret_val;

                     ret_val                             = AudioEntryInfoPtr->CallbackID;
                  }
                  else
                  {
                     /* Error, go ahead and delete the entry we added   */
                     /* locally.                                        */
                     if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, AudioEntryInfoPtr->CallbackID)) != NULL)
                        FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_EVENT_ALREADY_REGISTERED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Remote Control  */
   /* Event Callback (registered via a successful call to the           */
   /* AUDM_Register_Remote_Control_Event_Callback() function).  This    */
   /* function accepts as input the Audio Manager Remote Control Event  */
   /* Callback ID (return value from                                    */
   /* AUDM_Register_Remote_Control_Event_Callback() function).          */
void BTPSAPI AUDM_Un_Register_Remote_Control_Event_Callback(unsigned int AudioManagerRemoteControlCallbackID)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerRemoteControlCallbackID)
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the local handler.                                */
            if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, AudioManagerRemoteControlCallbackID)) != NULL)
            {
               /* Handler found, go ahead and delete it from the server.*/
               _AUDM_Un_Register_Remote_Control_Event_Callback(AudioEntryInfo->ConnectionStatus);

               /* All finished with the entry, delete it.               */
               FreeAudioEntryInfoEntryMemory(AudioEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(AudioManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

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
int BTPSAPI AUDM_Connect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   BD_ADDR_t           NULL_BD_ADDR;
   unsigned int        CallbackID;
   Audio_Entry_Info_t  AudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfoPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Attempt to wait for access to the Audio Manager State       */
         /* information.                                                */
         if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* Audio Entry list.                                     */
               BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

               AudioEntryInfo.CallbackID          = GetNextCallbackID();
               AudioEntryInfo.EventCallback       = CallbackFunction;
               AudioEntryInfo.CallbackParameter   = CallbackParameter;
               AudioEntryInfo.RemoteDeviceAddress = RemoteDeviceAddress;

               if(ConnectionStatus)
                  AudioEntryInfo.ConnectionEvent  = BTPS_CreateEvent(FALSE);

               if((!ConnectionStatus) || ((ConnectionStatus) && (AudioEntryInfo.ConnectionEvent)))
               {
                  if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&OutgoingRemoteControlList, &AudioEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Control Browsing 0x%08lX\n", ConnectionFlags));

                     /* Next, attempt to open the browsing channel.     */
                     ret_val = _AUDM_Connect_Remote_Control_Browsing(RemoteDeviceAddress, ConnectionFlags);

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((!ret_val) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Callback ID.                        */
                        CallbackID      = AudioEntryInfoPtr->CallbackID;

                        /* Note the Open Event.                         */
                        ConnectionEvent = AudioEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(AudioManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(AudioManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, CallbackID)) != NULL)
                           {
                              /* Note the connection status.            */
                              *ConnectionStatus = AudioEntryInfoPtr->ConnectionStatus;

                              BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);

                              /* Flag success to the caller.            */
                              ret_val = 0;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_CONTROL;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                     }
                     else
                     {
                        /* If we are not tracking this connection OR    */
                        /* there was an error, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((!CallbackFunction) || (ret_val))
                        {
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo.CallbackID)) != NULL)
                           {
                              if(AudioEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                           }
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
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               BTPS_ReleaseMutex(AudioManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* Browsing session.  This function returns zero if successful, or a */
   /* negative return error code if there was an error.                 */
int BTPSAPI AUDM_Disconnect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Disconnect the Browsing Channel                                */
      ret_val = _AUDM_Disconnect_Remote_Control_Browsing(RemoteDeviceAddress);
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


