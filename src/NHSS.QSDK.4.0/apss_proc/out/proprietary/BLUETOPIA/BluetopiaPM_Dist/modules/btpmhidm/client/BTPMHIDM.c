/*****< btpmhidm.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHIDM - HID Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/18/11  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHIDM.h"            /* BTPM HID Manager Prototypes/Constants.    */
#include "HIDMAPI.h"             /* HID Manager Prototypes/Constants.         */
#include "HIDMMSG.h"             /* BTPM HID Manager Message Formats.         */
#include "HIDMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHID_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ConnectionStatus;
   Event_t                      ConnectionEvent;
   unsigned long                Flags;
   BD_ADDR_t                    BD_ADDR;
   HIDM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagHID_Entry_Info_t *NextHIDEntryInfoPtr;
} HID_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HID_Entry_Info_t structure to denote various state information.   */
#define HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   HIDM_Event_Callback_t  EventCallback;
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
static Mutex_t HIDManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the current HID Events Callback ID           */
   /* (registered with the Server to receive events).                   */
static unsigned int HIDEventsCallbackID;

   /* Variable which holds a pointer to the first element in the HID    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static HID_Entry_Info_t *HIDEntryInfoList;

   /* Variable which holds a pointer to the first element of the HID    */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static HID_Entry_Info_t *HIDEntryInfoDataList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HID_Entry_Info_t *AddHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, HID_Entry_Info_t *EntryToAdd);
static HID_Entry_Info_t *SearchHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID);
static HID_Entry_Info_t *DeleteHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHIDEntryInfoEntryMemory(HID_Entry_Info_t *EntryToFree);
static void FreeHIDEntryInfoList(HID_Entry_Info_t **ListHead);

static void DispatchHIDEvent(HIDM_Event_Data_t *HIDMEventData);

static void ProcessIncomingConnectionRequestEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessHIDDeviceConnectedEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessHIDDeviceConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus);
static void ProcessHIDDeviceDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessBootKeyboardKeyPressEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, Boolean_t KeyDown, Byte_t KeyModifiers, Byte_t Key);
static void ProcessBootKeyboardKeyRepeatEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, Byte_t KeyModifiers, Byte_t Key);
static void ProcessBootMouseMouseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, SByte_t CX, SByte_t CY, SByte_t CZ, Byte_t ButtonState);

static void ProcessHIDReportDataReceivedEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, Word_t ReportLength, Byte_t *ReportData);
static void ProcessGetReportResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status, HIDM_Report_Type_t ReportType, Word_t ReportLength, Byte_t *ReportData);
static void ProcessSetReportResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status);
static void ProcessGetProtocolResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status, HIDM_Protocol_t Protocol);
static void ProcessSetProtocolResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status);
static void ProcessGetIdleResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status, Byte_t IdleRate);
static void ProcessSetIdleResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_HIDM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HIDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

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
static HID_Entry_Info_t *AddHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, HID_Entry_Info_t *EntryToAdd)
{
   HID_Entry_Info_t *AddedEntry = NULL;
   HID_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HID_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HID_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHIDEntryInfoPtr = NULL;

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
                     FreeHIDEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHIDEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHIDEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHIDEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static HID_Entry_Info_t *SearchHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HID_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHIDEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HID Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HID Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHIDEntryInfoEntryMemory().                   */
static HID_Entry_Info_t *DeleteHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HID_Entry_Info_t *FoundEntry = NULL;
   HID_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHIDEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHIDEntryInfoPtr = FoundEntry->NextHIDEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHIDEntryInfoPtr;

         FoundEntry->NextHIDEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified HID Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeHIDEntryInfoEntryMemory(HID_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified HID Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeHIDEntryInfoList(HID_Entry_Info_t **ListHead)
{
   HID_Entry_Info_t *EntryToFree;
   HID_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHIDEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeHIDEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HID event to every registered HID Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HID Manager Mutex*/
   /*          held.  Upon exit from this function it will free the HID */
   /*          Manager Mutex.                                           */
static void DispatchHIDEvent(HIDM_Event_Data_t *HIDMEventData)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HIDEntryInfoList) || (HIDEntryInfoDataList)) && (HIDMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      HIDEntryInfo    = HIDEntryInfoList;
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(HIDEntryInfo)
      {
         if((HIDEntryInfo->EventCallback) && (HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
      }

      /* We need to add the HID Data Entry Information List as well.    */
      HIDEntryInfo  = HIDEntryInfoDataList;
      while(HIDEntryInfo)
      {
         if(HIDEntryInfo->EventCallback)
            NumberCallbacks++;

         HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
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
            HIDEntryInfo    = HIDEntryInfoList;
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(HIDEntryInfo)
            {
               if((HIDEntryInfo->EventCallback) && (HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HIDEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HIDEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
            }

            /* We need to add the HID Data Entry Information List as    */
            /* well.                                                    */
            HIDEntryInfo  = HIDEntryInfoDataList;
            while(HIDEntryInfo)
            {
               if(HIDEntryInfo->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HIDEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HIDEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(HIDManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(HIDMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(HIDManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HIDManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Incoming */
   /* Connection Request asynchronous message.                          */
static void ProcessIncomingConnectionRequestEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HIDM_Event_Data_t HIDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                 = hetHIDDeviceConnectionRequest;
   HIDMEventData.EventLength                                               = HIDM_HID_DEVICE_CONNECTION_REQUEST_EVENT_DATA_SIZE;

   HIDMEventData.EventData.DeviceConnectionRequestData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHIDEvent(&HIDMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID      */
   /* Device Connected asynchronous message.                            */
static void ProcessHIDDeviceConnectedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HIDM_Event_Data_t HIDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                              = hetHIDDeviceConnected;
   HIDMEventData.EventLength                                            = HIDM_HID_DEVICE_CONNECTED_EVENT_DATA_SIZE;

   HIDMEventData.EventData.DeviceConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHIDEvent(&HIDMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID      */
   /* Device Connection Status asynchronous message.                    */
static void ProcessHIDDeviceConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus)
{
   void                  *CallbackParameter;
   Boolean_t              ReleaseMutex;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if(HIDEntryInfoList)
   {
      ReleaseMutex = TRUE;
      HIDEntryInfo = HIDEntryInfoList;
      while(HIDEntryInfo)
      {
         if((!(HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(HIDEntryInfo->BD_ADDR, RemoteDeviceAddress)))
         {
            /* Callback registered, now see if the callback is          */
            /* synchronous or asynchronous.                             */
            if(HIDEntryInfo->ConnectionEvent)
            {
               /* Synchronous.                                          */

               /* Note the Status.                                      */
               HIDEntryInfo->ConnectionStatus = ConnectionStatus;

               /* Set the Event.                                        */
               BTPS_SetEvent(HIDEntryInfo->ConnectionEvent);

               /* Break out of the list.                                */
               HIDEntryInfo = NULL;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HIDManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;
            }
            else
            {
               /* Asynchronous Entry, go ahead dispatch the result.     */

               /* Format up the Event.                                  */
               HIDMEventData.EventType                                                     = hetHIDDeviceConnectionStatus;
               HIDMEventData.EventLength                                                   = HIDM_HID_DEVICE_CONNECTION_STATUS_EVENT_DATA_SIZE;

               HIDMEventData.EventData.DeviceConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
               HIDMEventData.EventData.DeviceConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

               /* Note the Callback information.                        */
               EventCallback     = HIDEntryInfo->EventCallback;
               CallbackParameter = HIDEntryInfo->CallbackParameter;

               if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfo->CallbackID)) != NULL)
                  FreeHIDEntryInfoEntryMemory(HIDEntryInfo);

               /* Release the Mutex so we can dispatch the event.       */
               BTPS_ReleaseMutex(HIDManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&HIDMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Break out of the list.                                */
               HIDEntryInfo = NULL;
            }
         }
         else
            HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
      }

      /* If the Mutex was not released, then we need to make sure we    */
      /* release it.                                                    */
      if(ReleaseMutex)
         BTPS_ReleaseMutex(HIDManagerMutex);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID      */
   /* Device Disconnected asynchronous message.                         */
static void ProcessHIDDeviceDisconnectedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HIDM_Event_Data_t HIDMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                              = hetHIDDeviceDisconnected;
   HIDMEventData.EventLength                                            = HIDM_HID_DEVICE_DISCONNECTED_EVENT_DATA_SIZE;

   HIDMEventData.EventData.DeviceConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHIDEvent(&HIDMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Boot     */
   /* Keyboard Key Press Event asynchronous message.                    */
static void ProcessBootKeyboardKeyPressEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, Boolean_t KeyDown, Byte_t KeyModifiers, Byte_t Key)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                        = hetHIDBootKeyboardKeyPress;
   HIDMEventData.EventLength                                                      = HIDM_HID_BOOT_KEYBOARD_KEY_PRESS_EVENT_DATA_SIZE;

   HIDMEventData.EventData.BootKeyboardKeyPressEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.BootKeyboardKeyPressEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.BootKeyboardKeyPressEventData.KeyDown                  = KeyDown;
   HIDMEventData.EventData.BootKeyboardKeyPressEventData.KeyModifiers             = KeyModifiers;
   HIDMEventData.EventData.BootKeyboardKeyPressEventData.Key                      = Key;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                                  = HIDEntryInfo->EventCallback;
      CallbackParameter                                                              = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.BootKeyboardKeyPressEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Boot     */
   /* Keyboard Key Repeat Event asynchronous message.                   */
static void ProcessBootKeyboardKeyRepeatEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, Byte_t KeyModifiers, Byte_t Key)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                         = hetHIDBootKeyboardKeyRepeat;
   HIDMEventData.EventLength                                                       = HIDM_HID_BOOT_KEYBOARD_KEY_REPEAT_EVENT_DATA_SIZE;

   HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.KeyModifiers             = KeyModifiers;
   HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.Key                      = Key;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                                   = HIDEntryInfo->EventCallback;
      CallbackParameter                                                               = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Boot     */
   /* Mouse Mouse Event asynchronous message.                           */
static void ProcessBootMouseMouseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, SByte_t CX, SByte_t CY, SByte_t CZ, Byte_t ButtonState)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                  = hetHIDBootMouseEvent;
   HIDMEventData.EventLength                                                = HIDM_HID_BOOT_MOUSE_EVENT_EVENT_DATA_SIZE;

   HIDMEventData.EventData.BootMouseEventEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.BootMouseEventEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.BootMouseEventEventData.CX                       = CX;
   HIDMEventData.EventData.BootMouseEventEventData.CY                       = CY;
   HIDMEventData.EventData.BootMouseEventEventData.ButtonState              = ButtonState;
   HIDMEventData.EventData.BootMouseEventEventData.CZ                       = CZ;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                            = HIDEntryInfo->EventCallback;
      CallbackParameter                                                        = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.BootMouseEventEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID      */
   /* Report Data Received asynchronous message.                        */
static void ProcessHIDReportDataReceivedEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, Word_t ReportLength, Byte_t *ReportData)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   if((ReportLength) && (ReportData))
   {
      HIDMEventData.EventType                                                      = hetHIDReportDataReceived;
      HIDMEventData.EventLength                                                    = HIDM_HID_REPORT_DATA_RECEIVED_EVENT_DATA_SIZE;

      HIDMEventData.EventData.ReportDataReceivedEventData.HIDManagerDataCallbackID = 0;
      HIDMEventData.EventData.ReportDataReceivedEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
      HIDMEventData.EventData.ReportDataReceivedEventData.ReportLength             = ReportLength;
      HIDMEventData.EventData.ReportDataReceivedEventData.ReportData               = ReportData;

      /* Now that the event is formatted, dispatch it.                  */

      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
      {
         /* Note the Callback Information.                              */
         EventCallback                                                                = HIDEntryInfo->EventCallback;
         CallbackParameter                                                            = HIDEntryInfo->CallbackParameter;

         /* Note that we need to map the Server Callback to the Client  */
         /* Callback ID.                                                */
         HIDMEventData.EventData.ReportDataReceivedEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HIDManagerMutex);

         __BTPSTRY
         {
            (*EventCallback)(&HIDMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HIDManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID Get  */
   /* Report Response asynchronous message.                             */
static void ProcessGetReportResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status, HIDM_Report_Type_t ReportType, Word_t ReportLength, Byte_t *ReportData)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   if((ReportLength) && (ReportData))
   {
      HIDMEventData.EventType                                                     = hetHIDGetReportResponse;
      HIDMEventData.EventLength                                                   = HIDM_HID_GET_REPORT_RESPONSE_DATA_SIZE;

      HIDMEventData.EventData.GetReportResponseEventData.HIDManagerDataCallbackID = 0;
      HIDMEventData.EventData.GetReportResponseEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
      HIDMEventData.EventData.GetReportResponseEventData.ReportType               = ReportType;
      HIDMEventData.EventData.GetReportResponseEventData.ReportLength             = ReportLength;
      HIDMEventData.EventData.GetReportResponseEventData.ReportData               = ReportData;

      /* Now that the event is formatted, dispatch it.                  */

      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
      {
         /* Note the Callback Information.                              */
         EventCallback                                                               = HIDEntryInfo->EventCallback;
         CallbackParameter                                                           = HIDEntryInfo->CallbackParameter;

         /* Note that we need to map the Server Callback to the Client  */
         /* Callback ID.                                                */
         HIDMEventData.EventData.GetReportResponseEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HIDManagerMutex);

         __BTPSTRY
         {
            (*EventCallback)(&HIDMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HIDManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID Set  */
   /* Report Response asynchronous message.                             */
static void ProcessSetReportResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                     = hetHIDSetReportResponse;
   HIDMEventData.EventLength                                                   = HIDM_HID_SET_REPORT_RESPONSE_DATA_SIZE;

   HIDMEventData.EventData.SetReportResponseEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.SetReportResponseEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.SetReportResponseEventData.Status                   = Status;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                               = HIDEntryInfo->EventCallback;
      CallbackParameter                                                           = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.SetReportResponseEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID Get  */
   /* Protocol Response asynchronous message.                           */
static void ProcessGetProtocolResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status, HIDM_Protocol_t Protocol)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                       = hetHIDGetProtocolResponse;
   HIDMEventData.EventLength                                                     = HIDM_HID_GET_PROTOCOL_RESPONSE_DATA_SIZE;

   HIDMEventData.EventData.GetProtocolResponseEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.GetProtocolResponseEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.GetProtocolResponseEventData.Status                   = Status;
   HIDMEventData.EventData.GetProtocolResponseEventData.Protocol                 = Protocol;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                                 = HIDEntryInfo->EventCallback;
      CallbackParameter                                                             = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.GetProtocolResponseEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID Set  */
   /* Protocol Response asynchronous message.                           */
static void ProcessSetProtocolResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                       = hetHIDSetProtocolResponse;
   HIDMEventData.EventLength                                                     = HIDM_HID_SET_PROTOCOL_RESPONSE_DATA_SIZE;

   HIDMEventData.EventData.SetProtocolResponseEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.SetProtocolResponseEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.SetProtocolResponseEventData.Status                   = Status;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                                 = HIDEntryInfo->EventCallback;
      CallbackParameter                                                             = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.SetProtocolResponseEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID Get  */
   /* Idle Response asynchronous message.                               */
static void ProcessGetIdleResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status, Byte_t IdleRate)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                   = hetHIDGetIdleResponse;
   HIDMEventData.EventLength                                                 = HIDM_HID_GET_IDLE_RESPONSE_DATA_SIZE;

   HIDMEventData.EventData.GetIdleResponseEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.GetIdleResponseEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.GetIdleResponseEventData.Status                   = Status;
   HIDMEventData.EventData.GetIdleResponseEventData.IdleRate                 = IdleRate;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                             = HIDEntryInfo->EventCallback;
      CallbackParameter                                                         = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.GetIdleResponseEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID Set  */
   /* Idle Response asynchronous message.                               */
static void ProcessSetIdleResponseEvent(unsigned int HIDDataEventsHandlerID, BD_ADDR_t RemoteDeviceAddress, HIDM_Result_t Status)
{
   void                  *CallbackParameter;
   HIDM_Event_Data_t      HIDMEventData;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HIDMEventData.EventType                                                   = hetHIDSetIdleResponse;
   HIDMEventData.EventLength                                                 = HIDM_HID_SET_IDLE_RESPONSE_DATA_SIZE;

   HIDMEventData.EventData.SetIdleResponseEventData.HIDManagerDataCallbackID = 0;
   HIDMEventData.EventData.SetIdleResponseEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HIDMEventData.EventData.SetIdleResponseEventData.Status                   = Status;

   /* Now that the event is formatted, dispatch it.                     */

   /* Before going any further, check to see if someone has registered  */
   /* to process the data.                                              */
   if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
   {
      /* Note the Callback Information.                                 */
      EventCallback                                                             = HIDEntryInfo->EventCallback;
      CallbackParameter                                                         = HIDEntryInfo->CallbackParameter;

      /* Note that we need to map the Server Callback to the Client     */
      /* Callback ID.                                                   */
      HIDMEventData.EventData.SetIdleResponseEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);

      __BTPSTRY
      {
         (*EventCallback)(&HIDMEventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HIDManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessIncomingConnectionRequestEvent(((HIDM_Connection_Request_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connected Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_DEVICE_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connected Indication Event.                    */
               ProcessHIDDeviceConnectedEvent(((HIDM_HID_Device_Connected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connection Status Event.                       */
               ProcessHIDDeviceConnectionStatusEvent(((HIDM_HID_Device_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Device_Connection_Status_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Disconnected Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_DEVICE_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Disconnected Event.                            */
               ProcessHIDDeviceDisconnectedEvent(((HIDM_HID_Device_Disconnected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_BOOT_KEYBOARD_KEY_PRESS_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Boot Keyboard Key Press Event Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_BOOT_KEYBOARD_KEY_PRESS_EVENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Boot*/
               /* Keyboard Key Press Event.                             */
               ProcessBootKeyboardKeyPressEvent(((HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t *)Message)->KeyDown, ((HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t *)Message)->KeyModifiers, ((HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t *)Message)->Key);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_BOOT_KEYBOARD_KEY_REPEAT_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Boot Keyboard Key Repeat Event Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_BOOT_KEYBOARD_KEY_REPEAT_EVENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Boot*/
               /* Keyboard Key Repeat Event.                            */
               ProcessBootKeyboardKeyRepeatEvent(((HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t *)Message)->KeyModifiers, ((HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t *)Message)->Key);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_BOOT_MOUSE_MOUSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Boot Mouse Mouse Event Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_BOOT_MOUSE_MOUSE_EVENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Boot*/
               /* Mouse Mouse Event.                                    */
               ProcessBootMouseMouseEvent(((HIDM_HID_Boot_Mouse_Mouse_Event_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Boot_Mouse_Mouse_Event_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Boot_Mouse_Mouse_Event_Message_t *)Message)->CX, ((HIDM_HID_Boot_Mouse_Mouse_Event_Message_t *)Message)->CY, ((HIDM_HID_Boot_Mouse_Mouse_Event_Message_t *)Message)->CZ, ((HIDM_HID_Boot_Mouse_Mouse_Event_Message_t *)Message)->ButtonState);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_HID_REPORT_DATA_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Data Report Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_REPORT_DATA_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_REPORT_DATA_RECEIVED_MESSAGE_SIZE(((HIDM_HID_Report_Data_Received_Message_t *)Message)->ReportLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Data*/
               /* Report Received Event.                                */
               ProcessHIDReportDataReceivedEvent(((HIDM_HID_Report_Data_Received_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Report_Data_Received_Message_t *)Message)->RemoteDeviceAddress, (Word_t)(((HIDM_HID_Report_Data_Received_Message_t *)Message)->ReportLength), ((HIDM_HID_Report_Data_Received_Message_t *)Message)->ReportData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Get Report Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(((HIDM_HID_Get_Report_Response_Message_t *)Message)->ReportLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Report Response Event.                                */
               ProcessGetReportResponseEvent(((HIDM_HID_Get_Report_Response_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Get_Report_Response_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Get_Report_Response_Message_t *)Message)->Status, ((HIDM_HID_Get_Report_Response_Message_t *)Message)->ReportType, (Word_t)(((HIDM_HID_Get_Report_Response_Message_t *)Message)->ReportLength), ((HIDM_HID_Get_Report_Response_Message_t *)Message)->ReportData);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Set Report Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_SET_REPORT_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Report Response Event.                                */
               ProcessSetReportResponseEvent(((HIDM_HID_Set_Report_Response_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Set_Report_Response_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Set_Report_Response_Message_t *)Message)->Status);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_GET_PROTOCOL_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Get Protocol Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_GET_PROTOCOL_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Protocol Response Event.                              */
               ProcessGetProtocolResponseEvent(((HIDM_HID_Get_Protocol_Response_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Get_Protocol_Response_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Get_Protocol_Response_Message_t *)Message)->Status, ((HIDM_HID_Get_Protocol_Response_Message_t *)Message)->Protocol);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SET_PROTOCOL_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Set Protocol Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_SET_PROTOCOL_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Protocol Response Event.                              */
               ProcessSetProtocolResponseEvent(((HIDM_HID_Set_Protocol_Response_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Set_Protocol_Response_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Set_Protocol_Response_Message_t *)Message)->Status);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_GET_IDLE_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Get Idle Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_GET_IDLE_RESPONSE_MESSAGE_SIZE)

            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Protocol Response Event.                              */
               ProcessGetIdleResponseEvent(((HIDM_HID_Get_Idle_Response_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Set_Protocol_Response_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Get_Idle_Response_Message_t *)Message)->Status, ((HIDM_HID_Get_Idle_Response_Message_t *)Message)->IdleRate);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SET_IDLE_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Set Idle Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_HID_SET_IDLE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Protocol Response Event.                              */
               ProcessSetIdleResponseEvent(((HIDM_HID_Set_Idle_Response_Message_t *)Message)->HIDDataEventsHandlerID, ((HIDM_HID_Set_Protocol_Response_Message_t *)Message)->RemoteDeviceAddress, ((HIDM_HID_Set_Idle_Response_Message_t *)Message)->Status);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(HIDManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HID Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_HIDM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HID state information.    */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HID state information.    */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            HIDEntryInfo = HIDEntryInfoList;

            while(HIDEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if((!(HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HIDEntryInfo->ConnectionEvent))
               {
                  HIDEntryInfo->ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(HIDEntryInfo->ConnectionEvent);
               }

               HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HIDManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all HID Manager Messages.   */
static void BTPSAPI HIDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HID_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HIDM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
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
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HID Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HID Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HIDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int               Result;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((HIDManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process HID Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HID_MANAGER, HIDManagerGroupHandler, NULL))
            {
               /* Initialize the actual HID Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the HID Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _HIDM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the HID Manager Server. */
                  Result = _HIDM_Register_HID_Events();

                  if(Result > 0)
                  {
                     HIDEventsCallbackID = (unsigned int)Result;

                     /* Initialize a unique, starting HID Callback ID.  */
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
            if(HIDEventsCallbackID)
               _HIDM_Un_Register_HID_Events(HIDEventsCallbackID);

            if(HIDManagerMutex)
               BTPS_CloseMutex(HIDManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HID_MANAGER);

            /* Flag that none of the resources are allocated.           */
            HIDManagerMutex     = NULL;
            HIDEventsCallbackID = 0;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HID_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for HID Events.                              */
            if(HIDEventsCallbackID)
               _HIDM_Un_Register_HID_Events(HIDEventsCallbackID);

            /* Next, Un-Register for any HID Data Events.               */
            HIDEntryInfo = HIDEntryInfoDataList;
            while(HIDEntryInfo)
            {
               _HIDM_Un_Register_HID_Data_Events(HIDEntryInfo->ConnectionStatus);

               HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
            }

            /* Make sure we inform the HID Manager Implementation that  */
            /* we are shutting down.                                    */
            _HIDM_Cleanup();

            BTPS_CloseMutex(HIDManagerMutex);

            /* Make sure that the HID Entry Information List is empty.  */
            FreeHIDEntryInfoList(&HIDEntryInfoList);

            /* Make sure that the HID Entry Data Information List is    */
            /* empty.                                                   */
            FreeHIDEntryInfoList(&HIDEntryInfoDataList);

            /* Flag that the resources are no longer allocated.         */
            HIDManagerMutex     = NULL;
            CurrentPowerState   = FALSE;
            HIDEventsCallbackID = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HIDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* HID Entries and set any events that have synchronous  */
               /* operations pending.                                   */
               HIDEntryInfo = HIDEntryInfoList;

               while(HIDEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((!(HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HIDEntryInfo->ConnectionEvent))
                  {
                     HIDEntryInfo->ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(HIDEntryInfo->ConnectionEvent);
                  }

                  HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HIDManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming HID connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HID Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI HIDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept, unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Respond to the Connection Request.                             */
      ret_val = _HIDM_Connection_Request_Response(RemoteDeviceAddress, Accept, ConnectionFlags);
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote HID device.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHIDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HIDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI HIDM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int               ret_val;
   Event_t           ConnectionEvent;
   BD_ADDR_t         NULL_BD_ADDR;
   unsigned int      CallbackID;
   HID_Entry_Info_t  HIDEntryInfo;
   HID_Entry_Info_t *HIDEntryInfoPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* HID Entry list.                                       */
               BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

               HIDEntryInfo.CallbackID        = GetNextCallbackID();
               HIDEntryInfo.EventCallback     = CallbackFunction;
               HIDEntryInfo.CallbackParameter = CallbackParameter;
               HIDEntryInfo.BD_ADDR           = RemoteDeviceAddress;

               if(ConnectionStatus)
                  HIDEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

               if((!ConnectionStatus) || ((ConnectionStatus) && (HIDEntryInfo.ConnectionEvent)))
               {
                  if((HIDEntryInfoPtr = AddHIDEntryInfoEntry(&HIDEntryInfoList, &HIDEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote device 0x%08lX\n", ConnectionFlags));

                     /* Next, attempt to open the remote device.        */
                     if((ret_val = _HIDM_Connect_Remote_Device(RemoteDeviceAddress, ConnectionFlags)) != 0)
                     {
                        /* Error opening device, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfoPtr->CallbackID)) != NULL)
                        {
                           if(HIDEntryInfoPtr->ConnectionEvent)
                              BTPS_CloseEvent(HIDEntryInfoPtr->ConnectionEvent);

                           FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);
                        }
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((!ret_val) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Callback ID.                        */
                        CallbackID      = HIDEntryInfoPtr->CallbackID;

                        /* Note the Open Event.                         */
                        ConnectionEvent = HIDEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(HIDManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, CallbackID)) != NULL)
                           {
                              /* Note the connection status.            */
                              *ConnectionStatus = HIDEntryInfoPtr->ConnectionStatus;

                              BTPS_CloseEvent(HIDEntryInfoPtr->ConnectionEvent);

                              FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);

                              /* Flag success to the caller.            */
                              ret_val = 0;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_HID_DEVICE;
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
                           if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfo.CallbackID)) != NULL)
                           {
                              if(HIDEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(HIDEntryInfoPtr->ConnectionEvent);

                              FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);
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
               BTPS_ReleaseMutex(HIDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local device to disconnect a currently connected remote device    */
   /* (connected to the local device's HID Host).  This function accepts*/
   /* as input the Remote Device Address of the Remote HID Device to    */
   /* disconnect from the local HID Host followed by a BOOLEAN value    */
   /* that specifies whether or not the device is be disconnected via a */
   /* Virtual Cable Disconnection (TRUE), or merely disconnected at the */
   /* Bluetooth link (FALSE).  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.            */
int BTPSAPI HIDM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableDisconnect)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Disconnect the Remote Device.                                  */
      ret_val = _HIDM_Disconnect_Device(RemoteDeviceAddress, SendVirtualCableDisconnect);
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected HID     */
   /* Devices.  This function accepts a pointer to a buffer that will   */
   /* receive any currently connected HID devices.  The first parameter */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated with   */
   /* the total number of connected devices if the function is          */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns a  */
   /* non-negative value if successful which represents the number of   */
   /* connected devices that were copied into the specified input       */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI HIDM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* query the connected Devices.                                   */
      ret_val = _HIDM_Query_Connected_Devices(MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int BTPSAPI HIDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* update the Incoming Connection Flags.                          */
      ret_val = _HIDM_Change_Incoming_Connection_Flags(ConnectionFlags);
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is used to set the HID Host Keyboard Key   */
   /* Repeat behavior.  Some Host operating systems nativity support Key*/
   /* Repeat automatically, but for those Host operating systems that do*/
   /* not - this function will instruct the HID Manager to simulate Key */
   /* Repeat behavior (with the specified parameters).  This function   */
   /* accepts the initial amount to delay (in milliseconds) before      */
   /* starting the repeat functionality.  The final parameter specifies */
   /* the rate of repeat (in milliseconds).  This function returns zero */
   /* if successful or a negative value if there was an error.          */
   /* * NOTE * Specifying zero for the Repeat Delay (first parameter)   */
   /*          will disable HID Manager Key Repeat processing.  This    */
   /*          means that only Key Up/Down events will be dispatched    */
   /*          and No Key Repeat events will be dispatched.             */
   /* * NOTE * The Key Repeat parameters can only be changed when there */
   /*          are no actively connected HID devices.                   */
int BTPSAPI HIDM_Set_Keyboard_Repeat_Rate(unsigned int RepeatDelay, unsigned int RepeatRate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* set the actual Keyboard repeat rate.                           */
      ret_val = _HIDM_Set_Keyboard_Repeat_Rate(RepeatDelay, RepeatRate);
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* HID Report Data to a currently connected remote device.  This     */
   /* function accepts as input the HID Manager Data Handler ID         */
   /* (registered via call to the HIDM_Register_Data_Event_Callback()   */
   /* function), followed by the remote device address of the remote HID*/
   /* device to send the report data to, followed by the report data    */
   /* itself.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int BTPSAPI HIDM_Send_Report_Data(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int               ret_val;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the HID Report Data.                 */
               /* * NOTE * The ConnectionStatus member is what holds    */
               /*          the Server Data Event Callback ID, so we     */
               /*          must use that value when sending data to the */
               /*          server.                                      */
               ret_val = _HIDM_Send_Report_Data(HIDEntryInfo->ConnectionStatus, RemoteDeviceAddress, ReportDataLength, ReportData);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HIDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as input */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report requested.  The    */
   /* fourth parameter is the Report ID determined by the Device's SDP  */
   /* record.  Passing HIDM_INVALID_REPORT_ID as the value for this     */
   /* parameter will indicate that this parameter is not used and will  */
   /* exclude the appropriate byte from the transaction payload.  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Get_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID)
{
   int               ret_val;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the HID Request.                     */
               ret_val = _HIDM_Send_Get_Report_Request(HIDEntryInfo->ConnectionStatus, RemoteDeviceAddress, ReportType, ReportID);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input     */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report being sent.  The   */
   /* final two parameters to this function are the Length of the Report*/
   /* Data to send and a pointer to the Report Data that will be sent.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Set_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData)
{
   int               ret_val;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HIDManagerDataCallbackID) && (ReportDataLength) && (ReportData))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the HID request.                     */
               ret_val = _HIDM_Send_Set_Report_Request(HIDEntryInfo->ConnectionStatus, RemoteDeviceAddress, ReportType, ReportDataLength, ReportData);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Get_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the HID request.                     */
               ret_val = _HIDM_Send_Get_Protocol_Request(HIDEntryInfo->ConnectionStatus, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the protocol to be set.  This     */
   /* function returns a zero if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Set_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol)
{
   int               ret_val;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the HID request.                     */
               ret_val = _HIDM_Send_Set_Protocol_Request(HIDEntryInfo->ConnectionStatus, RemoteDeviceAddress, Protocol);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Get_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int               ret_val;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the HID request.                     */
               ret_val = _HIDM_Send_Get_Idle_Request(HIDEntryInfo->ConnectionStatus, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the Idle Rate to be set.  The Idle*/
   /* Rate LSB is weighted to 4ms (i.e. the Idle Rate resolution is 4ms */
   /* with a range from 4ms to 1.020s).  This function returns a zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Set_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate)
{
   int               ret_val;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the HID request.                     */
               ret_val = _HIDM_Send_Set_Idle_Request(HIDEntryInfo->ConnectionStatus, RemoteDeviceAddress, IdleRate);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   /*          HIDM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int BTPSAPI HIDM_Register_Event_Callback(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   HID_Entry_Info_t HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the HID Entry list.         */
            BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

            HIDEntryInfo.CallbackID         = GetNextCallbackID();
            HIDEntryInfo.EventCallback      = CallbackFunction;
            HIDEntryInfo.CallbackParameter  = CallbackParameter;
            HIDEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if(AddHIDEntryInfoEntry(&HIDEntryInfoList, &HIDEntryInfo))
               ret_val = HIDEntryInfo.CallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HIDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HIDM_RegisterEventCallback() function).  This function accepts as */
   /* input the HID Manager Event Callback ID (return value from        */
   /* HIDM_RegisterEventCallback() function).                           */
void BTPSAPI HIDM_Un_Register_Event_Callback(unsigned int HIDManagerCallbackID)
{
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HIDManagerCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeHIDEntryInfoEntryMemory(HIDEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HIDManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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
   /*          HIDM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HIDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HIDM_Register_Data_Event_Callback(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   HID_Entry_Info_t  HIDEntryInfo;
   HID_Entry_Info_t *HIDEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, Register the handler locally.                     */
            /* * NOTE * We will use the ConnectionStatus member to hold */
            /*          the real Event Callback ID returned from the    */
            /*          server.                                         */
            BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

            HIDEntryInfo.CallbackID         = GetNextCallbackID();
            HIDEntryInfo.EventCallback      = CallbackFunction;
            HIDEntryInfo.CallbackParameter  = CallbackParameter;
            HIDEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if((HIDEntryInfoPtr = AddHIDEntryInfoEntry(&HIDEntryInfoDataList, &HIDEntryInfo)) != NULL)
            {
               /* Attempt to register it with the system.               */
               if((ret_val = _HIDM_Register_HID_Data_Events()) > 0)
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  HIDEntryInfoPtr->ConnectionStatus = ret_val;

                  ret_val                           = HIDEntryInfoPtr->CallbackID;
               }
               else
               {
                  /* Error, go ahead and delete the entry we added      */
                  /* locally.                                           */
                  if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDEntryInfoPtr->CallbackID)) != NULL)
                     FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HIDManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HIDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from HIDM_Register_Data_Event_Callback() function).         */
void BTPSAPI HIDM_Un_Register_Data_Event_Callback(unsigned int HIDManagerDataCallbackID)
{
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HIDManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, delete the local handler.                         */
            if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* Handler found, go ahead and delete it from the server.*/
               _HIDM_Un_Register_HID_Data_Events(HIDEntryInfo->ConnectionStatus);

               /* All finished with the entry, delete it.               */
               FreeHIDEntryInfoEntryMemory(HIDEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HIDManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

