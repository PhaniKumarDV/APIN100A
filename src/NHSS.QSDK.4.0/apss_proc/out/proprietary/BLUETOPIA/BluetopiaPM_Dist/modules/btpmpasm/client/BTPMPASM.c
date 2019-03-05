/*****< btpmpasm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPASM - Phone Alert Status (PAS) Manager for Stonestreet One Bluetooth */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange        Initial creation.                              */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMPASM.h"            /* BTPM PAS Manager Prototypes/Constants.    */
#include "PASMAPI.h"             /* PAS Manager Prototypes/Constants.         */
#include "PASMMSG.h"             /* BTPM PAS Manager Message Formats.         */
#include "PASMGR.h"              /* PAS Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPASM_Entry_Info_t
{
   unsigned int                  CallbackID;
   unsigned int                  EventHandlerID;
   BD_ADDR_t                     BD_ADDR;
   PASM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagPASM_Entry_Info_t *NextPASMEntryInfoPtr;
} PASM_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   PASM_Event_Callback_t  EventCallback;
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
static Mutex_t PASManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the PAS    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static PASM_Entry_Info_t *PASMEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static PASM_Entry_Info_t *AddPASMEntryInfoEntry(PASM_Entry_Info_t **ListHead, PASM_Entry_Info_t *EntryToAdd);
static PASM_Entry_Info_t *SearchPASMEntryInfoEntry(PASM_Entry_Info_t **ListHead, unsigned int CallbackID);
static PASM_Entry_Info_t *SearchPASMEntryInfoEntryByHandlerID(PASM_Entry_Info_t **ListHead, unsigned int HandlerID);
static PASM_Entry_Info_t *DeletePASMEntryInfoEntry(PASM_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreePASMEntryInfoEntryMemory(PASM_Entry_Info_t *EntryToFree);
static void FreePASMEntryInfoList(PASM_Entry_Info_t **ListHead);

static void ProcessConnectedEvent(PASM_Connected_Message_t *Message);
static void ProcessDisconnectedEvent(PASM_Disconnected_Message_t *Message);
static void ProcessRingerControlPointCommandEvent(PASM_Ringer_Control_Point_Command_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_PASM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI PASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the PASM Entry Information List.                             */
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
static PASM_Entry_Info_t *AddPASMEntryInfoEntry(PASM_Entry_Info_t **ListHead, PASM_Entry_Info_t *EntryToAdd)
{
   PASM_Entry_Info_t *AddedEntry = NULL;
   PASM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (PASM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(PASM_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextPASMEntryInfoPtr = NULL;

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
                     FreePASMEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextPASMEntryInfoPtr)
                        tmpEntry = tmpEntry->NextPASMEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextPASMEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static PASM_Entry_Info_t *SearchPASMEntryInfoEntry(PASM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   PASM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextPASMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Handler ID.  This function returns NULL if either */
   /* the List Head is invalid, the Event Handler ID is invalid, or the */
   /* specified Event Handler ID was NOT found.                         */
static PASM_Entry_Info_t *SearchPASMEntryInfoEntryByHandlerID(PASM_Entry_Info_t **ListHead, unsigned int HandlerID)
{
   PASM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", HandlerID));

   /* Let's make sure the list and Event Handler ID to search for appear*/
   /* to be valid.                                                      */
   if((ListHead) && (HandlerID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventHandlerID != HandlerID))
         FoundEntry = FoundEntry->NextPASMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified PASM Entry          */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the PASM Entry    */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreePASMEntryInfoEntryMemory().                  */
static PASM_Entry_Info_t *DeletePASMEntryInfoEntry(PASM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   PASM_Entry_Info_t *FoundEntry = NULL;
   PASM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextPASMEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextPASMEntryInfoPtr = FoundEntry->NextPASMEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextPASMEntryInfoPtr;

         FoundEntry->NextPASMEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified PASM Entry Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreePASMEntryInfoEntryMemory(PASM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified PASM Entry Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreePASMEntryInfoList(PASM_Entry_Info_t **ListHead)
{
   PASM_Entry_Info_t *EntryToFree;
   PASM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextPASMEntryInfoPtr;

         FreePASMEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Connected*/
   /* asynchronous message.                                             */
static void ProcessConnectedEvent(PASM_Connected_Message_t *Message)
{
   void                  *CallbackParameter;
   PASM_Event_Data_t      EventData;
   PASM_Entry_Info_t     *PASMEntryInfo;
   PASM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for the event callback to dispatch this event to.       */
      if((PASMEntryInfo = SearchPASMEntryInfoEntryByHandlerID(&PASMEntryInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Format the event to dispatch.                               */
         EventData.EventType                                        = etPASConnected;
         EventData.EventLength                                      = PASM_CONNECTED_EVENT_DATA_SIZE;

         EventData.EventData.ConnectedEventData.CallbackID          = PASMEntryInfo->CallbackID;
         EventData.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;
         EventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

         /* Cache the callback information locally before we release the*/
         /* PAS Manager Mutex.                                          */
         EventCallback                                              = PASMEntryInfo->EventCallback;
         CallbackParameter                                          = PASMEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(PASManagerMutex);

         /* Go ahead and dispatch the event.                            */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&EventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(PASManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find Event Callback for Event Handler ID: %u.\n", Message->EventHandlerID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(PASManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Disconnected asynchronous message.                                */
static void ProcessDisconnectedEvent(PASM_Disconnected_Message_t *Message)
{
   void                  *CallbackParameter;
   PASM_Event_Data_t      EventData;
   PASM_Entry_Info_t     *PASMEntryInfo;
   PASM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for the event callback to dispatch this event to.       */
      if((PASMEntryInfo = SearchPASMEntryInfoEntryByHandlerID(&PASMEntryInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Format the event to dispatch.                               */
         EventData.EventType                                           = etPASDisconnected;
         EventData.EventLength                                         = PASM_DISCONNECTED_EVENT_DATA_SIZE;

         EventData.EventData.DisconnectedEventData.CallbackID          = PASMEntryInfo->CallbackID;
         EventData.EventData.DisconnectedEventData.ConnectionType      = Message->ConnectionType;
         EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

         /* Cache the callback information locally before we release the*/
         /* PAS Manager Mutex.                                          */
         EventCallback                                                 = PASMEntryInfo->EventCallback;
         CallbackParameter                                             = PASMEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(PASManagerMutex);

         /* Go ahead and dispatch the event.                            */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&EventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(PASManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find Event Callback for Event Handler ID: %u.\n", Message->EventHandlerID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(PASManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Ringer   */
   /* Control Point Command asynchronous message.                       */
static void ProcessRingerControlPointCommandEvent(PASM_Ringer_Control_Point_Command_Message_t *Message)
{
   void                  *CallbackParameter;
   PASM_Event_Data_t      EventData;
   PASM_Entry_Info_t     *PASMEntryInfo;
   PASM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for the event callback to dispatch this event to.       */
      if((PASMEntryInfo = SearchPASMEntryInfoEntryByHandlerID(&PASMEntryInfoList, Message->ServerEventHandlerID)) != NULL)
      {
         /* Format the event to dispatch.                               */
         EventData.EventType                                                         = etPASRingerControlPointCommand;
         EventData.EventLength                                                       = PASM_RINGER_CONTROL_POINT_COMMAND_EVENT_DATA_SIZE;

         EventData.EventData.RingerControlPointCommandEventData.ServerCallbackID     = PASMEntryInfo->CallbackID;
         EventData.EventData.RingerControlPointCommandEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
         EventData.EventData.RingerControlPointCommandEventData.RingerControlCommand = Message->RingerControlCommand;

         /* Cache the callback information locally before we release the*/
         /* PAS Manager Mutex.                                          */
         EventCallback                                                               = PASMEntryInfo->EventCallback;
         CallbackParameter                                                           = PASMEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(PASManagerMutex);

         /* Go ahead and dispatch the event.                            */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&EventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(PASManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find Event Callback for Event Handler ID: %u.\n", Message->ServerEventHandlerID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(PASManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the PAS Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PASM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PAS Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the PAS */
               /* Connection Event.                                     */
               ProcessConnectedEvent((PASM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PASM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PAS Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the PAS */
               /* Disconnection Event.                                  */
               ProcessDisconnectedEvent((PASM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PASM_MESSAGE_FUNCTION_RINGER_CONTROL_POINT_COMMAND:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PAS Ringer Control Point Command Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_RINGER_CONTROL_POINT_COMMAND_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the PAS */
               /* Ringer Control Point Command Event.                   */
               ProcessRingerControlPointCommandEvent((PASM_Ringer_Control_Point_Command_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(PASManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PAS Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_PASM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the PAS state information.    */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the PAS state information.    */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the Event Callback List.                            */
            FreePASMEntryInfoList(&PASMEntryInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PASManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all PAS Manager Messages.   */
static void BTPSAPI PASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a PAS Manager defined    */
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
               /* PAS Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_PASM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an PAS Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Non PAS Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Phone Alert Status (PASM) Manager  */
   /* Module.  This function should be registered with the Bluetopia    */
   /* Platform Manager Module Handler and will be called when the       */
   /* Platform Manager is initialized (or shut down).                   */
void BTPSAPI PASM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAS Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((PASManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process PAS Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER, PASManagerGroupHandler, NULL))
            {
               /* Initialize the actual PAS Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the PAS Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _PASM_Initialize()))
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
            if(PASManagerMutex)
               BTPS_CloseMutex(PASManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER);

            /* Flag that none of the resources are allocated.           */
            PASManagerMutex = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PAS Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we inform the PAS Manager Implementation that  */
            /* we are shutting down.                                    */
            _PASM_Cleanup();

            BTPS_CloseMutex(PASManagerMutex);

            /* Make sure that the PAS Entry Information List is empty.  */
            FreePASMEntryInfoList(&PASMEntryInfoList);

            /* Flag that the resources are no longer allocated.         */
            PASManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PASM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(PASManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Phone Alert Status Server callback function */
   /* with the Phone Alert Status (PAS) Manager Service.  This Callback */
   /* will be dispatched by the PAS Manager when various PAS Manager    */
   /* Server Events occur.  This function accepts the Callback Function */
   /* and Callback Parameter (respectively) to call when a PAS Manager  */
   /* Server Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          PASM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
   /* * NOTE * Only 1 Server Event Callback can be registered in the    */
   /*          system at a time.                                        */
int BTPSAPI PASM_Register_Server_Event_Callback(PASM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   PASM_Entry_Info_t PASMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the PAS Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the PAS Entry list.         */
            BTPS_MemInitialize(&PASMEntryInfo, 0, sizeof(PASM_Entry_Info_t));

            PASMEntryInfo.CallbackID        = GetNextCallbackID();
            PASMEntryInfo.EventCallback     = CallbackFunction;
            PASMEntryInfo.CallbackParameter = CallbackParameter;

            /* Attempt to register with the server for monitor events.  */
            if((ret_val = _PASM_Register_Server_Events()) > 0)
            {
               /* Save the Server Event Handler ID.                     */
               PASMEntryInfo.EventHandlerID = (unsigned int)ret_val;

               /* Attempt to add the entry to the local callback list.  */
               if(AddPASMEntryInfoEntry(&PASMEntryInfoList, &PASMEntryInfo))
                  ret_val = PASMEntryInfo.CallbackID;
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PASManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Phone Alert Status (PAS)      */
   /* Manager Server Event Callback (registered via a successful call to*/
   /* the PASM_Register_Server_Event_Callback() function).  This        */
   /* function accepts as input the PAS Manager Event Callback ID       */
   /* (return value from PASM_Register_Server_Event_Callback()          */
   /* function).                                                        */
void BTPSAPI PASM_Un_Register_Server_Event_Callback(unsigned int ServerCallbackID)
{
   PASM_Entry_Info_t *PASMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ServerCallbackID)
      {
         /* Attempt to wait for access to the PAS Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the specified callback.                           */
            if((PASMEntryInfo = DeletePASMEntryInfoEntry(&PASMEntryInfoList, ServerCallbackID)) != NULL)
            {
               /* Un-Register the Event Handler with the Server.        */
               _PASM_Un_Register_Server_Events(PASMEntryInfo->EventHandlerID);

               /* Free the memory because we are finished with it.      */
               FreePASMEntryInfoEntryMemory(PASMEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PASManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS).  This function is  */
   /* responsible for updating the Alert Status internally, as well as  */
   /* dispatching any Alert Notifications that have been registered by  */
   /* Phone Alert Status (PAS) clients.  This function accepts as it's  */
   /* parameter the Server callback ID that was returned from a         */
   /* successful call to PASM_Register_Server_Event_Callback() followed */
   /* by the Alert Status value to set.  This function returns zero if  */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI PASM_Set_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus)
{
   int                ret_val;
   PASM_Entry_Info_t *PASMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ServerCallbackID) && (AlertStatus))
      {
         /* Attempt to wait for access to the PAS Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PASMEntryInfo = SearchPASMEntryInfoEntry(&PASMEntryInfoList, ServerCallbackID)) != NULL)
            {
               /* Simply call the function to actually set the Alert    */
               /* Status.                                               */
               ret_val = _PASM_Set_Alert_Status(PASMEntryInfo->EventHandlerID, AlertStatus);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PASManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) alert       */
   /* status.  This function accepts as it's parameter the Server       */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured alert status upon successful   */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the AlertStatus    */
   /*          buffer will contain the currently configured alert       */
   /*          status.  If this function returns an error then the      */
   /*          contents of the AlertStatus buffer will be undefined.    */
int BTPSAPI PASM_Query_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus)
{
   int                ret_val;
   PASM_Entry_Info_t *PASMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ServerCallbackID) && (AlertStatus))
      {
         /* Attempt to wait for access to the PAS Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PASMEntryInfo = SearchPASMEntryInfoEntry(&PASMEntryInfoList, ServerCallbackID)) != NULL)
            {
               /* Simply call the function to actually query the Alert  */
               /* Status.                                               */
               ret_val = _PASM_Query_Alert_Status(PASMEntryInfo->EventHandlerID, AlertStatus);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PASManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS) ringer setting as   */
   /* well as dispatching any Alert Notifications that have been        */
   /* registered by PAS clients.  This function accepts as it's         */
   /* parameter the PAS Server callback ID that was returned from a     */
   /* successful call to PASM_Register_Server_Event_Callback() and the  */
   /* ringer value to configure.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI PASM_Set_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t RingerSetting)
{
   int                ret_val;
   PASM_Entry_Info_t *PASMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ServerCallbackID)
      {
         /* Attempt to wait for access to the PAS Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PASMEntryInfo = SearchPASMEntryInfoEntry(&PASMEntryInfoList, ServerCallbackID)) != NULL)
            {
               /* Simply call the function to actually set the Ringer   */
               /* Setting.                                              */
               ret_val = _PASM_Set_Ringer_Setting(PASMEntryInfo->EventHandlerID, RingerSetting);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PASManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) ringer      */
   /* setting.  This function accepts as it's parameter the Server      */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured ringer setting upon successful */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the RingerSetting  */
   /*          buffer will contain the currently configured ringer      */
   /*          setting.  If this function returns an error then the     */
   /*          contents of the RingerSetting buffer will be undefined.  */
int BTPSAPI PASM_Query_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t *RingerSetting)
{
   int                ret_val;
   PASM_Entry_Info_t *PASMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((ServerCallbackID) && (RingerSetting))
      {
         /* Attempt to wait for access to the PAS Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PASManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PASMEntryInfo = SearchPASMEntryInfoEntry(&PASMEntryInfoList, ServerCallbackID)) != NULL)
            {
               /* Simply call the function to actually query the Ringer */
               /* Setting.                                              */
               ret_val = _PASM_Query_Ringer_Setting(PASMEntryInfo->EventHandlerID, RingerSetting);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PASManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

