/*****< btpmhdsm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDSM - Headset Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/17/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHDSM.h"            /* BTPM HDSET Manager Prototypes/Constants.  */
#include "HDSMAPI.h"             /* HDSET Manager Prototypes/Constants.       */
#include "HDSMMSG.h"             /* BTPM HDSET Manager Message Formats.       */
#include "HDSMGR.h"              /* HDSET Manager Impl. Prototypes/Constants. */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHDSM_Entry_Info_t
{
   unsigned int                  CallbackID;
   unsigned int                  ServerCallbackID;
   unsigned int                  ConnectionStatus;
   Event_t                       ConnectionEvent;
   unsigned long                 Flags;
   BD_ADDR_t                     BD_ADDR;
   HDSM_Connection_Type_t        ConnectionType;
   HDSM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagHDSM_Entry_Info_t *NextHDSETEntryInfoPtr;
} HDSM_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HDSM_Entry_Info_t structure to denote various state information.  */
#define HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY          0x40000000
#define HDSET_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY     0x80000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   HDSM_Event_Callback_t  EventCallback;
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
static Mutex_t HDSETManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variables which hold the current Headset/Audio Gateway events     */
   /* callback ID (registered with the server to receive events).       */
static unsigned int HDSETEventsCallbackID_AG;
static unsigned int HDSETEventsCallbackID_HS;

   /* Variables which hold a pointer to the first element in the Headset*/
   /* entry information list (which holds all callbacks tracked by this */
   /* module).                                                          */
static HDSM_Entry_Info_t *HDSETEntryInfoList_AG;
static HDSM_Entry_Info_t *HDSETEntryInfoList_HS;

   /* Variables which hold a pointer to the first element in the Headset*/
   /* Entry control list (which holds all control callbacks tracked by  */
   /* this module).                                                     */
static HDSM_Entry_Info_t *HDSETEntryInfoList_AG_Control;
static HDSM_Entry_Info_t *HDSETEntryInfoList_HS_Control;

   /* Variables which hold a pointer to the first element in the Headset*/
   /* entry data list (which holds all data callbacks tracked by this   */
   /* module).                                                          */
static HDSM_Entry_Info_t *HDSETEntryInfoList_AG_Data;
static HDSM_Entry_Info_t *HDSETEntryInfoList_HS_Data;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HDSM_Entry_Info_t *AddHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, HDSM_Entry_Info_t *EntryToAdd);
static HDSM_Entry_Info_t *SearchHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID);
static HDSM_Entry_Info_t *DeleteHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHDSETEntryInfoEntryMemory(HDSM_Entry_Info_t *EntryToFree);
static void FreeHDSETEntryInfoList(HDSM_Entry_Info_t **ListHead);

static void DispatchHDSETEvent(Boolean_t ControlOnly, HDSM_Connection_Type_t ConnectionType, HDSM_Event_Data_t *HDSMEventData);

static void ProcessIncomingConnectionRequestEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessDeviceConnectionEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessDeviceConnectionStatusEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus);
static void ProcessDeviceDisconnectionEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason);
static void ProcessAudioConnectedEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessAudioConnectionStatusEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful);
static void ProcessAudioDisconnectedEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessAudioDataReceivedEvent(HDSM_Audio_Data_Received_Message_t *Message);
static void ProcessSpeakerGainIndicationEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain);
static void ProcessMicrophoneGainIndicationEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain);
static void ProcessRingIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessButtonPressedIndicationEvent(BD_ADDR_t RemoteDeviceAddress);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_HDSM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HeadsetManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique callback ID that can be used to add an entry    */
   /* into the Headset entry information list.                          */
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
static HDSM_Entry_Info_t *AddHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, HDSM_Entry_Info_t *EntryToAdd)
{
   HDSM_Entry_Info_t *AddedEntry = NULL;
   HDSM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HDSM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HDSM_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHDSETEntryInfoPtr = NULL;

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
                     FreeHDSETEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHDSETEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHDSETEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHDSETEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified callback ID.  This function returns NULL if either the  */
   /* list head is invalid, the callback ID is invalid, or the specified*/
   /* callback ID was NOT found.                                        */
static HDSM_Entry_Info_t *SearchHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HDSM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHDSETEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Headset entry       */
   /* information list for the specified callback ID and removes it from*/
   /* the List.  This function returns NULL if either the Headset entry */
   /* information list head is invalid, the callback ID is invalid, or  */
   /* the specified callback ID was NOT present in the list.  The entry */
   /* returned will have the next entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHDSETEntryInfoEntryMemory().                 */
static HDSM_Entry_Info_t *DeleteHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HDSM_Entry_Info_t *FoundEntry = NULL;
   HDSM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHDSETEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHDSETEntryInfoPtr = FoundEntry->NextHDSETEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHDSETEntryInfoPtr;

         FoundEntry->NextHDSETEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Headset entry information       */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeHDSETEntryInfoEntryMemory(HDSM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Headset entry information list.  Upon    */
   /* return of this function, the head pointer is set to NULL.         */
static void FreeHDSETEntryInfoList(HDSM_Entry_Info_t **ListHead)
{
   HDSM_Entry_Info_t *EntryToFree;
   HDSM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHDSETEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeHDSETEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Headset event to every registered Headset  */
   /* Event Callback.                                                   */
   /* * NOTE * This function should be called with the Headset Manager  */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Headset Manager Mutex.                               */
static void DispatchHDSETEvent(Boolean_t ControlOnly, HDSM_Connection_Type_t ConnectionType, HDSM_Event_Data_t *HDSMEventData)
{
   unsigned int       Index;
   unsigned int       NumberCallbacks;
   CallbackInfo_t     CallbackInfoArray[16];
   CallbackInfo_t    *CallbackInfoArrayPtr;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HDSETEntryInfoList_AG) || (HDSETEntryInfoList_HS) || (HDSETEntryInfoList_AG_Control) || (HDSETEntryInfoList_HS_Control) || (HDSETEntryInfoList_AG_Data) || (HDSETEntryInfoList_HS_Data)) && (HDSMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      if(!ControlOnly)
      {
         if(ConnectionType == sctAudioGateway)
            HDSETEntryInfo = HDSETEntryInfoList_AG;
         else
            HDSETEntryInfo = HDSETEntryInfoList_HS;

         while(HDSETEntryInfo)
         {
            if((HDSETEntryInfo->EventCallback) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
         }
      }

      /* Next, add the control handlers.                                */
      if(ConnectionType == sctAudioGateway)
         HDSETEntryInfo = HDSETEntryInfoList_AG_Control;
      else
         HDSETEntryInfo = HDSETEntryInfoList_HS_Control;

      while(HDSETEntryInfo)
      {
         if((HDSETEntryInfo->EventCallback) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
      }

      /* Next, add the data handlers.                                   */
      if(!ControlOnly)
      {
         if(ConnectionType == sctAudioGateway)
            HDSETEntryInfo = HDSETEntryInfoList_AG_Data;
         else
            HDSETEntryInfo = HDSETEntryInfoList_HS_Data;

         while(HDSETEntryInfo)
         {
            if((HDSETEntryInfo->EventCallback) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
         }
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
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfo = HDSETEntryInfoList_AG;
               else
                  HDSETEntryInfo = HDSETEntryInfoList_HS;

               while(HDSETEntryInfo)
               {
                  if((HDSETEntryInfo->EventCallback) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDSETEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDSETEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
               }
            }

            /* Next, add the control handlers.                          */
            if(ConnectionType == sctAudioGateway)
               HDSETEntryInfo = HDSETEntryInfoList_AG_Control;
            else
               HDSETEntryInfo = HDSETEntryInfoList_HS_Control;

            while(HDSETEntryInfo)
            {
               if((HDSETEntryInfo->EventCallback) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDSETEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDSETEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
            }

            /* Next, add the data handlers.                             */
            if(!ControlOnly)
            {
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfo = HDSETEntryInfoList_AG_Data;
               else
                  HDSETEntryInfo = HDSETEntryInfoList_HS_Data;

               while(HDSETEntryInfo)
               {
                  if((HDSETEntryInfo->EventCallback) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDSETEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDSETEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
               }
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(HDSETManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(HDSMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HDSETManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDSETManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the incoming */
   /* connection request asynchronous message.                          */
static void ProcessIncomingConnectionRequestEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                                        = hetHDSIncomingConnectionRequest;
   HDSMEventData.EventLength                                                      = HDSM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

   HDSMEventData.EventData.IncomingConnectionRequestEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(FALSE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* connection asynchronous message.                                  */
static void ProcessDeviceConnectionEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                        = hetHDSConnected;
   HDSMEventData.EventLength                                      = HDSM_CONNECTED_EVENT_DATA_SIZE;

   HDSMEventData.EventData.ConnectedEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(FALSE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* connection status asynchronous message.                           */
static void ProcessDeviceConnectionStatusEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus)
{
   void                  *CallbackParameter;
   Boolean_t              ReleaseMutex;
   HDSM_Event_Data_t      HDSMEventData;
   HDSM_Entry_Info_t     *HDSETEntryInfo;
   HDSM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if(((ConnectionType == sctAudioGateway) && (HDSETEntryInfoList_AG)) || ((ConnectionType == sctHeadset) && (HDSETEntryInfoList_HS)))
   {
      ReleaseMutex = TRUE;

      if(ConnectionType == sctAudioGateway)
         HDSETEntryInfo = HDSETEntryInfoList_AG;
      else
         HDSETEntryInfo = HDSETEntryInfoList_HS;

      while(HDSETEntryInfo)
      {
         if((!(HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(HDSETEntryInfo->BD_ADDR, RemoteDeviceAddress)))
         {
            /* Callback registered, now see if the callback is          */
            /* synchronous or asynchronous.                             */
            if(HDSETEntryInfo->ConnectionEvent)
            {
               /* Synchronous.                                          */

               /* Note the Status.                                      */
               HDSETEntryInfo->ConnectionStatus = ConnectionStatus;

               /* Set the Event.                                        */
               BTPS_SetEvent(HDSETEntryInfo->ConnectionEvent);

               /* Break out of the list.                                */
               HDSETEntryInfo = NULL;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HDSETManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;
            }
            else
            {
               /* Asynchronous Entry, go ahead dispatch the result.     */

               /* Format up the Event.                                  */
               HDSMEventData.EventType                                               = hetHDSConnectionStatus;
               HDSMEventData.EventLength                                             = HDSM_CONNECTION_STATUS_EVENT_DATA_SIZE;

               HDSMEventData.EventData.ConnectionStatusEventData.ConnectionType      = ConnectionType;
               HDSMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
               HDSMEventData.EventData.ConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

               /* Note the Callback information.                        */
               EventCallback     = HDSETEntryInfo->EventCallback;
               CallbackParameter = HDSETEntryInfo->CallbackParameter;

               if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfo->CallbackID)) != NULL)
                  FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);

               /* Release the Mutex so we can dispatch the event.       */
               BTPS_ReleaseMutex(HDSETManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&HDSMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Break out of the list.                                */
               HDSETEntryInfo = NULL;
            }
         }
         else
            HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
      }

      /* If the Mutex was not released, then we need to make sure we    */
      /* release it.                                                    */
      if(ReleaseMutex)
         BTPS_ReleaseMutex(HDSETManagerMutex);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDSETManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* disconnection asynchronous message.                               */
static void ProcessDeviceDisconnectionEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                           = hetHDSDisconnected;
   HDSMEventData.EventLength                                         = HDSM_DISCONNECTED_EVENT_DATA_SIZE;

   HDSMEventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HDSMEventData.EventData.DisconnectedEventData.DisconnectReason    = DisconnectReason;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(FALSE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* connected asynchronous message.                                   */
static void ProcessAudioConnectedEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                             = hetHDSAudioConnected;
   HDSMEventData.EventLength                                           = HDSM_AUDIO_CONNECTED_EVENT_DATA_SIZE;

   HDSMEventData.EventData.AudioConnectedEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.AudioConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(FALSE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* connection status asynchronous message.                           */
static void ProcessAudioConnectionStatusEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                                    = hetHDSAudioConnectionStatus;
   HDSMEventData.EventLength                                                  = HDSM_AUDIO_CONNECTION_STATUS_EVENT_DATA_SIZE;

   HDSMEventData.EventData.AudioConnectionStatusEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.AudioConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HDSMEventData.EventData.AudioConnectionStatusEventData.Successful          = Successful;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(FALSE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* disconnection asynchronous message.                               */
static void ProcessAudioDisconnectedEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                                = hetHDSAudioDisconnected;
   HDSMEventData.EventLength                                              = HDSM_AUDIO_DISCONNECTED_EVENT_DATA_SIZE;

   HDSMEventData.EventData.AudioDisconnectedEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.AudioDisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(FALSE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* data received asynchronous message.                               */
static void ProcessAudioDataReceivedEvent(HDSM_Audio_Data_Received_Message_t *Message)
{
   void                  *CallbackParameter;
   HDSM_Event_Data_t      HDSMEventData;
   HDSM_Entry_Info_t     *HDSETEntryInfo;
   HDSM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the event.                                              */
   if((Message) && (Message->AudioDataLength) && (Message->AudioData))
   {
      HDSMEventData.EventType                                        = hetHDSAudioData;
      HDSMEventData.EventLength                                      = HDSM_AUDIO_DATA_EVENT_DATA_SIZE;

      HDSMEventData.EventData.AudioDataEventData.DataEventsHandlerID = 0;
      HDSMEventData.EventData.AudioDataEventData.ConnectionType      = Message->ConnectionType;
      HDSMEventData.EventData.AudioDataEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      HDSMEventData.EventData.AudioDataEventData.AudioDataLength     = Message->AudioDataLength;
      HDSMEventData.EventData.AudioDataEventData.AudioData           = Message->AudioData;
      HDSMEventData.EventData.AudioDataEventData.AudioDataFlags      = Message->AudioDataFlags;

      /* Now that the event is formatted, dispatch it.                  */

      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if(((HDSETEntryInfo = ((Message->ConnectionType == sctAudioGateway)?HDSETEntryInfoList_AG_Data:HDSETEntryInfoList_HS_Data)) != NULL) && (HDSETEntryInfo->ConnectionStatus == Message->DataEventsHandlerID))
      {
         /* Note the Callback Information.                              */
         EventCallback                                                  = HDSETEntryInfo->EventCallback;
         CallbackParameter                                              = HDSETEntryInfo->CallbackParameter;

         /* Note that we need to map the Server Callback to the Client  */
         /* Callback ID.                                                */
         HDSMEventData.EventData.AudioDataEventData.DataEventsHandlerID = HDSETEntryInfo->CallbackID;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HDSETManagerMutex);

         __BTPSTRY
         {
            (*EventCallback)(&HDSMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HDSETManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDSETManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the set      */
   /* speaker gain indication asynchronous message.                     */
static void ProcessSpeakerGainIndicationEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                                    = hetHDSSpeakerGainIndication;
   HDSMEventData.EventLength                                                  = HDSM_SPEAKER_GAIN_INDICATION_EVENT_DATA_SIZE;

   HDSMEventData.EventData.SpeakerGainIndicationEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.SpeakerGainIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HDSMEventData.EventData.SpeakerGainIndicationEventData.SpeakerGain         = SpeakerGain;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(TRUE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the set      */
   /* microphone gain indication asynchronous message.                  */
static void ProcessMicrophoneGainIndicationEvent(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                                       = hetHDSMicrophoneGainIndication;
   HDSMEventData.EventLength                                                     = HDSM_MICROPHONE_GAIN_INDICATION_EVENT_DATA_SIZE;

   HDSMEventData.EventData.MicrophoneGainIndicationEventData.ConnectionType      = ConnectionType;
   HDSMEventData.EventData.MicrophoneGainIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HDSMEventData.EventData.MicrophoneGainIndicationEventData.MicrophoneGain      = MicrophoneGain;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(TRUE, ConnectionType, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ring     */
   /* indication asynchronous message.                                  */
static void ProcessRingIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                             = hetHDSRingIndication;
   HDSMEventData.EventLength                                           = HDSM_RING_INDICATION_EVENT_DATA_SIZE;

   HDSMEventData.EventData.RingIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(TRUE, sctHeadset, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the answer   */
   /* call indication asynchronous message.                             */
static void ProcessButtonPressedIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HDSM_Event_Data_t HDSMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HDSMEventData.EventType                                                    = hetHDSButtonPressedIndication;
   HDSMEventData.EventLength                                                  = HDSM_BUTTON_PRESSED_INDICATION_EVENT_DATA_SIZE;

   HDSMEventData.EventData.ButtonPressIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHDSETEvent(TRUE, sctAudioGateway, &HDSMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Headset Manager  */
   /*          Mutex held.  This function will release the Mutex before */
   /*          it exits (i.e.  the caller SHOULD NOT RELEASE THE MUTEX).*/
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* incoming connection request event.                    */
               ProcessIncomingConnectionRequestEvent(((HDSM_Connection_Request_Message_t *)Message)->ConnectionType, ((HDSM_Connection_Request_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_DEVICE_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device connection event.                              */
               ProcessDeviceConnectionEvent(((HDSM_Device_Connected_Message_t *)Message)->ConnectionType, ((HDSM_Device_Connected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device connection status event.                       */
               ProcessDeviceConnectionStatusEvent(((HDSM_Device_Connection_Status_Message_t *)Message)->ConnectionType, ((HDSM_Device_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((HDSM_Device_Connection_Status_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Disconnection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_DEVICE_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device disconnection event.                           */
               ProcessDeviceDisconnectionEvent(((HDSM_Device_Disconnected_Message_t *)Message)->ConnectionType, ((HDSM_Device_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((HDSM_Device_Disconnected_Message_t *)Message)->DisconnectReason);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_AUDIO_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_AUDIO_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio connected event.                                */
               ProcessAudioConnectedEvent(((HDSM_Audio_Connected_Message_t *)Message)->ConnectionType, ((HDSM_Audio_Connected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_AUDIO_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_AUDIO_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio connection status event.                        */
               ProcessAudioConnectionStatusEvent(((HDSM_Audio_Connection_Status_Message_t *)Message)->ConnectionType, ((HDSM_Audio_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((HDSM_Audio_Connection_Status_Message_t *)Message)->Successful);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_AUDIO_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio disconnected event.                             */
               ProcessAudioDisconnectedEvent(((HDSM_Audio_Disconnected_Message_t *)Message)->ConnectionType, ((HDSM_Audio_Disconnected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Data Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(((HDSM_Audio_Data_Received_Message_t *)Message)->AudioDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio data received event.                            */
               ProcessAudioDataReceivedEvent((HDSM_Audio_Data_Received_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Speaker Gain Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_SPEAKER_GAIN_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* speaker gain indication event.                        */
               ProcessSpeakerGainIndicationEvent(((HDSM_Speaker_Gain_Indication_Message_t *)Message)->ConnectionType, ((HDSM_Speaker_Gain_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HDSM_Speaker_Gain_Indication_Message_t *)Message)->SpeakerGain);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Microphone Gain Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_MICROPHONE_GAIN_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* microphone gain indication event.                     */
               ProcessMicrophoneGainIndicationEvent(((HDSM_Microphone_Gain_Indication_Message_t *)Message)->ConnectionType, ((HDSM_Microphone_Gain_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HDSM_Microphone_Gain_Indication_Message_t *)Message)->MicrophoneGain);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_RING_INDICATION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Ring Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_RING_INDICATION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the ring*/
               /* indication event.                                     */
               ProcessRingIndicationEvent(((HDSM_Ring_Indication_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_BUTTON_PRESSED_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Button Press Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_BUTTON_PRESSED_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* answer call indication event.                         */
               ProcessButtonPressedIndicationEvent(((HDSM_Button_Pressed_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(HDSETManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Headset Manager Asynchronous Events.        */
static void BTPSAPI BTPMDispatchCallback_HDSM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Headset state information.*/
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   unsigned int       Index;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Headset state information.*/
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            Index = 2;
            while(Index--)
            {
               if(Index)
                  HDSETEntryInfo = HDSETEntryInfoList_AG;
               else
                  HDSETEntryInfo = HDSETEntryInfoList_HS;

               while(HDSETEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((!(HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HDSETEntryInfo->ConnectionEvent))
                  {
                     HDSETEntryInfo->ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(HDSETEntryInfo->ConnectionEvent);
                  }

                  HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
               }
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Headset Manager         */
   /* Messages.                                                         */
static void BTPSAPI HeadsetManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HEADSET_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a Headset Manager        */
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
               /* Headset Manager thread.                               */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDSM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Headset Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Headset Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Headset Manager       */
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
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Headset Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Headset Manager module.  This      */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI HDSM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Headset Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((HDSETManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Headset Manager messages.                     */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEADSET_MANAGER, HeadsetManagerGroupHandler, NULL))
            {
               /* Initialize the actual Headset Manager Implementation  */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the HDSET       */
               /* Manager functionality - this module is just the       */
               /* framework shell).                                     */
               if(!(Result = _HDSM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the Headset Manager     */
                  /* Server.                                            */
                  HDSETEventsCallbackID_AG = 0;
                  HDSETEventsCallbackID_HS = 0;

                  Result = _HDSM_Register_Events(sctAudioGateway, FALSE);
                  if(Result > 0)
                     HDSETEventsCallbackID_AG = (unsigned int)Result;

                  Result = _HDSM_Register_Events(sctHeadset, FALSE);
                  if(Result > 0)
                     HDSETEventsCallbackID_HS = (unsigned int)Result;

                  if((HDSETEventsCallbackID_AG) || (HDSETEventsCallbackID_HS))
                  {
                     /* Initialize a unique, starting Headset Callback  */
                     /* ID.                                             */
                     NextCallbackID = 0x000000001;

                     /* Go ahead and flag that this module is           */
                     /* initialized.                                    */
                     Initialized    = TRUE;

                     /* Ensure that we are marked for Success.          */
                     Result         = 0;
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
         if(Result < 0)
         {
            if(HDSETEventsCallbackID_AG)
               _HDSM_Un_Register_Events(HDSETEventsCallbackID_AG);

            if(HDSETEventsCallbackID_HS)
               _HDSM_Un_Register_Events(HDSETEventsCallbackID_HS);

            if(HDSETManagerMutex)
               BTPS_CloseMutex(HDSETManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEADSET_MANAGER);

            /* Flag that none of the resources are allocated.           */
            HDSETManagerMutex        = NULL;
            HDSETEventsCallbackID_AG = 0;
            HDSETEventsCallbackID_HS = 0;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("HDSET Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEADSET_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for Headset Events.                          */
            if(HDSETEventsCallbackID_AG)
               _HDSM_Un_Register_Events(HDSETEventsCallbackID_AG);

            if(HDSETEventsCallbackID_HS)
               _HDSM_Un_Register_Events(HDSETEventsCallbackID_HS);

            /* Next, Un-Register for any Headset control events.        */
            if(HDSETEntryInfoList_AG_Control)
               _HDSM_Un_Register_Events(HDSETEntryInfoList_AG_Control->ServerCallbackID);

            if(HDSETEntryInfoList_HS_Control)
               _HDSM_Un_Register_Events(HDSETEntryInfoList_HS_Control->ServerCallbackID);

            /* Next, Un-Register for any Headset data events.           */
            if(HDSETEntryInfoList_AG_Data)
               _HDSM_Un_Register_Data_Events(HDSETEntryInfoList_AG_Data->ServerCallbackID);

            if(HDSETEntryInfoList_HS_Data)
               _HDSM_Un_Register_Data_Events(HDSETEntryInfoList_HS_Data->ServerCallbackID);

            /* Make sure we inform the Headset Manager Implementation   */
            /* that we are shutting down.                               */
            _HDSM_Cleanup();

            BTPS_CloseMutex(HDSETManagerMutex);

            /* Make sure that the Headset entry information list is     */
            /* empty.                                                   */
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_AG);
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_HS);

            /* Make sure that the Headset entry control information list*/
            /* is empty.                                                */
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_AG_Control);
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_HS_Control);

            /* Make sure that the Headset entry data information list is*/
            /* empty.                                                   */
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_AG_Data);
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_HS_Data);

            /* Flag that the resources are no longer allocated.         */
            HDSETManagerMutex        = NULL;
            CurrentPowerState        = FALSE;
            HDSETEventsCallbackID_AG = 0;
            HDSETEventsCallbackID_HS = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDSM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   unsigned int       Index;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* Headset entries and set any events that have          */
               /* synchronous operations pending.                       */
               Index = 2;
               while(Index--)
               {
                  if(Index)
                     HDSETEntryInfo = HDSETEntryInfoList_AG;
                  else
                     HDSETEntryInfo = HDSETEntryInfoList_HS;

                  while(HDSETEntryInfo)
                  {
                     /* Check to see if there is a synchronous open     */
                     /* operation.                                      */
                     if((!(HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HDSETEntryInfo->ConnectionEvent))
                     {
                        HDSETEntryInfo->ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                        BTPS_SetEvent(HDSETEntryInfo->ConnectionEvent);
                     }

                     HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
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
         BTPS_ReleaseMutex(HDSETManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Headset Manager Connection Management Functions.                  */

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened.  A  */
   /*          hetHDSConnected event will notify if the connection is   */
   /*          successful.                                              */
int BTPSAPI HDSM_Connection_Request_Response(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that the caller has registered for Control.             */
      HDSETEntryInfo = (ConnectionType == sctAudioGateway)?HDSETEntryInfoList_AG_Control:HDSETEntryInfoList_HS_Control;
      if((HDSETEntryInfo) && (HDSETEntryInfo->CallbackID == HeadsetManagerEventCallbackID))
      {
         /* Nothing to do here other than to call the actual function to   */
         /* respond to the Connection Request.                             */
         ret_val = _HDSM_Connection_Request_Response(ConnectionType, RemoteDeviceAddress, AcceptConnection);
      }
      else
         ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Connect_Remote_Device(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                ret_val;
   Event_t            ConnectionEvent;
   BD_ADDR_t          NULL_BD_ADDR;
   unsigned int       CallbackID;
   HDSM_Entry_Info_t  HDSETEntryInfo;
   HDSM_Entry_Info_t *HDSETEntryInfoPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that the caller has registered for Control.             */
      HDSETEntryInfoPtr = (ConnectionType == sctAudioGateway)?HDSETEntryInfoList_AG_Control:HDSETEntryInfoList_HS_Control;
      if((HDSETEntryInfoPtr) && (HDSETEntryInfoPtr->CallbackID == HeadsetManagerEventCallbackID))
      {
         /* Next, verify that the input parameters appear to be         */
         /* semi-valid.                                                 */
         if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
         {
            /* Attempt to wait for access to the Headset Manager state  */
            /* information.                                             */
            if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
            {
               /* Next, check to see if we are powered up.              */
               if(CurrentPowerState)
               {
                  /* Device is powered on, attempt to add an entry into */
                  /* the Headset entry list.                            */
                  BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

                  HDSETEntryInfo.CallbackID        = GetNextCallbackID();
                  HDSETEntryInfo.EventCallback     = CallbackFunction;
                  HDSETEntryInfo.CallbackParameter = CallbackParameter;
                  HDSETEntryInfo.ConnectionType    = ConnectionType;
                  HDSETEntryInfo.BD_ADDR           = RemoteDeviceAddress;

                  if(ConnectionStatus)
                     HDSETEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

                  if((!ConnectionStatus) || ((ConnectionStatus) && (HDSETEntryInfo.ConnectionEvent)))
                  {
                     if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, &HDSETEntryInfo)) != NULL)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote device %d 0x%08lX\n", RemoteServerPort, ConnectionFlags));

                        /* Next, attempt to open the remote device.     */
                        if((ret_val = _HDSM_Connect_Remote_Device(ConnectionType, RemoteDeviceAddress, RemoteServerPort, ConnectionFlags)) != 0)
                        {
                           /* Error opening device, go ahead and delete */
                           /* the entry that was added.                 */
                           if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfoPtr->CallbackID)) != NULL)
                           {
                              if(HDSETEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(HDSETEntryInfoPtr->ConnectionEvent);

                              FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);
                           }
                        }

                        /* Next, determine if the caller has requested a*/
                        /* blocking open.                               */
                        if((!ret_val) && (ConnectionStatus))
                        {
                           /* Blocking open, go ahead and wait for the  */
                           /* event.                                    */

                           /* Note the Callback ID.                     */
                           CallbackID      = HDSETEntryInfoPtr->CallbackID;

                           /* Note the open event.                      */
                           ConnectionEvent = HDSETEntryInfoPtr->ConnectionEvent;

                           /* Release the Mutex because we are finished */
                           /* with it.                                  */
                           BTPS_ReleaseMutex(HDSETManagerMutex);

                           BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                           /* Re-acquire the Mutex.                     */
                           if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
                           {
                              if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, CallbackID)) != NULL)
                              {
                                 /* Note the connection status.         */
                                 *ConnectionStatus = HDSETEntryInfoPtr->ConnectionStatus;

                                 BTPS_CloseEvent(HDSETEntryInfoPtr->ConnectionEvent);

                                 FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);

                                 /* Flag success to the caller.         */
                                 ret_val = 0;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                        }
                        else
                        {
                           /* If we are not tracking this connection OR */
                           /* there was an error, go ahead and delete   */
                           /* the entry that was added.                 */
                           if((!CallbackFunction) || (ret_val))
                           {
                              if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfo.CallbackID)) != NULL)
                              {
                                 if(HDSETEntryInfoPtr->ConnectionEvent)
                                    BTPS_CloseEvent(HDSETEntryInfoPtr->ConnectionEvent);

                                 FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);
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

               /* Release the Mutex because we are finished with it.    */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
                  BTPS_ReleaseMutex(HDSETManagerMutex);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Disconnect_Device(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that the caller has registered for Control.             */
      HDSETEntryInfo = (ConnectionType == sctAudioGateway)?HDSETEntryInfoList_AG_Control:HDSETEntryInfoList_HS_Control;
      if((HDSETEntryInfo) && (HDSETEntryInfo->CallbackID == HeadsetManagerEventCallbackID))
      {
         /* Nothing to do here other than to call the actual function to*/
         /* disconnect the remote device.                               */
         ret_val = _HDSM_Disconnect_Device(ConnectionType, RemoteDeviceAddress);
      }
      else
         ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Headset */
   /* or Audio Gateway Devices (specified by the first parameter).  This*/
   /* function accepts a the local service type to query, followed by   */
   /* buffer information to receive any currently connected device      */
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
int BTPSAPI HDSM_Query_Connected_Devices(HDSM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* query the connected Devices.                                   */
      ret_val = _HDSM_Query_Connected_Devices(ConnectionType, MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Headset or Audio   */
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
int BTPSAPI HDSM_Query_Current_Configuration(HDSM_Connection_Type_t ConnectionType, HDSM_Current_Configuration_t *CurrentConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* query the current configuration.                               */
      ret_val = _HDSM_Query_Current_Configuration(ConnectionType, CurrentConfiguration);
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Headset and   */
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI HDSM_Change_Incoming_Connection_Flags(HDSM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* change the current connection flags.                           */
      ret_val = _HDSM_Change_Incoming_Connection_Flags(ConnectionType, ConnectionFlags);
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Set_Remote_Speaker_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID)) != NULL)
            {
                /* Nothing to do here other than to call the actual     */
                /* function to set the remote speaker gain.             */
                ret_val = _HDSM_Set_Remote_Speaker_Gain(HDSETEntryInfo->ServerCallbackID, ConnectionType, RemoteDeviceAddress, SpeakerGain);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Set_Remote_Microphone_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID)) != NULL)
            {
                /* Nothing to do here other than to call the actual     */
                /* function to set the remote microphone gain.          */
                ret_val = _HDSM_Set_Remote_Microphone_Gain(HDSETEntryInfo->ServerCallbackID, ConnectionType, RemoteDeviceAddress, MicrophoneGain);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Send_Button_Press(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to answer an incoming call.                  */
               ret_val = _HDSM_Send_Button_Press(HDSETEntryInfo->ServerCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Ring_Indication(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, HeadsetManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send a ring indication.                   */
               ret_val = _HDSM_Ring_Indication(HDSETEntryInfo->ServerCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Setup_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t InBandRinging)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to setup the audio connection.               */
               ret_val = _HDSM_Setup_Audio_Connection(HDSETEntryInfo->ServerCallbackID, ConnectionType, RemoteDeviceAddress, InBandRinging);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Release_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to release the audio connection.             */
               ret_val = _HDSM_Release_Audio_Connection(HDSETEntryInfo->ServerCallbackID, ConnectionType, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Send_Audio_Data(unsigned int HeadsetManagerDataEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerDataEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (AudioDataLength) && (AudioData))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Data:&HDSETEntryInfoList_HS_Data, HeadsetManagerDataEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified audio data.            */
               ret_val = _HDSM_Send_Audio_Data(HDSETEntryInfo->ServerCallbackID, ConnectionType, RemoteDeviceAddress, AudioDataLength, AudioData);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerDataEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int BTPSAPI HDSM_Register_Event_Callback(HDSM_Connection_Type_t ConnectionType, Boolean_t ControlCallback, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   HDSM_Entry_Info_t  HDSETEntryInfo;
   HDSM_Entry_Info_t *HDSETEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDSET Manager has been initialized. */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a control event handler for the specified        */
            /* connection type (if control handler was specified).      */
            if(ControlCallback)
            {
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfoPtr = HDSETEntryInfoList_AG_Control;
               else
                  HDSETEntryInfoPtr = HDSETEntryInfoList_HS_Control;
            }
            else
               HDSETEntryInfoPtr = NULL;

            if(!HDSETEntryInfoPtr)
            {
               /* First, register the handler locally.                  */
               BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

               HDSETEntryInfo.CallbackID        = GetNextCallbackID();
               HDSETEntryInfo.EventCallback     = CallbackFunction;
               HDSETEntryInfo.CallbackParameter = CallbackParameter;
               HDSETEntryInfo.Flags             = HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               /* Note the connection type.                             */
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfo.Flags |= HDSET_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

               /* Check to see if we need to register a control handler */
               /* or a normal event handler.                            */
               if(ControlCallback)
               {
                  /* Control handler, add it the correct list, and      */
                  /* attempt to register it with the server.            */
                  if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, &HDSETEntryInfo)) != NULL)
                  {
                     /* Attempt to register it with the system.         */
                     if((ret_val = _HDSM_Register_Events(ConnectionType, TRUE)) > 0)
                     {
                        /* Control handler registered, go ahead and flag*/
                        /* success to the caller.                       */
                        HDSETEntryInfoPtr->ServerCallbackID = ret_val;

                        ret_val                             = HDSETEntryInfoPtr->CallbackID;
                     }
                     else
                     {
                        /* Error, go ahead and delete the entry we added*/
                        /* locally.                                     */
                        if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HDSETEntryInfoPtr->CallbackID)) != NULL)
                           FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
               {
                  if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, &HDSETEntryInfo)) != NULL)
                     ret_val = HDSETEntryInfoPtr->CallbackID;
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_ALREADY_REGISTERED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager event Callback*/
   /* (registered via a successful call to the                          */
   /* HDSM_Register_Event_Callback() function.  This function accepts as*/
   /* input the Headset Manager event callback ID (return value from the*/
   /* HDSM_Register_Event_Callback() function).                         */
void BTPSAPI HDSM_Un_Register_Event_Callback(unsigned int HeadsetManagerEventCallbackID)
{
   Boolean_t          ControlCallback;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HeadsetManagerEventCallbackID)
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* We need to determine what type of Callback this (as we   */
            /* process them differently).                               */
            ControlCallback = FALSE;
            if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG, HeadsetManagerEventCallbackID)) == NULL)
            {
               if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, HeadsetManagerEventCallbackID)) == NULL)
               {
                  if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS, HeadsetManagerEventCallbackID)) == NULL)
                  {
                     if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID)) != NULL)
                        ControlCallback = TRUE;
                  }
               }
               else
                  ControlCallback = TRUE;
            }

            /* Check to see if we found the callback and deleted it.    */
            if(HDSETEntryInfo)
            {
               /* Check to see if need to delete it from the server.    */
               if(ControlCallback)
               {
                  /* Handler found, go ahead and delete it from the     */
                  /* server.                                            */
                  _HDSM_Un_Register_Events(HDSETEntryInfo->ServerCallbackID);
               }

               /* Free the memory because we are finished with it.      */
               FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

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
int BTPSAPI HDSM_Register_Data_Event_Callback(HDSM_Connection_Type_t ConnectionType, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   HDSM_Entry_Info_t  HDSETEntryInfo;
   HDSM_Entry_Info_t *HDSETEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDSET Manager has been initialized. */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a data event handler for the specified connection*/
            /* type.                                                    */
            if(ConnectionType == sctAudioGateway)
               HDSETEntryInfoPtr = HDSETEntryInfoList_AG_Data;
            else
               HDSETEntryInfoPtr = HDSETEntryInfoList_HS_Data;

            if(!HDSETEntryInfoPtr)
            {
               /* First, register the handler locally.                  */
               BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

               HDSETEntryInfo.CallbackID        = GetNextCallbackID();
               HDSETEntryInfo.EventCallback     = CallbackFunction;
               HDSETEntryInfo.CallbackParameter = CallbackParameter;
               HDSETEntryInfo.Flags             = HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               /* Note the connection type.                             */
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfo.Flags |= HDSET_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

               if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Data:&HDSETEntryInfoList_HS_Data, &HDSETEntryInfo)) != NULL)
               {
                  /* Attempt to register it with the system.            */
                  if((ret_val = _HDSM_Register_Data_Events(ConnectionType)) > 0)
                  {
                     /* Data handler registered, go ahead and flag      */
                     /* success to the caller.                          */
                     HDSETEntryInfoPtr->ServerCallbackID = ret_val;

                     ret_val                             = HDSETEntryInfoPtr->CallbackID;
                  }
                  else
                  {
                     /* Error, go ahead and delete the entry we added   */
                     /* locally.                                        */
                     if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Data:&HDSETEntryInfoList_HS_Data, HDSETEntryInfoPtr->CallbackID)) != NULL)
                        FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HEADSET_DATA_HANDLER_ALREADY_REGISTERED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager data event    */
   /* callback (registered via a successful call to the                 */
   /* HDSM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Headset Manager data event callback ID       */
   /* (return value from HDSM_Register_Data_Event_Callback() function). */
void BTPSAPI HDSM_Un_Register_Data_Event_Callback(unsigned int HeadsetManagerDataCallbackID)
{
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(HeadsetManagerDataCallbackID)
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the local handler.                                */
            if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Data, HeadsetManagerDataCallbackID)) == NULL)
               HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Data, HeadsetManagerDataCallbackID);

            if(HDSETEntryInfo)
            {
               /* Handler found, go ahead and delete it from the server.*/
               _HDSM_Un_Register_Data_Events(HDSETEntryInfo->ServerCallbackID);

               /* All finished with the entry, delete it.               */
               FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Data Callback ID that is returned from     */
   /* a successful call to HDSM_Register_Data_Event_Callback().         */
   /* The second parameter is the local connection type of the SCO      */
   /* connection.  The third parameter is the address of the remote     */
   /* device of the SCO connection.  The fourth parameter is a pointer  */
   /* to the location to store the SCO Handle. This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int BTPSAPI HDSM_Query_SCO_Connection_Handle(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle)
{
   int                ret_val;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (SCOHandle))
      {
         /* Attempt to wait for access to the Headset Manager state     */
         /* information.                                                */
         if(BTPS_WaitMutex(HDSETManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified audio data.            */
               ret_val = _HDSM_Query_SCO_Connection_Handle(HDSETEntryInfo->ServerCallbackID, ConnectionType, RemoteDeviceAddress, SCOHandle);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDSETManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

