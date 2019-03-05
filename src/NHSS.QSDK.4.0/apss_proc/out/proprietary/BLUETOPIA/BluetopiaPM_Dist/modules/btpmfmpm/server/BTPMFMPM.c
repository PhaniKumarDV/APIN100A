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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

   /* The following defines the FMPM LE Configuration File Section Name.*/
#define FMPM_LE_CONFIGURATION_FILE_SECTION_NAME                   "FMPM"

   /* The following define the Key Names that are used with the FMPM    */
   /* Configuration File.                                               */
#define FMPM_KEY_NAME_PERSISTENT_UID                              "PU"

   /* The following defines the size of a Persistent UID that is stored */
   /* in the configuration file.                                        */
#define FMPM_PERSISTENT_UID_SIZE                                  (NON_ALIGNED_DWORD_SIZE + (NON_ALIGNED_WORD_SIZE*2))

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagFMPM_Event_Callback_Info_t
{
   unsigned int                           EventCallbackID;
   unsigned int                           ClientID;
   FMPM_Event_Callback_t                  EventCallback;
   void                                  *CallbackParameter;
   struct _tagFMPM_Event_Callback_Info_t *NextFMPMEventCallbackInfoPtr;
} FMPM_Event_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           CallbackID;
   unsigned int           ClientID;
   FMPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextEventCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the FMP    */
   /* Target Callback Info List (which holds all FMPM Target Event      */
   /* Callbacks registered with this module).                           */
static FMPM_Event_Callback_Info_t *TargetEventCallbackInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextEventCallbackID(void);

static FMPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(FMPM_Event_Callback_Info_t **ListHead, FMPM_Event_Callback_Info_t *EntryToAdd);
static FMPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(FMPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static FMPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(FMPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static void FreeEventCallbackInfoEntryMemory(FMPM_Event_Callback_Info_t *EntryToFree);
static void FreeEventCallbackInfoList(FMPM_Event_Callback_Info_t **ListHead);

static void DispatchFMPMEvent(FMPM_Event_Callback_Info_t **ListHead, FMPM_Event_Data_t *FMPMEventData, BTPM_Message_t *Message, unsigned int *EventCallbackID, unsigned int *MessageHandlerID);

static void DispatchFMPAlertEvent(FMPM_Connection_Type_t ConnectionType, BD_ADDR_t BD_ADDR, FMPM_Alert_Level_t AlertLevel);

static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store);
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static void ProcessRegisterFMPTargetEventsRequestMessage(FMPM_Register_Target_Events_Request_t *Message);
static void ProcessUnRegisterFMPTargetEventsRequestMessage(FMPM_Un_Register_Target_Events_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessIASControlPointCommand(IAS_Alert_Level_Control_Point_Command_Data_t *ControlPointCommandData);

static void ProcessIASEvent(FMPM_Target_Event_Data_t *TargetEventData);

static void BTPSAPI BTPMDispatchCallback_FMPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_IAS(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI FMPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the FMP Event Callback List.                                 */
static unsigned int GetNextEventCallbackID(void)
{
   ++NextEventCallbackID;

   if(NextEventCallbackID & 0x80000000)
      NextEventCallbackID = 1;

   return(NextEventCallbackID);
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            EventCallbackID field is the same as an entry already  */
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static FMPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(FMPM_Event_Callback_Info_t **ListHead, FMPM_Event_Callback_Info_t *EntryToAdd)
{
   FMPM_Event_Callback_Info_t *AddedEntry = NULL;
   FMPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->EventCallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (FMPM_Event_Callback_Info_t *)BTPS_AllocateMemory(sizeof(FMPM_Event_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                              = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextFMPMEventCallbackInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->EventCallbackID == AddedEntry->EventCallbackID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeEventCallbackInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextFMPMEventCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextFMPMEventCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextFMPMEventCallbackInfoPtr = AddedEntry;
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
static FMPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(FMPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   FMPM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
         FoundEntry = FoundEntry->NextFMPMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the Callback ID  */
   /* is invalid, or the specified Callback ID was NOT present in the   */
   /* list.  The entry returned will have the Next Entry field set to   */
   /* NULL, and the caller is responsible for deleting the memory       */
   /* associated with this entry by calling                             */
   /* FreeEventCallbackInfoEntryMemory().                               */
static FMPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(FMPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   FMPM_Event_Callback_Info_t *FoundEntry = NULL;
   FMPM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextFMPMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextFMPMEventCallbackInfoPtr = FoundEntry->NextFMPMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextFMPMEventCallbackInfoPtr;

         FoundEntry->NextFMPMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Event Callback member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeEventCallbackInfoEntryMemory(FMPM_Event_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Event Callback List.  Upon return of this*/
   /* function, the Head Pointer is set to NULL.                        */
static void FreeEventCallbackInfoList(FMPM_Event_Callback_Info_t **ListHead)
{
   FMPM_Event_Callback_Info_t *EntryToFree;
   FMPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Event Callback ID: 0x%08X\n", EntryToFree->EventCallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextFMPMEventCallbackInfoPtr;

         FreeEventCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified FMP event to every registered FMP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the FMP Manager Lock */
   /*          held.  Upon exit from this function it will free the FMP */
   /*          Manager Lock.                                            */
static void DispatchFMPMEvent(FMPM_Event_Callback_Info_t **ListHead, FMPM_Event_Data_t *FMPMEventData, BTPM_Message_t *Message, unsigned int *EventCallbackID, unsigned int *MessageHandlerID)
{
   unsigned int                Index;
   unsigned int                ServerID;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   FMPM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((ListHead) && (FMPMEventData) && (Message) && (EventCallbackID) && (MessageHandlerID))
   {
      /* Next, let's determine how many callbacks are registered.       */
      CallbackInfoPtr = *ListHead;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(CallbackInfoPtr)
      {
         if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
            NumberCallbacks++;

         CallbackInfoPtr = CallbackInfoPtr->NextFMPMEventCallbackInfoPtr;
      }

      if(NumberCallbacks)
      {
         if(NumberCallbacks <= (sizeof(CallbackInfoArray)/sizeof(Callback_Info_t)))
            CallbackInfoArrayPtr = CallbackInfoArray;
         else
            CallbackInfoArrayPtr = BTPS_AllocateMemory((NumberCallbacks*sizeof(Callback_Info_t)));

         /* Make sure that we have memory to copy the Callback List     */
         /* into.                                                       */
         if(CallbackInfoArrayPtr)
         {
            CallbackInfoPtr = *ListHead;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackInfoPtr)
            {
               if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackID        = CallbackInfoPtr->EventCallbackID;
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackInfoPtr->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfoPtr = CallbackInfoPtr->NextFMPMEventCallbackInfoPtr;
            }

            /* Release the Lock because we have already built the       */
            /* Callback Array.                                          */
            DEVM_ReleaseLock();

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* * NOTE * It is possible that we have already          */
               /*          dispatched the event to the client (case     */
               /*          would occur if a single client has registered*/
               /*          for FMP events and Data Events.              */
               /*          To avoid this case we need to walk the       */
               /*          list of previously dispatched events to check*/
               /*          to see if it has already been dispatched     */
               /*          (we need to do this with Client Address ID's */
               /*          for messages - Event Callbacks are local     */
               /*          and therefore unique so we don't have to do  */
               /*          this filtering.                              */

               /* Determine the type of event that needs to be          */
               /* dispatched.                                           */
               if(CallbackInfoArrayPtr[Index].ClientID == ServerID)
               {
                  /* Go ahead and make the callback.                    */
                  /* * NOTE * If the callback was deleted (or new ones  */
                  /*          were added, they will not be caught for   */
                  /*          this message dispatch).  Under normal     */
                  /*          operating circumstances this case         */
                  /*          shouldn't matter because these groups     */
                  /*          aren't really dynamic and are only        */
                  /*          registered at initialization time.        */
                  __BTPSTRY
                  {
                     if(CallbackInfoArrayPtr[Index].EventCallback)
                     {
                        /* Update the Event Callback ID to be the       */
                        /* Callback ID for the callback that is         */
                        /* currently being dispatched.                  */
                        *EventCallbackID = CallbackInfoArrayPtr[Index].CallbackID;

                        (*CallbackInfoArrayPtr[Index].EventCallback)(FMPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Update the Handler ID to be the Handler ID for the */
                  /* message handler that is currently being dispatched */
                  /* to.                                                */
                  *MessageHandlerID                = CallbackInfoArrayPtr[Index].CallbackID;

                  /* Dispatch the Message.                              */
                  Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

                  MSG_SendMessage(Message);
               }

               Index++;
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a FMP Alert Event to all registered callbacks.           */
static void DispatchFMPAlertEvent(FMPM_Connection_Type_t ConnectionType, BD_ADDR_t BD_ADDR, FMPM_Alert_Level_t AlertLevel)
{
   FMPM_Event_Data_t               EventData;
   FMPM_Alert_Message_t            Message;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionType == fctTarget) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Query the remote device properties to get the proper BD_ADDR to*/
      /* use.                                                           */
      if(!DEVM_QueryRemoteDeviceProperties(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties))
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                    = aetFMPAlert;
         EventData.EventLength                                  = FMPM_ALERT_EVENT_DATA_SIZE;

         EventData.EventData.AlertEventData.ConnectionType      = ConnectionType;
         EventData.EventData.AlertEventData.RemoteDeviceAddress = RemoteDeviceProperties.BD_ADDR;
         EventData.EventData.AlertEventData.AlertLevel          = AlertLevel;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_FIND_ME_MANAGER;
         Message.MessageHeader.MessageFunction = FMPM_MESSAGE_FUNCTION_FMP_ALERT;
         Message.MessageHeader.MessageLength   = (FMPM_ALERT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionType;
         Message.RemoteDeviceAddress           = RemoteDeviceProperties.BD_ADDR;
         Message.AlertLevel                    = AlertLevel;

         /* Dispatch the event to all registered callbacks.             */
         DispatchFMPMEvent(&TargetEventCallbackInfoList, &EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.AlertEventData.TargetCallbackID), &(Message.TargetEventHandlerID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to store*/
   /* Persistent UID for the IAS service registered by this module from */
   /* the Low Energy Configuration File.                                */
static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store)
{
   char          TempString[64];
   unsigned int  Index;
   unsigned char TempBuffer[FMPM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(((Store == FALSE) || ((PersistentUID) && (ServiceHandleRange))))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, FMPM_KEY_NAME_PERSISTENT_UID);

      /* Check to see if we are storing something to flash or deleting  */
      /* something.                                                     */
      if(Store)
      {
         /* Reset the Index.                                            */
         Index  = 0;

         /* Format the Persistent UID and the Service Handle Range.     */
         ASSIGN_HOST_DWORD_TO_BIG_ENDIAN_UNALIGNED_DWORD(&(TempBuffer[Index]), PersistentUID);

         Index += NON_ALIGNED_DWORD_SIZE;

         ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(TempBuffer[Index]), ServiceHandleRange->Starting_Handle);

         Index += NON_ALIGNED_WORD_SIZE;

         ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(TempBuffer[Index]), ServiceHandleRange->Ending_Handle);

         /* Now write out the new Key-Value Pair.                       */
         SET_WriteBinaryData(FMPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
      else
      {
         /* Delete the configuration stored.                            */
         SET_WriteBinaryData(FMPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the Persistent UID for the IAS service registered by this  */
   /* module from the Low Energy Configuration File.  This function     */
   /* returns TRUE if the Persistent UID was reloaded or false          */
   /* otherwise.                                                        */
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   char          TempString[64];
   Boolean_t     ret_val = FALSE;
   unsigned int  Index;
   unsigned char TempBuffer[FMPM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((PersistentUID) && (ServiceHandleRange))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, FMPM_KEY_NAME_PERSISTENT_UID);

      /* Attempt to reload the configuration for this device.           */
      if(SET_ReadBinaryData(FMPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == sizeof(TempBuffer))
      {
         /* Reset the Index.                                            */
         Index                                = 0;

         /* Reload the Persistent UID and Service Handle Range.         */
         *PersistentUID                       = READ_UNALIGNED_DWORD_BIG_ENDIAN(&(TempBuffer[Index]));

         Index                               += NON_ALIGNED_DWORD_SIZE;

         ServiceHandleRange->Starting_Handle  = READ_UNALIGNED_WORD_BIG_ENDIAN(&(TempBuffer[Index]));

         Index                               += NON_ALIGNED_WORD_SIZE;

         ServiceHandleRange->Ending_Handle    = READ_UNALIGNED_WORD_BIG_ENDIAN(&(TempBuffer[Index]));

         /* Return success to the caller.                               */
         ret_val                              = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Persistent UID: 0x%08X\n", (unsigned int)*PersistentUID));
         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Service Range:  0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* either load from the configuration file or generate a new Service */
   /* Handle Range for the IAS Service that is registered by this       */
   /* module.  This function returns TRUE if a Handle Range was         */
   /* calculated or FALSE otherwise.                                    */
static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   DWord_t      PersistentUID;
   Boolean_t    RegisterPersistent;
   Boolean_t    ret_val = FALSE;
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ServiceHandleRange)
   {
      /* Query the number of attributes needed by the service.          */
      if((NumberOfAttributes = _FMPM_Query_Number_Attributes()) > 0)
      {
         /* Attempt to re-load a previously registered Persistent UID   */
         /* from the file.                                              */
         if(ReloadPersistentUID(&PersistentUID, ServiceHandleRange))
         {
            /* Verify this handle range has enough attributes to satisfy*/
            /* the request.                                             */
            if(((unsigned int)((ServiceHandleRange->Ending_Handle - ServiceHandleRange->Starting_Handle) + 1)) != NumberOfAttributes)
            {
               /* Delete the old stored Persistent UID.                 */
               StorePersistentUID(0, NULL, FALSE);

               /* We don't have enough handles for this service so      */
               /* allocate a new range after deleting what we have      */
               /* stored now.                                           */
               RegisterPersistent = TRUE;
            }
            else
            {
               /* We already have allocated the Service Handle Range so */
               /* just return success.                                  */
               ret_val            = TRUE;
               RegisterPersistent = FALSE;
            }
         }
         else
            RegisterPersistent = TRUE;

         /* If requested attempt to allocate a new Persistent UID.      */
         if(RegisterPersistent)
         {
            /* Attempt to register a new Persistent UID for the         */
            /* requested number of attributes.                          */
            /* * NOTE * We will subtract 1 attribute (for the service   */
            /*          declaration because GATM_RegisterPersistentUID()*/
            /*          already includes this in it's calculation.      */
            if(!GATM_RegisterPersistentUID((NumberOfAttributes - 1), &PersistentUID, ServiceHandleRange))
            {
               /* Store this configure into the LE Configuration file.  */
               StorePersistentUID(PersistentUID, ServiceHandleRange, TRUE);

               /* Return success to the caller.                         */
               ret_val = TRUE;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function processes the specified Register FMP Events*/
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the FMP Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterFMPTargetEventsRequestMessage(FMPM_Register_Target_Events_Request_t *Message)
{
   int                                    Result;
   FMPM_Event_Callback_Info_t             EventCallbackEntry;
   FMPM_Register_Target_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Attempt to add an entry into the Event Callback Entry list.    */
      BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(FMPM_Event_Callback_Info_t));

      EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
      EventCallbackEntry.ClientID          = Message->MessageHeader.AddressID;
      EventCallbackEntry.EventCallback     = NULL;
      EventCallbackEntry.CallbackParameter = 0;

      if(AddEventCallbackInfoEntry(&TargetEventCallbackInfoList, &EventCallbackEntry))
         Result = EventCallbackEntry.EventCallbackID;
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = FMPM_REGISTER_TARGET_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                       = 0;

         ResponseMessage.FMPTargetEventHandlerID      = (unsigned int)Result;
      }
      else
      {
         ResponseMessage.Status                       = Result;

         ResponseMessage.FMPTargetEventHandlerID      = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register FMP    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the FMP Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterFMPTargetEventsRequestMessage(FMPM_Un_Register_Target_Events_Request_t *Message)
{
   int                                        Result;
   FMPM_Event_Callback_Info_t                *EventCallbackEntryPtr;
   FMPM_Un_Register_Target_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the callback that is being registered and verify    */
      /* that the process that is un-registering the callback is the    */
      /* same process that registered the callback.                     */
      if(((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&TargetEventCallbackInfoList, Message->FMPTargetEventHandlerID)) != NULL) && (EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID))
      {
         /* Delete the specified callback entry.                        */
         if((EventCallbackEntryPtr = DeleteEventCallbackInfoEntry(&TargetEventCallbackInfoList, Message->FMPTargetEventHandlerID)) != NULL)
         {
            /* Free the memory allocated for this event callback.       */
            FreeEventCallbackInfoEntryMemory(EventCallbackEntryPtr);

            /* Return success.                                          */
            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = FMPM_UN_REGISTER_TARGET_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the FMP Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case FMPM_MESSAGE_FUNCTION_REGISTER_TARGET_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Register FMP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= FMPM_REGISTER_TARGET_EVENTS_REQUEST_SIZE)
               ProcessRegisterFMPTargetEventsRequestMessage((FMPM_Register_Target_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case FMPM_MESSAGE_FUNCTION_UN_REGISTER_TARGET_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register FMP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= FMPM_UN_REGISTER_TARGET_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterFMPTargetEventsRequestMessage((FMPM_Un_Register_Target_Events_Request_t *)Message);
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

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   FMPM_Event_Callback_Info_t *EventCallback;
   FMPM_Event_Callback_Info_t *tmpEventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Loop through the event callback list and delete all callbacks  */
      /* registered for this callback.                                  */
      EventCallback = TargetEventCallbackInfoList;
      while(EventCallback)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(EventCallback->ClientID == ClientID)
         {
            /* Note the next Event Callback Entry in the list (we are   */
            /* about to delete the current entry).                      */
            tmpEventCallback = EventCallback->NextFMPMEventCallbackInfoPtr;

            /* Go ahead and delete the Event Callback Entry and clean up*/
            /* the resources.                                           */
            if((EventCallback = DeleteEventCallbackInfoEntry(&TargetEventCallbackInfoList, EventCallback->EventCallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreeEventCallbackInfoEntryMemory(EventCallback);
            }

            /* Go ahead and set the next Event Callback Entry (past the */
            /* one we just deleted).                                    */
            EventCallback = tmpEventCallback;
         }
         else
            EventCallback = EventCallback->NextFMPMEventCallbackInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Control Point Command event from a remote client.       */
static void ProcessIASControlPointCommand(IAS_Alert_Level_Control_Point_Command_Data_t *ControlPointCommandData)
{
   FMPM_Alert_Level_t AlertLevel;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ControlPointCommandData)
   {
      /* Determine what the client is attempting to read.               */
      switch(ControlPointCommandData->Command)
      {
         case cpNoAlert:
         case cpMildAlert:
         case cpHighAlert:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Client issued alert: %u\n", (unsigned int)ControlPointCommandData->Command));

            /* Convert the alert level to a PM Alert Level.             */
            if(ControlPointCommandData->Command == cpNoAlert)
               AlertLevel = falNoAlert;
            else
            {
               if(ControlPointCommandData->Command == cpMildAlert)
                  AlertLevel = falMildAlert;
               else
                  AlertLevel = falHighAlert;
            }

            /* Dispatch a PM Target Alert.                              */
            DispatchFMPAlertEvent(fctTarget, ControlPointCommandData->RemoteDevice, AlertLevel);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Control Point Command: %u\n", (unsigned int)ControlPointCommandData->Command));
            break;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing IAS Events that have been received.  This function     */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessIASEvent(FMPM_Target_Event_Data_t *TargetEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(TargetEventData)
   {
      /* Process the event based on the event type.                     */
      switch(TargetEventData->Event_Data_Type)
      {
         case etIAS_Server_Alert_Level_Control_Point_Command:
            /* Process the Control Point Command event.                 */
            ProcessIASControlPointCommand(&(TargetEventData->Event_Data.ControlPointCommand));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown IAS Event Type: %d\n", TargetEventData->Event_Data_Type));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid IAS Event Data\n"));
   }

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
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Process the Message.                                     */
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Note we do not have to release the Lock because          */
            /* ProcessReceivedMessage() is documented that it will be   */
            /* called with the Lock being held and it will release the  */
            /* Lock when it is finished with it.                        */
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process IAS Events.                                 */
static void BTPSAPI BTPMDispatchCallback_IAS(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Double check that this is an IAS Event Update.           */
            if(((FMPM_Update_Data_t *)CallbackParameter)->UpdateType == utFMPTargetEvent)
               ProcessIASEvent(&(((FMPM_Update_Data_t *)CallbackParameter)->UpdateData.TargetEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
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
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Process the Client Un-Register Message.                  */
            ProcessClientUnRegister((unsigned int)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
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
               /* Dispatch to the main handler that a client has        */
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
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing FMP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process FMP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_FIND_ME_MANAGER, FMPManagerGroupHandler, NULL))
         {
            /* Initialize the actual FMP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the FMP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _FMPM_Initialize()))
            {
               /* Determine the current Device Power State.             */
               CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               /* Initialize a unique, starting FMP Callback ID.        */
               NextEventCallbackID     = 0;

               /* Go ahead and flag that this module is initialized.    */
               Initialized             = TRUE;

               /* Flag success.                                         */
               Result                  = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _FMPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_FIND_ME_MANAGER);
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

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the FMP Manager Implementation that  */
            /* we are shutting down.                                    */
            _FMPM_Cleanup();

            /* Free the Event Callback Info List.                       */
            FreeEventCallbackInfoList(&TargetEventCallbackInfoList);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState       = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
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
   int                           Result;
   GATT_Attribute_Handle_Group_t ServiceHandleRange;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the FMP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the FMP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  /* Attempt to calculate the Service Handle Range for  */
                  /* this service in the GATT database.                 */
                  if(CalculateServiceHandleRange(&ServiceHandleRange))
                     _FMPM_SetBluetoothStackID((unsigned int)Result, &ServiceHandleRange);
                  else
                     _FMPM_SetBluetoothStackID((unsigned int)Result, NULL);
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the FMP Manager that the Stack has been closed.*/
               _FMPM_SetBluetoothStackID(0, NULL);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the FMP Manager of a specific Update Event.  The FMP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t FMPM_NotifyUpdate(FMPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utFMPTargetEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing IAS Event: %d\n", UpdateData->UpdateData.TargetEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Message.                             */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_IAS, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
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
   int                        ret_val;
   FMPM_Event_Callback_Info_t EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry into the Event Callback Entry    */
            /* list.                                                    */
            BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(FMPM_Event_Callback_Info_t));

            EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
            EventCallbackEntry.ClientID          = MSG_GetServerAddressID();
            EventCallbackEntry.EventCallback     = CallbackFunction;
            EventCallbackEntry.CallbackParameter = CallbackParameter;

            if(AddEventCallbackInfoEntry(&TargetEventCallbackInfoList, &EventCallbackEntry))
               ret_val = EventCallbackEntry.EventCallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
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
   FMPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(TargetCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the person who */
            /* is doing the un-registering.                             */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&TargetEventCallbackInfoList, TargetCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&TargetEventCallbackInfoList, TargetCallbackID)) != NULL)
                  {
                     /* Free the memory because we are finished with it.*/
                     FreeEventCallbackInfoEntryMemory(EventCallbackPtr);
                  }
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

