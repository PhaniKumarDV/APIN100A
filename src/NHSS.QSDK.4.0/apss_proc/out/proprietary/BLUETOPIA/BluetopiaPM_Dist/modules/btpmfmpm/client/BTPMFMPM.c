/*****< btpmfmpm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMFMPM - FMP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/27/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMFMPM.h"            /* BTPM FMP Manager Prototypes/Constants.    */
#include "FMPMAPI.h"             /* FMP Manager Prototypes/Constants.         */
#include "FMPMMSG.h"             /* BTPM FMP Manager Message Formats.         */
#include "FMPMGR.h"              /* FMP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagFMP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 EventHandlerID;
   FMPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagFMP_Entry_Info_t *NextFMPEntryInfoPtr;
} FMP_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   FMPM_Event_Callback_t  EventCallback;
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
static Mutex_t FMPManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the FMP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static FMP_Entry_Info_t *FMPEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static FMP_Entry_Info_t *AddFMPEntryInfoEntry(FMP_Entry_Info_t **ListHead, FMP_Entry_Info_t *EntryToAdd);
static FMP_Entry_Info_t *SearchFMPEntryInfoEntryByHandlerID(FMP_Entry_Info_t **ListHead, unsigned int EventHandlerID);
static FMP_Entry_Info_t *DeleteFMPEntryInfoEntry(FMP_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeFMPEntryInfoEntryMemory(FMP_Entry_Info_t *EntryToFree);
static void FreeFMPEntryInfoList(FMP_Entry_Info_t **ListHead);

static void ProcessFMPAlertMessage(FMPM_Alert_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_FMPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI FMPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the FMP Entry Information List.                              */
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
static FMP_Entry_Info_t *AddFMPEntryInfoEntry(FMP_Entry_Info_t **ListHead, FMP_Entry_Info_t *EntryToAdd)
{
   FMP_Entry_Info_t *AddedEntry = NULL;
   FMP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (FMP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(FMP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextFMPEntryInfoPtr = NULL;

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
                     FreeFMPEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextFMPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextFMPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextFMPEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static FMP_Entry_Info_t *SearchFMPEntryInfoEntryByHandlerID(FMP_Entry_Info_t **ListHead, unsigned int EventHandlerID)
{
   FMP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventHandlerID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventHandlerID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventHandlerID != EventHandlerID))
         FoundEntry = FoundEntry->NextFMPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified FMP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the FMP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeFMPEntryInfoEntryMemory().                   */
static FMP_Entry_Info_t *DeleteFMPEntryInfoEntry(FMP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   FMP_Entry_Info_t *FoundEntry = NULL;
   FMP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextFMPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextFMPEntryInfoPtr = FoundEntry->NextFMPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextFMPEntryInfoPtr;

         FoundEntry->NextFMPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified FMP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeFMPEntryInfoEntryMemory(FMP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified FMP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeFMPEntryInfoList(FMP_Entry_Info_t **ListHead)
{
   FMP_Entry_Info_t *EntryToFree;
   FMP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextFMPEntryInfoPtr;

         FreeFMPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the FMP Alert*/
   /* asynchronous message.                                             */
static void ProcessFMPAlertMessage(FMPM_Alert_Message_t *Message)
{
   void                  *CallbackParameter;
   FMP_Entry_Info_t      *FMPEntryInfo;
   FMPM_Event_Data_t      FMPMEventData;
   FMPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(Message)
   {
      /* Search for the callback entry that this event should be        */
      /* dispatched to.                                                 */
      if((FMPEntryInfo = SearchFMPEntryInfoEntryByHandlerID(&FMPEntryInfoList, Message->TargetEventHandlerID)) != NULL)
      {
         /* Format up the Event.                                        */
         FMPMEventData.EventType                                        = aetFMPAlert;
         FMPMEventData.EventLength                                      = FMPM_ALERT_EVENT_DATA_SIZE;

         FMPMEventData.EventData.AlertEventData.TargetCallbackID        = FMPEntryInfo->CallbackID;
         FMPMEventData.EventData.AlertEventData.ConnectionType          = Message->ConnectionType;
         FMPMEventData.EventData.AlertEventData.RemoteDeviceAddress     = Message->RemoteDeviceAddress;
         FMPMEventData.EventData.AlertEventData.AlertLevel              = Message->AlertLevel;

         /* Store the event callback and the callback parameter locally */
         /* before we release the FMP Manager Mutex.                    */
         EventCallback     = FMPEntryInfo->EventCallback;
         CallbackParameter = FMPEntryInfo->CallbackParameter;

         /* Release the Mutex before we make the callback.              */
         BTPS_ReleaseMutex(FMPManagerMutex);

         /* Go ahead and make the callback.                             */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&FMPMEventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error occurred so just release the manager mutex.           */
         BTPS_ReleaseMutex(FMPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Event Handler ID\n"));
      }
   }
   else
   {
      /* Error occurred so just release the manager mutex.              */
      BTPS_ReleaseMutex(FMPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the FMP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case FMPM_MESSAGE_FUNCTION_FMP_ALERT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("FMP Alert\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= FMPM_ALERT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Alert Event.                                          */
               ProcessFMPAlertMessage((FMPM_Alert_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(FMPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process FMP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_FMPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the FMP state information.    */
         if(BTPS_WaitMutex(FMPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the FMP state information.    */
         if(BTPS_WaitMutex(FMPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the Event Callback List.                            */
            FreeFMPEntryInfoList(&FMPEntryInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(FMPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all FMP Manager Messages.   */
static void BTPSAPI FMPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_FIND_ME_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("FMP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a FMP Manager defined    */
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
               /* FMP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_FMPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue FMP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue FMP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an FMP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Non FMP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager FMP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI FMPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing FMP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((FMPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process FMP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_FIND_ME_MANAGER, FMPManagerGroupHandler, NULL))
            {
               /* Initialize the actual FMP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the FMP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _FMPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique starting Callback ID.          */
                  NextCallbackID    = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized       = TRUE;
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
            if(FMPManagerMutex)
               BTPS_CloseMutex(FMPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_FIND_ME_MANAGER);

            /* Flag that none of the resources are allocated.           */
            FMPManagerMutex = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("FMP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_FIND_ME_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(FMPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we inform the FMP Manager Implementation that  */
            /* we are shutting down.                                    */
            _FMPM_Cleanup();

            BTPS_CloseMutex(FMPManagerMutex);

            /* Make sure that the FMP Entry Information List is empty.  */
            FreeFMPEntryInfoList(&FMPEntryInfoList);

            /* Flag that the resources are no longer allocated.         */
            FMPManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI FMPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the FMP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(FMPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(FMPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (FMP) Manager Service.  This Callback will be        */
   /* dispatched by the FMP Manager when various FMP Manager Server     */
   /* Events occur.  This function accepts the Callback Function and    */
   /* Callback Parameter (respectively) to call when a FMP Manager      */
   /* Server Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          FMPM_Un_Register_Target_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI FMPM_Register_Target_Event_Callback(FMPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   FMP_Entry_Info_t FMPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the FMP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the FMP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(FMPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the FMP Entry list.         */
            BTPS_MemInitialize(&FMPEntryInfo, 0, sizeof(FMP_Entry_Info_t));

            FMPEntryInfo.CallbackID        = GetNextCallbackID();
            FMPEntryInfo.EventCallback     = CallbackFunction;
            FMPEntryInfo.CallbackParameter = CallbackParameter;

            /* Attempt to register with the server.                     */
            if((ret_val = _FMPM_Register_Target_Events()) > 0)
            {
               /* Save the Event Handler ID for the callback.           */
               FMPEntryInfo.EventHandlerID = (unsigned int)ret_val;

               /* Attempt to add the event callack to the callback list.*/
               if(AddFMPEntryInfoEntry(&FMPEntryInfoList, &FMPEntryInfo))
                  ret_val = FMPEntryInfo.CallbackID;
               else
               {
                  /* Un-register the event handler from the server.     */
                  _FMPM_Un_Register_Target_Events(FMPEntryInfo.EventHandlerID);

                  /* Return an error to the caller.                     */
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(FMPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_FIND_ME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered FMP Manager Server Event      */
   /* Callback (registered via a successful call to the                 */
   /* FMPM_Register_Target_Event_Callback() function).  This function   */
   /* accepts as input the FMP Manager Target Event Callback ID (return */
   /* value from FMPM_Register_Target_Event_Callback() function).       */
void BTPSAPI FMPM_Un_Register_Target_Event_Callback(unsigned int TargetCallbackID)
{
   FMP_Entry_Info_t *FMPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the FMP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(TargetCallbackID)
      {
         /* Attempt to wait for access to the FMP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(FMPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to delete the callback from the list.            */
            if((FMPEntryInfo = DeleteFMPEntryInfoEntry(&FMPEntryInfoList, TargetCallbackID)) != NULL)
            {
               /* Un-register the event handler from the server.        */
               _FMPM_Un_Register_Target_Events(FMPEntryInfo->EventHandlerID);

               /* Free the memory because we are finished with it.      */
               FreeFMPEntryInfoEntryMemory(FMPEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(FMPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

