/*****< btpmtdsm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMTDSM - 3D Sync Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMTDSM.h"            /* BTPM TDS Manager Prototypes/Constants.    */
#include "TDSMAPI.h"             /* TDS Manager Prototypes/Constants.         */
#include "TDSMMSG.h"             /* BTPM TDS Manager Message Formats.         */
#include "TDSMGR.h"              /* TDS Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagTDSM_Callback_Info_t
{
   unsigned int                     CallbackID;
   unsigned int                     ClientID;
   TDSM_Event_Callback_t            EventCallback;
   void                            *CallbackParameter;
   struct _tagTDSM_Callback_Info_t *NextCallbackInfoPtr;
} TDSM_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   TDSM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the 3D Sync*/
   /* callback info list.                                               */
static TDSM_Callback_Info_t *CallbackInfoList;
static TDSM_Callback_Info_t *ControlCallbackInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static TDSM_Callback_Info_t *AddCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, TDSM_Callback_Info_t *EntryToAdd);
static TDSM_Callback_Info_t *SearchCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID);
static TDSM_Callback_Info_t *DeleteCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID);
static void FreeCallbackInfoEntryMemory(TDSM_Callback_Info_t *EntryToFree);
static void FreeCallbackInfoList(TDSM_Callback_Info_t **ListHead);

static void DispatchTDSEvent(Boolean_t Control, TDSM_Event_Data_t *TDSMEventData, BTPM_Message_t *Message);

static int ProcessWriteSynchronizationTrainParameters(unsigned int ClientID, unsigned int CallbackID, TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult);
static int ProcessStartSynchronizationTrain(unsigned int ClientID, unsigned int CallbackID);
static int ProcessEnableConnectionlessSlaveBroadcast(unsigned int ClientID, unsigned int CallbackID, TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult);
static int ProcessDisableConnectionlessSlaveBroadcast(unsigned int ClientID, unsigned int CallbackID);
static int ProcessGetCurrentBroadcastInformation(unsigned int ClientID, TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation);
static int ProcessUpdateBroadcastInformation(unsigned int ClientID, unsigned int CallbackID, TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate);
static int ProcessRegisterEventCallback(unsigned int ClientID, Boolean_t Control, TDSM_Event_Callback_t CallbackFunction, void *CallbackParameter);
static int ProcessUnRegisterEventCallback(unsigned int ClientID, unsigned int CallbackID);

static void ProcessWriteSynchronizationTrainParametersMessage(TDSM_Write_Synchronization_Train_Parameters_Request_t *Message);
static void ProcessStartSynchronizationTrainMessage(TDSM_Start_Synchronization_Train_Request_t *Message);
static void ProcessEnableConnectionlessSlaveBroadcastMessage(TDSM_Enable_Connectionless_Slave_Broadcast_Request_t *Message);
static void ProcessDisableConnectionlessSlaveBroadcastMessage(TDSM_Disable_Connectionless_Slave_Broadcast_Request_t *Message);
static void ProcessGetCurrentBroadcastInformationMessage(TDSM_Get_Current_Broadcast_Information_Request_t *Message);
static void ProcessUpdateBroadcastInformationMessage(TDSM_Update_Broadcast_Information_Request_t *Message);
static void ProcessRegisterEventsMessage(TDSM_Register_Events_Request_t *Message);
static void ProcessUnRegisterEventsMessage(TDSM_Un_Register_Events_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessDisplayConnectionAnnouncementEvent(TDS_Display_Connection_Announcement_Data_t *DisplayConnectionAnnouncementData);
static void ProcessDisplaySynchronizationTrainCompleteEvent(TDS_Display_Synchronization_Train_Complete_Data_t *DisplaySynchronizationTrainCompleteData);
static void ProcessDisplayCSBSupervisionTimeoutEvent(void);
static void ProcessDisplayChannelMapChangeEvent(TDS_Display_Channel_Map_Change_Data_t *DisplayChannelMapChangeData);
static void ProcessDisplaySlavePageResponseTimeoutEvent(void);

static void ProcessTDSEvent(TDSM_TDS_Event_Data_t *TDSEventData);

static void BTPSAPI BTPMDispatchCallback_TDSM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_TDS(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI TDSManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique callback ID that can be used to add an entry    */
   /* into the 3D Sync entry information list.                          */
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
   /*            CallbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static TDSM_Callback_Info_t *AddCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, TDSM_Callback_Info_t *EntryToAdd)
{
   TDSM_Callback_Info_t *ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = (TDSM_Callback_Info_t *)BSC_AddGenericListEntry(sizeof(TDSM_Callback_Info_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(TDSM_Callback_Info_t, CallbackID), sizeof(TDSM_Callback_Info_t), BTPS_STRUCTURE_OFFSET(TDSM_Callback_Info_t, NextCallbackInfoPtr), (void **)ListHead, (void *)EntryToAdd);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function searches the specified List for the        */
   /* specified callback ID.  This function returns NULL if either the  */
   /* list head is invalid, the callback ID is invalid, or the specified*/
   /* callback ID was NOT found.                                        */
static TDSM_Callback_Info_t *SearchCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID)
{
   TDSM_Callback_Info_t *ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = (TDSM_Callback_Info_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(TDSM_Callback_Info_t, CallbackID), BTPS_STRUCTURE_OFFSET(TDSM_Callback_Info_t, NextCallbackInfoPtr), (void **)ListHead);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function searches the specified 3D Sync entry       */
   /* information list for the specified callback ID and removes it from*/
   /* the List.  This function returns NULL if either the 3D Sync entry */
   /* information list head is invalid, the callback ID is invalid, or  */
   /* the specified callback ID was NOT present in the list.  The entry */
   /* returned will have the next entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeCallbackInfoEntryMemory().                   */
static TDSM_Callback_Info_t *DeleteCallbackInfoEntry(TDSM_Callback_Info_t **ListHead, unsigned int CallbackID)
{
   TDSM_Callback_Info_t *ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = (TDSM_Callback_Info_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(TDSM_Callback_Info_t, CallbackID), BTPS_STRUCTURE_OFFSET(TDSM_Callback_Info_t, NextCallbackInfoPtr), (void **)ListHead);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* This function frees the specified 3D Sync entry information       */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeCallbackInfoEntryMemory(TDSM_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   BSC_FreeGenericListEntryMemory((void *)EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified 3D Sync entry information list.  Upon    */
   /* return of this function, the head pointer is set to NULL.         */
static void FreeCallbackInfoList(TDSM_Callback_Info_t **ListHead)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   BSC_FreeGenericListEntryList((void **)ListHead, BTPS_STRUCTURE_OFFSET(TDSM_Callback_Info_t, NextCallbackInfoPtr));

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified 3D Sync event to every registered 3D Sync  */
   /* Event Callback.                                                   */
   /* * NOTE * This function should be called with the 3D Sync Manager  */
   /*          Lock held.  Upon exit from this function it will free the*/
   /*          3D Sync Manager Lock.                                    */
static void DispatchTDSEvent(Boolean_t Control, TDSM_Event_Data_t *TDSMEventData, BTPM_Message_t *Message)
{
   unsigned int          Index;
   unsigned int          Index1;
   unsigned int          ServerID;
   unsigned int          NumberCallbacks;
   Callback_Info_t       CallbackInfoArray[16];
   Callback_Info_t      *CallbackInfoArrayPtr;
   TDSM_Callback_Info_t *TDSMCallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((ControlCallbackInfoList) || ((!Control) && CallbackInfoList)) && (TDSMEventData) && (Message))
   {
      /* Determine how many callbacks are registered.                   */
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, count the non-control handlers.                         */
      if(!Control)
      {
         TDSMCallbackInfo = CallbackInfoList;

         while(TDSMCallbackInfo)
         {
            if((TDSMCallbackInfo->EventCallback) || (TDSMCallbackInfo->ClientID != ServerID))
               NumberCallbacks++;

            TDSMCallbackInfo = TDSMCallbackInfo->NextCallbackInfoPtr;
         }
      }

      /* Now, count the control handlers.                               */
      TDSMCallbackInfo = ControlCallbackInfoList;

      while(TDSMCallbackInfo)
      {
         if((TDSMCallbackInfo->EventCallback) || (TDSMCallbackInfo->ClientID != ServerID))
            NumberCallbacks++;

         TDSMCallbackInfo = TDSMCallbackInfo->NextCallbackInfoPtr;
      }

      /* Copy the callback information.                                 */
      if(NumberCallbacks)
      {
         if(NumberCallbacks <= (sizeof(CallbackInfoArray)/sizeof(Callback_Info_t)))
            CallbackInfoArrayPtr = CallbackInfoArray;
         else
            CallbackInfoArrayPtr = BTPS_AllocateMemory((NumberCallbacks*sizeof(Callback_Info_t)));

         if(CallbackInfoArrayPtr)
         {
            NumberCallbacks = 0;

            if(!Control)
            {
               TDSMCallbackInfo = CallbackInfoList;

               while(TDSMCallbackInfo)
               {
                  if((TDSMCallbackInfo->EventCallback) || (TDSMCallbackInfo->ClientID != ServerID))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].ClientID          = TDSMCallbackInfo->ClientID;
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = TDSMCallbackInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = TDSMCallbackInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  TDSMCallbackInfo = TDSMCallbackInfo->NextCallbackInfoPtr;
               }
            }

            TDSMCallbackInfo = ControlCallbackInfoList;

            while(TDSMCallbackInfo)
            {
               if((TDSMCallbackInfo->EventCallback) || (TDSMCallbackInfo->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = TDSMCallbackInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = TDSMCallbackInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = TDSMCallbackInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               TDSMCallbackInfo = TDSMCallbackInfo->NextCallbackInfoPtr;
            }

            /* Release the Lock because we have already built the       */
            /* callback array.                                          */
            DEVM_ReleaseLock();

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* * NOTE * It is possible that we have already          */
               /*          dispatched the event to the client (case     */
               /*          would occur if a single client has registered*/
               /*          for 3D Sync events and Data Events.  To avoid*/
               /*          this case we need to walk the list of        */
               /*          previously dispatched events to check to see */
               /*          if it has already been dispatched (we need to*/
               /*          do this with Client Address ID's for messages*/
               /*          - Event Callbacks are local and therefore    */
               /*          unique so we don't have to do this filtering.*/

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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(TDSMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Walk the proceding list and see if we have already */
                  /* dispatched this event to this client.              */
                  for(Index1=0;Index1<Index;Index1++)
                  {
                     if((CallbackInfoArrayPtr[Index1].ClientID != ServerID) && (CallbackInfoArrayPtr[Index1].ClientID == CallbackInfoArrayPtr[Index].ClientID))
                        break;
                  }

                  if(Index1 == Index)
                  {
                     /* Dispatch the Message.                           */
                     Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

                     MSG_SendMessage(Message);
                  }
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

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Helper function to process Write Synchronization Train Parameters */
   /* IPC messages and API calls.                                       */
static int ProcessWriteSynchronizationTrainParameters(unsigned int ClientID, unsigned int CallbackID, TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure we area currently powered up.                           */
   if(CurrentPowerState)
   {
      /* Make sure the CallbackID is registered.                           */
      if(((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoList, CallbackID)) != NULL) && (ClientID == CallbackInfo->ClientID))
      {
         ret_val = _TDSM_Write_Synchronization_Train_Parameters(SyncTrainParams, IntervalResult);
      }
      else
         ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Helper function to process Start Synchronization Train IPC        */
   /* messages and API calls.                                           */
static int ProcessStartSynchronizationTrain(unsigned int ClientID, unsigned int CallbackID)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure we area currently powered up.                           */
   if(CurrentPowerState)
   {
      /* Make sure the CallbackID is registered.                           */
      if(((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoList, CallbackID)) != NULL) && (ClientID == CallbackInfo->ClientID))
      {
         ret_val = _TDSM_Start_Synchronization_Train();
      }
      else
         ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Helper function to process Enable Connectionless Slave Broadcast  */
   /* IPC messages and API calls.                                       */
static int ProcessEnableConnectionlessSlaveBroadcast(unsigned int ClientID, unsigned int CallbackID, TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure we area currently powered up.                           */
   if(CurrentPowerState)
   {
      /* Make sure the CallbackID is registered.                           */
      if(((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoList, CallbackID)) != NULL) && (ClientID == CallbackInfo->ClientID))
      {
         ret_val = _TDSM_Enable_Connectionless_Slave_Broadcast(ConnectionlessSlaveBroadcastParams, IntervalResult);
      }
      else
         ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Helper function to process Disable Connectionless Slave Broadcast */
   /* IPC messages and API calls.                                       */
static int ProcessDisableConnectionlessSlaveBroadcast(unsigned int ClientID, unsigned int CallbackID)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure we area currently powered up.                           */
   if(CurrentPowerState)
   {
      /* Make sure the CallbackID is registered.                           */
      if(((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoList, CallbackID)) != NULL) && (ClientID == CallbackInfo->ClientID))
      {
         ret_val = _TDSM_Disable_Connectionless_Slave_Broadcast();
      }
      else
         ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Helper function to process Get Current Broadcast Information IPC  */
   /* messages and API calls.                                           */
static int ProcessGetCurrentBroadcastInformation(unsigned int ClientID, TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure we area currently powered up.                           */
   if(CurrentPowerState)
   {
      ret_val = _TDSM_Get_Current_Broadcast_Information(CurrentBroadcastInformation);
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Helper function to process Update Broadcast Information IPC       */
   /* messages and API calls.                                           */
static int ProcessUpdateBroadcastInformation(unsigned int ClientID, unsigned int CallbackID, TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure we area currently powered up.                           */
   if(CurrentPowerState)
   {
      /* Make sure the CallbackID is registered.                           */
      if(((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoList, CallbackID)) != NULL) && (ClientID == CallbackInfo->ClientID))
      {
         ret_val = _TDSM_Update_Broadcast_Information(BroadcastInformationUpdate);
      }
      else
         ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Helper function to process Register Event Callback IPC messages   */
   /* and API calls.                                                    */
static int ProcessRegisterEventCallback(unsigned int ClientID, Boolean_t Control, TDSM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                   ret_val;
   TDSM_Callback_Info_t  CallbackInfo;
   TDSM_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((!Control) || (!ControlCallbackInfoList))
   {
      BTPS_MemInitialize(&CallbackInfo, 0, sizeof(CallbackInfo));

      CallbackInfo.CallbackID        = GetNextCallbackID();
      CallbackInfo.ClientID          = ClientID;
      CallbackInfo.EventCallback     = CallbackFunction;
      CallbackInfo.CallbackParameter = CallbackParameter;

      if((CallbackInfoPtr = AddCallbackInfoEntry(Control?&ControlCallbackInfoList:&CallbackInfoList, &CallbackInfo)) != NULL)
         ret_val = CallbackInfoPtr->CallbackID;
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_CONTROL_ALREADY_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Helper function to process Un Register Event Callback IPC messages*/
   /* and API calls.                                                    */
static int ProcessUnRegisterEventCallback(unsigned int ClientID, unsigned int CallbackID)
{
   int                   ret_val;
   TDSM_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((CallbackInfo = SearchCallbackInfoEntry(&CallbackInfoList, CallbackID)) != NULL) || ((CallbackInfo = SearchCallbackInfoEntry(&ControlCallbackInfoList, CallbackID)) != NULL))
   {
      if(CallbackInfo->ClientID == ClientID)
      {
         if(((CallbackInfo = DeleteCallbackInfoEntry(&CallbackInfoList, CallbackID)) != NULL) || ((CallbackInfo = DeleteCallbackInfoEntry(&ControlCallbackInfoList, CallbackID)) != NULL))
            FreeCallbackInfoEntryMemory(CallbackInfo);

         ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function processes the specified Write              */
   /* Synchronization Train Parameters message and responds to the      */
   /* message accordingly.  This function does not verify the integrity */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessWriteSynchronizationTrainParametersMessage(TDSM_Write_Synchronization_Train_Parameters_Request_t *Message)
{
   TDSM_Write_Synchronization_Train_Parameters_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessWriteSynchronizationTrainParameters(Message->MessageHeader.AddressID, Message->TDSMControlCallbackID, &Message->SyncTrainParams, &ResponseMessage.IntervalResult);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Start              */
   /* Synchronization Train message and responds to the message         */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessStartSynchronizationTrainMessage(TDSM_Start_Synchronization_Train_Request_t *Message)
{
   TDSM_Start_Synchronization_Train_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_START_SYNCHRONIZATION_TRAIN_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessStartSynchronizationTrain(Message->MessageHeader.AddressID, Message->TDSMControlCallbackID);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Enable             */
   /* Connectionless Slave Broadcast message and responds to the        */
   /* message accordingly.  This function does not verify the integrity */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableConnectionlessSlaveBroadcastMessage(TDSM_Enable_Connectionless_Slave_Broadcast_Request_t *Message)
{
   TDSM_Enable_Connectionless_Slave_Broadcast_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessEnableConnectionlessSlaveBroadcast(Message->MessageHeader.AddressID, Message->TDSMControlCallbackID, &Message->ConnectionlessSlaveBroadcastParams, &ResponseMessage.IntervalResult);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Disable            */
   /* Connectionless Slave Broadcast message and responds to the        */
   /* message accordingly.  This function does not verify the integrity */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessDisableConnectionlessSlaveBroadcastMessage(TDSM_Disable_Connectionless_Slave_Broadcast_Request_t *Message)
{
   TDSM_Disable_Connectionless_Slave_Broadcast_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessDisableConnectionlessSlaveBroadcast(Message->MessageHeader.AddressID, Message->TDSMControlCallbackID);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Current        */
   /* Broadcast Information message and responds to the message         */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessGetCurrentBroadcastInformationMessage(TDSM_Get_Current_Broadcast_Information_Request_t *Message)
{
   TDSM_Get_Current_Broadcast_Information_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_GET_CURRENT_BROADCAST_INFORMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessGetCurrentBroadcastInformation(Message->MessageHeader.AddressID, &ResponseMessage.CurrentBroadcastInformation);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Update Broadcast   */
   /* Information message and responds to the message accordingly.  This*/
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessUpdateBroadcastInformationMessage(TDSM_Update_Broadcast_Information_Request_t *Message)
{
   TDSM_Update_Broadcast_Information_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_UPDATE_BROADCAST_INFORMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessUpdateBroadcastInformation(Message->MessageHeader.AddressID, Message->TDSMControlCallbackID, &Message->BroadcastInformationUpdate);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Events    */
   /* message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the message   */
   /* before calling this function.                                     */
static void ProcessRegisterEventsMessage(TDSM_Register_Events_Request_t *Message)
{
   TDSM_Register_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_REGISTER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessRegisterEventCallback(Message->MessageHeader.AddressID, Message->Control, NULL, NULL);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un Register Events */
   /* message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the message   */
   /* before calling this function.                                     */
static void ProcessUnRegisterEventsMessage(TDSM_Un_Register_Events_Request_t *Message)
{
   TDSM_Un_Register_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TDSM_UN_REGISTER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = ProcessUnRegisterEventCallback(Message->MessageHeader.AddressID, Message->TDSManagerEventCallbackID);

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the 3D Sync lock     */
   /*          held.  This function will release the lock before it     */
   /*          exits (i.e.  the caller SHOULD NOT RELEASE THE LOCK).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case TDSM_MESSAGE_FUNCTION_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Write Synchronization Train Parameters \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessWriteSynchronizationTrainParametersMessage((TDSM_Write_Synchronization_Train_Parameters_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_START_SYNCHRONIZATION_TRAIN:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Start Synchronization Train \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_START_SYNCHRONIZATION_TRAIN_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessStartSynchronizationTrainMessage((TDSM_Start_Synchronization_Train_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Connectionless Slave Broadcast \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessEnableConnectionlessSlaveBroadcastMessage((TDSM_Enable_Connectionless_Slave_Broadcast_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable Connectionless Slave Broadcast \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisableConnectionlessSlaveBroadcastMessage((TDSM_Disable_Connectionless_Slave_Broadcast_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_GET_CURRENT_BROADCAST_INFORMATION:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Current Broadcast Information \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_GET_CURRENT_BROADCAST_INFORMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessGetCurrentBroadcastInformationMessage((TDSM_Get_Current_Broadcast_Information_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_UPDATE_BROADCAST_INFORMATION:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Broadcast Information \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_UPDATE_BROADCAST_INFORMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessUpdateBroadcastInformationMessage((TDSM_Update_Broadcast_Information_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_REGISTER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Events \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_REGISTER_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessRegisterEventsMessage((TDSM_Register_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TDSM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Register Events \n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TDSM_UN_REGISTER_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessUnRegisterEventsMessage((TDSM_Un_Register_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   TDSM_Callback_Info_t  *CallbackInfo;
   TDSM_Callback_Info_t  *tmpCallbackInfo;
   TDSM_Callback_Info_t **CallbackListHead;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Start at the beginning of the non-control list if it is not    */
      /* empty.                                                         */
      CallbackListHead = (CallbackInfoList)?&CallbackInfoList:&ControlCallbackInfoList;
      CallbackInfo     = *CallbackListHead;

      /* Loop through the list checking for the client.                 */
      while(CallbackInfo)
      {
         if(CallbackInfo->ClientID == ClientID)
         {
            /* Note the next entry in the list since we are going to    */
            /* delete the entry.                                        */
            tmpCallbackInfo = CallbackInfo->NextCallbackInfoPtr;

            /* Simply delete the client's entry.                        */
            if((CallbackInfo = DeleteCallbackInfoEntry(CallbackListHead, CallbackInfo->CallbackID)) != NULL)
               FreeCallbackInfoEntryMemory(CallbackInfo);

            /* Move on to the next entry.                               */
            CallbackInfo = tmpCallbackInfo;
         }
         else
            CallbackInfo = CallbackInfo->NextCallbackInfoPtr;

         /* We need to check the control list if we are done with the   */
         /* non-control one.                                            */
         if((!CallbackInfo) && (CallbackListHead == &CallbackInfoList))
         {
            CallbackInfo     = ControlCallbackInfoList;
            CallbackListHead = &ControlCallbackInfoList;
         }
      }

   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* Display Connection Announcement event that has been received.     */
   /* This function should be called with the lock protecting the 3D    */
   /* Sync Manager information held.                                    */
static void ProcessDisplayConnectionAnnouncementEvent(TDS_Display_Connection_Announcement_Data_t *DisplayConnectionAnnouncementData)
{
   TDSM_Event_Data_t                              TDSMEventData;
   TDSM_Display_Connection_Announcement_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(DisplayConnectionAnnouncementData)
   {
      /* Format the event to dispatch.                                  */
      BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

      TDSMEventData.EventType                                                       = tetTDSM_Display_Connection_Announcement;
      TDSMEventData.EventDataSize                                                   = TDSM_DISPLAY_CONNECTION_ANNOUNCEMENT_DATA_SIZE;

      TDSMEventData.EventData.DisplayConnectionAnnouncementData.RemoteDeviceAddress = DisplayConnectionAnnouncementData->BD_ADDR;
      TDSMEventData.EventData.DisplayConnectionAnnouncementData.Flags               = DisplayConnectionAnnouncementData->Flags;
      TDSMEventData.EventData.DisplayConnectionAnnouncementData.BatteryLevel        = DisplayConnectionAnnouncementData->BatteryLevel;

      /* Next, format up the message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
      Message.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_DISPLAY_CONNECTION_ANNOUNCEMENT;
      Message.MessageHeader.MessageLength   = (TDSM_DISPLAY_CONNECTION_ANNOUNCEMENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.BD_ADDR                       = DisplayConnectionAnnouncementData->BD_ADDR;
      Message.Flags                         = DisplayConnectionAnnouncementData->Flags;
      Message.BatteryLevel                  = DisplayConnectionAnnouncementData->BatteryLevel;

      /* Dispatch the formatted event and message.                      */
      DispatchTDSEvent(FALSE, &TDSMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* Display Synchronization Train Complete event that has been        */
   /* received.  This function should be called with the lock protecting*/
   /* the 3D Sync Manager information held.                             */
static void ProcessDisplaySynchronizationTrainCompleteEvent(TDS_Display_Synchronization_Train_Complete_Data_t *DisplaySynchronizationTrainCompleteData)
{
   TDSM_Event_Data_t                                     TDSMEventData;
   TDSM_Display_Synchronization_Train_Complete_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(DisplaySynchronizationTrainCompleteData)
   {
      /* Format the event to dispatch.                                  */
      BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

      TDSMEventData.EventType                                                = tetTDSM_Display_Synchronization_Train_Complete;
      TDSMEventData.EventDataSize                                            = TDSM_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_DATA_SIZE;

      TDSMEventData.EventData.DisplaySynchronizationTrainCompleteData.Status = DisplaySynchronizationTrainCompleteData->Status;

      /* Next, format up the message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
      Message.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE;
      Message.MessageHeader.MessageLength   = (TDSM_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.Status                        = DisplaySynchronizationTrainCompleteData->Status;

      /* Dispatch the formatted event and message.                      */
      DispatchTDSEvent(FALSE, &TDSMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* Display CSB Supervision Timeout event that has been received.     */
   /* This function should be called with the lock protecting the 3D    */
   /* Sync Manager information held.                                    */
static void ProcessDisplayCSBSupervisionTimeoutEvent(void)
{
   TDSM_Event_Data_t                              TDSMEventData;
   TDSM_Display_CSB_Supervision_Timeout_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event to dispatch.                                     */
   BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

   TDSMEventData.EventType     = tetTDSM_Display_CSB_Supervision_Timeout;
   TDSMEventData.EventDataSize = 0;

   /* Next, format up the message to dispatch.                          */
   BTPS_MemInitialize(&Message, 0, sizeof(Message));

   Message.MessageHeader.AddressID       = 0;
   Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
   Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
   Message.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_DISPLAY_CSB_SUPERVISION_TIMEOUT;
   Message.MessageHeader.MessageLength   = (TDSM_DISPLAY_CSB_SUPERVISION_TIMEOUT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

   /* Dispatch the formatted event and message.                         */
   DispatchTDSEvent(FALSE, &TDSMEventData, (BTPM_Message_t *)&Message);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* Display Channel Map Change event that has been received.  This    */
   /* function should be called with the lock protecting the 3D Sync    */
   /* Manager information held.                                         */
static void ProcessDisplayChannelMapChangeEvent(TDS_Display_Channel_Map_Change_Data_t *DisplayChannelMapChangeData)
{
   TDSM_Event_Data_t                         TDSMEventData;
   TDSM_Display_Channel_Map_Change_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(DisplayChannelMapChangeData)
   {
      /* Format the event to dispatch.                                  */
      BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

      TDSMEventData.EventType                                        = tetTDSM_Display_Channel_Map_Change;
      TDSMEventData.EventDataSize                                    = TDSM_DISPLAY_CHANNEL_MAP_CHANGE_DATA_SIZE;

      TDSMEventData.EventData.DisplayChannelMapChangeData.ChannelMap = DisplayChannelMapChangeData->ChannelMap;

      /* Next, format up the message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
      Message.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_DISPLAY_CHANNEL_MAP_CHANGE;
      Message.MessageHeader.MessageLength   = (TDSM_DISPLAY_CHANNEL_MAP_CHANGE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ChannelMap                    = DisplayChannelMapChangeData->ChannelMap;

      /* Dispatch the formatted event and message.                      */
      DispatchTDSEvent(FALSE, &TDSMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* Display Slave Page Response Timeout event that has been received. */
   /* This function should be called with the lock protecting the 3D    */
   /* Sync Manager information held.                                    */
static void ProcessDisplaySlavePageResponseTimeoutEvent(void)
{
   TDSM_Event_Data_t                                  TDSMEventData;
   TDSM_Display_Slave_Page_Response_Timeout_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event to dispatch.                                     */
   BTPS_MemInitialize(&TDSMEventData, 0, sizeof(TDSMEventData));

   TDSMEventData.EventType     = tetTDSM_Display_Slave_Page_Response_Timeout;
   TDSMEventData.EventDataSize = 0;

   /* Next, format up the message to dispatch.                          */
   BTPS_MemInitialize(&Message, 0, sizeof(Message));

   Message.MessageHeader.AddressID       = 0;
   Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
   Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
   Message.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_DISPLAY_SLAVE_PAGE_RESPONSE_TIMEOUT;
   Message.MessageHeader.MessageLength   = (TDSM_DISPLAY_SLAVE_PAGE_RESPONSE_TIMEOUT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

   /* Dispatch the formatted event and message.                         */
   DispatchTDSEvent(FALSE, &TDSMEventData, (BTPM_Message_t *)&Message);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing 3D Sync Events that have been received.  This function */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessTDSEvent(TDSM_TDS_Event_Data_t *TDSEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(TDSEventData)
   {
      /* Process the event based on the event type.                     */
      switch(TDSEventData->EventType)
      {
         case etTDS_Display_Connection_Announcement:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display Connection Announcement\n"));

            ProcessDisplayConnectionAnnouncementEvent(&(TDSEventData->EventData.TDS_Display_Connection_Announcement_Data));
            break;
         case etTDS_Display_CSB_Supervision_Timeout:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display CSB Supervision Timeout\n"));

            ProcessDisplayCSBSupervisionTimeoutEvent();
            break;
         case etTDS_Display_Synchronization_Train_Complete:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display Synchronization Train Complete\n"));

            ProcessDisplaySynchronizationTrainCompleteEvent(&(TDSEventData->EventData.TDS_Display_Synchronization_Train_Complete_Data));
            break;
         case etTDS_Display_Channel_Map_Change:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display Channel Map Change\n"));

            ProcessDisplayChannelMapChangeEvent(&(TDSEventData->EventData.TDS_Display_Channel_Map_Change_Data));
            break;
         case etTDS_Display_Slave_Page_Response_Timeout:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Display Slave Page Response Timeout\n"));

            ProcessDisplaySlavePageResponseTimeoutEvent();
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown 3D Sync Event Type: %d\n", TDSEventData->EventType));
            break;
      }
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Invalid 3D Sync Event Data\n"));

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process 3D Sync IPC Message Events.                 */
static void BTPSAPI BTPMDispatchCallback_TDSM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process 3D Sync Manager Notification Events.        */
static void BTPSAPI BTPMDispatchCallback_TDS(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is a 3D Sync Event Update.        */
            if(((TDSM_Update_Data_t *)CallbackParameter)->UpdateType == utTDSEvent)
            {
               /* Process the Notification.                             */
               ProcessTDSEvent(&(((TDSM_Update_Data_t *)CallbackParameter)->UpdateData.TDSEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all 3D Sync Manager         */
   /* Messages.                                                         */
static void BTPSAPI TDSManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("3D Sync Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a 3D Sync Manager        */
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
               /* 3D Sync Manager Thread.                               */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_TDSM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue 3D Sync Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue 3D Sync Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an 3D Sync Manager       */
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
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Non 3D Sync Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager 3D Sync Manager module.  This      */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI TDSM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                         Result;
   TDSM_Initialization_Info_t *InitializationInfo;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      /* Check to see if this module has already been initialized.      */
      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing 3D Sync\n"));

         /* Make sure that there is actual Initialization data.         */
         if((InitializationInfo = (TDSM_Initialization_Info_t *)InitializationData) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process 3D Sync Manager messages.                     */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER, TDSManagerGroupHandler, NULL))
            {
               /* Initialize the actual 3D Sync Manager Implementation  */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the 3D Sync     */
               /* Manager functionality - this module is just the       */
               /* framework shell).                                     */
               if(!(Result = _TDSM_Initialize(InitializationInfo)))
               {
                  /* Finally determine the current Device Power State.  */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting 3D Sync Callback ID. */
                  NextCallbackID    = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized       = TRUE;
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

            /* If an error occurred then we need to free all resources  */
            /* that were allocated.                                     */
            if(Result)
            {
               _TDSM_Cleanup();

               MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER);
            }
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("3D Sync Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the 3D Sync Manager Implementation   */
            /* that we are shutting down.                               */
            _TDSM_Cleanup();

            /* Make sure the callback lists are empty.                  */
            FreeCallbackInfoList(&CallbackInfoList);
            FreeCallbackInfoList(&ControlCallbackInfoList);

            /* Flag that the resources are no longer allocated.         */
            CurrentPowerState     = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized           = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI TDSM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the 3D Sync Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the 3D Sync Manager that it should*/
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _TDSM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the 3D Sync Manager that the Stack has been    */
               /* closed.                                               */
               _TDSM_SetBluetoothStackID(0);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the 3D Sync Manager of a specific Update Event.  The    */
   /* 3D Sync Manager can then take the correct action to process the   */
   /* update.                                                           */
Boolean_t TDSM_NotifyUpdate(TDSM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utTDSEvent:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing 3D Sync Event: %d\n", UpdateData->UpdateData.TDSEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_TDS, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function will configure the Synchronization Train   */
   /* parameters on the Bluetooth controller.  The TDSMControlCallbackID*/
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The SyncTrainParams parameter is a pointer to the       */
   /* Synchronization Train parameters to be set.  The IntervalResult   */
   /* parameter is a pointer to the variable that will be populated with*/
   /* the Synchronization Interval chosen by the Bluetooth controller.  */
   /* This function will return zero on success; otherwise, a negative  */
   /* error value will be returned.                                     */
   /* * NOTE * The Timout value in the                                  */
   /*          TDS_Synchronization_Train_Parameters_t structure must be */
   /*          a value of at least 120 seconds, or the function call    */
   /*          will fail.                                               */
int BTPSAPI TDSM_Write_Synchronization_Train_Parameters(unsigned int TDSMControlCallbackID, TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if((TDSMControlCallbackID) && (SyncTrainParams) && (IntervalResult))
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ret_val = ProcessWriteSynchronizationTrainParameters(MSG_GetServerAddressID(), TDSMControlCallbackID, SyncTrainParams, IntervalResult);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function will enable the Synchronization Train. The */
   /* TDSMControlCallbackID parameter is an ID returned from a          */
   /* succesfull call to TDSM_Register_Event_Callback() with the Control*/
   /* parameter set to TRUE.  This function will return zero on success;*/
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The TDSM_Write_Synchronization_Train_Parameters function */
   /*          should be called at least once after initializing the    */
   /*          stack and before calling this function.                  */
   /* * NOTE * The tetTDSM_Display_Synchronization_Train_Complete event */
   /*          will be triggered when the Synchronization Train         */
   /*          completes.  This function can be called again at this    */
   /*          time to restart the Synchronization Train.               */
int BTPSAPI TDSM_Start_Synchronization_Train(unsigned int TDSMControlCallbackID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if(TDSMControlCallbackID)
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ret_val = ProcessStartSynchronizationTrain(MSG_GetServerAddressID(), TDSMControlCallbackID);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function will configure and enable                  */
   /* the Connectionless Slave Broadcast channel on the                 */
   /* Bluetooth controller.  The TDSMControlCallbackID                  */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The ConnectionlessSlaveBroadcastParams parameter is a   */
   /* pointer to the Connectionless Slave Broadcast parameters to be    */
   /* set.  The IntervalResult parameter is a pointer to the variable   */
   /* that will be populated with the Broadcast Interval chosen by the  */
   /* Bluetooth controller.  This function will return zero on success; */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The MinInterval value should be greater than or equal to */
   /*          50 milliseconds, and the MaxInterval value should be less*/
   /*          than or equal to 100 milliseconds; otherwise, the        */
   /*          function will fail.                                      */
int BTPSAPI TDSM_Enable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID, TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if((TDSMControlCallbackID) && (ConnectionlessSlaveBroadcastParams) && (IntervalResult))
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ret_val = ProcessEnableConnectionlessSlaveBroadcast(MSG_GetServerAddressID(), TDSMControlCallbackID, ConnectionlessSlaveBroadcastParams, IntervalResult);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.  The TDSMControlCallbackID*/
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.                                                             */
   /* * NOTE * Calling this function will terminate the Synchronization */
   /*          Train (if it is currently enabled).                      */
int BTPSAPI TDSM_Disable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if(TDSMControlCallbackID)
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ret_val = ProcessDisableConnectionlessSlaveBroadcast(MSG_GetServerAddressID(), TDSMControlCallbackID);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is used to get the current information     */
   /* being used int the synchronization broadcasts.  The               */
   /* CurrentBroadcastInformation parameter to a structure in which the */
   /* current information will be placed.  This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
int BTPSAPI TDSM_Get_Current_Broadcast_Information(TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if(CurrentBroadcastInformation)
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ret_val = ProcessGetCurrentBroadcastInformation(MSG_GetServerAddressID(), CurrentBroadcastInformation);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is used to update the information being    */
   /* sent in the synchronization broadcasts.  The TDSMControlCallbackID*/
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.  The BroadcastInformationUpdate parameter is a pointer to a */
   /* structure which contains the information to update.  This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI TDSM_Update_Broadcast_Information(unsigned int TDSMControlCallbackID, TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if((TDSMControlCallbackID) && (BroadcastInformationUpdate))
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ret_val = ProcessUpdateBroadcastInformation(MSG_GetServerAddressID(), TDSMControlCallbackID, BroadcastInformationUpdate);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the 3D Sync Profile  */
   /* Manager Service.  This Callback will be dispatched by the 3D Sync */
   /* Manager when various 3D Sync Manager events occur.  This function */
   /* accepts the callback function and callback parameter              */
   /* (respectively) to call when a 3D Sync Manager event needs to be   */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TDSM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
   /* * NOTE * Any registered TDSM Callback will get all TDSM events,   */
   /*          but only a callback registered with the Control parameter*/
   /*          set to TRUE can manage the Sync Train and Broadcast      */
   /*          information. There can only be one of these Control      */
   /*          callbacks registered.                                    */
int BTPSAPI TDSM_Register_Event_Callback(Boolean_t Control, TDSM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if((CallbackFunction) && (CallbackParameter))
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ret_val = ProcessRegisterEventCallback(MSG_GetServerAddressID(), Control, CallbackFunction, CallbackParameter);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered 3D Sync Manager event Callback*/
   /* (registered via a successful call to the                          */
   /* TDSM_Register_Event_Callback() function.  This function accepts as*/
   /* input the 3D Sync Manager event callback ID (return value from the*/
   /* TDSM_Register_Event_Callback() function).                         */
void BTPSAPI TDSM_Un_Register_Event_Callback(unsigned int TDSManagerEventCallbackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if this module is initialized.                              */
   if(Initialized)
   {
      /* Check that the parameters are at least semi-valid.             */
      if((TDSManagerEventCallbackID))
      {
         /* Attempt to acquire the lock the guards this modules.        */
         if(DEVM_AcquireLock())
         {
            /* Simply call the helper function to process the command.  */
            ProcessUnRegisterEventCallback(MSG_GetServerAddressID(), TDSManagerEventCallbackID);

            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

