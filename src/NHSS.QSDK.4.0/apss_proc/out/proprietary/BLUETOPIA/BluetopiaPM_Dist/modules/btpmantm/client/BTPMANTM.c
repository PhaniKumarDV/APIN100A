/*****< btpmantm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMANTM - ANT Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/02/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMANTM.h"            /* BTPM ANT Manager Prototypes/Constants.    */
#include "ANTMAPI.h"             /* ANT Manager Prototypes/Constants.         */
#include "ANTMMSG.h"             /* BTPM ANT Manager Message Formats.         */
#include "ANTMGR.h"              /* ANT Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagANT_Entry_Info_t
{
   unsigned int                  CallbackID;
   ANTM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagANT_Entry_Info_t *NextANTEntryInfoPtr;
} ANT_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   unsigned int           CallbackID;
   ANTM_Event_Callback_t  EventCallback;
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
static Mutex_t ANTManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the ANT    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static ANT_Entry_Info_t *ANTEntryInfoList;

   /* Variable which tracks the local client's CallbackID from the PM   */
   /* server.                                                           */
static unsigned int ANTEventCallbackID;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static ANT_Entry_Info_t *AddANTEntryInfoEntry(ANT_Entry_Info_t **ListHead, ANT_Entry_Info_t *EntryToAdd);
static ANT_Entry_Info_t *SearchANTEntryInfoEntry(ANT_Entry_Info_t **ListHead, unsigned int CallbackID);
static ANT_Entry_Info_t *DeleteANTEntryInfoEntry(ANT_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeANTEntryInfoEntryMemory(ANT_Entry_Info_t *EntryToFree);
static void FreeANTEntryInfoList(ANT_Entry_Info_t **ListHead);

static void DispatchANTMEvent(ANTM_Event_Data_t *ANTMEventData);

static void ProcessStartupMessageEvent(ANTM_Startup_Message_t *Message);
static void ProcessChannelResponseEvent(ANTM_Channel_Response_Message_t *Message);
static void ProcessChannelStatusEvent(ANTM_Channel_Status_Message_t *Message);
static void ProcessChannelIDEvent(ANTM_Channel_ID_Message_t *Message);
static void ProcessANTVersionEvent(ANTM_ANT_Version_Message_t *Message);
static void ProcessCapabilitiesEvent(ANTM_Capabilities_Message_t *Message);
static void ProcessBroadcastDataPacketEvent(ANTM_Broadcast_Data_Packet_Message_t *Message);
static void ProcessAcknowledgedDataPacketEvent(ANTM_Acknowledged_Data_Packet_Message_t *Message);
static void ProcessBurstDataPacketEvent(ANTM_Burst_Data_Packet_Message_t *Message);
static void ProcessExtendedBroadcastDataPacketEvent(ANTM_Extended_Broadcast_Data_Packet_Message_t *Message);
static void ProcessExtendedAcknowledgedDataPacketEvent(ANTM_Extended_Acknowledged_Data_Packet_Message_t *Message);
static void ProcessExtendedBurstDataPacketEvent(ANTM_Extended_Burst_Data_Packet_Message_t *Message);
static void ProcessRawDataPacketEvent(ANTM_Raw_Data_Packet_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_ANTM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI ANTManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the ANT Entry Information List.                              */
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
static ANT_Entry_Info_t *AddANTEntryInfoEntry(ANT_Entry_Info_t **ListHead, ANT_Entry_Info_t *EntryToAdd)
{
   ANT_Entry_Info_t *AddedEntry = NULL;
   ANT_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (ANT_Entry_Info_t *)BTPS_AllocateMemory(sizeof(ANT_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextANTEntryInfoPtr = NULL;

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
                     FreeANTEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextANTEntryInfoPtr)
                        tmpEntry = tmpEntry->NextANTEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextANTEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static ANT_Entry_Info_t *SearchANTEntryInfoEntry(ANT_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   ANT_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextANTEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified ANTM Entry          */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List. This function returns NULL if either the ANTM Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list. The entry  */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeANTEntryInfoEntryMemory().                   */
static ANT_Entry_Info_t *DeleteANTEntryInfoEntry(ANT_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   ANT_Entry_Info_t *FoundEntry = NULL;
   ANT_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextANTEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextANTEntryInfoPtr = FoundEntry->NextANTEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextANTEntryInfoPtr;

         FoundEntry->NextANTEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified ANTM Entry Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeANTEntryInfoEntryMemory(ANT_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified ANTM Entry Information List. Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeANTEntryInfoList(ANT_Entry_Info_t **ListHead)
{
   ANT_Entry_Info_t *EntryToFree;
   ANT_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextANTEntryInfoPtr;

         FreeANTEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void DispatchANTMEvent(ANTM_Event_Data_t *ANTMEventData)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   ANT_Entry_Info_t *ANTEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((ANTEntryInfoList) && (ANTMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      ANTEntryInfo    = ANTEntryInfoList;
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(ANTEntryInfo)
      {
         if(ANTEntryInfo->EventCallback)
            NumberCallbacks++;

         ANTEntryInfo = ANTEntryInfo->NextANTEntryInfoPtr;
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
            ANTEntryInfo    = ANTEntryInfoList;
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(ANTEntryInfo)
            {
               if(ANTEntryInfo->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackID        = ANTEntryInfo->CallbackID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = ANTEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = ANTEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               ANTEntryInfo = ANTEntryInfo->NextANTEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(ANTManagerMutex);

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* Note the callback ID.  (Since CallbackID is the at the*/
               /* beginning of every event data structure, we can simply*/
               /* pick one to insert).                                  */
               ANTMEventData->EventData.StartupMessageEventData.CallbackID = CallbackInfoArrayPtr[Index].CallbackID;

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(ANTMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(ANTManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessStartupMessageEvent(ANTM_Startup_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType                                        = aetANTMStartupMessage;
   ANTMEventData.EventLength                                      = ANTM_STARTUP_MESSAGE_EVENT_DATA_SIZE;

   ANTMEventData.EventData.StartupMessageEventData.StartupMessage = Message->StartupMessage;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchANTMEvent(&ANTMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessChannelResponseEvent(ANTM_Channel_Response_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMChannelResponse;
   ANTMEventData.EventLength = ANTM_CHANNEL_RESPONSE_EVENT_DATA_SIZE;

   ANTMEventData.EventData.ChannelResponseEventData.ChannelNumber = Message->ChannelNumber;
   ANTMEventData.EventData.ChannelResponseEventData.MessageID     = Message->MessageID;
   ANTMEventData.EventData.ChannelResponseEventData.MessageCode   = Message->MessageCode;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchANTMEvent(&ANTMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessChannelStatusEvent(ANTM_Channel_Status_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMChannelStatus;
   ANTMEventData.EventLength = ANTM_CHANNEL_STATUS_EVENT_DATA_SIZE;

   ANTMEventData.EventData.ChannelStatusEventData.ChannelNumber = Message->ChannelNumber;
   ANTMEventData.EventData.ChannelStatusEventData.ChannelStatus = Message->ChannelStatus;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchANTMEvent(&ANTMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessChannelIDEvent(ANTM_Channel_ID_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMChannelID;
   ANTMEventData.EventLength = ANTM_CHANNEL_ID_EVENT_DATA_SIZE;

   ANTMEventData.EventData.ChannelIDEventData.ChannelNumber    = Message->ChannelNumber;
   ANTMEventData.EventData.ChannelIDEventData.DeviceNumber     = Message->DeviceNumber;
   ANTMEventData.EventData.ChannelIDEventData.DeviceTypeID     = Message->DeviceTypeID;
   ANTMEventData.EventData.ChannelIDEventData.TransmissionType = Message->TransmissionType;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchANTMEvent(&ANTMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessANTVersionEvent(ANTM_ANT_Version_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType                                       = aetANTMANTVersion;
   ANTMEventData.EventLength                                     = ANTM_ANT_VERSION_EVENT_DATA_SIZE;

   ANTMEventData.EventData.ANTVersionEventData.VersionDataLength = Message->VersionDataLength;

   if((ANTMEventData.EventData.ANTVersionEventData.VersionData = (Byte_t *)BTPS_AllocateMemory(Message->VersionDataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.ANTVersionEventData.VersionData, Message->VersionData, Message->VersionDataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.ANTVersionEventData.VersionData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessCapabilitiesEvent(ANTM_Capabilities_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMCapabilities;
   ANTMEventData.EventLength = ANTM_CAPABILITIES_EVENT_DATA_SIZE;

   ANTMEventData.EventData.CapabilitiesEventData.MaxChannels      = Message->MaxChannels;
   ANTMEventData.EventData.CapabilitiesEventData.MaxNetworks      = Message->MaxNetworks;
   ANTMEventData.EventData.CapabilitiesEventData.StandardOptions  = Message->StandardOptions;
   ANTMEventData.EventData.CapabilitiesEventData.AdvancedOptions  = Message->AdvancedOptions;
   ANTMEventData.EventData.CapabilitiesEventData.AdvancedOptions2 = Message->AdvancedOptions2;
   ANTMEventData.EventData.CapabilitiesEventData.Reserved         = Message->Reserved;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchANTMEvent(&ANTMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessBroadcastDataPacketEvent(ANTM_Broadcast_Data_Packet_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMBroadcastDataPacket;
   ANTMEventData.EventLength = ANTM_BROADCAST_DATA_PACKET_EVENT_DATA_SIZE;

   ANTMEventData.EventData.BroadcastDataPacketEventData.ChannelNumber = Message->ChannelNumber;
   ANTMEventData.EventData.BroadcastDataPacketEventData.DataLength    = Message->DataLength;

   if((ANTMEventData.EventData.BroadcastDataPacketEventData.Data = (Byte_t *)BTPS_AllocateMemory(Message->DataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.BroadcastDataPacketEventData.Data, Message->Data, Message->DataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.BroadcastDataPacketEventData.Data);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessAcknowledgedDataPacketEvent(ANTM_Acknowledged_Data_Packet_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMAcknowledgedDataPacket;
   ANTMEventData.EventLength = ANTM_ACKNOWLEDGED_DATA_PACKET_EVENT_DATA_SIZE;

   ANTMEventData.EventData.AcknowledgedDataPacketEventData.ChannelNumber = Message->ChannelNumber;
   ANTMEventData.EventData.AcknowledgedDataPacketEventData.DataLength    = Message->DataLength;

   if((ANTMEventData.EventData.AcknowledgedDataPacketEventData.Data = (Byte_t *)BTPS_AllocateMemory(Message->DataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.AcknowledgedDataPacketEventData.Data, Message->Data, Message->DataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.AcknowledgedDataPacketEventData.Data);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessBurstDataPacketEvent(ANTM_Burst_Data_Packet_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMBurstDataPacket;
   ANTMEventData.EventLength = ANTM_BURST_DATA_PACKET_EVENT_DATA_SIZE;

   ANTMEventData.EventData.BurstDataPacketEventData.SequenceChannelNumber = Message->SequenceChannelNumber;
   ANTMEventData.EventData.BurstDataPacketEventData.DataLength            = Message->DataLength;

   if((ANTMEventData.EventData.BurstDataPacketEventData.Data = (Byte_t *)BTPS_AllocateMemory(Message->DataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.BurstDataPacketEventData.Data, Message->Data, Message->DataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.BurstDataPacketEventData.Data);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessExtendedBroadcastDataPacketEvent(ANTM_Extended_Broadcast_Data_Packet_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMExtendedBroadcastDataPacket;
   ANTMEventData.EventLength = ANTM_EXTENDED_BROADCAST_DATA_PACKET_EVENT_DATA_SIZE;

   ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.ChannelNumber    = Message->ChannelNumber;
   ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.DeviceNumber     = Message->DeviceNumber;
   ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.DeviceType       = Message->DeviceType;
   ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.TransmissionType = Message->TransmissionType;
   ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.DataLength       = Message->DataLength;

   if((ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.Data = (Byte_t *)BTPS_AllocateMemory(Message->DataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.Data, Message->Data, Message->DataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.ExtendedBroadcastDataPacketEventData.Data);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessExtendedAcknowledgedDataPacketEvent(ANTM_Extended_Acknowledged_Data_Packet_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMExtendedAcknowledgedDataPacket;
   ANTMEventData.EventLength = ANTM_EXTENDED_ACKNOWLEDGED_DATA_PACKET_EVENT_DATA_SIZE;

   ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.ChannelNumber    = Message->ChannelNumber;
   ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.DeviceNumber     = Message->DeviceNumber;
   ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.DeviceType       = Message->DeviceType;
   ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.TransmissionType = Message->TransmissionType;
   ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.DataLength       = Message->DataLength;

   if((ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.Data = (Byte_t *)BTPS_AllocateMemory(Message->DataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.Data, Message->Data, Message->DataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.ExtendedAcknowledgedDataPacketEventData.Data);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessExtendedBurstDataPacketEvent(ANTM_Extended_Burst_Data_Packet_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMExtendedBurstDataPacket;
   ANTMEventData.EventLength = ANTM_EXTENDED_BURST_DATA_PACKET_EVENT_DATA_SIZE;

   ANTMEventData.EventData.ExtendedBurstDataPacketEventData.SequenceChannelNumber = Message->SequenceChannelNumber;
   ANTMEventData.EventData.ExtendedBurstDataPacketEventData.DeviceNumber          = Message->DeviceNumber;
   ANTMEventData.EventData.ExtendedBurstDataPacketEventData.DeviceType            = Message->DeviceType;
   ANTMEventData.EventData.ExtendedBurstDataPacketEventData.TransmissionType      = Message->TransmissionType;
   ANTMEventData.EventData.ExtendedBurstDataPacketEventData.DataLength            = Message->DataLength;

   if((ANTMEventData.EventData.ExtendedBurstDataPacketEventData.Data = (Byte_t *)BTPS_AllocateMemory(Message->DataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.ExtendedBurstDataPacketEventData.Data, Message->Data, Message->DataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.ExtendedBurstDataPacketEventData.Data);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessRawDataPacketEvent(ANTM_Raw_Data_Packet_Message_t *Message)
{
   ANTM_Event_Data_t ANTMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BTPS_MemInitialize(&ANTMEventData, 0, ANTM_EVENT_DATA_SIZE);

   ANTMEventData.EventType   = aetANTMRawDataPacket;
   ANTMEventData.EventLength = ANTM_RAW_DATA_PACKET_EVENT_DATA_SIZE;

   ANTMEventData.EventData.RawDataPacketEventData.DataLength = Message->DataLength;

   if((ANTMEventData.EventData.RawDataPacketEventData.Data = (Byte_t *)BTPS_AllocateMemory(Message->DataLength)) != NULL)
   {
      BTPS_MemCopy(ANTMEventData.EventData.RawDataPacketEventData.Data, Message->Data, Message->DataLength);

      /* Now that the Event is formatted, dispatch it.                  */
      DispatchANTMEvent(&ANTMEventData);

      /* Free our allocated memory.                                     */
      BTPS_FreeMemory(ANTMEventData.EventData.RawDataPacketEventData.Data);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANTManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Hands Free       */
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case ANTM_MESSAGE_FUNCTION_STARTUP_MESSAGE:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_STARTUP_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessStartupMessageEvent((ANTM_Startup_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_CHANNEL_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_CHANNEL_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessChannelResponseEvent((ANTM_Channel_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_CHANNEL_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_CHANNEL_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessChannelStatusEvent((ANTM_Channel_Status_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_CHANNEL_ID:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_CHANNEL_ID_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessChannelIDEvent((ANTM_Channel_ID_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_ANT_VERSION:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_VERSION_MESSAGE_SIZE(((ANTM_ANT_Version_Message_t *)Message)->VersionDataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessANTVersionEvent((ANTM_ANT_Version_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_CAPABILITIES:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_CAPABILITIES_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessCapabilitiesEvent((ANTM_Capabilities_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_BROADCAST_DATA_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_BROADCAST_DATA_PACKET_MESSAGE_SIZE(((ANTM_Broadcast_Data_Packet_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessBroadcastDataPacketEvent((ANTM_Broadcast_Data_Packet_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_ACKNOWLEDGED_DATA_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_ACKNOWLEDGED_DATA_PACKET_MESSAGE_SIZE(((ANTM_Acknowledged_Data_Packet_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessAcknowledgedDataPacketEvent((ANTM_Acknowledged_Data_Packet_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_BURST_DATA_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_BURST_DATA_PACKET_MESSAGE_SIZE(((ANTM_Burst_Data_Packet_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessBurstDataPacketEvent((ANTM_Burst_Data_Packet_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_EXTENDED_BROADCAST_DATA_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_EXTENDED_BROADCAST_DATA_PACKET_MESSAGE_SIZE(((ANTM_Extended_Broadcast_Data_Packet_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessExtendedBroadcastDataPacketEvent((ANTM_Extended_Broadcast_Data_Packet_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_EXTENDED_ACKNOWLEDGED_DATA_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_EXTENDED_ACKNOWLEDGED_DATA_PACKET_MESSAGE_SIZE(((ANTM_Extended_Acknowledged_Data_Packet_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessExtendedAcknowledgedDataPacketEvent((ANTM_Extended_Acknowledged_Data_Packet_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_EXTENDED_BURST_DATA_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_EXTENDED_BURST_DATA_PACKET_MESSAGE_SIZE(((ANTM_Extended_Burst_Data_Packet_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessExtendedBurstDataPacketEvent((ANTM_Extended_Burst_Data_Packet_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANTM_MESSAGE_FUNCTION_RAW_DATA_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Raw Data Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_RAW_DATA_PACKET_MESSAGE_SIZE(((ANTM_Raw_Data_Packet_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the Raw */
               /* Data Event.                                           */
               ProcessRawDataPacketEvent((ANTM_Raw_Data_Packet_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(ANTManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process ANT Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_ANTM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the ANT state information.    */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the ANT state information.    */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the ANT Entry Info List.                            */
            FreeANTEntryInfoList(&ANTEntryInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all ANT Manager Messages.   */
static void BTPSAPI ANTManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANT Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a ANT Manager defined    */
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
               /* ANT Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_ANTM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue ANT Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue ANT Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an ANT Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Non ANT Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager ANT Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI ANTM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANT+ Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((ANTManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process ANT Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER, ANTManagerGroupHandler, NULL))
            {
               /* Initialize the actual ANT Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the ANT Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _ANTM_Initialize()));
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting ANT Callback ID.     */
                  NextCallbackID      = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized         = TRUE;

                  /* Return 0 to the caller.                            */
                  Result              = 0;
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
            if(ANTManagerMutex)
               BTPS_CloseMutex(ANTManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER);

            /* Flag that none of the resources are allocated.           */
            ANTManagerMutex     = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANT Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            _ANTM_Un_Register_ANT_Events(ANTEventCallbackID);;

            /* Make sure we inform the ANT Manager Implementation that  */
            /* we are shutting down.                                    */
            _ANTM_Cleanup();;

            /* Make sure that the ANT Entry Information List is empty.  */
            FreeANTEntryInfoList(&ANTEntryInfoList);

            /* Close the ANT Manager Mutex.                             */
            BTPS_CloseMutex(ANTManagerMutex);

            /* Flag that the resources are no longer allocated.         */
            ANTManagerMutex     = NULL;
            CurrentPowerState   = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI ANTM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(ANTManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a event callback function with the ANT+       */
   /* Manager Service.  This Callback will be dispatched by the ANT+    */
   /* Manager when various ANT+ Manager Events occur.  This function    */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a ANT+ Manager Event needs to be      */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANTM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
int BTPSAPI ANTM_Register_Event_Callback(ANTM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   ANT_Entry_Info_t  ANTEntryInfo;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to register with the remote server if we have not*/
            /* already.                                                 */
            if(!ANTEventCallbackID)
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Registering with server.\n"));
               if((ret_val = _ANTM_Register_ANT_Events()) > 0)
                  ANTEventCallbackID = ret_val;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Already registered with server.\n"));
               ret_val = 0;
            }

            /* Make sure we are registered with the remote server.      */
            if(ANTEventCallbackID)
            {
               /* Attempt to add an entry into the ANT Entry list.      */
               BTPS_MemInitialize(&ANTEntryInfo, 0, sizeof(ANT_Entry_Info_t));

               ANTEntryInfo.CallbackID         = GetNextCallbackID();
               ANTEntryInfo.EventCallback      = CallbackFunction;
               ANTEntryInfo.CallbackParameter  = CallbackParameter;

               if((ANTEntryInfoPtr = AddANTEntryInfoEntry(&ANTEntryInfoList, &ANTEntryInfo)) != NULL)
               {
                  /* Return the CallbackID to the caller.               */
                  ret_val = ANTEntryInfoPtr->CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANT+ Manager Event Callback   */
   /* (registered via a successful call to the                          */
   /* ANTM_Register_Event_Callback() function).  This function accepts  */
   /* as input the ANT+ Manager Event Callback ID (return value from    */
   /* ANTM_Register_Event_Callback() function).                         */
void BTPSAPI ANTM_Un_Register_Event_Callback(unsigned int ANTManagerCallbackID)
{
   ANT_Entry_Info_t *ANTEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the entry from the list.                          */
            if((ANTEntryInfo = DeleteANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* If this was the last callback, un-register with the   */
               /* server.                                               */
               if(!ANTEntryInfoList)
               {
                  _ANTM_Un_Register_ANT_Events(ANTEventCallbackID);

                  /* Note we no longer are registered.                  */
                  ANTEventCallbackID = 0;
               }

               /* Free the memory because we are finished with it.      */
               FreeANTEntryInfoEntryMemory(ANTEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is responsible for assigning an ANT channel*/
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to register.  This function   */
   /* accepts as it's third argument, the channel type to be assigned to*/
   /* the channel.  This function accepts as it's fourth argument, the  */
   /* network number to be used for the channel.  Zero should be        */
   /* specified for this argument to use the default public network.    */
   /* This function accepts as it's fifth argument, the extended        */
   /* assignment to be used for the channel.  Zero should be specified  */
   /* for this argument if no extended capabilities are to be used.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int BTPSAPI ANTM_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ChannelType, unsigned int NetworkNumber, unsigned int ExtendedAssignment)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Assign_Channel(ANTEventCallbackID, ChannelNumber, ChannelType, NetworkNumber, ExtendedAssignment);;
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for un-assigning an ANT     */
   /* channel on the local ANT+ system.  A channel must be unassigned   */
   /* before it can be reassigned using the ANTM_Assign_Channel() API.  */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to un-assign.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
int BTPSAPI ANTM_Un_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Un_Assign_Channel(ANTEventCallbackID, ChannelNumber);;
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, the channel number to configure.  The ANT   */
   /* channel must be assigned using ANTM_Assign_Channel() before       */
   /* calling this function.  This function accepts as it's third       */
   /* argument, the device number to search for on the channel.  Zero   */
   /* should be specified for this argument to scan for any device      */
   /* number.  This function accepts as it's fourth argument, the device*/
   /* type to search for on the channel.  Zero should be specified for  */
   /* this argument to scan for any device type.  This function accepts */
   /* as it's fifth argument, the transmission type to search for on the*/
   /* channel.  Zero should be specified for this argument to scan for  */
   /* any transmission type.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
int BTPSAPI ANTM_Set_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Channel_ID(ANTEventCallbackID, ChannelNumber, DeviceNumber, DeviceType, TransmissionType);;
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the local ANT+ system.     */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel messaging period to   */
   /* set on the channel.  This function returns zero if successful,    */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
int BTPSAPI ANTM_Set_Channel_Period(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessagingPeriod)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Channel_Period(ANTEventCallbackID, ChannelNumber, MessagingPeriod);;
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument the     */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the search timeout to set on the  */
   /* channel.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will result in an infinite search timeout.  Specifying   */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
int BTPSAPI ANTM_Set_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Channel_Search_Timeout(ANTEventCallbackID, ChannelNumber, SearchTimeout);;
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel.  This function accepts as it's first*/
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel frequency to set on   */
   /* the channel.  This function returns zero if successful, otherwise */
   /* this function returns a negative error code.                      */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (2400 + RFFrequency) MHz.                        */
int BTPSAPI ANTM_Set_Channel_RF_Frequency(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int RFFrequency)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Channel_RF_Frequency(ANTEventCallbackID, ChannelNumber, RFFrequency);;
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring the network */
   /* key for an ANT channel.  This function accepts as it's first      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, a pointer to the ANT network key  */
   /* to set on the channel.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
int BTPSAPI ANTM_Set_Network_Key(unsigned int ANTManagerCallbackID, unsigned int NetworkNumber, ANT_Network_Key_t NetworkKey)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Network_Key(ANTEventCallbackID, NetworkNumber, NetworkKey);;
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring the transmit*/
   /* power on the local ANT system.  This function accepts as it's     */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument the transmit power to set on the device.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int BTPSAPI ANTM_Set_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int TransmitPower)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Transmit_Power(ANTEventCallbackID, TransmitPower);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to add to the */
   /* list.  This function accepts as it's third argument, the device   */
   /* number to add to the list.  This function accepts as it's fourth  */
   /* argument, the device type to add to the list.  This function      */
   /* accepts as it's fifth argument, the transmission type to add to   */
   /* the list.  This function accepts as it's sixth argument, the the  */
   /* list index to overwrite with the updated entry.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Add_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType, unsigned int ListIndex)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Add_Channel_ID(ANTEventCallbackID, ChannelNumber, DeviceNumber, DeviceType, TransmissionType, ListIndex);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* on which the list should be configured.  This function accepts as */
   /* it's third argument, the size of the list.  This function accepts */
   /* as it's fourth argument, the list type.  Zero should be specified */
   /* to configure the list for inclusion, and one should be specified  */
   /* to configure the list for exclusion.  This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Configure_Inclusion_Exclusion_List(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ListSize, unsigned int Exclude)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Configure_Inclusion_Exclusion_List(ANTEventCallbackID, ChannelNumber, ListSize, Exclude);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the transmit power level for the  */
   /* specified channel.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Set_Channel_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int TransmitPower)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Channel_Transmit_Power(ANTEventCallbackID, ChannelNumber, TransmitPower);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to   */
   /* configure.  This function accepts as it's third argument, the     */
   /* search timeout to set on the channel.  This function returns zero */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will result in an   */
   /*          infinite low priority search timeout.                    */
int BTPSAPI ANTM_Set_Low_Priority_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Low_Priority_Channel_Search_Timeout(ANTEventCallbackID, ChannelNumber, SearchTimeout);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function configures the   */
   /* channel ID in the same way as ANTM_Set_Channel_ID(), except it    */
   /* uses the two LSB of the device's serial number as the device's    */
   /* number.  This function accepts as it's first argument the Callback*/
   /* ID that was returned from a successful call                       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  The ANT channel*/
   /* must be assigned using ANTM_Assign_Channel() before calling this  */
   /* function.  This function accepts as it's third argument, the      */
   /* device type to search for on the channel.  Zero should be         */
   /* specified for this argument to scan for any device type.  This    */
   /* function accepts as it's fourth argument, the transmission type to*/
   /* search for on the channel.  Zero should be specified for this     */
   /* argument to scan for any transmission type.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Set_Serial_Number_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Serial_Number_Channel_ID(ANTEventCallbackID, ChannelNumber, DeviceType, TransmissionType);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument whether or not to enable extended Rx messages.    */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Enable_Extended_Messages(unsigned int ANTManagerCallbackID, Boolean_t Enable)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Enable_Extended_Messages(ANTEventCallbackID, Enable);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for enabling or disabling   */
   /* the LED on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, whether or not to enable the LED.  This     */
   /* function returns zero if successful, otherwise this function      */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Enable_LED(unsigned int ANTManagerCallbackID, Boolean_t Enable)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Enable_LED(ANTEventCallbackID, Enable);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the local ANT+ system.  This function accepts as */
   /* it's only argument the Callback ID that was returned from a       */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
int BTPSAPI ANTM_Enable_Crystal(unsigned int ANTManagerCallbackID)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Enable_Crystal(ANTEventCallbackID);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the bitmask of extended */
   /* Rx messages that shall be enabled or disabled.  This function     */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Configure_Extended_Messages(unsigned int ANTManagerCallbackID, unsigned int EnabledExtendedMessagesMask)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Configure_Extended_Messages(ANTEventCallbackID, EnabledExtendedMessagesMask);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel.  This function accepts  */
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to configure. */
   /* This function accepts as it's third, fourth, and fifth arguments, */
   /* the three operating agility frequencies to set.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANTM_Assign_Channel.  Frequency agility      */
   /*          should NOT be used with shared, Tx only, or Rx only      */
   /*          channels.                                                */
int BTPSAPI ANTM_Configure_Frequency_Agility(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int FrequencyAgility1, unsigned int FrequencyAgility2, unsigned int FrequencyAgility3)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Configure_Frequency_Agility(ANTEventCallbackID, ChannelNumber, FrequencyAgility1, FrequencyAgility2, FrequencyAgility3);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the local ANT+ system.  This      */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search threshold to set.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
int BTPSAPI ANTM_Set_Proximity_Search(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchThreshold)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Proximity_Search(ANTEventCallbackID, ChannelNumber, SearchThreshold);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search priority to set.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Set_Channel_Search_Priority(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchPriority)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_Channel_Search_Priority(ANTEventCallbackID, ChannelNumber, SearchPriority);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the local ANT+ system.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the descriptor string type to    */
   /* set.  This function accepts as it's third argument, the           */
   /* NULL-terminated descriptor string to be set.  This function       */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Set_USB_Descriptor_String(unsigned int ANTManagerCallbackID, unsigned int StringNumber, char *DescriptorString)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ANTManagerCallbackID) && (DescriptorString))
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_USB_Descriptor_String(ANTEventCallbackID, StringNumber, DescriptorString);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for resetting the ANT module*/
   /* on the local ANT+ system.  This function accepts as it's only     */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  A delay of at least 500ms is     */
   /* suggested after calling this function to allow time for the module*/
   /* to reset.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
int BTPSAPI ANTM_Reset_System(unsigned int ANTManagerCallbackID)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Reset_System(ANTEventCallbackID);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for opening an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  The channel    */
   /* specified must have been assigned and configured before calling   */
   /* this function.  This function returns zero if successful,         */
   /* otherwise this function returns a negative error code.            */
int BTPSAPI ANTM_Open_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Open_Channel(ANTEventCallbackID, ChannelNumber);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for closing an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the aetANTMChannelResponse event has been received */
   /*          with the Message_Code member specifying:                 */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
int BTPSAPI ANTM_Close_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Close_Channel(ANTEventCallbackID, ChannelNumber);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number that the request will be sent */
   /* to.  This function accepts as it's third argument, the message ID */
   /* being requested from the channel.  This function returns zero if  */
   /* successful, otherwise this function returns a negative error code.*/
int BTPSAPI ANTM_Request_Message(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessageID)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Request_Message(ANTEventCallbackID, ChannelNumber, MessageID);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the local ANT+ system.  This function  */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to be*/
   /* opened.  The channel specified must have been assigned and        */
   /* configured as a SLAVE Rx ONLY channel before calling this         */
   /* function.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
int BTPSAPI ANTM_Open_Rx_Scan_Mode(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Close_Channel(ANTEventCallbackID, ChannelNumber);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for putting the ANT+ system */
   /* in ultra low-power mode.  This function accepts as it's only      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(!MSGREADY) line on the ANT chip to the appropriate*/
   /*          value.                                                   */
int BTPSAPI ANTM_Sleep_Message(unsigned int ANTManagerCallbackID)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Sleep_Message(ANTEventCallbackID);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Data Message API.                                                 */

   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the local ANT+ system.  This function      */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be broadcast on.  This function accepts as it's     */
   /* third argument the length of the data to send.  This function     */
   /* accepts as it's fourth argument a pointer to a byte array of the  */
   /* broadcast data to send.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
int BTPSAPI ANTM_Send_Broadcast_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ANTManagerCallbackID) && (Data))
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Send_Broadcast_Data(ANTEventCallbackID, ChannelNumber, DataLength, Data);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be sent on.  This function accepts as it's third    */
   /* argument the length of the data to send.  This function accepts as*/
   /* it's fourth argument, a pointer to a byte array of the            */
   /* acknowledged data to send.  This function returns zero if         */
   /* successful, otherwise this function returns a negative error code.*/
int BTPSAPI ANTM_Send_Acknowledged_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ANTManagerCallbackID) && (Data))
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Send_Acknowledged_Data(ANTEventCallbackID, ChannelNumber, DataLength, Data);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the sequence / channel  */
   /* number that the data will be sent on.  The upper three bits of    */
   /* this argument are the sequence number, and the lower five bits are*/
   /* the channel number.  This function accepts as it's third argument */
   /* the length of the data to send.  This function accepts as it's    */
   /* fourth argument, a pointer to a byte array of the burst data to   */
   /* send.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
int BTPSAPI ANTM_Send_Burst_Transfer_Data(unsigned int ANTManagerCallbackID, unsigned int SequenceChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ANTManagerCallbackID) && (Data))
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Send_Burst_Transfer_Data(ANTEventCallbackID, SequenceChannelNumber, DataLength, Data);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for putting the ANT+ system */
   /* in CW test mode.  This function accepts as it's only argument the */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
int BTPSAPI ANTM_Initialize_CW_Test_Mode(unsigned int ANTManagerCallbackID)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Initialize_CW_Test_Mode(ANTEventCallbackID);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument the      */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the transmit power level to be used.  This       */
   /* function accepts as it's third argument, the RF frequency to be   */
   /* used.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANTM_Initialize_CW_Test_Mode().                  */
int BTPSAPI ANTM_Set_CW_Test_Mode(unsigned int ANTManagerCallbackID, unsigned int TxPower, unsigned int RFFrequency)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANTManagerCallbackID)
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Set_CW_Test_Mode(ANTEventCallbackID, TxPower, RFFrequency);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a raw ANT       */
   /* packet.  This function accepts as it's first argument, the        */
   /* Callback ID that was returned from a successful call to           */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int BTPSAPI ANTM_Send_Raw_Packet(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ANTManagerCallbackID) && (PacketData) && ((DataSize + sizeof(NonAlignedWord_t)) <= (HCI_COMMAND_MAX_SIZE - HCI_COMMAND_HEADER_SIZE)))
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Send_Raw_Packet(ANTEventCallbackID, DataSize, PacketData);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a raw ANT       */
   /* packet without waiting for the command to be queued for sending   */
   /* to the chip.  This function accepts as it's first argument,       */
   /* the Callback ID that was returned from a successful call to       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int BTPSAPI ANTM_Send_Raw_Packet_Async(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData)
{
   int               ret_val;
   ANT_Entry_Info_t *ANTEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ANTManagerCallbackID) && (PacketData) && ((DataSize + sizeof(NonAlignedWord_t)) <= (HCI_COMMAND_MAX_SIZE - HCI_COMMAND_HEADER_SIZE)))
      {
         /* Attempt to wait for access to the ANT Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANTManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANTEntryInfoPtr = SearchANTEntryInfoEntry(&ANTEntryInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Go ahead and submit the command.                      */
               ret_val = _ANTM_Send_Raw_Packet_Async(ANTEventCallbackID, DataSize, PacketData);
            }
            else
               ret_val = BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANTManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
