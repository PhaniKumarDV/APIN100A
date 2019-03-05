/*****< btpmpanm.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPANM - PAN Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/28/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMPANM.h"            /* BTPM PAN Manager Prototypes/Constants.    */
#include "PANMAPI.h"             /* PAN Manager Prototypes/Constants.         */
#include "PANMMSG.h"             /* BTPM PAN Manager Message Formats.         */
#include "PANMGR.h"              /* PAN Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPAN_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ConnectionStatus;
   Event_t                      ConnectionEvent;
   BD_ADDR_t                    BD_ADDR;
   PANM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagPAN_Entry_Info_t *NextPANEntryInfoPtr;
} PAN_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   PANM_Event_Callback_t  EventCallback;
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
static Mutex_t PANManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the current PAN Events Callback ID           */
   /* (registered with the Server to receive events).                   */
static unsigned int PANEventsCallbackID;

   /* Variable which holds a pointer to the first element in the PAN    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static PAN_Entry_Info_t *PANEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static PAN_Entry_Info_t *AddPANEntryInfoEntry(PAN_Entry_Info_t **ListHead, PAN_Entry_Info_t *EntryToAdd);
static PAN_Entry_Info_t *DeletePANEntryInfoEntry(PAN_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreePANEntryInfoEntryMemory(PAN_Entry_Info_t *EntryToFree);
static void FreePANEntryInfoList(PAN_Entry_Info_t **ListHead);

static void DispatchPANEvent(PANM_Event_Data_t *PANMEventData);

static void ProcessIncomingConnectionRequestEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessDeviceConnectedEvent(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t ServiceType);
static void ProcessConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t ServiceType, unsigned int Status);
static void ProcessCloseConnectionEvent(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t ServiceType);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_PANM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI PANManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the PAN Entry Information List.                              */
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
static PAN_Entry_Info_t *AddPANEntryInfoEntry(PAN_Entry_Info_t **ListHead, PAN_Entry_Info_t *EntryToAdd)
{
   PAN_Entry_Info_t *AddedEntry = NULL;
   PAN_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (PAN_Entry_Info_t *)BTPS_AllocateMemory(sizeof(PAN_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextPANEntryInfoPtr = NULL;

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
                     FreePANEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextPANEntryInfoPtr)
                        tmpEntry = tmpEntry->NextPANEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextPANEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified PAN Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the PAN Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreePANEntryInfoEntryMemory().                   */
static PAN_Entry_Info_t *DeletePANEntryInfoEntry(PAN_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   PAN_Entry_Info_t *FoundEntry = NULL;
   PAN_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextPANEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextPANEntryInfoPtr = FoundEntry->NextPANEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextPANEntryInfoPtr;

         FoundEntry->NextPANEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified PAN Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreePANEntryInfoEntryMemory(PAN_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified PAN Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreePANEntryInfoList(PAN_Entry_Info_t **ListHead)
{
   PAN_Entry_Info_t *EntryToFree;
   PAN_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextPANEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreePANEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified PAN event to every registered PAN Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the PAN Manager Mutex*/
   /*          held.  Upon exit from this function it will free the PAN */
   /*          Manager Mutex.                                           */
static void DispatchPANEvent(PANM_Event_Data_t *PANMEventData)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   PAN_Entry_Info_t *PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((PANEntryInfoList) && (PANMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      PANEntryInfo    = PANEntryInfoList;
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(PANEntryInfo)
      {
         if(PANEntryInfo->EventCallback)
            NumberCallbacks++;

         PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
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
            PANEntryInfo    = PANEntryInfoList;
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(PANEntryInfo)
            {
               if(PANEntryInfo->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = PANEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = PANEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(PANManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(PANMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(PANManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(PANManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(PANManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Incoming */
   /* Connection Request asynchronous message.                          */
static void ProcessIncomingConnectionRequestEvent(BD_ADDR_t RemoteDeviceAddress)
{
   PANM_Event_Data_t PANMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   PANMEventData.EventType                                                        = petPANMIncomingConnectionRequest;
   PANMEventData.EventLength                                                      = PANM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

   PANMEventData.EventData.IncomingConnectionReqeustEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchPANEvent(&PANMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Device   */
   /* Connected asynchronous message.                                   */
static void ProcessDeviceConnectedEvent(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t ServiceType)
{
   PANM_Event_Data_t PANMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   PANMEventData.EventType                                        = petPANMConnected;
   PANMEventData.EventLength                                      = PANM_CONNECTED_EVENT_DATA_SIZE;

   PANMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   PANMEventData.EventData.ConnectedEventData.ServiceType         = ServiceType;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchPANEvent(&PANMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Connection Status asynchronous message.                           */
static void ProcessConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t ServiceType, unsigned int Status)
{
   void                  *CallbackParameter;
   Boolean_t              ReleaseMutex;
   PANM_Event_Data_t      PANMEventData;
   PAN_Entry_Info_t      *PANEntryInfo;
   PANM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if(PANEntryInfoList)
   {
      ReleaseMutex = TRUE;
      PANEntryInfo = PANEntryInfoList;
      while(PANEntryInfo)
      {
         if(COMPARE_BD_ADDR(PANEntryInfo->BD_ADDR, RemoteDeviceAddress))
         {
            /* Callback registered, now see if the callback is          */
            /* synchronous or asynchronous.                             */
            if(PANEntryInfo->ConnectionEvent)
            {
               /* Synchronous.                                          */

               /* Note the Status.                                      */
               PANEntryInfo->ConnectionStatus = Status;

               /* Set the Event.                                        */
               BTPS_SetEvent(PANEntryInfo->ConnectionEvent);

               /* Break out of the list.                                */
               PANEntryInfo = NULL;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(PANManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;
            }
            else
            {
               /* Asynchronous Entry, go ahead dispatch the result.     */

               /* Format up the Event.                                  */
               PANMEventData.EventType                                               = petPANMConnectionStatus;
               PANMEventData.EventLength                                             = PANM_CONNECTION_STATUS_EVENT_DATA_SIZE;

               PANMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
               PANMEventData.EventData.ConnectionStatusEventData.ServiceType         = ServiceType;
               PANMEventData.EventData.ConnectionStatusEventData.Status              = Status;

               /* Note the Callback information.                        */
               EventCallback                                                         = PANEntryInfo->EventCallback;
               CallbackParameter                                                     = PANEntryInfo->CallbackParameter;

               if((PANEntryInfo = DeletePANEntryInfoEntry(&PANEntryInfoList, PANEntryInfo->CallbackID)) != NULL)
                  FreePANEntryInfoEntryMemory(PANEntryInfo);

               /* Release the Mutex so we can dispatch the event.       */
               BTPS_ReleaseMutex(PANManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&PANMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Break out of the list.                                */
               PANEntryInfo = NULL;
            }
         }
         else
            PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
      }

      /* If the Mutex was not released, then we need to make sure we    */
      /* release it.                                                    */
      if(ReleaseMutex)
         BTPS_ReleaseMutex(PANManagerMutex);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(PANManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Close    */
   /* Connection asynchronous message.                                  */
static void ProcessCloseConnectionEvent(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t ServiceType)
{
   PANM_Event_Data_t PANMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   PANMEventData.EventType                                           = petPANMDisconnected;
   PANMEventData.EventLength                                         = PANM_DISCONNECTED_EVENT_DATA_SIZE;

   PANMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   PANMEventData.EventData.DisconnectedEventData.ServiceType         = ServiceType;

   /* Now that the Event is formatted, dispatch it.                     */
   DispatchPANEvent(&PANMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessIncomingConnectionRequestEvent(((PANM_Connection_Request_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_DEVICE_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_DEVICE_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connected Event.                               */
               ProcessDeviceConnectedEvent(((PANM_Device_Connected_Message_t *)Message)->RemoteDeviceAddress, ((PANM_Device_Connected_Message_t *)Message)->ServiceType);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connection Status Event.                              */
               ProcessConnectionStatusEvent(((PANM_Device_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((PANM_Device_Connection_Status_Message_t *)Message)->ServiceType, ((PANM_Device_Connection_Status_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_DEVICE_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Disconnected Event.                            */
               ProcessCloseConnectionEvent(((PANM_Device_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((PANM_Device_Disconnected_Message_t *)Message)->ServiceType);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(PANManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PAN Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_PANM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the PAN state information.    */
         if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   PAN_Entry_Info_t *PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the PAN state information.    */
         if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            PANEntryInfo = PANEntryInfoList;
            while(PANEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if(PANEntryInfo->ConnectionEvent)
               {
                  PANEntryInfo->ConnectionStatus = PANM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(PANEntryInfo->ConnectionEvent);
               }

               PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PANManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all PAN Manager Messages.   */
static void BTPSAPI PANManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_PAN_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a PAN Manager defined    */
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
               /* PAN Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_PANM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAN Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAN Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an PAN Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Non PAN Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager PAN Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI PANM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Client Side Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAN Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((PANManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process PAN Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PAN_MANAGER, PANManagerGroupHandler, NULL))
            {
               /* Initialize the actual PAN Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the PAN Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _PANM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the PAN Manager Server. */
                  Result = _PANM_Register_Events();

                  if(Result > 0)
                  {
                     PANEventsCallbackID = (unsigned int)Result;

                     /* Initialize a unique, starting PAN Callback ID.  */
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
            if(PANEventsCallbackID)
               _PANM_Un_Register_Events(PANEventsCallbackID);

            if(PANManagerMutex)
               BTPS_CloseMutex(PANManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PAN_MANAGER);

            /* Flag that none of the resources are allocated.           */
            PANManagerMutex     = NULL;
            PANEventsCallbackID = 0;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PAN_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for PAN Events.                              */
            if(PANEventsCallbackID)
               _PANM_Un_Register_Events(PANEventsCallbackID);

            /* Make sure we inform the PAN Manager Implementation that  */
            /* we are shutting down.                                    */
            _PANM_Cleanup();

            BTPS_CloseMutex(PANManagerMutex);

            /* Make sure that the PAN Entry Information List is empty.  */
            FreePANEntryInfoList(&PANEntryInfoList);

            /* Flag that the resources are no longer allocated.         */
            PANManagerMutex     = NULL;
            CurrentPowerState   = FALSE;
            PANEventsCallbackID = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PANM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   PAN_Entry_Info_t *PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* PAN Entries and set any events that have synchronous  */
               /* operations pending.                                   */
               PANEntryInfo = PANEntryInfoList;

               while(PANEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(PANEntryInfo->ConnectionEvent)
                  {
                     PANEntryInfo->ConnectionStatus = PANM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(PANEntryInfo->ConnectionEvent);
                  }

                  PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(PANManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a local PAN server.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the connection has been successfully       */
   /*          opened. A petPANMConnected event will notifiy of this    */
   /*          status.                                                  */
int BTPSAPI PANM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Respond to the Open Request.                                   */
      ret_val = _PANM_Connection_Request_Response(RemoteDeviceAddress, AcceptConnection);
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to request to open a remote PANM server connection.  This  */
   /* function returns zero if successful and a negative value if there */
   /* was an error.                                                     */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          PANM Connection Status Event (if specified).             */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          petPANMConnectionStatus event will be dispatched to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the PANM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI PANM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, unsigned long ConnectionFlags, PANM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int               ret_val;
   Event_t           ConnectionEvent;
   unsigned int      CallbackID;
   PAN_Entry_Info_t  PANEntryInfo;
   PAN_Entry_Info_t *PANEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Attempt to wait for access to the PAN Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next check to see if we are powered up.                  */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry to the  */
               /* PAN Entry List.                                       */
               BTPS_MemInitialize(&PANEntryInfo, 0, sizeof(PAN_Entry_Info_t));

               PANEntryInfo.CallbackID        = GetNextCallbackID();
               PANEntryInfo.EventCallback     = EventCallback;
               PANEntryInfo.CallbackParameter = CallbackParameter;
               PANEntryInfo.BD_ADDR           = RemoteDeviceAddress;

               if(ConnectionStatus)
                  PANEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

               if((!ConnectionStatus) || ((ConnectionStatus) && (PANEntryInfo.ConnectionEvent)))
               {
                  if((PANEntryInfoPtr = AddPANEntryInfoEntry(&PANEntryInfoList, &PANEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Open Remote Device\n"));

                     /* Next, attempt to open the remote device.        */
                     if((ret_val = _PANM_Connect_Remote_Device(RemoteDeviceAddress, LocalServiceType, RemoteServiceType, ConnectionFlags)) != 0)
                     {
                        /* Error opening device, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((PANEntryInfoPtr = DeletePANEntryInfoEntry(&PANEntryInfoList, PANEntryInfoPtr->CallbackID)) != NULL)
                        {
                           if(PANEntryInfoPtr->ConnectionEvent)
                              BTPS_CloseEvent(PANEntryInfoPtr->ConnectionEvent);

                           FreePANEntryInfoEntryMemory(PANEntryInfoPtr);
                        }
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((!ret_val) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Callback ID.                        */
                        CallbackID      = PANEntryInfoPtr->CallbackID;

                        /* Note the Open Event.                         */
                        ConnectionEvent = PANEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(PANManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((PANEntryInfoPtr = DeletePANEntryInfoEntry(&PANEntryInfoList, CallbackID)) != NULL)
                           {
                              *ConnectionStatus = PANEntryInfoPtr->ConnectionStatus;

                              BTPS_CloseEvent(PANEntryInfoPtr->ConnectionEvent);

                              FreePANEntryInfoEntryMemory(PANEntryInfoPtr);

                              /* Flag success to the caller.            */
                              ret_val = 0;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_PAN_UNABLE_TO_CONNECT_TO_DEVICE;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                     }
                     else
                     {
                        /* Either there was an error or we aren't       */
                        /* blocking for this connection. Go ahead and   */
                        /* delete the entry if necessary.               */
                        if((PANEntryInfoPtr) && ((!EventCallback) || (ret_val)))
                        {
                           /* There's nothing to track, we can delete.  */
                           if((PANEntryInfoPtr = DeletePANEntryInfoEntry(&PANEntryInfoList, PANEntryInfoPtr->CallbackID)) != NULL)
                           {
                              if(PANEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(PANEntryInfoPtr->ConnectionEvent);

                              FreePANEntryInfoEntryMemory(PANEntryInfoPtr);
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

            /* Release the mutex because we are finished with it.       */
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               BTPS_ReleaseMutex(PANManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to close a previously opened connection.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * This function does not unregister a PAN server.  It only */
   /*          disconnects any currently active connection.             */
int BTPSAPI PANM_Close_Connection(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* Close the Connection.                                          */
      ret_val = _PANM_Close_Connection(RemoteDeviceAddress);
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Personal*/
   /* Area Networking devices.  This function accepts the buffer        */
   /* information to receive any currently connected devices.  The first*/
   /* parameter specifies the maximum number of BD_ADDR entries that the*/
   /* buffer will support (i.e. can be copied into the buffer).  The    */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of connected devices if the function is     */
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
int BTPSAPI PANM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* query the currently connected devices.                         */
      ret_val = _PANM_Query_Connected_Devices(MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for the Personal Area  */
   /* Networking (PAN) Manager.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI PANM_Query_Current_Configuration(PANM_Current_Configuration_t *CurrentConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* query the current configuration.                               */
      ret_val = _PANM_Query_Current_Configuration(CurrentConfiguration);
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to change the Incoming Connection Flags. This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI PANM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* update the Incoming Connection Flags.                          */
      ret_val = _PANM_Change_Incoming_Connection_Flags(ConnectionFlags);
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Personal Area    */
   /* Network (PAN) Manager Service. This Callback will be dispatched   */
   /* by the PAN Manager when various PAN Manager Events occur. This    */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a PAN Manager Event needs to be       */
   /* dispatched. This function returns a positive (non-zero) value if  */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          PANM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
int BTPSAPI PANM_Register_Event_Callback(PANM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   PAN_Entry_Info_t PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the PAN Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the PAN Entry list.         */
            BTPS_MemInitialize(&PANEntryInfo, 0, sizeof(PAN_Entry_Info_t));

            PANEntryInfo.CallbackID        = GetNextCallbackID();
            PANEntryInfo.EventCallback     = CallbackFunction;
            PANEntryInfo.CallbackParameter = CallbackParameter;

            if(AddPANEntryInfoEntry(&PANEntryInfoList, &PANEntryInfo))
               ret_val = PANEntryInfo.CallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PANManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PAN Manager Event Callback.   */
   /* This function accepts as input the PAN Manager Event Callback ID  */
   /* (return value from PANM_Register_Event_Callback() function).      */
void BTPSAPI PANM_Un_Register_Event_Callback(unsigned int PANManagerCallbackID)
{
   PAN_Entry_Info_t *PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(PANManagerCallbackID)
      {
         /* Attempt to wait for access to the PAN Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PANManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((PANEntryInfo = DeletePANEntryInfoEntry(&PANEntryInfoList, PANManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreePANEntryInfoEntryMemory(PANEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PANManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

