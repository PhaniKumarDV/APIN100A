/*****< btpmbasm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMBASM - BAS Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMBASM.h"            /* BTPM BAS Manager Prototypes/Constants.    */
#include "BASMAPI.h"             /* BAS Manager Prototypes/Constants.         */
#include "BASMMSG.h"             /* BTPM BAS Manager Message Formats.         */
#include "BASMGR.h"              /* BAS Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagCallback_Entry_t
{
   unsigned int                 CallbackID;
   BASM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagCallback_Entry_t *NextCallbackEntry;
} Callback_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t BASManagerMutex;

   /* Variable which holds a pointer to the first element in the        */
   /* Callback Entry List (which holds all Callbacks tracked by this    */
   /* module).                                                          */
static Callback_Entry_t *CallbackEntryList;

   /* Internal Function Prototypes.                                     */
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t *EntryToAdd);
static Callback_Entry_t *SearchCallbackEntry(unsigned int CallbackID);
static Callback_Entry_t *DeleteCallbackEntry(unsigned int CallbackID);
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);
static void FreeCallbackEntryList(void);

static void DispatchBASEvent(Callback_Entry_t *CallbackEntry, BASM_Event_Data_t *BASMEventData);
static void DispatchBroadcastBASEvent(BASM_Event_Data_t *BASMEventData);

static void DispatchConnected(BASM_Connected_Message_t *Message);
static void DispatchDisconnected(BASM_Disconnected_Message_t *Message);
static void DispatchBatteryLevel(BASM_Battery_Level_Message_t *Message);
static void DispatchBatteryIdentification(BASM_Battery_Identification_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_BASM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI BASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function adds the Callback Entry to the Callback    */
   /* Entry List.  This function allocates and adds an entry to the list*/
   /* that has the same attributes as the Entry passed into this        */
   /* function.  This function will return NULL if NO Entry was added.  */
   /* This can occur if the element passed in was deemed invalid.       */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            CallbackID field is the same as an entry already in the*/
   /*            list.  When this occurs, this function returns NULL.   */
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t *EntryToAdd)
{
   Callback_Entry_t *AddedEntry = NULL;
   Callback_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if(EntryToAdd)
   {
      /* OK, data seems semi-valid, let's allocate a new data structure */
      /* to add to the list.                                            */
      AddedEntry = (Callback_Entry_t *)BTPS_AllocateMemory(sizeof(Callback_Entry_t));

      if(AddedEntry)
      {
         /* Copy All Data over.                                         */
         *AddedEntry                     = *EntryToAdd;

         /* Now Add it to the end of the list.                          */
         AddedEntry->NextCallbackEntry = NULL;

         /* First, let's check to see if there are any elements already */
         /* present in the List that was passed in.                     */
         if((tmpEntry = CallbackEntryList) != NULL)
         {
            /* Head Pointer was not NULL, so we will traverse the list  */
            /* until we reach the last element.                         */
            while(tmpEntry)
            {
               if(tmpEntry->CallbackID == AddedEntry->CallbackID)
               {
                  /* Entry was already added, so free the memory and    */
                  /* flag an error to the caller.                       */
                  FreeCallbackEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the Search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* OK, we need to see if we are at the last element of*/
                  /* the List.  If we are, we simply break out of the   */
                  /* list traversal because we know there are NO        */
                  /* duplicates AND we are at the end of the list.      */
                  if(tmpEntry->NextCallbackEntry)
                     tmpEntry = tmpEntry->NextCallbackEntry;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Last element found, simply Add the entry.             */
               tmpEntry->NextCallbackEntry = AddedEntry;
            }
         }
         else
            CallbackEntryList = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the module's Callback Entry List  */
   /* for a Callback Entry based on the specified Callback ID.  This    */
   /* function returns NULL if the specified Entry was NOT present in   */
   /* the list.                                                         */
static Callback_Entry_t *SearchCallbackEntry(unsigned int CallbackID)
{
   Callback_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Now, let's search the list until we find the correct entry.       */
   FoundEntry = CallbackEntryList;

   while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      FoundEntry = FoundEntry->NextCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the module's Callback Entry List  */
   /* for the Callback Entry with the specified Callback ID and removes */
   /* it from the List.  This function returns NULL if the specified    */
   /* Entry was NOT present in the list.  The entry returned will have  */
   /* the Next Entry field set to NULL, and the caller is responsible   */
   /* for deleting the memory associated with this entry by calling     */
   /* FreeCallbackEntryMemory().                                        */
static Callback_Entry_t *DeleteCallbackEntry(unsigned int CallbackID)
{
   Callback_Entry_t *FoundEntry = NULL;
   Callback_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Now, let's search the list until we find the correct entry.       */
   FoundEntry = CallbackEntryList;

   while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
   {
      LastEntry  = FoundEntry;
      FoundEntry = FoundEntry->NextCallbackEntry;
   }

   /* Check to see if we found the specified entry.                     */
   if(FoundEntry)
   {
      /* OK, now let's remove the entry from the list.  We have to check*/
      /* to see if the entry was the first entry in the list.           */
      if(LastEntry)
      {
         /* Entry was NOT the first entry in the list.                  */
         LastEntry->NextCallbackEntry = FoundEntry->NextCallbackEntry;
      }
      else
         CallbackEntryList = FoundEntry->NextCallbackEntry;

      FoundEntry->NextCallbackEntry = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Device Entry member.  No check  */
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Callback Entry List.  Upon return of this */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeCallbackEntryList(void)
{
   Callback_Entry_t *EntryToFree;
   Callback_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply traverse the list and free every element present.          */
   EntryToFree = CallbackEntryList;

   while(EntryToFree)
   {
      tmpEntry    = EntryToFree;
      EntryToFree = EntryToFree->NextCallbackEntry;

      FreeCallbackEntryMemory(tmpEntry);
   }

   /* Make sure the List appears to be empty.                           */
   CallbackEntryList = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified module event to the BAS Event Callback (if */
   /* registered).                                                      */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchBASEvent(Callback_Entry_t *CallbackEntry, BASM_Event_Data_t *BASMEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Release the Mutex because we are finished with it.                */
   BTPS_ReleaseMutex(BASManagerMutex);

   __BTPSTRY
   {
      if(CallbackEntry->EventCallback)
      {
         (*CallbackEntry->EventCallback)(BASMEventData, CallbackEntry->CallbackParameter);
      }
   }
   __BTPSEXCEPT(1)
   {
      /* Do Nothing.                                                    */
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified module event to all BAS Event Callbacks (if*/
   /* registered).                                                      */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchBroadcastBASEvent(BASM_Event_Data_t *BASMEventData)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Release the Mutex because we are finished with it.                */
   BTPS_ReleaseMutex(BASManagerMutex);

   for(CallbackEntry = CallbackEntryList; CallbackEntry; CallbackEntry = CallbackEntry->NextCallbackEntry)
   {
      /* Assign the correct Callback ID.                                */
      BASMEventData->EventCallbackID = CallbackEntry->CallbackID;

      __BTPSTRY
      {
         if(CallbackEntry->EventCallback)
         {
            (*CallbackEntry->EventCallback)(BASMEventData, CallbackEntry->CallbackParameter);
         }
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Connected Asynchronous Event to the local callback.  It*/
   /* is the caller's responsibility to verify the message before       */
   /* calling this function.                                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchConnected(BASM_Connected_Message_t *Message)
{
   BASM_Event_Data_t BASMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BASMEventData.EventType                                        = betBASConnected;
   BASMEventData.EventLength                                      = BASM_CONNECTED_EVENT_DATA_SIZE;
   BASMEventData.EventCallbackID                                  = Message->CallbackID;

   BASMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   BASMEventData.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;
   BASMEventData.EventData.ConnectedEventData.ConnectedFlags      = Message->ConnectedFlags;
   BASMEventData.EventData.ConnectedEventData.NumberOfInstances   = Message->NumberOfInstances;

   /* Now that the event is formatted, dispatch it to every registered  */
   /* callback.                                                         */
   DispatchBroadcastBASEvent(&BASMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Disconnected Asynchronous Event to the local callback. */
   /* It is the caller's responsibility to verify the message before    */
   /* calling this function.                                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchDisconnected(BASM_Disconnected_Message_t *Message)
{
   BASM_Event_Data_t BASMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   BASMEventData.EventType                                           = betBASDisconnected;
   BASMEventData.EventLength                                         = BASM_DISCONNECTED_EVENT_DATA_SIZE;
   BASMEventData.EventCallbackID                                     = Message->CallbackID;

   BASMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   BASMEventData.EventData.DisconnectedEventData.ConnectionType      = Message->ConnectionType;
   BASMEventData.EventData.DisconnectedEventData.DisconnectedFlags   = 0;

   /* Now that the event is formatted, dispatch it to every registered  */
   /* callback.                                                         */
   DispatchBroadcastBASEvent(&BASMEventData);


   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Battery Level Asynchronous Event to the local callback.*/
   /* It is the caller's responsibility to verify the message before    */
   /* calling this function.                                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchBatteryLevel(BASM_Battery_Level_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   BASM_Event_Data_t  BASMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(Message->CallbackID)) != NULL)
   {
      /* Format up the Event.                                           */
      BASMEventData.EventType                                           = betBASBatteryLevel;
      BASMEventData.EventLength                                         = BASM_BATTERY_LEVEL_EVENT_DATA_SIZE;
      BASMEventData.EventCallbackID                                     = CallbackEntry->CallbackID;

      BASMEventData.EventData.BatteryLevelEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      BASMEventData.EventData.BatteryLevelEventData.InstanceID          = Message->InstanceID;
      BASMEventData.EventData.BatteryLevelEventData.TransactionID       = Message->TransactionID;
      BASMEventData.EventData.BatteryLevelEventData.Status              = Message->Status;
      BASMEventData.EventData.BatteryLevelEventData.BatteryLevel        = Message->BatteryLevel;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchBASEvent(CallbackEntry, &BASMEventData);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(BASManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Battery Level Notification Asynchronous Event to the   */
   /* local callback.  It is the caller's responsibility to verify the  */
   /* message before calling this function.                             */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchBatteryLevelNotification(BASM_Battery_Level_Notification_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   BASM_Event_Data_t  BASMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(Message->CallbackID)) != NULL)
   {
      /* Format up the Event.                                           */
      BASMEventData.EventType                                                       = betBASBatteryLevelNotification;
      BASMEventData.EventLength                                                     = BASM_BATTERY_LEVEL_NOTIFICATION_EVENT_DATA_SIZE;
      BASMEventData.EventCallbackID                                                 = CallbackEntry->CallbackID;

      BASMEventData.EventData.BatteryLevelNotificationEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      BASMEventData.EventData.BatteryLevelNotificationEventData.InstanceID          = Message->InstanceID;
      BASMEventData.EventData.BatteryLevelNotificationEventData.BatteryLevel        = Message->BatteryLevel;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchBASEvent(CallbackEntry, &BASMEventData);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(BASManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Battery Identification Asynchronous Event to the local */
   /* callback.  It is the caller's responsibility to verify the message*/
   /* before calling this function.                                     */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchBatteryIdentification(BASM_Battery_Identification_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   BASM_Event_Data_t  BASMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(Message->CallbackID)) != NULL)
   {
      /* Format up the Event.                                           */
      BASMEventData.EventType                                                    = betBASBatteryIdentification;
      BASMEventData.EventLength                                                  = BASM_BATTERY_IDENTIFICATION_EVENT_DATA_SIZE;
      BASMEventData.EventCallbackID                                              = CallbackEntry->CallbackID;

      BASMEventData.EventData.BatteryIdentificationEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      BASMEventData.EventData.BatteryIdentificationEventData.InstanceID          = Message->InstanceID;
      BASMEventData.EventData.BatteryIdentificationEventData.TransactionID       = Message->TransactionID;
      BASMEventData.EventData.BatteryIdentificationEventData.Status              = Message->Status;
      BASMEventData.EventData.BatteryIdentificationEventData.Namespace           = Message->Namespace;
      BASMEventData.EventData.BatteryIdentificationEventData.Description         = Message->Description;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchBASEvent(CallbackEntry, &BASMEventData);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(BASManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case BASM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               DispatchConnected((BASM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Disconnection Request Event.                 */
               DispatchDisconnected((BASM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_BATTERY_LEVEL:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Battery Level\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_BATTERY_LEVEL_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Battery Level Event.                                  */
               DispatchBatteryLevel((BASM_Battery_Level_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_BATTERY_LEVEL_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Battery Level Notification\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_BATTERY_LEVEL_NOTIFICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Battery Level Notification Event.                     */
               DispatchBatteryLevelNotification((BASM_Battery_Level_Notification_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_BATTERY_IDENTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Battery Identification\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_BATTERY_IDENTIFICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Battery Identification Event.                         */
               DispatchBatteryIdentification((BASM_Battery_Identification_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(BASManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process BAS Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_BASM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the BAS state information.    */
         if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the BAS state information.    */
         if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete any event callbacks.                              */
            FreeCallbackEntryList();

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(BASManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Module Manager Messages.*/
static void BTPSAPI BASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("BAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a BAS Manager defined    */
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
               /* Module Manager Thread.                                */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_BASM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue BAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue BAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an BAS Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Non BAS Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager BAS Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI BASM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing BAS Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((BASManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process BAS Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER, BASManagerGroupHandler, NULL))
            {
               /* Initialize the BAS Manager Implementation Module (this*/
               /* is the module that is responsible for implementing the*/
               /* BAS Manager functionality - this module is just the   */
               /* framework shell).                                     */
               if((Result = _BASM_Initialize()) == 0)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Success\n"));
                  Initialized = TRUE;
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
            if(BASManagerMutex)
               BTPS_CloseMutex(BASManagerMutex);

            /* Flag that none of the resources are allocated.           */
            BASManagerMutex = NULL;

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("BAS Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for BAS Events.                              */
            for(CallbackEntry = CallbackEntryList; CallbackEntry; CallbackEntry = CallbackEntry->NextCallbackEntry)
            {
               _BASM_Un_Register_Client_Event_Callback(CallbackEntry->CallbackID);
            }

            /* Free the callback list.                                  */
            FreeCallbackEntryList();

            /* Make sure we inform the BAS Manager Implementation that  */
            /* we are shutting down.                                    */
            _BASM_Cleanup();

            BTPS_CloseMutex(BASManagerMutex);

            /* Flag that the resources are no longer allocated.         */
            BASManagerMutex = NULL;

            /* Flag that this module is no longer initialized.          */
            Initialized     = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Initialized));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Battery Service  */
   /* Manager (BASM).  This Callback will be dispatched by the BAS      */
   /* Manager when various BAS Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a BAS Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BASM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI BASM_Register_Client_Event_Callback(BASM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   Callback_Entry_t CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the BAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the BAS Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to register with the server for monitor events.  */
            if((ret_val = _BASM_Register_Client_Event_Callback()) > 0)
            {
               /* Attempt to add an entry into the BAS Entry list.      */
               CallbackEntry.EventCallback     = CallbackFunction;
               CallbackEntry.CallbackParameter = CallbackParameter;
               CallbackEntry.CallbackID        = (unsigned int)ret_val;

               /* Attempt to add the entry to the local callback list.  */
               if(!AddCallbackEntry(&CallbackEntry))
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(BASManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BAS Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BASM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the BAS Manager Event Callback ID (return value  */
   /* from BASM_Register_Client_Event_Callback() function).             */
void BTPSAPI BASM_Un_Register_Client_Event_Callback(unsigned int ClientCallbackID)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the BAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the BAS Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Delete the specified callback.                              */
         if((CallbackEntry = DeleteCallbackEntry(ClientCallbackID)) != NULL)
         {
            /* Un-Register the Event Callback with the Server.          */
            _BASM_Un_Register_Client_Event_Callback(CallbackEntry->CallbackID);

            /* Free the memory because we are finished with it.         */
            FreeCallbackEntryMemory(CallbackEntry);
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(BASManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to enable */
   /* notifications on a specified Battery Server instance.  This       */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
   /* * NOTE * Battery Level Notification support is optional for a     */
   /*          given Battery Server Instance.  This function will return*/
   /*          BTPM_ERROR_CODE_BATTERY_NOTIFY_UNSUPPORTED if the        */
   /*          specified instance does not support Battery Level        */
   /*          Notifications.                                           */
int BTPSAPI BASM_Enable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int               ret_val;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Attempt to wait for access to the BAS Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(ClientCallbackID)) != NULL)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Enable Notifications.                                 */
            ret_val = _BASM_Enable_Notifications(CallbackEntry->CallbackID, &RemoteServer, InstanceID);
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(BASManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to disable*/
   /* notifications on a specified Battery Server instance.  This       */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int BTPSAPI BASM_Disable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int               ret_val;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Attempt to wait for access to the BAS Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(ClientCallbackID)) != NULL)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Disable Notifications.                                */
            ret_val = _BASM_Disable_Notifications(CallbackEntry->CallbackID, &RemoteServer, InstanceID);
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(BASManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Level Request to a remote server.  This function    */
   /* accepts as input the Callback ID (return value from               */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns a positive Transaction ID on success; otherwise, a        */
   /* negative error value is returned.                                 */
int BTPSAPI BASM_Get_Battery_Level(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int               ret_val;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Attempt to wait for access to the BAS Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(ClientCallbackID)) != NULL)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to get the Battery Level.                                */
            ret_val = _BASM_Get_Battery_Level(CallbackEntry->CallbackID, &RemoteServer, InstanceID);
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(BASManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Identification Request to a remote server.  This    */
   /* will retrieve the Namespace and Description values from the       */
   /* Presentation Format Descriptor of the remote server.  This        */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Identification information from.  The*/
   /* third parameter is the Instance ID of the Battery Server.  This   */
   /* function returns a positive Transaction ID on success; otherwise, */
   /* a negative error value is returned.                               */
   /* * NOTE * Identification Information is only available on a remote */
   /*          device that have multiple BAS Server Instances.  This    */
   /*          function will return                                     */
   /*          BTPM_ERROR_CODE_BATTERY_IDENTIFICATION_UNSUPPORTED if the*/
   /*          specified remote device does not have multiple BAS Server*/
   /*          Instances.                                               */
int BTPSAPI BASM_Get_Battery_Identification(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int               ret_val;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Attempt to wait for access to the BAS Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(ClientCallbackID)) != NULL)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to get the Battery Identification information.           */
            ret_val = _BASM_Get_Battery_Identification(CallbackEntry->CallbackID, &RemoteServer, InstanceID);
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(BASManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (return value from               */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Transaction ID returned by*/
   /* a previously called function in this module.  This function       */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int BTPSAPI BASM_Cancel_Transaction(unsigned int ClientCallbackID, unsigned int TransactionID)
{
   int               ret_val;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Attempt to wait for access to the BAS Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(BASManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(ClientCallbackID)) != NULL)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to get the Battery Identification information.           */
            ret_val = _BASM_Cancel_Transaction(CallbackEntry->CallbackID, TransactionID);
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(BASManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
