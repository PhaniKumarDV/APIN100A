/*****< btpmhddm.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDDM - HDD Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/12/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHDDM.h"            /* BTPM HID Manager Prototypes/Constants.    */
#include "HDDMAPI.h"             /* HID Manager Prototypes/Constants.         */
#include "HDDMMSG.h"             /* BTPM HID Manager Message Formats.         */
#include "HDDMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHDD_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ConnectionStatus;
   Event_t                      ConnectionEvent;
   unsigned long                Flags;
   BD_ADDR_t                    BD_ADDR;
   HDDM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagHDD_Entry_Info_t *NextHDDEntryInfoPtr;
} HDD_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HDD_Entry_Info_t structure to denote various state information.   */
#define HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   HDDM_Event_Callback_t  EventCallback;
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
static Mutex_t HDDManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the current HDD Events Callback ID           */
   /* (registered with the Server to receive events).                   */
static unsigned int HDDEventsCallbackID;
static unsigned int HDDDataEventsCallbackID;

   /* Variable which holds a pointer to the first element in the HDD    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static HDD_Entry_Info_t *HDDEntryInfoList;

   /* Variable which holds a pointer to the first element of the HDD    */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static HDD_Entry_Info_t *HDDEntryInfoDataList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HDD_Entry_Info_t *AddHDDEntryInfoEntry(HDD_Entry_Info_t **ListHead, HDD_Entry_Info_t *EntryToAdd);
static HDD_Entry_Info_t *SearchHDDEntryInfoEntry(HDD_Entry_Info_t **ListHead, unsigned int CallbackID);
static HDD_Entry_Info_t *DeleteHDDEntryInfoEntry(HDD_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHDDEntryInfoEntryMemory(HDD_Entry_Info_t *EntryToFree);
static void FreeHDDEntryInfoList(HDD_Entry_Info_t **ListHead);

static void DispatchHDDEvent(HDDM_Event_Data_t *HDDMEventData);
static void DispatchHDDDataEvent(HDDM_Event_Data_t *HDDMEventData);

static void ProcessConnectionRequestEvent(HDDM_Connection_Request_Message_t *Message);
static void ProcessConnectedEvent(HDDM_Connected_Message_t *Message);
static void ProcessConnectionStatusEvent(HDDM_Connection_Status_Message_t *Message);
static void ProcessDisconnectedEvent(HDDM_Disconnected_Message_t *Message);
static void ProcessControlEvent(HDDM_Control_Event_Message_t *Message);
static void ProcessReportDataReceivedEvent(HDDM_Report_Data_Received_Message_t *Message);
static void ProcessGetReportRequestEvent(HDDM_Get_Report_Request_Message_t *Message);
static void ProcessSetReportRequestEvent(HDDM_Set_Report_Request_Message_t *Message);
static void ProcessGetProtocolRequestEvent(HDDM_Get_Protocol_Request_Message_t *Message);
static void ProcessSetProtocolRequestEvent(HDDM_Set_Protocol_Request_Message_t *Message);
static void ProcessGetIdleRequestEvent(HDDM_Get_Idle_Request_Message_t *Message);
static void ProcessSetIdleRequestEvent(HDDM_Set_Idle_Request_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI HDDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the HID Entry Information List.                              */
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
static HDD_Entry_Info_t *AddHDDEntryInfoEntry(HDD_Entry_Info_t **ListHead, HDD_Entry_Info_t *EntryToAdd)
{
   HDD_Entry_Info_t *AddedEntry = NULL;
   HDD_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HDD_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HDD_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHDDEntryInfoPtr = NULL;

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
                     FreeHDDEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHDDEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHDDEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHDDEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static HDD_Entry_Info_t *SearchHDDEntryInfoEntry(HDD_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HDD_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHDDEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HID Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HID Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHDDEntryInfoEntryMemory().                   */
static HDD_Entry_Info_t *DeleteHDDEntryInfoEntry(HDD_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HDD_Entry_Info_t *FoundEntry = NULL;
   HDD_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHDDEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHDDEntryInfoPtr = FoundEntry->NextHDDEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHDDEntryInfoPtr;

         FoundEntry->NextHDDEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified HID Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeHDDEntryInfoEntryMemory(HDD_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified HID Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeHDDEntryInfoList(HDD_Entry_Info_t **ListHead)
{
   HDD_Entry_Info_t *EntryToFree;
   HDD_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHDDEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeHDDEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HID event to every registered HID Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HID Manager Mutex*/
   /*          held.  Upon exit from this function it will free the HID */
   /*          Manager Mutex.                                           */
static void DispatchHDDEvent(HDDM_Event_Data_t *HDDMEventData)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HDDEntryInfoList) || (HDDEntryInfoDataList)) && (HDDMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      HDDEntryInfo    = HDDEntryInfoList;
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(HDDEntryInfo)
      {
         if((HDDEntryInfo->EventCallback) && (HDDEntryInfo->Flags & HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HDDEntryInfo = HDDEntryInfo->NextHDDEntryInfoPtr;
      }

      /* We need to add the HID Data Entry Information List as well.    */
      HDDEntryInfo  = HDDEntryInfoDataList;
      while(HDDEntryInfo)
      {
         if(HDDEntryInfo->EventCallback)
            NumberCallbacks++;

         HDDEntryInfo = HDDEntryInfo->NextHDDEntryInfoPtr;
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
            HDDEntryInfo    = HDDEntryInfoList;
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(HDDEntryInfo)
            {
               if((HDDEntryInfo->EventCallback) && (HDDEntryInfo->Flags & HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDDEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDDEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HDDEntryInfo = HDDEntryInfo->NextHDDEntryInfoPtr;
            }

            /* We need to add the HID Data Entry Information List as    */
            /* well.                                                    */
            HDDEntryInfo  = HDDEntryInfoDataList;
            while(HDDEntryInfo)
            {
               if(HDDEntryInfo->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDDEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDDEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HDDEntryInfo = HDDEntryInfo->NextHDDEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(HDDManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(HDDMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HDDManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void DispatchHDDDataEvent(HDDM_Event_Data_t *HDDMEventData)
{
   void                  *CallbackParameter;
   HDD_Entry_Info_t      *HDDEntryInfo;
   HDDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters seem valid.                              */
   if(HDDMEventData)
   {
      /* Attempt to get the data callback.                              */
      if((HDDEntryInfo = HDDEntryInfoDataList) != NULL)
      {
         /* Note the callback information.                              */
         EventCallback     = HDDEntryInfo->EventCallback;
         CallbackParameter = HDDEntryInfo->CallbackParameter;

         /* Release the mutex to make the callback.                     */
         BTPS_ReleaseMutex(HDDManagerMutex);

         __BTPSTRY
         {
            (*EventCallback)(HDDMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
         }
         else
         {
            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
   }
   else
   {
      /* Release the mutex.                                             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectionRequestEvent(HDDM_Connection_Request_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                                = hetHDDConnectionRequest;
      EventData.EventLength                                              = HDDM_CONNECTION_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.ConnectionRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      DispatchHDDEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectedEvent(HDDM_Connected_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                        = hetHDDConnected;
      EventData.EventLength                                      = HDDM_CONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      DispatchHDDEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectionStatusEvent(HDDM_Connection_Status_Message_t *Message)
{
   Boolean_t          ReleaseMutex;
   HDDM_Event_Data_t  HDDMEventData;
   HDD_Entry_Info_t  *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the the message is semi-valid.                              */
   if(Message)
   {
      /* Determine if there is an Event Callback waiting on this        */
      /* connection result.                                             */
      if(HDDEntryInfoList)
      {
         ReleaseMutex = TRUE;
         HDDEntryInfo = HDDEntryInfoList;
         while(HDDEntryInfo)
         {
            if((!(HDDEntryInfo->Flags & HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(HDDEntryInfo->BD_ADDR, Message->RemoteDeviceAddress)))
            {
               /* Callback registered, now see if the callback is       */
               /* synchronous or asynchronous.                          */
               if(HDDEntryInfo->ConnectionEvent)
               {
                  /* Synchronous.                                       */

                  /* Note the Status.                                   */
                  HDDEntryInfo->ConnectionStatus = Message->ConnectionStatus;

                  /* Set the Event.                                     */
                  BTPS_SetEvent(HDDEntryInfo->ConnectionEvent);

                  /* Break out of the list.                             */
                  HDDEntryInfo = NULL;

                  /* Release the Mutex because we are finished with it. */
                  BTPS_ReleaseMutex(HDDManagerMutex);

                  /* Flag that the Mutex does not need to be released.  */
                  ReleaseMutex = FALSE;
               }
               else
               {
                  /* Asynchronous Entry, go ahead dispatch the result.  */

                  /* Format up the Event.                               */
                  HDDMEventData.EventType                                               = hetHDDConnectionStatus;
                  HDDMEventData.EventLength                                             = HDDM_CONNECTION_STATUS_EVENT_DATA_SIZE;

                  HDDMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
                  HDDMEventData.EventData.ConnectionStatusEventData.ConnectionStatus    = Message->ConnectionStatus;

                  DispatchHDDEvent(&HDDMEventData);

                  /* Note that the previous call released the mutex.    */
                  ReleaseMutex = FALSE;

                  /* Break out of the list.                             */
                  HDDEntryInfo = NULL;
               }
            }
            else
               HDDEntryInfo = HDDEntryInfo->NextHDDEntryInfoPtr;
         }

         /* If the Mutex was not released, then we need to make sure we */
         /* release it.                                                 */
         if(ReleaseMutex)
            BTPS_ReleaseMutex(HDDManagerMutex);
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HDDManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisconnectedEvent(HDDM_Disconnected_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                           = hetHDDDisconnected;
      EventData.EventLength                                         = HDDM_DISCONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      DispatchHDDEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlEvent(HDDM_Control_Event_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                      = hetHDDControlEvent;
      EventData.EventLength                                    = HDDM_CONTROL_EVENT_DATA_SIZE;

      EventData.EventData.ControlEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.ControlEventData.ControlOperation    = Message->ControlOperation;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessReportDataReceivedEvent(HDDM_Report_Data_Received_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                                 = hetHDDReportDataReceived;
      EventData.EventLength                                               = HDDM_REPORT_DATA_RECEIVED_EVENT_DATA_SIZE;

      EventData.EventData.ReportDataReceivedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.ReportDataReceivedEventData.ReportLength        = Message->ReportLength;
      EventData.EventData.ReportDataReceivedEventData.ReportData          = Message->ReportData;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetReportRequestEvent(HDDM_Get_Report_Request_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                               = hetHDDGetReportRequest;
      EventData.EventLength                                             = HDDM_GET_REPORT_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.GetReportRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.GetReportRequestEventData.ReportType          = Message->ReportType;
      EventData.EventData.GetReportRequestEventData.SizeType            = Message->SizeType;
      EventData.EventData.GetReportRequestEventData.ReportID            = Message->ReportID;
      EventData.EventData.GetReportRequestEventData.BufferSize          = Message->BufferSize;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetReportRequestEvent(HDDM_Set_Report_Request_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                               = hetHDDSetReportRequest;
      EventData.EventLength                                             = HDDM_SET_REPORT_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.SetReportRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.SetReportRequestEventData.ReportType          = Message->ReportType;
      EventData.EventData.SetReportRequestEventData.ReportLength        = Message->ReportLength;
      EventData.EventData.SetReportRequestEventData.ReportData          = Message->ReportData;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetProtocolRequestEvent(HDDM_Get_Protocol_Request_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                                 = hetHDDGetProtocolRequest;
      EventData.EventLength                                               = HDDM_GET_PROTOCOL_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.GetProtocolRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetProtocolRequestEvent(HDDM_Set_Protocol_Request_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                                 = hetHDDSetProtocolRequest;
      EventData.EventLength                                               = HDDM_SET_PROTOCOL_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.SetProtocolRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.SetProtocolRequestEventData.Protocol            = Message->Protocol;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetIdleRequestEvent(HDDM_Get_Idle_Request_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                             = hetHDDGetIdleRequest;
      EventData.EventLength                                           = HDDM_GET_IDLE_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.GetIdleRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetIdleRequestEvent(HDDM_Set_Idle_Request_Message_t *Message)
{
   HDDM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the message is semi-valid.                              */
   if(Message)
   {
      /* Format the event data.                                         */
      EventData.EventType                                             = hetHDDSetIdleRequest;
      EventData.EventLength                                           = HDDM_SET_IDLE_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.SetIdleRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.SetIdleRequestEventData.IdleRate            = Message->IdleRate;

      DispatchHDDDataEvent(&EventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HDDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the HID Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessConnectionRequestEvent((HDDM_Connection_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessConnectedEvent((HDDM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessConnectionStatusEvent((HDDM_Connection_Status_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessDisconnectedEvent((HDDM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_CONTROL_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Event Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_CONTROL_EVENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessControlEvent((HDDM_Control_Event_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_REPORT_DATA_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Report Data Received Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_REPORT_DATA_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_REPORT_DATA_RECEIVED_MESSAGE_SIZE(((HDDM_Report_Data_Received_Message_t *)Message)->ReportLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessReportDataReceivedEvent((HDDM_Report_Data_Received_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_GET_REPORT_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Report Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_GET_REPORT_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessGetReportRequestEvent((HDDM_Get_Report_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SET_REPORT_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Report Request Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SET_REPORT_REQUEST_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SET_REPORT_REQUEST_MESSAGE_SIZE(((HDDM_Set_Report_Request_Message_t *)Message)->ReportLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessSetReportRequestEvent((HDDM_Set_Report_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_GET_PROTOCOL_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Protocol Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_GET_PROTOCOL_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessGetProtocolRequestEvent((HDDM_Get_Protocol_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SET_PROTOCOL_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Protocol Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SET_PROTOCOL_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessSetProtocolRequestEvent((HDDM_Set_Protocol_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_GET_IDLE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Idle Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_GET_IDLE_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessGetIdleRequestEvent((HDDM_Get_Idle_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SET_IDLE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Idle Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SET_IDLE_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessSetIdleRequestEvent((HDDM_Set_Idle_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(HDDManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI BTPMDispatchCallback_HDDM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HDD state information.    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HDD state information.    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            HDDEntryInfo = HDDEntryInfoList;

            while(HDDEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if((!(HDDEntryInfo->Flags & HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HDDEntryInfo->ConnectionEvent))
               {
                  HDDEntryInfo->ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(HDDEntryInfo->ConnectionEvent);
               }

               HDDEntryInfo = HDDEntryInfo->NextHDDEntryInfoPtr;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI HDDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HDD_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HDD Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a HID Manager defined    */
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
               /* HID Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDDM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HDD Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HDD Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an HID Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HDD Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HID Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HDDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((HDDManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process HDD Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HDD_MANAGER, HDDManagerGroupHandler, NULL))
            {
               /* Initialize the actual HDD Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the HDD Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _HDDM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the HDD Manager Server. */
                  Result = _HDDM_Register_Event_Callback();

                  if(Result > 0)
                  {
                     HDDEventsCallbackID = (unsigned int)Result;

                     /* Initialize a unique, starting HDD Callback ID.  */
                     NextCallbackID      = 0x000000001;

                     /* Go ahead and flag that this module is           */
                     /* initialized.                                    */
                     Initialized         = TRUE;
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
            if(HDDEventsCallbackID)
               _HDDM_Un_Register_Event_Callback(HDDEventsCallbackID);

            if(HDDManagerMutex)
               BTPS_CloseMutex(HDDManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HDD_MANAGER);

            /* Flag that none of the resources are allocated.           */
            HDDManagerMutex     = NULL;
            HDDEventsCallbackID = 0;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HDD Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HDD_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for HDD Events.                              */
            if(HDDEventsCallbackID)
               _HDDM_Un_Register_Event_Callback(HDDEventsCallbackID);

            /* Next, Un-Register for any HDD Data Events.               */
            if(HDDDataEventsCallbackID)
               _HDDM_Un_Register_Data_Event_Callback(HDDDataEventsCallbackID);

            /* Make sure we inform the HDD Manager Implementation that  */
            /* we are shutting down.                                    */
            _HDDM_Cleanup();

            BTPS_CloseMutex(HDDManagerMutex);

            /* Make sure that the HDD Entry Information List is empty.  */
            FreeHDDEntryInfoList(&HDDEntryInfoList);

            /* Make sure that the HDD Entry Data Information List is    */
            /* empty.                                                   */
            FreeHDDEntryInfoList(&HDDEntryInfoDataList);

            /* Flag that the resources are no longer allocated.         */
            HDDManagerMutex     = NULL;
            CurrentPowerState   = FALSE;
            HDDEventsCallbackID = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* HID Entries and set any events that have synchronous  */
               /* operations pending.                                   */
               HDDEntryInfo = HDDEntryInfoList;

               while(HDDEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((!(HDDEntryInfo->Flags & HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HDDEntryInfo->ConnectionEvent))
                  {
                     HDDEntryInfo->ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(HDDEntryInfo->ConnectionEvent);
                  }

                  HDDEntryInfo = HDDEntryInfo->NextHDDEntryInfoPtr;
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HDDManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Accept or reject (authorize) an incoming HID connection from a    */
   /* remote HID Host.  This function returns zero if successful, or a  */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HDD Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI HDDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Submit the request to the PM Server.                        */
         ret_val = _HDDM_Connection_Request_Response(RemoteDeviceAddress, Accept);
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Connect to a remote HID Host device.  The RemoteDeviceAddress is  */
   /* the Bluetooth Address of the remote HID Host.  The ConnectionFlags*/
   /* specifiy whay security, if any, is required for the connection.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHDDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HDDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI HDDM_Connect_Remote_Host(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int               ret_val;
   Event_t           ConnectionEvent;
   BD_ADDR_t         NULL_BD_ADDR;
   unsigned int      CallbackID;
   HDD_Entry_Info_t  HDDEntryInfo;
   HDD_Entry_Info_t *HDDEntryInfoPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDD Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Attempt to wait for access to the HDD Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* HID Entry list.                                       */
               BTPS_MemInitialize(&HDDEntryInfo, 0, sizeof(HDD_Entry_Info_t));

               HDDEntryInfo.CallbackID        = GetNextCallbackID();
               HDDEntryInfo.EventCallback     = CallbackFunction;
               HDDEntryInfo.CallbackParameter = CallbackParameter;
               HDDEntryInfo.BD_ADDR           = RemoteDeviceAddress;

               if(ConnectionStatus)
                  HDDEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

               if((!ConnectionStatus) || ((ConnectionStatus) && (HDDEntryInfo.ConnectionEvent)))
               {
                  if((HDDEntryInfoPtr = AddHDDEntryInfoEntry(&HDDEntryInfoList, &HDDEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote device 0x%08lX\n", ConnectionFlags));

                     /* Next, attempt to open the remote device.        */
                     if((ret_val = _HDDM_Connect_Remote_Host(RemoteDeviceAddress, ConnectionFlags)) != 0)
                     {
                        /* Error opening device, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((HDDEntryInfoPtr = DeleteHDDEntryInfoEntry(&HDDEntryInfoList, HDDEntryInfoPtr->CallbackID)) != NULL)
                        {
                           if(HDDEntryInfoPtr->ConnectionEvent)
                              BTPS_CloseEvent(HDDEntryInfoPtr->ConnectionEvent);

                           FreeHDDEntryInfoEntryMemory(HDDEntryInfoPtr);
                        }
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((!ret_val) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Callback ID.                        */
                        CallbackID      = HDDEntryInfoPtr->CallbackID;

                        /* Note the Open Event.                         */
                        ConnectionEvent = HDDEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(HDDManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((HDDEntryInfoPtr = DeleteHDDEntryInfoEntry(&HDDEntryInfoList, CallbackID)) != NULL)
                           {
                              /* Note the connection status.            */
                              *ConnectionStatus = HDDEntryInfoPtr->ConnectionStatus;

                              BTPS_CloseEvent(HDDEntryInfoPtr->ConnectionEvent);

                              FreeHDDEntryInfoEntryMemory(HDDEntryInfoPtr);

                              /* Flag success to the caller.            */
                              ret_val = 0;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_HDD_UNABLE_TO_CONNECT_REMOTE_HOST;
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
                           if((HDDEntryInfoPtr = DeleteHDDEntryInfoEntry(&HDDEntryInfoList, HDDEntryInfo.CallbackID)) != NULL)
                           {
                              if(HDDEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(HDDEntryInfoPtr->ConnectionEvent);

                              FreeHDDEntryInfoEntryMemory(HDDEntryInfoPtr);
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
               BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Disconnect from a remote HID Host.  The RemoteDeviceAddress       */
   /* is the Bluetooth Address of the remote HID Host.  The             */
   /* SendVirtualCableUnplug parameter indicates whether the device     */
   /* should be disconnected with a Virtual Cable Unplug (TRUE) or      */
   /* simply at the Bluetooth Link (FALSE).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int BTPSAPI HDDM_Disconnect(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Submit the request to the PM Server.                        */
         ret_val = _HDDM_Disconnect(RemoteDeviceAddress, SendVirtualCableUnplug);
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Determine if there are currently any connected HID Hosts.  This   */
   /* function accepts a pointer to a buffer that will receive any      */
   /* currently connected HID Hosts.  The first parameter specifies the */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer).  The next parameter is      */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI HDDM_Query_Connected_Hosts(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Submit the request to the PM Server.                           */
      ret_val = _HDDM_Query_Connected_Hosts(MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int BTPSAPI HDDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Submit the request to the PM Server.                        */
         ret_val = _HDDM_Change_Incoming_Connection_Flags(ConnectionFlags);
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Send the specified HID Report Data to a currently connected       */
   /* remote device.  This function accepts as input the HDD            */
   /* Manager Report Data Handler ID (registered via call to the        */
   /* HDDM_Register_Data_Event_Callback() function), followed by the    */
   /* remote device address of the remote HID Host to send the report   */
   /* data to, followed by the report data itself.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI HDDM_Send_Report_Data(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int               ret_val;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Acquire the lock that protects the list.                    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure the Data Callback ID is registered.            */
            if((HDDEntryInfo = SearchHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Submit the request to the PM Server.                  */
               ret_val = _HDDM_Send_Report_Data(HDDDataEventsCallbackID, RemoteDeviceAddress, ReportDataLength, ReportData);
            }
            else
               ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED;

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a GetReportRequest.  The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* ReportType indicates the type of report being sent as the         */
   /* response.  The ReportDataLength indicates the size of the report  */
   /* data.  ReportData is a pointer to the report data buffer.  This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Get_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int               ret_val;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Acquire the lock that protects the list.                    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure the Data Callback ID is registered.            */
            if((HDDEntryInfo = SearchHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Submit the request to the PM Server.                  */
               ret_val = _HDDM_Get_Report_Response(HDDDataEventsCallbackID, RemoteDeviceAddress, Result, ReportType, ReportDataLength, ReportData);
            }
            else
               ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED;

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Responsd to a SetReportRequest. The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Set_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int               ret_val;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Acquire the lock that protects the list.                    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure the Data Callback ID is registered.            */
            if((HDDEntryInfo = SearchHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Submit the request to the PM Server.                  */
               ret_val = _HDDM_Set_Report_Response(HDDDataEventsCallbackID, RemoteDeviceAddress, Result);
            }
            else
               ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED;

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a GetProtocolRequest.  The HDDManagerDataCallback      */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* Protocol indicates the current HID Protocol.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int BTPSAPI HDDM_Get_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol)
{
   int               ret_val;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Acquire the lock that protects the list.                    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure the Data Callback ID is registered.            */
            if((HDDEntryInfo = SearchHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Submit the request to the PM Server.                  */
               ret_val = _HDDM_Get_Protocol_Response(HDDDataEventsCallbackID, RemoteDeviceAddress, Result, Protocol);
            }
            else
               ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED;

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a SetProtocolResponse.  The HDDManagerDataCallback     */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Set_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int               ret_val;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Acquire the lock that protects the list.                    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure the Data Callback ID is registered.            */
            if((HDDEntryInfo = SearchHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Submit the request to the PM Server.                  */
               ret_val = _HDDM_Set_Protocol_Response(HDDDataEventsCallbackID, RemoteDeviceAddress, Result);
            }
            else
               ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED;

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a GetIdleResponse.  The HDDManagerDataCallback         */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* IdleRate is the current Idle Rate.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
int BTPSAPI HDDM_Get_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate)
{
   int               ret_val;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Acquire the lock that protects the list.                    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure the Data Callback ID is registered.            */
            if((HDDEntryInfo = SearchHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Submit the request to the PM Server.                  */
               ret_val = _HDDM_Get_Idle_Response(HDDDataEventsCallbackID, RemoteDeviceAddress, Result, IdleRate);
            }
            else
               ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED;

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a SetIdleRequest.  The HDDManagerDataCallback          */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Set_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int               ret_val;
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Make sure the device is powered on.                            */
      if(CurrentPowerState)
      {
         /* Acquire the lock that protects the list.                    */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure the Data Callback ID is registered.            */
            if((HDDEntryInfo = SearchHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Submit the request to the PM Server.                  */
               ret_val = _HDDM_Set_Idle_Response(HDDDataEventsCallbackID, RemoteDeviceAddress, Result);
            }
            else
               ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED;

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service.  This Callback will be dispatched by*/
   /* the HID Manager when various HID Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HID Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          HDDM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int BTPSAPI HDDM_Register_Event_Callback(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   HDD_Entry_Info_t HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDD Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the HDD Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the HID Entry list.         */
            BTPS_MemInitialize(&HDDEntryInfo, 0, sizeof(HDD_Entry_Info_t));

            HDDEntryInfo.CallbackID         = GetNextCallbackID();
            HDDEntryInfo.EventCallback      = CallbackFunction;
            HDDEntryInfo.CallbackParameter  = CallbackParameter;
            HDDEntryInfo.Flags              = HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if(AddHDDEntryInfoEntry(&HDDEntryInfoList, &HDDEntryInfo))
               ret_val = HDDEntryInfo.CallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_RegisterEventCallback() function).  This function accepts as */
   /* input the HID Manager Event Callback ID (return value from        */
   /* HDDM_RegisterEventCallback() function).                           */
void BTPSAPI HDDM_Un_Register_Event_Callback(unsigned int HDDManagerCallbackID)
{
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDD Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HDDManagerCallbackID)
      {
         /* Attempt to wait for access to the HDD Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((HDDEntryInfo = DeleteHDDEntryInfoEntry(&HDDEntryInfoList, HDDManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeHDDEntryInfoEntryMemory(HDDEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service to explicitly process HID report     */
   /* data.  This Callback will be dispatched by the HID Manager when   */
   /* various HID Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HID Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HDDM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HDDM_Register_Data_Event_Callback(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   HDD_Entry_Info_t  HDDEntryInfo;
   HDD_Entry_Info_t *HDDEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDD Manager has been initialized.   */
   if(Initialized)
   {
      /* HDD Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the HDD Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, Register the handler locally.                     */
            BTPS_MemInitialize(&HDDEntryInfo, 0, sizeof(HDD_Entry_Info_t));

            HDDEntryInfo.CallbackID         = GetNextCallbackID();
            HDDEntryInfo.EventCallback      = CallbackFunction;
            HDDEntryInfo.CallbackParameter  = CallbackParameter;
            HDDEntryInfo.Flags              = HDD_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if((HDDEntryInfoPtr = AddHDDEntryInfoEntry(&HDDEntryInfoDataList, &HDDEntryInfo)) != NULL)
            {
               /* Attempt to register it with the system.               */
               if((ret_val = _HDDM_Register_Data_Event_Callback()) > 0)
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  HDDDataEventsCallbackID = (unsigned int)ret_val;

                  ret_val                 = HDDEntryInfoPtr->CallbackID;
               }
               else
               {
                  /* Error, go ahead and delete the entry we added      */
                  /* locally.                                           */
                  if((HDDEntryInfoPtr = DeleteHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDEntryInfoPtr->CallbackID)) != NULL)
                     FreeHDDEntryInfoEntryMemory(HDDEntryInfoPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from HDDM_Register_Data_Event_Callback() function).         */
void BTPSAPI HDDM_Un_Register_Data_Event_Callback(unsigned int HDDManagerDataCallbackID)
{
   HDD_Entry_Info_t *HDDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDD Manager has been initialized.   */
   if(Initialized)
   {
      /* HDD Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HDDManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HDD Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HDDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, delete the local handler.                         */
            if((HDDEntryInfo = DeleteHDDEntryInfoEntry(&HDDEntryInfoDataList, HDDManagerDataCallbackID)) != NULL)
            {
               /* Handler found, go ahead and delete it from the server.*/
               _HDDM_Un_Register_Data_Event_Callback(HDDManagerDataCallbackID);

               /* All finished with the entry, delete it.               */
               FreeHDDEntryInfoEntryMemory(HDDEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDDManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

