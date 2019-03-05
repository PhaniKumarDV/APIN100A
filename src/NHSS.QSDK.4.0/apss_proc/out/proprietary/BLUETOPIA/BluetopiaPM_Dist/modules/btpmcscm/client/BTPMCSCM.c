/*****< btpmcscm.c >***********************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMCSCM - CSC Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/07/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCSCM.h"            /* BTPM CSC Manager Prototypes/Constants.    */
#include "CSCMAPI.h"             /* CSC Manager Prototypes/Constants.         */
#include "CSCMMSG.h"             /* BTPM CSC Manager Message Formats.         */
#include "CSCMGR.h"              /* CSC Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track callback information related to  */
   /* this module.                                                      */
typedef struct _tagCallback_Entry_t
{
   unsigned int CallbackID;
   CSCM_Event_Callback_t EventCallback;
   void *CallbackParameter;
   struct _tagCallback_Entry_t *NextCallbackEntryPtr;
} Callback_Entry_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   CSCM_Event_Callback_t  EventCallback;
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
static Mutex_t CSCManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Varaible which holds the Callback ID returned from the PM Server. */
static unsigned int ServerCollectorCallbackID;

   /* Variable which holds a pointer to the first element in the        */
   /* Callback Entry List for this module.                              */
static Callback_Entry_t *CallbackEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **ListHead, Callback_Entry_t *EntryToAdd);
static Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID);
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);
static void FreeCallbackEntryList(Callback_Entry_t **ListHead);

static void DispatchCSCMEvent(CSCM_Event_Data_t *EventData);

static void ProcessConnectedMessage(CSCM_Connected_Message_t *Message);
static void ProcessDisconnectedMessage(CSCM_Disconnected_Message_t *Message);
static void ProcessConfigurationStatusChangedMessage(CSCM_Configuration_Status_Changed_Message_t *Message);
static void ProcessMeasurementMessage(CSCM_Measurement_Message_t *Message);
static void ProcessSensorLocationResponseMessage(CSCM_Sensor_Location_Response_Message_t *Message);
static void ProcessCumulativeValueUpdatedMessage(CSCM_Cumulative_Value_Updated_Message_t *Message);
static void ProcessSensorLocationUpdatedMessage(CSCM_Sensor_Location_Updated_Message_t *Message);
static void ProcessProcedureCompleteMessage(CSCM_Procedure_Complete_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_CSCM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI CSCManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the CSC Entry Information List.                              */
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
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **ListHead, Callback_Entry_t *EntryToAdd)
{
   Callback_Entry_t *AddedEntry = NULL;
   Callback_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Callback_Entry_t *)BTPS_AllocateMemory(sizeof(Callback_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextCallbackEntryPtr = NULL;

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
                     FreeCallbackEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextCallbackEntryPtr)
                        tmpEntry = tmpEntry->NextCallbackEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextCallbackEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified CSC Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the CSC Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeCallbackEntryMemory().                   */
static Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID)
{
   Callback_Entry_t *FoundEntry = NULL;
   Callback_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextCallbackEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextCallbackEntryPtr = FoundEntry->NextCallbackEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextCallbackEntryPtr;

         FoundEntry->NextCallbackEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified CSC Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified CSC Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeCallbackEntryList(Callback_Entry_t **ListHead)
{
   Callback_Entry_t *EntryToFree;
   Callback_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextCallbackEntryPtr;

         FreeCallbackEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified CSC event to every registered HID Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the CSC Manager Mutex*/
   /*          held.  Upon exit from this function it will free the CSC */
   /*          Manager Mutex.                                           */
static void DispatchCSCMEvent(CSCM_Event_Data_t *EventData)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((CallbackEntryList) && (EventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      CallbackEntry    = CallbackEntryList;
      while(CallbackEntry)
      {
         if(CallbackEntry->EventCallback)
            NumberCallbacks++;

         CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
      }

      /* Only continue if we have some callbacks to dispatch to.        */
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

            CallbackEntry    = CallbackEntryList;
            while(CallbackEntry)
            {
               if(CallbackEntry->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackEntry->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackEntry->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(CSCManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(EventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(CSCManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(CSCManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(CSCManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Connected*/
   /* asynchronous message.                                             */
static void ProcessConnectedMessage(CSCM_Connected_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                                    = cetCSCConnected;
      EventData.EventLength                                                  = CSCM_CONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.ConnectedEventData.RemoteDeviceAddress             = Message->RemoteDeviceAddress;
      EventData.EventData.ConnectedEventData.Configured                      = Message->Configured;
      EventData.EventData.ConnectedEventData.SupportedOptionalCharateristics = Message->SupportedOptionalCharateristics;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Disconnected asynchronous message.                                */
static void ProcessDisconnectedMessage(CSCM_Disconnected_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                           = cetCSCDisconnected;
      EventData.EventLength                                         = CSCM_DISCONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Configuration State Changed asynchronous message.                 */
static void ProcessConfigurationStatusChangedMessage(CSCM_Configuration_Status_Changed_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                                         = cetCSCConfigurationStatusChanged;
      EventData.EventLength                                                       = CSCM_CONFIGURATION_STATUS_CHANGED_EVENT_DATA_SIZE;

      EventData.EventData.ConfigurationStatusChangedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.ConfigurationStatusChangedEventData.Status              = Message->Status;
      EventData.EventData.ConfigurationStatusChangedEventData.Configured          = Message->Configured;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Measurement asynchronous message.                                 */
static void ProcessMeasurementMessage(CSCM_Measurement_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                          = cetCSCMeasurement;
      EventData.EventLength                                        = CSCM_MEASUREMENT_EVENT_DATA_SIZE;

      EventData.EventData.MeasurementEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.MeasurementEventData.Flags               = Message->Flags;
      EventData.EventData.MeasurementEventData.WheelRevolutionData = Message->WheelRevolutionData;
      EventData.EventData.MeasurementEventData.CrankRevolutionData = Message->CrankRevolutionData;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Sensor   */
   /* Location Response asynchronous message.                           */
static void ProcessSensorLocationResponseMessage(CSCM_Sensor_Location_Response_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                                     = cetCSCSensorLocationResponse;
      EventData.EventLength                                                   = CSCM_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE;

      EventData.EventData.SensorLocationResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.SensorLocationResponseEventData.Status              = Message->Status;
      EventData.EventData.SensorLocationResponseEventData.SensorLocation      = Message->SensorLocation;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Cumulative Value Updated asynchronous message.                    */
static void ProcessCumulativeValueUpdatedMessage(CSCM_Cumulative_Value_Updated_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                                     = cetCSCCumulativeValueUpdated;
      EventData.EventLength                                                   = CSCM_CUMULATIVE_VALUE_UPDATED_EVENT_DATA_SIZE;

      EventData.EventData.CumulativeValueUpdatedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.CumulativeValueUpdatedEventData.CumulativeValue     = Message->CumulativeValue;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Sensor   */
   /* Location Updated asynchronous message.                            */
static void ProcessSensorLocationUpdatedMessage(CSCM_Sensor_Location_Updated_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                                    = cetCSCSensorLocationUpdated;
      EventData.EventLength                                                  = CSCM_SENSOR_LOCATION_UPDATED_EVENT_DATA_SIZE;

      EventData.EventData.SensorLocationUpdatedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.SensorLocationUpdatedEventData.SensorLocation      = Message->SensorLocation;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Procedure*/
   /* Complete asynchronous message.                                    */
static void ProcessProcedureCompleteMessage(CSCM_Procedure_Complete_Message_t *Message)
{
   CSCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      EventData.EventType                                                = cetCSCProcedureComplete;
      EventData.EventLength                                              = CSCM_PROCEDURE_COMPLETE_EVENT_DATA_SIZE;

      EventData.EventData.ProcedureCompleteEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.ProcedureCompleteEventData.ProcedureID         = Message->ProcedureID;
      EventData.EventData.ProcedureCompleteEventData.Status              = Message->Status;
      EventData.EventData.ProcedureCompleteEventData.ResponseErrorCode   = Message->ResponseErrorCode;

      DispatchCSCMEvent(&EventData);
   }
   else
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the CSC Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case CSCM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessConnectedMessage((CSCM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case CSCM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisconnectedMessage((CSCM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case CSCM_MESSAGE_FUNCTION_CONFIGURATION_STATUS_CHANGED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Configuration Status Changed\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_CONFIGURATION_STATUS_CHANGED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessConfigurationStatusChangedMessage((CSCM_Configuration_Status_Changed_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case CSCM_MESSAGE_FUNCTION_MEASUREMENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Measurement\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_MEASUREMENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessMeasurementMessage((CSCM_Measurement_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case CSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Sensor Location Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessSensorLocationResponseMessage((CSCM_Sensor_Location_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case CSCM_MESSAGE_FUNCTION_CUMULATIVE_VALUE_UPDATED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Cumulative Value Updated\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_CUMULATIVE_VALUE_UPDATED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessCumulativeValueUpdatedMessage((CSCM_Cumulative_Value_Updated_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case CSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_UPDATED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Sensor Location Updated\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_SENSOR_LOCATION_UPDATED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessSensorLocationUpdatedMessage((CSCM_Sensor_Location_Updated_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case CSCM_MESSAGE_FUNCTION_PROCEDURE_COMPLETE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Procedure Complete\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CSCM_PROCEDURE_COMPLETE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessProcedureCompleteMessage((CSCM_Procedure_Complete_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(CSCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process CSC Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_CSCM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the CSC state information.    */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the CSC state information.    */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the Event Callback List.                            */
            FreeCallbackEntryList(&CallbackEntryList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(CSCManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all CSC Manager Messages.   */
static void BTPSAPI CSCManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("CSC Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a CSC Manager defined    */
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
               /* CSC Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_CSCM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue CSC Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue CSC Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an CSC Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Non CSC Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager CSC Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI CSCM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing CSC Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((CSCManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process CSC Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER, CSCManagerGroupHandler, NULL))
            {
               /* Initialize the actual CSC Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the CSC Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _CSCM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique starting Callback ID.          */
                  NextCallbackID      = 0x000000001;

                  /* Initialize the server callback ID.                 */
                  ServerCollectorCallbackID = 0;

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
            if(CSCManagerMutex)
               BTPS_CloseMutex(CSCManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER);

            /* Flag that none of the resources are allocated.           */
            CSCManagerMutex = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("CSC Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we inform the CSC Manager Implementation that  */
            /* we are shutting down.                                    */
            _CSCM_Cleanup();

            BTPS_CloseMutex(CSCManagerMutex);

            /* Make sure that the Callback List is empty.               */
            FreeCallbackEntryList(&CallbackEntryList);

            /* Flag that the resources are no longer allocated.         */
            CSCManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI CSCM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the CSC Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(CSCManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Collector callback function with the CSC    */
   /* Manager Service.  This Callback will be dispatched by the CSC     */
   /* Manager when various CSC Manager Collector Events occur.  This    */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a CSC Manager Collector Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          CSCM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI CSCM_Register_Collector_Event_Callback(CSCM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   Callback_Entry_t  CallbackEntry;
   Callback_Entry_t *CallbackEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(CallbackFunction)
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Format the new Callback Entry.                           */
            BTPS_MemInitialize(&CallbackEntry, 0, sizeof(CallbackEntry));

            CallbackEntry.CallbackID        = GetNextCallbackID();
            CallbackEntry.EventCallback     = CallbackFunction;
            CallbackEntry.CallbackParameter = CallbackParameter;

            /* Attempt to add the entry to the list.                    */
            if((CallbackEntryPtr = AddCallbackEntry(&CallbackEntryList, &CallbackEntry)) != NULL)
            {
               /* Check if we need to register with the server.         */
               if(!ServerCollectorCallbackID)
               {
                  if((ret_val = _CSCM_Register_Collector_Event_Callback()) > 0)
                  {
                     /* Note the server's callback ID.                  */
                     ServerCollectorCallbackID = (unsigned int)ret_val;
                     ret_val                   = CallbackEntryPtr->CallbackID;
                  }
                  else
                  {
                     /* Registration failed, so clear out the list      */
                     /* entry.                                          */
                     if((CallbackEntryPtr = DeleteCallbackEntry(&CallbackEntryList, CallbackEntryPtr->CallbackID)) != NULL)
                        FreeCallbackEntryMemory(CallbackEntryPtr);
                  }
               }
               else
                  ret_val = CallbackEntryPtr->CallbackID;
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered CSC Manager Collector         */
   /* Event Callback (registered via a successful call to the           */
   /* CSCM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the Collector Event Callback ID (return value    */
   /* from CSCM_Register_Collector_Event_Callback() function).          */
void BTPSAPI CSCM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(CollectorCallbackID)
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the entry from list.                              */
            if((CallbackEntry = DeleteCallbackEntry(&CallbackEntryList, CollectorCallbackID)) != NULL)
            {
               /* Check if we need to remove our registration with the  */
               /* server.                                               */
               if(!CallbackEntryList)
               {
                  _CSCM_Un_Register_Collector_Event_Callback(ServerCollectorCallbackID);
                  ServerCollectorCallbackID = 0;
               }

               FreeCallbackEntryMemory(CallbackEntry);
            }

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of entries that the buffer will      */
   /* support (i.e. can be copied into the buffer).  The next parameter */
   /* is optional and, if specified, will be populated with the total   */
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
int BTPSAPI CSCM_Query_Connected_Sensors(unsigned int MaximumRemoteDeviceListEntries, CSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(((MaximumRemoteDeviceListEntries) && (ConnectedDeviceList)) || (TotalNumberConnectedDevices))
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the call to the server.                    */
            ret_val = _CSCM_Query_Connected_Sensors(MaximumRemoteDeviceListEntries, ConnectedDeviceList, TotalNumberConnectedDevices);

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function will attempt to configure a remote device  */
   /* which supports the CSC sensor role. It will register measurement  */
   /* and control point notifications and set the device up for         */
   /* usage. The RemoteDeviceAddress is the address of the remote       */
   /* sensor. This function returns zero if successful and a negative   */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return from this call does not mean the     */
   /*          device has been configured. An                           */
   /*          etCSCConfigurationStatusChanged event will indicate the  */
   /*          status of the attempt to configure.                      */
int BTPSAPI CSCM_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the call to the server.                    */
            ret_val = _CSCM_Configure_Remote_Sensor(RemoteDeviceAddress, Flags);

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function will un-configure a remote sensor which was*/
   /* previously configured. All notifications will be disabled. The    */
   /* RemoteDeviceAddress is the address of the remote sensor. This     */
   /* function returns zero if success and a negative return code if    */
   /* there was an error.                                               */
int BTPSAPI CSCM_Un_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the call to the server.                    */
            ret_val = _CSCM_Un_Configure_Remote_Sensor(RemoteDeviceAddress);

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function queries information about a known remote   */
   /* sensor. The RemoteDeviceAddress parameter is the address of       */
   /* the remote sensor. The DeviceInfo parameter is a pointer to       */
   /* the Connected Sensor structure in which the data should be        */
   /* populated. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
int BTPSAPI CSCM_Get_Connected_Sensor_Info(BD_ADDR_t RemoteDeviceAddress, CSCM_Connected_Sensor_t *DeviceInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (DeviceInfo))
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the call to the server.                    */
            ret_val = _CSCM_Get_Connected_Sensor_Info(RemoteDeviceAddress, DeviceInfo);

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function queries the current sensor location from a */
   /* remote sensor. The RemoteDeviceAddress parameter is the address of*/
   /* the remote sensor. This function returns zero if successful or a  */
   /* negative error code if there was an error.                        */
   /* * NOTE * If this function is succesful, the status of the request */
   /*          will be returned in a cetCSCSensorLocationResponse event.*/
int BTPSAPI CSCM_Get_Sensor_Location(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the call to the server.                    */
            ret_val = _CSCM_Get_Sensor_Location(RemoteDeviceAddress);

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function sends a control point opcode to update     */
   /* the cumulative value on a remote sensor. The RemoteDeviceAddress  */
   /* parameter is the address of the remote sensor. The CumulativeValue*/
   /* parameter is the value to set on the remote sensor. If successful,*/
   /* this function returns a postive integer which represents the      */
   /* Procedure ID associated with this procedure. If there is an error,*/
   /* this function returns a negative error code.                      */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetCSCCumulativeValueUpdated event will notify all       */
   /*          registered callbacks that the value has changed.         */
int BTPSAPI CSCM_Update_Cumulative_Value(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the call to the server.                    */
            ret_val = _CSCM_Update_Cumulative_Value(RemoteDeviceAddress, CumulativeValue);

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function sends a control point opcode to update     */
   /* the sensor location on a remote sensor. The RemoteDeviceAddress   */
   /* parameter is the address of the remote sensor. The SensorLocation */
   /* parameter is the new location to set.  If successful, this        */
   /* function returns a postive integer which represents the Procedure */
   /* ID associated with this procedure. If there is an error, this     */
   /* function returns a negative error code.                           */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * Only one procedure can be outstanding at a time. If this */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetCSCSensorLocationUpdated event will notify all        */
   /*          registered callbacks that the location has changed.      */
int BTPSAPI CSCM_Update_Sensor_Location(BD_ADDR_t RemoteDeviceAddress, CSCM_Sensor_Location_t SensorLocation)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the module is initialized.                              */
   if(Initialized)
   {
      /* Make sure the parameters seem valid.                           */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Acquire the lock which guards access to this module.        */
         if(BTPS_WaitMutex(CSCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Simply submit the call to the server.                    */
            ret_val = _CSCM_Update_Sensor_Location(RemoteDeviceAddress, SensorLocation);

            BTPS_ReleaseMutex(CSCManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_CSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}
