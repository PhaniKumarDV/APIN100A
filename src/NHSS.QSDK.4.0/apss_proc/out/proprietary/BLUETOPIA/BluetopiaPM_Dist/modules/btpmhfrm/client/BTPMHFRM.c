/*****< btpmhfrm.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHFRM - Hands Free Manager for Stonestreet One Bluetooth Protocol      */
/*             Stack Platform Manager.                                        */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHFRM.h"            /* BTPM HFRE Manager Prototypes/Constants.   */
#include "HFRMAPI.h"             /* HFRE Manager Prototypes/Constants.        */
#include "HFRMMSG.h"             /* BTPM HFRE Manager Message Formats.        */
#include "HFRMGR.h"              /* HFRE Manager Impl. Prototypes/Constants.  */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHFRE_Entry_Info_t
{
   unsigned int                  CallbackID;
   unsigned int                  ControlCallbackID;
   unsigned int                  DataCallbackID;
   unsigned int                  ConnectionStatus;
   Event_t                       ConnectionEvent;
   unsigned long                 Flags;
   BD_ADDR_t                     BD_ADDR;
   HFRM_Connection_Type_t        ConnectionType;
   HFRM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagHFRE_Entry_Info_t *NextHFREEntryInfoPtr;
} HFRE_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HFRE_Entry_Info_t structure to denote various state information.  */
#define HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY          0x40000000
#define HFRE_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY     0x80000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   HFRM_Event_Callback_t  EventCallback;
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
static Mutex_t HFREManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variables which hold the current Hands Free/Audio Gateway events  */
   /* callback ID (registered with the server to receive events).       */
static unsigned int HFREEventsCallbackID_AG;
static unsigned int HFREEventsCallbackID_HF;

   /* Variables which hold a pointer to the first element in the Hands  */
   /* Free entry information list (which holds all callbacks tracked by */
   /* this module).                                                     */
static HFRE_Entry_Info_t *HFREEntryInfoList_AG;
static HFRE_Entry_Info_t *HFREEntryInfoList_HF;

   /* Variables which hold a pointer to the first element in the Hands  */
   /* Free Entry control list (which holds all control callbacks tracked*/
   /* by this module).                                                  */
static HFRE_Entry_Info_t *HFREEntryInfoList_AG_Control;
static HFRE_Entry_Info_t *HFREEntryInfoList_HF_Control;

   /* Variables which hold a pointer to the first element in the Hands  */
   /* Free entry data list (which holds all data callbacks tracked by   */
   /* this module).                                                     */
static HFRE_Entry_Info_t *HFREEntryInfoList_AG_Data;
static HFRE_Entry_Info_t *HFREEntryInfoList_HF_Data;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HFRE_Entry_Info_t *AddHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, HFRE_Entry_Info_t *EntryToAdd);
static HFRE_Entry_Info_t *SearchHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, unsigned int CallbackID);
static HFRE_Entry_Info_t *DeleteHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHFREEntryInfoEntryMemory(HFRE_Entry_Info_t *EntryToFree);
static void FreeHFREEntryInfoList(HFRE_Entry_Info_t **ListHead);

static void DispatchHFREEvent(Boolean_t ControlOnly, HFRM_Connection_Type_t ConnectionType, HFRM_Event_Data_t *HFRMEventData);

static void ProcessIncomingConnectionRequestEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessDeviceConnectionEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessDeviceConnectionStatusEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus);
static void ProcessDeviceDisconnectionEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason);
static void ProcessServiceLevelConnectionEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t RemoteSupportedFeaturesValid, unsigned long RemoteSupportedFeatures, unsigned long RemoteCallHoldMultipartySupport);
static void ProcessAudioConnectedEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessAudioConnectionStatusEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful);
static void ProcessAudioDisconnectedEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);
static void ProcessAudioDataReceivedEvent(HFRM_Audio_Data_Received_Message_t *Message);
static void ProcessVoiceRecognitionIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t VoiceRecognitionActive);
static void ProcessSpeakerGainIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain);
static void ProcessMicrophoneGainIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain);
static void ProcessIncomingCallStateIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState);
static void ProcessIncomingCallStateConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState);
static void ProcessControlIndicatorStatusIndicationEvent(BD_ADDR_t RemoteDeviceAddress, HFRM_Control_Indicator_Entry_t *ControlIndicatorEntry);
static void ProcessControlIndicatorStatusConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRM_Control_Indicator_Entry_t *ControlIndicatorEntry);
static void ProcessCallHoldMultipartySupportConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t CallHoldSupportMaskValid, unsigned long CallHoldSupportMask);
static void ProcessCallWaitingNotificationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber);
static void ProcessCallLineIDNotificationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber);
static void ProcessRingIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessInBandRingToneSettingIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled);
static void ProcessVoiceTagRequestConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber);
static void ProcessQueryCurrentCallsListConfirmationEvent_v1(BD_ADDR_t RemoteDeviceAddress, HFRM_Call_List_List_Entry_v1_t *CallListEntry);
static void ProcessQueryCurrentCallsListConfirmationEvent_v2(BD_ADDR_t RemoteDeviceAddress, HFRM_Call_List_List_Entry_v2_t *CallListEntry);
static void ProcessNetworkOperatorSelectionConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int NetworkMode, unsigned int NetworkOperatorLength, char *NetworkOperator);
static void ProcessSubscriberNumberConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRM_Subscriber_Information_List_Entry_t *SubscriberInformationEntry);
static void ProcessResponseHoldStatusConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState);
static void ProcessCommandResultEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Extended_Result_t ResultType, unsigned int ResultValue);
static void ProcessArbitraryResponseIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ArbitraryResponseLength, char *ArbitraryResponse);
static void ProcessCodecSelectIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID);
static void ProcessCallHoldMultipartySelectionIndicationEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index);
static void ProcessCallWaitingNotificationActivationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled);
static void ProcessCallLineIDNotificationActivationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled);
static void ProcessDisableSoundEnhancmentIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessDialPhoneNumberIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber);
static void ProcessDialPhoneNumberFromMemoryIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int MemoryLocation);
static void ProcessReDialLastPhoneNumberIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessGenerateDTMFCodeIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned char DTMFCode);
static void ProcessAnswerCallIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessVoiceTagRequestIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessHangUpIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessQueryCurrentCallsListIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessNetworkOperatorSelectionFormatIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int Format);
static void ProcessNetworkOperatorSelectionIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessExtendedErrorResultActivationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled);
static void ProcessSubscriberNumberInformationIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessResponseHoldStatusIndicationEvent(BD_ADDR_t RemoteDeviceAddress);
static void ProcessArbitraryCommandIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ArbitraryCommandLength, char *ArbitraryCommand);
static void ProcessAvailableCodecListIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList);
static void ProcessCodecSelectConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID);
static void ProcessCodecConnectIndicationEvent(BD_ADDR_t RemoteDeviceAddress);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_HFRM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HandsFreeManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique callback ID that can be used to add an entry    */
   /* into the Hands Free entry information list.                       */
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
   /*            callbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static HFRE_Entry_Info_t *AddHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, HFRE_Entry_Info_t *EntryToAdd)
{
   HFRE_Entry_Info_t *AddedEntry = NULL;
   HFRE_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HFRE_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HFRE_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHFREEntryInfoPtr = NULL;

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
                     FreeHFREEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHFREEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHFREEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHFREEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified callback ID.  This function returns NULL if either the  */
   /* list head is invalid, the callback ID is invalid, or the specified*/
   /* callback ID was NOT found.                                        */
static HFRE_Entry_Info_t *SearchHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HFRE_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHFREEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Hands Free entry    */
   /* information list for the specified callback ID and removes it from*/
   /* the List.  This function returns NULL if either the Hands Free    */
   /* entry information list head is invalid, the callback ID is        */
   /* invalid, or the specified callback ID was NOT present in the list.*/
   /* The entry returned will have the next entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeHFREEntryInfoEntryMemory().             */
static HFRE_Entry_Info_t *DeleteHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HFRE_Entry_Info_t *FoundEntry = NULL;
   HFRE_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHFREEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHFREEntryInfoPtr = FoundEntry->NextHFREEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHFREEntryInfoPtr;

         FoundEntry->NextHFREEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Hands Free entry information    */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeHFREEntryInfoEntryMemory(HFRE_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Hands Free entry information list.  Upon */
   /* return of this function, the head pointer is set to NULL.         */
static void FreeHFREEntryInfoList(HFRE_Entry_Info_t **ListHead)
{
   HFRE_Entry_Info_t *EntryToFree;
   HFRE_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHFREEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeHFREEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Hands Free event to every registered Hands */
   /* Free Event Callback.                                              */
   /* * NOTE * This function should be called with the Hands Free       */
   /*          Manager Mutex held.  Upon exit from this function it will*/
   /*          free the Hands Free Manager Mutex.                       */
static void DispatchHFREEvent(Boolean_t ControlOnly, HFRM_Connection_Type_t ConnectionType, HFRM_Event_Data_t *HFRMEventData)
{
   unsigned int       Index;
   unsigned int       NumberCallbacks;
   CallbackInfo_t     CallbackInfoArray[16];
   CallbackInfo_t    *CallbackInfoArrayPtr;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HFREEntryInfoList_AG) || (HFREEntryInfoList_HF) || (HFREEntryInfoList_AG_Control) || (HFREEntryInfoList_HF_Control) || (HFREEntryInfoList_AG_Data) || (HFREEntryInfoList_HF_Data)) && (HFRMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      if(!ControlOnly)
      {
         if(ConnectionType == hctAudioGateway)
            HFREEntryInfo = HFREEntryInfoList_AG;
         else
            HFREEntryInfo = HFREEntryInfoList_HF;

         while(HFREEntryInfo)
         {
            if((HFREEntryInfo->EventCallback) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
         }
      }

      /* Next, add the control handlers.                                */
      if(ConnectionType == hctAudioGateway)
         HFREEntryInfo = HFREEntryInfoList_AG_Control;
      else
         HFREEntryInfo = HFREEntryInfoList_HF_Control;

      while(HFREEntryInfo)
      {
         if((HFREEntryInfo->EventCallback) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
      }

      /* Next, add the data handlers.                                   */
      if(!ControlOnly)
      {
         if(ConnectionType == hctAudioGateway)
            HFREEntryInfo = HFREEntryInfoList_AG_Data;
         else
            HFREEntryInfo = HFREEntryInfoList_HF_Data;

         while(HFREEntryInfo)
         {
            if((HFREEntryInfo->EventCallback) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
         }
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
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            if(!ControlOnly)
            {
               if(ConnectionType == hctAudioGateway)
                  HFREEntryInfo = HFREEntryInfoList_AG;
               else
                  HFREEntryInfo = HFREEntryInfoList_HF;

               while(HFREEntryInfo)
               {
                  if((HFREEntryInfo->EventCallback) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HFREEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HFREEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
               }
            }

            /* Next, add the control handlers.                          */
            if(ConnectionType == hctAudioGateway)
               HFREEntryInfo = HFREEntryInfoList_AG_Control;
            else
               HFREEntryInfo = HFREEntryInfoList_HF_Control;

            while(HFREEntryInfo)
            {
               if((HFREEntryInfo->EventCallback) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HFREEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HFREEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
            }

            /* Next, add the data handlers.                             */
            if(!ControlOnly)
            {
               if(ConnectionType == hctAudioGateway)
                  HFREEntryInfo = HFREEntryInfoList_AG_Data;
               else
                  HFREEntryInfo = HFREEntryInfoList_HF_Data;

               while(HFREEntryInfo)
               {
                  if((HFREEntryInfo->EventCallback) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HFREEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HFREEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
               }
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(HFREManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(HFRMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HFREManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HFREManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the incoming */
   /* connection request asynchronous message.                          */
static void ProcessIncomingConnectionRequestEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                        = hetHFRIncomingConnectionRequest;
   HFRMEventData.EventLength                                                      = HFRM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

   HFRMEventData.EventData.IncomingConnectionRequestEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* connection asynchronous message.                                  */
static void ProcessDeviceConnectionEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                        = hetHFRConnected;
   HFRMEventData.EventLength                                      = HFRM_CONNECTED_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ConnectedEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* connection status asynchronous message.                           */
static void ProcessDeviceConnectionStatusEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus)
{
   void                  *CallbackParameter;
   Boolean_t              ReleaseMutex;
   HFRM_Event_Data_t      HFRMEventData;
   HFRE_Entry_Info_t     *HFREEntryInfo;
   HFRM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if(((ConnectionType == hctAudioGateway) && (HFREEntryInfoList_AG)) || ((ConnectionType == hctHandsFree) && (HFREEntryInfoList_HF)))
   {
      ReleaseMutex = TRUE;

      if(ConnectionType == hctAudioGateway)
         HFREEntryInfo = HFREEntryInfoList_AG;
      else
         HFREEntryInfo = HFREEntryInfoList_HF;

      while(HFREEntryInfo)
      {
         if((!(HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(HFREEntryInfo->BD_ADDR, RemoteDeviceAddress)))
         {
            /* Callback registered, now see if the callback is          */
            /* synchronous or asynchronous.                             */
            if(HFREEntryInfo->ConnectionEvent)
            {
               /* Synchronous.                                          */

               /* Note the Status.                                      */
               HFREEntryInfo->ConnectionStatus = ConnectionStatus;

               /* Set the Event.                                        */
               BTPS_SetEvent(HFREEntryInfo->ConnectionEvent);

               /* Break out of the list.                                */
               HFREEntryInfo = NULL;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HFREManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;
            }
            else
            {
               /* Asynchronous Entry, go ahead dispatch the result.     */

               /* Format up the Event.                                  */
               HFRMEventData.EventType                                               = hetHFRConnectionStatus;
               HFRMEventData.EventLength                                             = HFRM_CONNECTION_STATUS_EVENT_DATA_SIZE;

               HFRMEventData.EventData.ConnectionStatusEventData.ConnectionType      = ConnectionType;
               HFRMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
               HFRMEventData.EventData.ConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

               /* Note the Callback information.                        */
               EventCallback     = HFREEntryInfo->EventCallback;
               CallbackParameter = HFREEntryInfo->CallbackParameter;

               if((HFREEntryInfo = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, HFREEntryInfo->CallbackID)) != NULL)
                  FreeHFREEntryInfoEntryMemory(HFREEntryInfo);

               /* Release the Mutex so we can dispatch the event.       */
               BTPS_ReleaseMutex(HFREManagerMutex);

               /* Flag that the Mutex does not need to be released.     */
               ReleaseMutex = FALSE;

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&HFRMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Break out of the list.                                */
               HFREEntryInfo = NULL;
            }
         }
         else
            HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
      }

      /* If the Mutex was not released, then we need to make sure we    */
      /* release it.                                                    */
      if(ReleaseMutex)
         BTPS_ReleaseMutex(HFREManagerMutex);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HFREManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* disconnection asynchronous message.                               */
static void ProcessDeviceDisconnectionEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                           = hetHFRDisconnected;
   HFRMEventData.EventLength                                         = HFRM_DISCONNECTED_EVENT_DATA_SIZE;

   HFRMEventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.DisconnectedEventData.DisconnectReason    = DisconnectReason;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* disconnection asynchronous message.                               */
static void ProcessServiceLevelConnectionEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t RemoteSupportedFeaturesValid, unsigned long RemoteSupportedFeatures, unsigned long RemoteCallHoldMultipartySupport)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                            = hetHFRServiceLevelConnectionEstablished;
   HFRMEventData.EventLength                                                                          = HFRM_SERVICE_LEVEL_CONNECTION_ESTABLISHED_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.ConnectionType                  = ConnectionType;
   HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteDeviceAddress             = RemoteDeviceAddress;
   HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteSupportedFeaturesValid    = RemoteSupportedFeaturesValid;
   HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteSupportedFeatures         = RemoteSupportedFeatures;
   HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteCallHoldMultipartySupport = RemoteCallHoldMultipartySupport;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* connected asynchronous message.                                   */
static void ProcessAudioConnectedEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                             = hetHFRAudioConnected;
   HFRMEventData.EventLength                                           = HFRM_AUDIO_CONNECTED_EVENT_DATA_SIZE;

   HFRMEventData.EventData.AudioConnectedEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.AudioConnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* connection status asynchronous message.                           */
static void ProcessAudioConnectionStatusEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Successful)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                    = hetHFRAudioConnectionStatus;
   HFRMEventData.EventLength                                                  = HFRM_AUDIO_CONNECTION_STATUS_EVENT_DATA_SIZE;

   HFRMEventData.EventData.AudioConnectionStatusEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.AudioConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.AudioConnectionStatusEventData.Successful          = Successful;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* disconnection asynchronous message.                               */
static void ProcessAudioDisconnectedEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                = hetHFRAudioDisconnected;
   HFRMEventData.EventLength                                              = HFRM_AUDIO_DISCONNECTED_EVENT_DATA_SIZE;

   HFRMEventData.EventData.AudioDisconnectedEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.AudioDisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the audio    */
   /* data received asynchronous message.                               */
static void ProcessAudioDataReceivedEvent(HFRM_Audio_Data_Received_Message_t *Message)
{
   void                  *CallbackParameter;
   HFRM_Event_Data_t      HFRMEventData;
   HFRE_Entry_Info_t     *HFREEntryInfo;
   HFRM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the event.                                              */
   if((Message) && (Message->AudioDataLength) && (Message->AudioData))
   {
      HFRMEventData.EventType                                        = hetHFRAudioData;
      HFRMEventData.EventLength                                      = HFRM_AUDIO_DATA_EVENT_DATA_SIZE;

      HFRMEventData.EventData.AudioDataEventData.DataEventsHandlerID = 0;
      HFRMEventData.EventData.AudioDataEventData.ConnectionType      = Message->ConnectionType;
      HFRMEventData.EventData.AudioDataEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      HFRMEventData.EventData.AudioDataEventData.AudioDataFlags      = Message->AudioDataFlags;
      HFRMEventData.EventData.AudioDataEventData.AudioDataLength     = Message->AudioDataLength;
      HFRMEventData.EventData.AudioDataEventData.AudioData           = Message->AudioData;

      /* Now that the event is formatted, dispatch it.                  */

      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if(((HFREEntryInfo = ((Message->ConnectionType == hctAudioGateway)?HFREEntryInfoList_AG_Data:HFREEntryInfoList_HF_Data)) != NULL) && (HFREEntryInfo->ConnectionStatus == Message->DataEventsHandlerID))
      {
         /* Note the Callback Information.                              */
         EventCallback                                                  = HFREEntryInfo->EventCallback;
         CallbackParameter                                              = HFREEntryInfo->CallbackParameter;

         /* Note that we need to map the Server Callback to the Client  */
         /* Callback ID.                                                */
         HFRMEventData.EventData.AudioDataEventData.DataEventsHandlerID = HFREEntryInfo->CallbackID;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HFREManagerMutex);

         __BTPSTRY
         {
            (*EventCallback)(&HFRMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HFREManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HFREManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the voice    */
   /* recognitiion indication asynchronous message.                     */
static void ProcessVoiceRecognitionIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t VoiceRecognitionActive)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                            = hetHFRVoiceRecognitionIndication;
   HFRMEventData.EventLength                                                          = HFRM_VOICE_RECOGNITION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.VoiceRecognitionIndicationEventData.ConnectionType         = ConnectionType;
   HFRMEventData.EventData.VoiceRecognitionIndicationEventData.RemoteDeviceAddress    = RemoteDeviceAddress;
   HFRMEventData.EventData.VoiceRecognitionIndicationEventData.VoiceRecognitionActive = VoiceRecognitionActive;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the set      */
   /* speaker gain indication asynchronous message.                     */
static void ProcessSpeakerGainIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                    = hetHFRSpeakerGainIndication;
   HFRMEventData.EventLength                                                  = HFRM_SPEAKER_GAIN_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.SpeakerGainIndicationEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.SpeakerGainIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.SpeakerGainIndicationEventData.SpeakerGain         = SpeakerGain;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the set      */
   /* microphone gain indication asynchronous message.                  */
static void ProcessMicrophoneGainIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                       = hetHFRMicrophoneGainIndication;
   HFRMEventData.EventLength                                                     = HFRM_MICROPHONE_GAIN_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.MicrophoneGainIndicationEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.MicrophoneGainIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.MicrophoneGainIndicationEventData.MicrophoneGain      = MicrophoneGain;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the incoming */
   /* call state indication asynchronous message.                       */
static void ProcessIncomingCallStateIndicationEvent(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                          = hetHFRIncomingCallStateIndication;
   HFRMEventData.EventLength                                                        = HFRM_INCOMING_CALL_STATE_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.IncomingCallStateIndicationEventData.ConnectionType      = ConnectionType;
   HFRMEventData.EventData.IncomingCallStateIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.IncomingCallStateIndicationEventData.CallState           = CallState;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, ConnectionType, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the incoming */
   /* call state confirmation asynchronous message.                     */
static void ProcessIncomingCallStateConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                            = hetHFRIncomingCallStateConfirmation;
   HFRMEventData.EventLength                                                          = HFRM_INCOMING_CALL_STATE_CONFIRMATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.IncomingCallStateConfirmationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.IncomingCallStateConfirmationEventData.CallState           = CallState;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the control  */
   /* indicator status indication asynchronous message.                 */
static void ProcessControlIndicatorStatusIndicationEvent(BD_ADDR_t RemoteDeviceAddress, HFRM_Control_Indicator_Entry_t *ControlIndicatorEntry)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                                      = hetHFRControlIndicatorStatusIndication;
   HFRMEventData.EventLength                                                                                    = HFRM_CONTROL_INDICATOR_STATUS_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.RemoteDeviceAddress                        = RemoteDeviceAddress;
   HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.IndicatorDescription = ControlIndicatorEntry->IndicatorDescription;
   HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.ControlIndicatorType = ControlIndicatorEntry->ControlIndicatorType;

   /* Copy the control indicator data.                                  */
   BTPS_MemCopy(&(HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.Control_Indicator_Data), &(ControlIndicatorEntry->Control_Indicator_Data), sizeof(ControlIndicatorEntry->Control_Indicator_Data));

   /* Make sure the indicator description is NULL terminated.           */
   ControlIndicatorEntry->IndicatorDescription[sizeof(ControlIndicatorEntry->IndicatorDescription) - 1] = '\0';

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the control  */
   /* indicator status confirmation asynchronous message.               */
static void ProcessControlIndicatorStatusConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRM_Control_Indicator_Entry_t *ControlIndicatorEntry)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                                        = hetHFRControlIndicatorStatusConfirmation;
   HFRMEventData.EventLength                                                                                      = HFRM_CONTROL_INDICATOR_STATUS_CONFIRMATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.RemoteDeviceAddress                        = RemoteDeviceAddress;
   HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.IndicatorDescription = ControlIndicatorEntry->IndicatorDescription;
   HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.ControlIndicatorType = ControlIndicatorEntry->ControlIndicatorType;

   /* Copy the Control Indicator data.                                  */
   BTPS_MemCopy(&(HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.Control_Indicator_Data), &(ControlIndicatorEntry->Control_Indicator_Data), sizeof(ControlIndicatorEntry->Control_Indicator_Data));

   /* Make sure the Indicator Description is NULL terminated.           */
   ControlIndicatorEntry->IndicatorDescription[sizeof(ControlIndicatorEntry->IndicatorDescription) - 1] = '\0';

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the call     */
   /* hold/multi-party support confirmation asynchronous message.       */
static void ProcessCallHoldMultipartySupportConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t CallHoldSupportMaskValid, unsigned long CallHoldSupportMask)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                         = hetHFRCallHoldMultipartySupportConfirmation;
   HFRMEventData.EventLength                                                                       = HFRM_CALL_HOLD_MULTIPARTY_SUPPORT_CONFIRMATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.RemoteDeviceAddress      = RemoteDeviceAddress;
   HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMaskValid = CallHoldSupportMaskValid;
   HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMask      = CallHoldSupportMask;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the call     */
   /* waiting notification indication asynchronous message.             */
static void ProcessCallWaitingNotificationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                = hetHFRCallWaitingNotificationIndication;
   HFRMEventData.EventLength                                                              = HFRM_CALL_WAITING_NOTIFICATION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CallWaitingNotificationIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   if((PhoneNumber) && (PhoneNumberLength != 1))
   {
      HFRMEventData.EventData.CallWaitingNotificationIndicationEventData.PhoneNumber = PhoneNumber;

      /* Make sure the phone number is NULL terminated.                 */
      PhoneNumber[PhoneNumberLength - 1]                                             = '\0';
   }
   else
      HFRMEventData.EventData.CallWaitingNotificationIndicationEventData.PhoneNumber = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the call line*/
   /* ID notification indication asynchronous message.                  */
static void ProcessCallLineIDNotificationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                           = hetHFRCallLineIdentificationNotificationIndication;
   HFRMEventData.EventLength                                                                         = HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CallLineIdentificationNotificationIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   if((PhoneNumber) && (PhoneNumberLength != 1))
   {
      HFRMEventData.EventData.CallLineIdentificationNotificationIndicationEventData.PhoneNumber = PhoneNumber;

      /* Make sure the phone number is NULL terminated.                 */
      PhoneNumber[PhoneNumberLength - 1]                                                        = '\0';
   }
   else
      HFRMEventData.EventData.CallLineIdentificationNotificationIndicationEventData.PhoneNumber = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ring     */
   /* indication asynchronous message.                                  */
static void ProcessRingIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                             = hetHFRRingIndication;
   HFRMEventData.EventLength                                           = HFRM_RING_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.RingIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the in-band  */
   /* ring tone setting indication asynchronous message.                */
static void ProcessInBandRingToneSettingIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                              = hetHFRInBandRingToneSettingIndication;
   HFRMEventData.EventLength                                                            = HFRM_IN_BAND_RING_TONE_SETTING_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.InBandRingToneSettingIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.InBandRingToneSettingIndicationEventData.Enabled             = Enabled;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the voice tag*/
   /* request confirmation asynchronous message.                        */
static void ProcessVoiceTagRequestConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                          = hetHFRVoiceTagRequestConfirmation;
   HFRMEventData.EventLength                                                        = HFRM_VOICE_TAG_REQUEST_CONFIRMATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.VoiceTagRequestConfirmationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   if((PhoneNumber) && (PhoneNumberLength != 1))
   {
      HFRMEventData.EventData.VoiceTagRequestConfirmationEventData.PhoneNumber = PhoneNumber;

      /* Make sure the phone number is NULL terminated.                 */
      PhoneNumber[PhoneNumberLength - 1]                                       = '\0';
   }
   else
      HFRMEventData.EventData.VoiceTagRequestConfirmationEventData.PhoneNumber = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the current  */
   /* calls list confirmation asynchronous message.                     */
static void ProcessQueryCurrentCallsListConfirmationEvent_v1(BD_ADDR_t RemoteDeviceAddress, HFRM_Call_List_List_Entry_v1_t *CallListEntry)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallListEntry)
   {
      /* Format up the Event.                                           */
      BTPS_MemInitialize(&HFRMEventData, 0, sizeof(HFRMEventData));

      HFRMEventData.EventType                                                            = hetHFRCurrentCallsListConfirmation;
      HFRMEventData.EventLength                                                          = HFRM_CURRENT_CALLS_LIST_CONFIRMATION_EVENT_DATA_SIZE;

      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.RemoteDeviceAddress  = RemoteDeviceAddress;
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry = CallListEntry->CallListEntry;

      if(CallListEntry->PhoneNumberLength > 1)
      {
         HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhoneNumber = CallListEntry->PhoneNumber;

         /* Make sure the phone number is NULL terminated.              */
         CallListEntry->PhoneNumber[CallListEntry->PhoneNumberLength - 1]                               = '\0';
      }
      else
         HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhoneNumber = NULL;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessQueryCurrentCallsListConfirmationEvent_v2(BD_ADDR_t RemoteDeviceAddress, HFRM_Call_List_List_Entry_v2_t *CallListEntry)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallListEntry)
   {
      /* Format up the Event.                                           */
      BTPS_MemInitialize(&HFRMEventData, 0, sizeof(HFRMEventData));

      HFRMEventData.EventType                                                                          = hetHFRCurrentCallsListConfirmation;
      HFRMEventData.EventLength                                                                        = HFRM_CURRENT_CALLS_LIST_CONFIRMATION_EVENT_DATA_SIZE;

      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.RemoteDeviceAddress                = RemoteDeviceAddress;

      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.Index         = CallListEntry->Index;
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.CallDirection = CallListEntry->CallDirection;
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.CallStatus    = CallListEntry->CallStatus;
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.CallMode      = CallListEntry->CallMode;
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.Multiparty    = CallListEntry->Multiparty;
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhoneNumber   = ((CallListEntry->Flags & HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONE_NUMBER_VALID) ? CallListEntry->PhoneNumber : NULL);
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.NumberFormat  = CallListEntry->NumberFormat;
      HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhonebookName = ((CallListEntry->Flags & HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONEBOOK_NAME_VALID) ? CallListEntry->PhonebookName : NULL);

      /* Protect against badly formatted messages                       */
      CallListEntry->PhoneNumber[HFRE_PHONE_NUMBER_LENGTH_MAXIMUM]     = '\0';
      CallListEntry->PhonebookName[HFRE_PHONEBOOK_NAME_LENGTH_MAXIMUM] = '\0';

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the network  */
   /* operator selection confirmation asynchronous message.             */
static void ProcessNetworkOperatorSelectionConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int NetworkMode, unsigned int NetworkOperatorLength, char *NetworkOperator)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                            = hetHFRNetworkOperatorSelectionConfirmation;
   HFRMEventData.EventLength                                                          = HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.NetworkOperatorSelectionConfirmationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.NetworkOperatorSelectionConfirmationEventData.NetworkMode         = NetworkMode;

   if((NetworkOperator) && (NetworkOperatorLength != 1))
   {
      HFRMEventData.EventData.NetworkOperatorSelectionConfirmationEventData.NetworkOperator = NetworkOperator;

      /* Make sure the netowork operator is NULL terminated.            */
      NetworkOperator[NetworkOperatorLength - 1]                                            = '\0';
   }
   else
      HFRMEventData.EventData.NetworkOperatorSelectionConfirmationEventData.NetworkOperator = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* subscriber number confirmation asynchronous message.              */
static void ProcessSubscriberNumberConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRM_Subscriber_Information_List_Entry_t *SubscriberInformationEntry)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(SubscriberInformationEntry)
   {
      /* Format up the Event.                                           */
      HFRMEventData.EventType                                                                              = hetHFRSubscriberNumberInformationConfirmation;
      HFRMEventData.EventLength                                                                            = HFRM_SUBSCRIBER_NUMBER_INFORMATION_CONFIRMATION_EVENT_DATA_SIZE;

      HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.RemoteDeviceAddress         = RemoteDeviceAddress;
      HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation = SubscriberInformationEntry->SubscriberNumberInformationEntry;

      if(SubscriberInformationEntry->PhoneNumberLength > 1)
      {
         HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.PhoneNumber = SubscriberInformationEntry->PhoneNumber;

         /* Make sure the phone number is NULL terminated.              */
         SubscriberInformationEntry->PhoneNumber[SubscriberInformationEntry->PhoneNumberLength - 1]                       = '\0';
      }
      else
         HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.PhoneNumber = NULL;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* response/hold status confirmation asynchronous message.           */
static void ProcessResponseHoldStatusConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                             = hetHFRResponseHoldStatusConfirmation;
   HFRMEventData.EventLength                                                           = HFRM_RESPONSE_HOLD_STATUS_CONFIRMATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ResponseHoldStatusConfirmationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.ResponseHoldStatusConfirmationEventData.CallState           = CallState;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the command  */
   /* result asynchronous message.                                      */
static void ProcessCommandResultEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Extended_Result_t ResultType, unsigned int ResultValue)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                            = hetHFRCommandResult;
   HFRMEventData.EventLength                                          = HFRM_COMMAND_RESULT_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CommandResultEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.CommandResultEventData.ResultType          = ResultType;
   HFRMEventData.EventData.CommandResultEventData.ResultValue         = ResultValue;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the arbitrary*/
   /* response indication asynchronous message.                         */
static void ProcessArbitraryResponseIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ArbitraryResponseLength, char *ArbitraryResponse)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                          = hetHFRArbitraryResponseIndication;
   HFRMEventData.EventLength                                                        = HFRM_ARBITRARY_RESPONSE_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ArbitraryResponseIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   if((ArbitraryResponse) && (ArbitraryResponseLength != 1))
   {
      HFRMEventData.EventData.ArbitraryResponseIndicationEventData.ResponseData = ArbitraryResponse;

      /* Make sure the network operator is NULL terminated.             */
      ArbitraryResponse[ArbitraryResponseLength - 1]                            = '\0';
   }
   else
      HFRMEventData.EventData.ArbitraryResponseIndicationEventData.ResponseData = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the codec select indication event. The hands free device  */
   /* receives this indication when the remote audio gateway sends the  */
   /* selected codec ID as part of the codec negotiation.               */
static void ProcessCodecSelectIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                    = hetHFRCodecSelectIndication;
   HFRMEventData.EventLength                                                  = HFRM_CODEC_SELECT_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CodecSelectIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.CodecSelectIndicationEventData.CodecID             = CodecID;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctHandsFree, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the call     */
   /* hold/multi-party selection indication asynchronous message.       */
static void ProcessCallHoldMultipartySelectionIndicationEvent(BD_ADDR_t RemoteDeviceAddress, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                           = hetHFRCallHoldMultipartySelectionIndication;
   HFRMEventData.EventLength                                                                         = HFRM_CALL_HOLD_MULTIPARTY_SELECTION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CallHoldMultipartySelectionIndicationEventData.RemoteDeviceAddress        = RemoteDeviceAddress;
   HFRMEventData.EventData.CallHoldMultipartySelectionIndicationEventData.CallHoldMultipartyHandling = CallHoldMultipartyHandling;
   HFRMEventData.EventData.CallHoldMultipartySelectionIndicationEventData.Index                      = Index;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the call     */
   /* waiting notification activation indication asynchronous message.  */
static void ProcessCallWaitingNotificationActivationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                          = hetHFRCallWaitingNotificationActivationIndication;
   HFRMEventData.EventLength                                                                        = HFRM_CALL_WAITING_NOTIFICATION_ACTIVATION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CallWaitingNotificationActivationIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.CallWaitingNotificationActivationIndicationEventData.Enabled             = Enabled;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the call line*/
   /* ID notification activation indication asynchronous message.       */
static void ProcessCallLineIDNotificationActivationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                                     = hetHFRCallLineIdentificationNotificationActivationIndication;
   HFRMEventData.EventLength                                                                                   = HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_ACTIVATION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CallLineIdentificationNotificationActivationIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.CallLineIdentificationNotificationActivationIndicationEventData.Enabled             = Enabled;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the disable  */
   /* sound enhancement indication asynchronous message.                */
static void ProcessDisableSoundEnhancmentIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                = hetHFRDisableSoundEnhancementIndication;
   HFRMEventData.EventLength                                                              = HFRM_DISABLE_SOUND_ENHANCEMENT_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.DisableSoundEnhancementIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the dial     */
   /* phone number indication asynchronous message.                     */
static void ProcessDialPhoneNumberIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int PhoneNumberLength, char *PhoneNumber)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                        = hetHFRDialPhoneNumberIndication;
   HFRMEventData.EventLength                                                      = HFRM_DIAL_PHONE_NUMBER_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.DialPhoneNumberIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   if((PhoneNumber) && (PhoneNumberLength != 1))
   {
      HFRMEventData.EventData.DialPhoneNumberIndicationEventData.PhoneNumber = PhoneNumber;

      /* Make sure the phone number is NULL terminated.                 */
      PhoneNumber[PhoneNumberLength - 1]                                     = '\0';
   }
   else
      HFRMEventData.EventData.DialPhoneNumberIndicationEventData.PhoneNumber = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the dial     */
   /* phone number from memory indication asynchronous message.         */
static void ProcessDialPhoneNumberFromMemoryIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int MemoryLocation)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                  = hetHFRDialPhoneNumberFromMemoryIndication;
   HFRMEventData.EventLength                                                                = HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.DialPhoneNumberFromMemoryIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.DialPhoneNumberFromMemoryIndicationEventData.MemoryLocation      = MemoryLocation;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the re-dial  */
   /* last phone number indication asynchronous message.                */
static void ProcessReDialLastPhoneNumberIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                              = hetHFRReDialLastPhoneNumberIndication;
   HFRMEventData.EventLength                                                            = HFRM_RE_DIAL_LAST_PHONE_NUMBER_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ReDialLastPhoneNumberIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the generate */
   /* DTMF code indication asynchronous message.                        */
static void ProcessGenerateDTMFCodeIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned char DTMFCode)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                         = hetHFRGenerateDTMFCodeIndication;
   HFRMEventData.EventLength                                                       = HFRM_GENERATE_DTMF_CODE_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.GenerateDTMFCodeIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.GenerateDTMFCodeIndicationEventData.DTMFCode            = DTMFCode;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the answer   */
   /* call indication asynchronous message.                             */
static void ProcessAnswerCallIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                   = hetHFRAnswerCallIndication;
   HFRMEventData.EventLength                                                 = HFRM_ANSWER_CALL_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.AnswerCallIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the voice tag*/
   /* request indication asynchronous message.                          */
static void ProcessVoiceTagRequestIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                        = hetHFRVoiceTagRequestIndication;
   HFRMEventData.EventLength                                                      = HFRM_VOICE_TAG_REQUEST_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.VoiceTagRequestIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the hang up  */
   /* indication asynchronous message.                                  */
static void ProcessHangUpIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                               = hetHFRHangUpIndication;
   HFRMEventData.EventLength                                             = HFRM_HANG_UP_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.HangUpIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the query    */
   /* current calls list indication asynchronous message.               */
static void ProcessQueryCurrentCallsListIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                         = hetHFRCurrentCallsListIndication;
   HFRMEventData.EventLength                                                       = HFRM_CURRENT_CALLS_LIST_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CurrentCallsListIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the network  */
   /* operator selection format indication asynchronous message.        */
static void ProcessNetworkOperatorSelectionFormatIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int Format)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                       = hetHFRNetworkOperatorSelectionFormatIndication;
   HFRMEventData.EventLength                                                                     = HFRM_NETWORK_OPERATOR_SELECTION_FORMAT_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.NetworkOperatorSelectionFormatIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.NetworkOperatorSelectionFormatIndicationEventData.Format              = Format;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the network  */
   /* operator selection indication asynchronous message.               */
static void ProcessNetworkOperatorSelectionIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                 = hetHFRNetworkOperatorSelectionIndication;
   HFRMEventData.EventLength                                                               = HFRM_NETWORK_OPERATOR_SELECTION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.NetworkOperatorSelectionIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the extended */
   /* error result activation indication asynchronous message.          */
static void ProcessExtendedErrorResultActivationIndicationEvent(BD_ADDR_t RemoteDeviceAddress, Boolean_t Enabled)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                      = hetHFRExtendedErrorResultActivationIndication;
   HFRMEventData.EventLength                                                                    = HFRM_EXTENDED_ERROR_RESULT_ACTIVATION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ExtendedErrorResultActivationIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.ExtendedErrorResultActivationIndicationEventData.Enabled             = Enabled;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* subscriber number information indication asynchronous message.    */
static void ProcessSubscriberNumberInformationIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                                    = hetHFRSubscriberNumberInformationIndication;
   HFRMEventData.EventLength                                                                  = HFRM_SUBSCRIBER_NUMBER_INFORMATION_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.SubscriberNumberInformationIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* response/hold status indication asynchronous message.             */
static void ProcessResponseHoldStatusIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                           = hetHFRResponseHoldStatusIndication;
   HFRMEventData.EventLength                                                         = HFRM_RESPONSE_HOLD_STATUS_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ResponseHoldStatusIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the arbitrary*/
   /* command indication asynchronous message.                          */
static void ProcessArbitraryCommandIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ArbitraryCommandLength, char *ArbitraryCommand)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                         = hetHFRArbitraryCommandIndication;
   HFRMEventData.EventLength                                                       = HFRM_ARBITRARY_COMMAND_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.ArbitraryCommandIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   if((ArbitraryCommand) && (ArbitraryCommandLength != 1))
   {
      HFRMEventData.EventData.ArbitraryCommandIndicationEventData.CommandData = ArbitraryCommand;

      /* Make sure the netowork operator is NULL terminated.            */
      ArbitraryCommand[ArbitraryCommandLength - 1]                            = '\0';
   }
   else
      HFRMEventData.EventData.ArbitraryCommandIndicationEventData.CommandData = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process available codec list indication event. The audio gateway  */
   /* device receives this indication when a list of supported codecs   */
   /* has been sent from the hands free device.                         */
static void ProcessAvailableCodecListIndicationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                             = hetHFRAvailableCodecListIndication;
   HFRMEventData.EventLength                                                           = HFRM_AVAILABLE_CODEC_LIST_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.AvailableCodecListIndicationEventData.RemoteDeviceAddress   = RemoteDeviceAddress;
   HFRMEventData.EventData.AvailableCodecListIndicationEventData.NumberSupportedCodecs = NumberSupportedCodecs;

   if((NumberSupportedCodecs) && (AvailableCodecList))
      HFRMEventData.EventData.AvailableCodecListIndicationEventData.AvailableCodecList = AvailableCodecList;
   else
      HFRMEventData.EventData.AvailableCodecListIndicationEventData.AvailableCodecList = NULL;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the codec connect indication event. The audio gateway     */
   /* receives this indication when, as part of the audio connection    */
   /* setup, the codec connection is complete.                          */
static void ProcessCodecConnectIndicationEvent(BD_ADDR_t RemoteDeviceAddress)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                     = hetHFRCodecConnectionSetupIndication;
   HFRMEventData.EventLength                                                   = HFRM_CODEC_CONNECTION_SETUP_INDICATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CodecConnectionSetupIndicationEventData.RemoteDeviceAddress = RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the codec select confirmation event. The audio gateway    */
   /* receives this confirmation when the remote hands free device      */
   /* sends a codec ID to complete the codec negotiation.               */
static void ProcessCodecSelectConfirmationEvent(BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID)
{
   HFRM_Event_Data_t HFRMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HFRMEventData.EventType                                                      = hetHFRCodecSelectConfirmation;
   HFRMEventData.EventLength                                                    = HFRM_CODEC_SELECT_CONFIRMATION_EVENT_DATA_SIZE;

   HFRMEventData.EventData.CodecSelectConfirmationEventData.RemoteDeviceAddress = RemoteDeviceAddress;
   HFRMEventData.EventData.CodecSelectConfirmationEventData.AcceptedCodec       = CodecID;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHFREEvent(TRUE, hctAudioGateway, &HFRMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* incoming connection request event.                    */
               ProcessIncomingConnectionRequestEvent(((HFRM_Connection_Request_Message_t *)Message)->ConnectionType, ((HFRM_Connection_Request_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DEVICE_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device connection event.                              */
               ProcessDeviceConnectionEvent(((HFRM_Device_Connected_Message_t *)Message)->ConnectionType, ((HFRM_Device_Connected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device connection status event.                       */
               ProcessDeviceConnectionStatusEvent(((HFRM_Device_Connection_Status_Message_t *)Message)->ConnectionType, ((HFRM_Device_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Device_Connection_Status_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Disconnection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DEVICE_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device disconnection event.                           */
               ProcessDeviceDisconnectionEvent(((HFRM_Device_Disconnected_Message_t *)Message)->ConnectionType, ((HFRM_Device_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Device_Disconnected_Message_t *)Message)->DisconnectReason);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SERVICE_LEVEL_CONNECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Service Level Connection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SERVICE_LEVEL_CONNECTION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* service level connection event.                       */
               ProcessServiceLevelConnectionEvent(((HFRM_Service_Level_Connection_Message_t *)Message)->ConnectionType, ((HFRM_Service_Level_Connection_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Service_Level_Connection_Message_t *)Message)->RemoteSupportedFeaturesValid, ((HFRM_Service_Level_Connection_Message_t *)Message)->RemoteSupportedFeatures, ((HFRM_Service_Level_Connection_Message_t *)Message)->RemoteCallHoldMultipartySupport);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_AUDIO_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio connected event.                                */
               ProcessAudioConnectedEvent(((HFRM_Audio_Connected_Message_t *)Message)->ConnectionType, ((HFRM_Audio_Connected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_AUDIO_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio connection status event.                        */
               ProcessAudioConnectionStatusEvent(((HFRM_Audio_Connection_Status_Message_t *)Message)->ConnectionType, ((HFRM_Audio_Connection_Status_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Audio_Connection_Status_Message_t *)Message)->Successful);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_AUDIO_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio disconnected event.                             */
               ProcessAudioDisconnectedEvent(((HFRM_Audio_Disconnected_Message_t *)Message)->ConnectionType, ((HFRM_Audio_Disconnected_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Data Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(((HFRM_Audio_Data_Received_Message_t *)Message)->AudioDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* audio data received event.                            */
               ProcessAudioDataReceivedEvent((HFRM_Audio_Data_Received_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_VOICE_RECOGNITION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Recognition Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_VOICE_RECOGNITION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* voice recoginition indication event.                  */
               ProcessVoiceRecognitionIndicationEvent(((HFRM_Voice_Recognition_Indication_Message_t *)Message)->ConnectionType, ((HFRM_Voice_Recognition_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Voice_Recognition_Indication_Message_t *)Message)->VoiceRecognitionActive);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Speaker Gain Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SPEAKER_GAIN_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* speaker gain indication event.                        */
               ProcessSpeakerGainIndicationEvent(((HFRM_Speaker_Gain_Indication_Message_t *)Message)->ConnectionType, ((HFRM_Speaker_Gain_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Speaker_Gain_Indication_Message_t *)Message)->SpeakerGain);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Microphone Gain Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_MICROPHONE_GAIN_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* microphone gain indication event.                     */
               ProcessMicrophoneGainIndicationEvent(((HFRM_Microphone_Gain_Indication_Message_t *)Message)->ConnectionType, ((HFRM_Microphone_Gain_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Microphone_Gain_Indication_Message_t *)Message)->MicrophoneGain);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Call State Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_INCOMING_CALL_STATE_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* incoming call state indication event.                 */
               ProcessIncomingCallStateIndicationEvent(((HFRM_Incoming_Call_State_Indication_Message_t *)Message)->ConnectionType, ((HFRM_Incoming_Call_State_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Incoming_Call_State_Indication_Message_t *)Message)->CallState);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Call State Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_INCOMING_CALL_STATE_CONFIRMATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* incoming call state confirmation event.               */
               ProcessIncomingCallStateConfirmationEvent(((HFRM_Incoming_Call_State_Confirmation_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Incoming_Call_State_Confirmation_Message_t *)Message)->CallState);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Indicator Status Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CONTROL_INDICATOR_STATUS_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* control indicator status indication event.            */
               ProcessControlIndicatorStatusIndicationEvent(((HFRM_Control_Indicator_Status_Indication_Message_t *)Message)->RemoteDeviceAddress, &(((HFRM_Control_Indicator_Status_Indication_Message_t *)Message)->ControlIndicatorEntry));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Indicator Status Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CONTROL_INDICATOR_STATUS_CONFIRMATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* control indicator status confirmation event.          */
               ProcessControlIndicatorStatusConfirmationEvent(((HFRM_Control_Indicator_Status_Confirmation_Message_t *)Message)->RemoteDeviceAddress, &(((HFRM_Control_Indicator_Status_Confirmation_Message_t *)Message)->ControlIndicatorEntry));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SUPPORT_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Hold/Multi-party Support Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_HOLD_MULTIPARTY_SUPPORT_CONFIRMATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the call*/
               /* hold/multi-party support confirmation event.          */
               ProcessCallHoldMultipartySupportConfirmationEvent(((HFRM_Call_Hold_Multiparty_Support_Confirmation_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Call_Hold_Multiparty_Support_Confirmation_Message_t *)Message)->CallHoldSupportMaskValid, ((HFRM_Call_Hold_Multiparty_Support_Confirmation_Message_t *)Message)->CallHoldSupportMask);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOTIFICATION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Waiting Notification Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_WAITING_NOTIFICATION_INDICATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_WAITING_NOTIFICATION_INDICATION_MESSAGE_SIZE(((HFRM_Call_Waiting_Notification_Indication_Message_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the call*/
               /* waiting notification indication event.                */
               ProcessCallWaitingNotificationIndicationEvent(((HFRM_Call_Waiting_Notification_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Call_Waiting_Notification_Indication_Message_t *)Message)->PhoneNumberLength, ((HFRM_Call_Waiting_Notification_Indication_Message_t *)Message)->PhoneNumber);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOTIFICATION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Line ID Notification Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_MESSAGE_SIZE(((HFRM_Call_Line_Identification_Notification_Indication_Message_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the call*/
               /* line ID notification indication event.                */
               ProcessCallLineIDNotificationIndicationEvent(((HFRM_Call_Line_Identification_Notification_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Call_Line_Identification_Notification_Indication_Message_t *)Message)->PhoneNumberLength, ((HFRM_Call_Line_Identification_Notification_Indication_Message_t *)Message)->PhoneNumber);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_RING_INDICATION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Ring Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_RING_INDICATION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the ring*/
               /* indication event.                                     */
               ProcessRingIndicationEvent(((HFRM_Ring_Indication_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_IN_BAND_RING_TONE_SETTING_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("In-band Ring Tone Setting Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_IN_BAND_RING_TONE_SETTING_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* in-band ring tone indication event.                   */
               ProcessInBandRingToneSettingIndicationEvent(((HFRM_In_Band_Ring_Tone_Setting_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_In_Band_Ring_Tone_Setting_Indication_Message_t *)Message)->Enabled);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Tag Request Confirmation Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_REQUEST_CONFIRMATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_REQUEST_CONFIRMATION_MESSAGE_SIZE(((HFRM_Voice_Tag_Request_Confirmation_Message_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* voice tag request confirmation event.                 */
               ProcessVoiceTagRequestConfirmationEvent(((HFRM_Voice_Tag_Request_Confirmation_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Voice_Tag_Request_Confirmation_Message_t *)Message)->PhoneNumberLength, ((HFRM_Voice_Tag_Request_Confirmation_Message_t *)Message)->PhoneNumber);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V1:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Calls List Confirmation Message (V1 - DEPRECATED)\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V1_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* current calls list confirmation event.                */
               ProcessQueryCurrentCallsListConfirmationEvent_v1(((HFRM_Query_Current_Calls_List_Confirmation_Message_v1_t *)Message)->RemoteDeviceAddress, &(((HFRM_Query_Current_Calls_List_Confirmation_Message_v1_t *)Message)->CallListEntry));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V2:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Calls List Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V2_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* current calls list confirmation event.                */
               ProcessQueryCurrentCallsListConfirmationEvent_v2(((HFRM_Query_Current_Calls_List_Confirmation_Message_v2_t *)Message)->RemoteDeviceAddress, &(((HFRM_Query_Current_Calls_List_Confirmation_Message_v2_t *)Message)->CallListEntry));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Network Operator Selection Confirmation Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_MESSAGE_SIZE(((HFRM_Network_Operator_Selection_Confirmation_Message_t *)Message)->NetworkOperatorLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* network operator selection confirmation event.        */
               ProcessNetworkOperatorSelectionConfirmationEvent(((HFRM_Network_Operator_Selection_Confirmation_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Network_Operator_Selection_Confirmation_Message_t *)Message)->NetworkMode, ((HFRM_Network_Operator_Selection_Confirmation_Message_t *)Message)->NetworkOperatorLength, ((HFRM_Network_Operator_Selection_Confirmation_Message_t *)Message)->NetworkOperator);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INFO_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Subscriber Number Information Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SUBSCRIBER_NUMBER_INFORMATION_CONFIRMATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* subscriber number information confirmation event.     */
               ProcessSubscriberNumberConfirmationEvent(((HFRM_Subscriber_Number_Information_Confirmation_Message_t *)Message)->RemoteDeviceAddress, &(((HFRM_Subscriber_Number_Information_Confirmation_Message_t *)Message)->SubscriberInformationEntry));

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Response/Hold Status Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_RESPONSE_HOLD_STATUS_CONFIRMATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* response/hold status confirmation event.              */
               ProcessResponseHoldStatusConfirmationEvent(((HFRM_Response_Hold_Status_Confirmation_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Response_Hold_Status_Confirmation_Message_t *)Message)->CallState);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_COMMAND_RESULT:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Command Result Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_COMMAND_RESULT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* command result event.                                 */
               ProcessCommandResultEvent(((HFRM_Command_Result_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Command_Result_Message_t *)Message)->ResultType, ((HFRM_Command_Result_Message_t *)Message)->ResultValue);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ARBITRARY_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Arbitrary Response Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ARBITRARY_RESPONSE_INDICATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ARBITRARY_RESPONSE_INDICATION_MESSAGE_SIZE(((HFRM_Arbitrary_Response_Indication_Message_t *)Message)->ArbitraryResponseLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* arbitrary response indication event.                  */
               ProcessArbitraryResponseIndicationEvent(((HFRM_Arbitrary_Response_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Arbitrary_Response_Indication_Message_t *)Message)->ArbitraryResponseLength, ((HFRM_Arbitrary_Response_Indication_Message_t *)Message)->ArbitraryResponse);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SELECT_CODEC_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Codec Select Indication Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CODEC_SELECT_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* codec select indication event.                        */
               ProcessCodecSelectIndicationEvent(((HFRM_Codec_Select_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Codec_Select_Indication_Message_t *)Message)->CodecID);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SELECTION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Hold/Multi-party Selection Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_HOLD_MULTIPARTY_SELECTION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the call*/
               /* hold/multi-party selection indication event.          */
               ProcessCallHoldMultipartySelectionIndicationEvent(((HFRM_Call_Hold_Multiparty_Selection_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Call_Hold_Multiparty_Selection_Indication_Message_t *)Message)->CallHoldMultipartyHandling, ((HFRM_Call_Hold_Multiparty_Selection_Indication_Message_t *)Message)->Index);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOT_ACTIVATION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Waiting Notification Activation Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_WAITING_NOTIFICATION_ACTIVATION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the call*/
               /* waiting notification activation indication event.     */
               ProcessCallWaitingNotificationActivationIndicationEvent(((HFRM_Call_Waiting_Notification_Activation_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Call_Waiting_Notification_Activation_Indication_Message_t *)Message)->Enabled);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOT_ACTIVATION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Line ID Notification Activation Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_ACTIVATION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the call*/
               /* line ID notification activation indication event.     */
               ProcessCallLineIDNotificationActivationIndicationEvent(((HFRM_Call_Line_Identification_Notification_Activation_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Call_Line_Identification_Notification_Activation_Indication_Message_t *)Message)->Enabled);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DISABLE_SOUND_ENHANCEMENT_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable Sound Enhancement Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DISABLE_SOUND_ENHANCEMENT_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* disable sound enhancement indication event.           */
               ProcessDisableSoundEnhancmentIndicationEvent(((HFRM_Disable_Sound_Enhancement_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Dial Phone Number Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_INDICATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_INDICATION_MESSAGE_SIZE(((HFRM_Dial_Phone_Number_Indication_Message_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the dial*/
               /* phone number indication event.                        */
               ProcessDialPhoneNumberIndicationEvent(((HFRM_Dial_Phone_Number_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Dial_Phone_Number_Indication_Message_t *)Message)->PhoneNumberLength, ((HFRM_Dial_Phone_Number_Indication_Message_t *)Message)->PhoneNumber);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEM_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Dial Phone Number (from memory) Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the dial*/
               /* phone number (from memory) indication event.          */
               ProcessDialPhoneNumberFromMemoryIndicationEvent(((HFRM_Dial_Phone_Number_From_Memory_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Dial_Phone_Number_From_Memory_Indication_Message_t *)Message)->MemoryLocation);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Re-dial Last Phone Number Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_RE_DIAL_LAST_PHONE_NUMBER_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* re-dial last phone number indication event.           */
               ProcessReDialLastPhoneNumberIndicationEvent(((HFRM_Re_Dial_Last_Phone_Number_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_GENERATE_DTMF_CODE_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Generate DTMF Code Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_GENERATE_DTMF_CODE_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* generate DTMF code indication event.                  */
               ProcessGenerateDTMFCodeIndicationEvent(((HFRM_Generate_DTMF_Code_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Generate_DTMF_Code_Indication_Message_t *)Message)->DTMFCode);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ANSWER_CALL_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Answer Call Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ANSWER_CALL_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* answer call indication event.                         */
               ProcessAnswerCallIndicationEvent(((HFRM_Answer_Call_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Tag Request Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_REQUEST_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* voice tag request indication event.                   */
               ProcessVoiceTagRequestIndicationEvent(((HFRM_Voice_Tag_Request_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_HANG_UP_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hang-up Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_HANG_UP_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* hang-up indication event.                             */
               ProcessHangUpIndicationEvent(((HFRM_Hang_Up_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Calls List Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CALLS_LIST_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query current calls list indication event.            */
               ProcessQueryCurrentCallsListIndicationEvent(((HFRM_Query_Current_Calls_List_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_FORMAT_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Network Operator Selection Format Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_NETWORK_OPERATOR_SELECTION_FORMAT_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* network operator selection format indication event.   */
               ProcessNetworkOperatorSelectionFormatIndicationEvent(((HFRM_Network_Operator_Selection_Format_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Network_Operator_Selection_Format_Indication_Message_t *)Message)->Format);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Network Operator Selection Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_NETWORK_OPERATOR_SELECTION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* network operator selection indication event.          */
               ProcessNetworkOperatorSelectionIndicationEvent(((HFRM_Network_Operator_Selection_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_EXTENDED_ERROR_RESULT_ACT_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Extended Error Result Activation Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_EXTENDED_ERROR_RESULT_ACTIVATION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* extended error result activation indication event.    */
               ProcessExtendedErrorResultActivationIndicationEvent(((HFRM_Extended_Error_Result_Activation_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Extended_Error_Result_Activation_Indication_Message_t *)Message)->Enabled);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INF_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Subscriber Number Information Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SUBSCRIBER_NUMBER_INFORMATION_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* subscriber number information indication event.       */
               ProcessSubscriberNumberInformationIndicationEvent(((HFRM_Subscriber_Number_Information_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Response/Hold Status Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_RESPONSE_HOLD_STATUS_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* response/hold status indication event.                */
               ProcessResponseHoldStatusIndicationEvent(((HFRM_Response_Hold_Status_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ARBITRARY_COMMAND_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Arbitrary Command Indication Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ARBITRARY_COMMAND_INDICATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ARBITRARY_COMMAND_INDICATION_MESSAGE_SIZE(((HFRM_Arbitrary_Command_Indication_Message_t *)Message)->ArbitraryCommandLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* arbitrary command indication event.                   */
               ProcessArbitraryCommandIndicationEvent(((HFRM_Arbitrary_Command_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Arbitrary_Command_Indication_Message_t *)Message)->ArbitraryCommandLength, ((HFRM_Arbitrary_Command_Indication_Message_t *)Message)->ArbitraryCommand);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_AVAILABLE_CODEC_LIST_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Available Codec List Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_AVAILABLE_CODEC_LIST_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* available codec list indication event.                */
               ProcessAvailableCodecListIndicationEvent(((HFRM_Available_Codec_List_Indication_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Available_Codec_List_Indication_Message_t *)Message)->NumberSupportedCodecs, ((HFRM_Available_Codec_List_Indication_Message_t *)Message)->AvailableCodecList);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SELECT_CODEC_CFM:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Codec Select Confirmation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CODEC_SELECT_CONFIRMATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* codec select confirmation event.                      */
               ProcessCodecSelectConfirmationEvent(((HFRM_Codec_Select_Confirmation_Message_t *)Message)->RemoteDeviceAddress, ((HFRM_Codec_Select_Confirmation_Message_t *)Message)->CodecID);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CONNECT_CODEC_IND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Codec Connect Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CODEC_CONNECTION_SETUP_INDICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* codec connect indication event.                       */
               ProcessCodecConnectIndicationEvent(((HFRM_Codec_Connection_Setup_Indication_Message_t *)Message)->RemoteDeviceAddress);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(HFREManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Hands Free Manager Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_HFRM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Hands Free state          */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   unsigned int       Index;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Hands Free state          */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            Index = 2;
            while(Index--)
            {
               if(Index)
                  HFREEntryInfo = HFREEntryInfoList_AG;
               else
                  HFREEntryInfo = HFREEntryInfoList_HF;

               while(HFREEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((!(HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HFREEntryInfo->ConnectionEvent))
                  {
                     HFREEntryInfo->ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(HFREEntryInfo->ConnectionEvent);
                  }

                  HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
               }
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Hands Free Manager      */
   /* Messages.                                                         */
static void BTPSAPI HandsFreeManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a Hands Free Manager     */
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
               /* Hands Free Manager thread.                            */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HFRM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Hands Free Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Hands Free Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Hands Free Manager    */
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
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Hands Free Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Hands Free Manager module.  This   */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI HFRM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Hands Free Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((HFREManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Hands Free Manager messages.                  */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER, HandsFreeManagerGroupHandler, NULL))
            {
               /* Initialize the actual Hands Free Manager              */
               /* Implementation Module (this is the module that is     */
               /* actually responsible for actually implementing the    */
               /* HFRE Manager functionality - this module is just the  */
               /* framework shell).                                     */
               if(!(Result = _HFRM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the Hands Free Manager  */
                  /* Server.                                            */
                  Result = _HFRM_Register_Events(hctAudioGateway, FALSE);

                  if((Result > 0) || (Result == BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED))
                  {
                     if(Result > 0)
                        HFREEventsCallbackID_AG = (unsigned int)Result;
                     else
                        HFREEventsCallbackID_AG = 0;

                     Result = _HFRM_Register_Events(hctHandsFree, FALSE);

                     if((Result > 0) || (Result == BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED))
                     {
                        if(Result > 0)
                           HFREEventsCallbackID_HF = (unsigned int)Result;
                        else
                           HFREEventsCallbackID_HF = 0;
                     }
                  }

                  if((HFREEventsCallbackID_AG) || (HFREEventsCallbackID_HF))
                  {
                     /* Initialize a unique, starting Hands Free        */
                     /* Callback ID.                                    */
                     NextCallbackID = 0x000000001;

                     /* Go ahead and flag that this module is           */
                     /* initialized.                                    */
                     Initialized    = TRUE;
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
            if(HFREEventsCallbackID_AG)
               _HFRM_Un_Register_Events(HFREEventsCallbackID_AG);

            if(HFREEventsCallbackID_HF)
               _HFRM_Un_Register_Events(HFREEventsCallbackID_HF);

            if(HFREManagerMutex)
               BTPS_CloseMutex(HFREManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER);

            /* Flag that none of the resources are allocated.           */
            HFREManagerMutex        = NULL;
            HFREEventsCallbackID_AG = 0;
            HFREEventsCallbackID_HF = 0;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("HFRE Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for Hands Free Events.                       */
            if(HFREEventsCallbackID_AG)
               _HFRM_Un_Register_Events(HFREEventsCallbackID_AG);

            if(HFREEventsCallbackID_HF)
               _HFRM_Un_Register_Events(HFREEventsCallbackID_HF);

            /* Next, Un-Register for any Hands Free control events.     */
            if(HFREEntryInfoList_AG_Control)
               _HFRM_Un_Register_Events(HFREEntryInfoList_AG_Control->CallbackID);

            if(HFREEntryInfoList_HF_Control)
               _HFRM_Un_Register_Events(HFREEntryInfoList_HF_Control->CallbackID);

            /* Next, Un-Register for any Hands Free data events.        */
            if(HFREEntryInfoList_AG_Data)
               _HFRM_Un_Register_Data_Events(HFREEntryInfoList_AG_Data->CallbackID);

            if(HFREEntryInfoList_HF_Data)
               _HFRM_Un_Register_Data_Events(HFREEntryInfoList_HF_Data->CallbackID);

            /* Make sure we inform the Hands Free Manager Implementation*/
            /* that we are shutting down.                               */
            _HFRM_Cleanup();

            BTPS_CloseMutex(HFREManagerMutex);

            /* Make sure that the Hands Free entry information list is  */
            /* empty.                                                   */
            FreeHFREEntryInfoList(&HFREEntryInfoList_AG);
            FreeHFREEntryInfoList(&HFREEntryInfoList_HF);

            /* Make sure that the Hands Free entry control information  */
            /* list is empty.                                           */
            FreeHFREEntryInfoList(&HFREEntryInfoList_AG_Control);
            FreeHFREEntryInfoList(&HFREEntryInfoList_HF_Control);

            /* Make sure that the Hands Free entry data information list*/
            /* is empty.                                                */
            FreeHFREEntryInfoList(&HFREEntryInfoList_AG_Data);
            FreeHFREEntryInfoList(&HFREEntryInfoList_HF_Data);

            /* Flag that the resources are no longer allocated.         */
            HFREManagerMutex        = NULL;
            CurrentPowerState       = FALSE;
            HFREEventsCallbackID_AG = 0;
            HFREEventsCallbackID_HF = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HFRM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   unsigned int       Index;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* Hands Free entries and set any events that have       */
               /* synchronous operations pending.                       */
               Index = 2;
               while(Index--)
               {
                  if(Index)
                     HFREEntryInfo = HFREEntryInfoList_AG;
                  else
                     HFREEntryInfo = HFREEntryInfoList_HF;

                  while(HFREEntryInfo)
                  {
                     /* Check to see if there is a synchronous open     */
                     /* operation.                                      */
                     if((!(HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (HFREEntryInfo->ConnectionEvent))
                     {
                        HFREEntryInfo->ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                        BTPS_SetEvent(HFREEntryInfo->ConnectionEvent);
                     }

                     HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
                  }
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HFREManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Hands Free Manager Connection Management Functions.               */

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server. This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened. A   */
   /*          hetHFRConnected event will notify if the connection is   */
   /*          successful.                                              */
int BTPSAPI HFRM_Connection_Request_Response(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that there is someone to handle the events.             */
      if((ConnectionType == hctAudioGateway)?HFREEntryInfoList_AG_Control:HFREEntryInfoList_HF_Control)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* respond to the Connection Request.                          */
         ret_val = _HFRM_Connection_Request_Response(ConnectionType, RemoteDeviceAddress, AcceptConnection);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_FEATURE_ACTION;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Hands Free/Audio Gateway device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.  This function accepts the      */
   /* connection type to make as the first parameter.  This parameter   */
   /* specifies the LOCAL connection type (i.e. if the caller would     */
   /* like to connect the local Hands Free service to a remote Audio    */
   /* Gateway device, the Hands Free connection type would be specified */
   /* for this parameter).  This function also accepts the connection   */
   /* information for the remote device (address and server port).      */
   /* This function accepts the connection flags to apply to control    */
   /* how the connection is made regarding encryption and/or            */
   /* authentication.                                                   */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Hands Free Manager Connection Status Event (if           */
   /*          specified).                                              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHFRConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HFRM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI HFRM_Connect_Remote_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                ret_val;
   Event_t            ConnectionEvent;
   BD_ADDR_t          NULL_BD_ADDR;
   unsigned int       CallbackID;
   HFRE_Entry_Info_t  HFREEntryInfo;
   HFRE_Entry_Info_t *HFREEntryInfoPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that there is someone to handle the events.             */
      if((ConnectionType == hctAudioGateway)?HFREEntryInfoList_AG_Control:HFREEntryInfoList_HF_Control)
      {
         /* Next, verify that the input parameters appear to be         */
         /* semi-valid.                                                 */
         if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
         {
            /* Attempt to wait for access to the Hands Free Manager     */
            /* state information.                                       */
            if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
            {
               /* Next, check to see if we are powered up.              */
               if(CurrentPowerState)
               {
                  /* Device is powered on, attempt to add an entry into */
                  /* the Hands Free entry list.                         */
                  BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

                  HFREEntryInfo.CallbackID        = GetNextCallbackID();
                  HFREEntryInfo.EventCallback     = CallbackFunction;
                  HFREEntryInfo.CallbackParameter = CallbackParameter;
                  HFREEntryInfo.ConnectionType    = ConnectionType;
                  HFREEntryInfo.BD_ADDR           = RemoteDeviceAddress;

                  if(ConnectionStatus)
                     HFREEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

                  if((!ConnectionStatus) || ((ConnectionStatus) && (HFREEntryInfo.ConnectionEvent)))
                  {
                     if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, &HFREEntryInfo)) != NULL)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote device %d 0x%08lX\n", RemoteServerPort, ConnectionFlags));

                        /* Next, attempt to open the remote device.     */
                        if((ret_val = _HFRM_Connect_Remote_Device(ConnectionType, RemoteDeviceAddress, RemoteServerPort, ConnectionFlags)) != 0)
                        {
                           /* Error opening device, go ahead and delete */
                           /* the entry that was added.                 */
                           if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, HFREEntryInfoPtr->CallbackID)) != NULL)
                           {
                              if(HFREEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(HFREEntryInfoPtr->ConnectionEvent);

                              FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);
                           }
                        }

                        /* Next, determine if the caller has requested a*/
                        /* blocking open.                               */
                        if((!ret_val) && (ConnectionStatus))
                        {
                           /* Blocking open, go ahead and wait for the  */
                           /* event.                                    */

                           /* Note the Callback ID.                     */
                           CallbackID      = HFREEntryInfoPtr->CallbackID;

                           /* Note the open event.                      */
                           ConnectionEvent = HFREEntryInfoPtr->ConnectionEvent;

                           /* Release the Mutex because we are finished */
                           /* with it.                                  */
                           BTPS_ReleaseMutex(HFREManagerMutex);

                           BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                           /* Re-acquire the Mutex.                     */
                           if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
                           {
                              if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, CallbackID)) != NULL)
                              {
                                 /* Note the connection status.         */
                                 *ConnectionStatus = HFREEntryInfoPtr->ConnectionStatus;

                                 BTPS_CloseEvent(HFREEntryInfoPtr->ConnectionEvent);

                                 FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);

                                 /* Flag success to the caller.         */
                                 ret_val = 0;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                        }
                        else
                        {
                           /* If we are not tracking this connection OR */
                           /* there was an error, go ahead and delete   */
                           /* the entry that was added.                 */
                           if((!CallbackFunction) || (ret_val))
                           {
                              if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, HFREEntryInfo.CallbackID)) != NULL)
                              {
                                 if(HFREEntryInfoPtr->ConnectionEvent)
                                    BTPS_CloseEvent(HFREEntryInfoPtr->ConnectionEvent);

                                 FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);
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

               /* Release the Mutex because we are finished with it.    */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
                  BTPS_ReleaseMutex(HFREManagerMutex);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_FEATURE_ACTION;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Hands Free or    */
   /* Audio Gateway connection that was previously opened by any of the */
   /* following mechanisms:                                             */
   /*   - Successful call to HFRM_Connect_Remote_Device() function.     */
   /*   - Incoming open request (Hands Free or Audio Gateway) which was */
   /*     accepted either automatically or by a call to                 */
   /*     HFRM_Connection_Request_Response().                           */
   /* This function accepts as input the type of the local connection   */
   /* which should close its active connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.  This function does NOT un-register any Hands Free or Audio*/
   /* Gateway services from the system, it ONLY disconnects any         */
   /* connection that is currently active on the specified service.     */
int BTPSAPI HFRM_Disconnect_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that there is someone to handle the events.             */
      if((ConnectionType == hctAudioGateway)?HFREEntryInfoList_AG_Control:HFREEntryInfoList_HF_Control)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* disconnect the remote device.                               */
         ret_val = _HFRM_Disconnect_Device(ConnectionType, RemoteDeviceAddress);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_FEATURE_ACTION;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Hands   */
   /* Free or Audio Gateway Devices (specified by the first parameter). */
   /* This function accepts a the local service type to query, followed */
   /* by buffer information to receive any currently connected device   */
   /* addresses of the specified connection type.  The first parameter  */
   /* specifies the local service type to query the connection          */
   /* information for.  The second parameter specifies the maximum      */
   /* number of BD_ADDR entries that the buffer will support (i.e.  can */
   /* be copied into the buffer).  The next parameter is optional and,  */
   /* if specified, will be populated with the total number of connected*/
   /* devices if the function is successful.  The final parameter can be*/
   /* used to retrieve the total number of connected devices (regardless*/
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of connected devices that were copied into  */
   /* the specified input buffer.  This function returns a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI HFRM_Query_Connected_Devices(HFRM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* query the connected Devices.                                   */
      ret_val = _HFRM_Query_Connected_Devices(ConnectionType, MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Hands Free or Audio*/
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * On input the TotalNumberAdditionalIndicators member of   */
   /*          the structure should be set to the total number of       */
   /*          additional indicator entry structures that the           */
   /*          AdditionalIndicatorList member points to.  On return from*/
   /*          this function this structure member holds the total      */
   /*          number of additional indicator entries that are supported*/
   /*          by the connection.  The NumberAdditionalIndicators       */
   /*          member will hold (on return) the number of indicator     */
   /*          entries that are actually present in the list.           */
   /* * NOTE * It is possible to not query the additional indicators    */
   /*          by passing zero for the TotalNumberAdditionalIndicators  */
   /*          member (the AdditionalIndicatorList member will be       */
   /*          ignored in this case).  This member will still hold the  */
   /*          total supported indicators on return of this function,   */
   /*          however, no indicators will be returned.                 */
int BTPSAPI HFRM_Query_Current_Configuration(HFRM_Connection_Type_t ConnectionType, HFRM_Current_Configuration_t *CurrentConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* query the current configuration.                               */
      ret_val = _HFRM_Query_Current_Configuration(ConnectionType, CurrentConfiguration);
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Hands Free and*/
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI HFRM_Change_Incoming_Connection_Flags(HFRM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* change the current connection flags.                           */
      ret_val = _HFRM_Change_Incoming_Connection_Flags(ConnectionType, ConnectionFlags);
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Shared Hands Free/Audio Gateway Functions.                        */

   /* This function is responsible for disabling echo cancellation and  */
   /* noise reduction on the remote device.  This function may be       */
   /* performed by both the Hands Free and the Audio Gateway connections*/
   /* for which a valid service level connection exists but no audio    */
   /* connection exists.  This function accepts as its input parameter  */
   /* the connection type indicating which local service which will send*/
   /* this command.  This function returns zero if successful or a      */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * It is not possible to enable this feature once it has    */
   /*          been disbled because the specification provides no means */
   /*          to re-enable this feature.  This feature will remained   */
   /*          disabled until the current service level connection has  */
   /*          been dropped.                                            */
int BTPSAPI HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to disable remote echo cancellation/noise    */
               /* reduction.                                            */
               ret_val = _HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* When called by a Hands Free device, this function is responsible  */
   /* for requesting activation or deactivation of the voice recognition*/
   /* which resides on the remote Audio Gateway.  When called by an     */
   /* Audio Gateway, this function is responsible for informing the     */
   /* remote Hands Free device of the current activation state of the   */
   /* local voice recognition function.  This function may only be      */
   /* called by local devices that were opened with support for voice   */
   /* recognition.  This function accepts as its input parameters the   */
   /* connection type indicating the local connection which will process*/
   /* the command and a BOOLEAN flag specifying the type of request or  */
   /* notification to send.  When active the voice recognition function */
   /* on the Audio Gateway is turned on, when inactive the voice        */
   /* recognition function on the Audio Gateway is turned off.  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Set_Remote_Voice_Recognition_Activation(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t VoiceRecognitionActive)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set the remote voice recoginition         */
               /* activation setting.                                   */
               ret_val = _HFRM_Set_Remote_Voice_Recognition_Activation(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress, VoiceRecognitionActive);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  This function may    */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current speaker   */
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the speaker gain*/
   /* of the remote Hands Free device.  This function accepts as its    */
   /* input parameters the connection type indicating the local         */
   /* connection which will process the command and the speaker gain to */
   /* be sent to the remote device.  The speaker gain Parameter *MUST*  */
   /* be between the values:                                            */
   /*                                                                   */
   /*    HFRE_SPEAKER_GAIN_MINIMUM                                      */
   /*    HFRE_SPEAKER_GAIN_MAXIMUM                                      */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Set_Remote_Speaker_Gain(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
                /* Nothing to do here other than to call the actual     */
                /* function to set the remote speaker gain.             */
                ret_val = _HFRM_Set_Remote_Speaker_Gain(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress, SpeakerGain);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  This function may */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current microphone*/
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the microphone  */
   /* gain of the remote Hands Free device.  This function accepts as   */
   /* its input parameters the connection type indicating the local     */
   /* connection which will process the command and the microphone gain */
   /* to be sent to the remote device.  The microphone gain Parameter   */
   /* *MUST* be between the values:                                     */
   /*                                                                   */
   /*    HFRE_MICROPHONE_GAIN_MINIMUM                                   */
   /*    HFRE_MICROPHONE_GAIN_MAXIMUM                                   */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Set_Remote_Microphone_Gain(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
                /* Nothing to do here other than to call the actual     */
                /* function to set the remote microphone gain.          */
                ret_val = _HFRM_Set_Remote_Microphone_Gain(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress, MicrophoneGain);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Send a codec ID. The audio gateway uses this function to send the */
   /* preferred codec. The hands free device uses the function to       */
   /* confirm the audio gateway's choice. The EventCallback ID is       */
   /* returned from HFRM_Register_Event_Callback(). ConnectionType      */
   /* indicates whether the local role is audio gateway or hands free.  */
   /* RemoteDeviceAddress is the address of the remote device. CodecId  */
   /* identifies the selected codec. This function returns zero if      */
   /* successful or a negative code in case of error.                   */
int BTPSAPI HFRM_Send_Select_Codec(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HandsFree Manager has been          */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* HandsFree Manager has been initialized, let's check the input  */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (CodecID))
      {
         /* Attempt to wait for access to the HandsFree Manager state   */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the preferred codec.                 */
               ret_val = _HFRM_Send_Select_Codec(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress, CodecID);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Hands Free Functions.                                             */

   /* The following function is responsible for querying the remote     */
   /* control indicator status.  This function may only be performed by */
   /* a local Hands Free unit with a valid service level connection to a*/
   /* connected remote Audio Gateway.  The results to this query will be*/
   /* returned as part of the control indicator status confirmation     */
   /* event (hetHFRControlIndicatorStatus).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Query_Remote_Control_Indicator_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
                /* Nothing to do here other than to call the actual     */
                /* function to query the remote control indicator       */
                /* status.                                              */
                ret_val = _HFRM_Query_Remote_Control_Indicator_Status(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling the        */
   /* indicator event notification on a remote Audio Gateway.  This     */
   /* function may only be performed by Hands Free devices that have a  */
   /* valid service level connection to a connected remote Audio        */
   /* Gateway.  When enabled, the remote Audio Gateway device will send */
   /* unsolicited responses to update the local device of the current   */
   /* control indicator values.  This function accepts as its input     */
   /* parameter a BOOLEAN flag used to enable or disable event          */
   /* notification.  This function returns zero if successful or a      */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Enable_Remote_Indicator_Event_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableEventNotification)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable/disable remote indicator event     */
               /* notification.                                         */
               ret_val = _HFRM_Enable_Remote_Indicator_Event_Notification(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, EnableEventNotification);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Updating the Remote     */
   /* Indicator State.  This function may only be performed by          */
   /* Hands-Free units that have a valid Service Level Connection.  This*/
   /* function accepts as its input parameters the Bluetooth Stack ID   */
   /* for which the HFRE Port ID is valid as well as the HFRE Port ID.  */
   /* The third parameter to this function is the number of name/state  */
   /* pairs that are present in the list.  The final parameter to this  */
   /* function is a list of name/state pairs for the indicators to be   */
   /* updated.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Update_Remote_Indicator_Notification_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberUpdateIndicators, HFRE_Notification_Update_t UpdateIndicators[])
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberUpdateIndicators) && (UpdateIndicators))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query remote call holding/multi-party     */
               /* service support.                                      */
               ret_val = _HFRM_Update_Remote_Indicator_Notification_State(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, NumberUpdateIndicators, UpdateIndicators);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for querying the call holding and    */
   /* multi-party services which are supported by the remote Audio      */
   /* Gateway.  This function is used by Hands Free connections which   */
   /* support three way calling and call waiting to determine the       */
   /* features supported by the remote Audio Gateway.  This function can*/
   /* only be used if a valid service level connection to a connected   */
   /* remote Audio Gateway exists.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query remote call holding/multi-party     */
               /* service support.                                      */
               ret_val = _HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing the control of multiple */
   /* concurrent calls and provides a means for holding calls, releasing*/
   /* calls, switching between two calls and adding a call to a         */
   /* multi-party conference.  This function may only be performed by   */
   /* Hands Free units that support call waiting and multi-party        */
   /* services as well as have a valid service level connection to a    */
   /* connected remote Audio Gateway.  The selection which is made      */
   /* should be one that is supported by the remote Audio Gateway       */
   /* (queried via a call to the                                        */
   /* HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support()       */
   /* function).  This function accepts as its input parameter the      */
   /* selection of how to handle the currently waiting call.  If the    */
   /* selected handling type requires an index it should be provided in */
   /* the last parameter.  Otherwise the final paramter is ignored.     */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Call_Holding_Multiparty_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the call holding/multi-party         */
               /* selection.                                            */
               ret_val = _HFRM_Send_Call_Holding_Multiparty_Selection(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, CallHoldMultipartyHandling, Index);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling call       */
   /* waiting notification on a remote Audio Gateway.  By default the   */
   /* call waiting notification is enabled in the network but disabled  */
   /* for notification via the service level connection (between Hands  */
   /* Free and Audio Gateway).  This function may only be performed by a*/
   /* Hands Free unit for which a valid service level connection to a   */
   /* connected remote Audio Gateway exists.  This function may only be */
   /* used to enable call waiting notifications if the local Hands Free */
   /* service supports call waiting and multi-party services.  This     */
   /* function accepts as its input parameter a BOOLEAN flag specifying */
   /* if this is a call to enable or disable this functionality.  This  */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Enable_Remote_Call_Waiting_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableNotification)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable/disable remote call waiting        */
               /* notifications.                                        */
               ret_val = _HFRM_Enable_Remote_Call_Waiting_Notification(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, EnableNotification);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling call line  */
   /* identification notification on a remote Audio Gateway.  By        */
   /* default, the call line identification notification via the service*/
   /* level connection is disabled.  This function may only be performed*/
   /* by Hands Free units for which a valid service level connection to */
   /* a connected remote Audio Gateway exists.  This function may only  */
   /* be used to enable call line notifications if the local Hands Free */
   /* unit supports call line identification.  This function accepts as */
   /* its input parameters a BOOLEAN flag specifying if this is a call  */
   /* to enable or disable this functionality.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Enable_Remote_Call_Line_Identification_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableNotification)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable remote call line identification    */
               /* notifications.                                        */
               ret_val = _HFRM_Enable_Remote_Call_Line_Identification_Notification(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, EnableNotification);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for dialing a phone number on a      */
   /* remote Audio Gateway.  This function may only be performed by     */
   /* Hands Free units for which a valid service level connection to a  */
   /* remote Audio Gateway exists.  This function accepts as its input  */
   /* parameter the phone number to dial on the remote Audio Gateway.   */
   /* This parameter should be a pointer to a NULL terminated string and*/
   /* its length *MUST* be between the values of:                       */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Dial_Phone_Number(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to dial the specified phone number.          */
               ret_val = _HFRM_Dial_Phone_Number(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, PhoneNumber);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for dialing a phone number from a    */
   /* memory location (index) found on the remote Audio Gateway.  This  */
   /* function may only be performed by Hands Free devices for which a  */
   /* valid service level connection to a connected remote Audio Gateway*/
   /* exists.  This function accepts as its input parameter the memory  */
   /* location (index) for which the phone number to dial already exists*/
   /* on the remote Audio Gateway.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Dial_Phone_Number_From_Memory(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MemoryLocation)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to dial the specified phone number (from     */
               /* memory location).                                     */
               ret_val = _HFRM_Dial_Phone_Number_From_Memory(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, MemoryLocation);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for re-dialing the last number dialed*/
   /* on a remote Audio Gateway.  This function may only be performed by*/
   /* Hands Free devices for which a valid service level connection to a*/
   /* connected remote Audio Gateway exists.  This function returns zero*/
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Redial_Last_Phone_Number(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to re-dial the last dialed phone number.     */
               ret_val = _HFRM_Redial_Last_Phone_Number(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending the command to a remote  */
   /* Audi Gateway to answer an incoming call.  This function may only  */
   /* be performed by Hands Free devices for which a valid service level*/
   /* connection to a connected remote Audio Gateway exists.  This      */
   /* function return zero if successful or a negative return error code*/
   /* if there was an error.                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Answer_Incoming_Call(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to answer an incoming call.                  */
               ret_val = _HFRM_Answer_Incoming_Call(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for transmitting DTMF codes to a     */
   /* remote Audio Gateway to be sent as a DTMF code over an on-going   */
   /* call.  This function may only be performed by Hands Free devices  */
   /* for which a valid service level connection to a connected remote  */
   /* Audio Gateway exists and an on-going call exists.  This function  */
   /* accepts as input the DTMF code to be transmitted.  This Code must */
   /* be one of the characters:                                         */
   /*                                                                   */
   /*   0-9, *, #, or A-D.                                              */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Transmit_DTMF_Code(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char DTMFCode)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to transmit a DTMF code.                     */
               ret_val = _HFRM_Transmit_DTMF_Code(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, DTMFCode);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for retrieving a phone number to     */
   /* associate with a unique voice tag to be stored in memory by the   */
   /* local Hands Free device.  This function may only be performed by a*/
   /* Hands Free device for which a valid service level connection to a */
   /* connected remote Audio Gateway exists.  The Hands Free unit must  */
   /* also support voice recognition to be able to use this function.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * When this function is called no other function may be    */
   /*          called until a voice tag response is received from the   */
   /*          remote Audio Gateway.                                    */
int BTPSAPI HFRM_Voice_Tag_Request(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to issue a voice tag request.                */
               ret_val = _HFRM_Voice_Tag_Request(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a hang-up command to a   */
   /* remote Audio Gateway.  This function may be used to reject an     */
   /* incoming call or to terminate an on-going call.  This function may*/
   /* only be performed by Hands Free devices for which a valid service */
   /* level connection exists.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Hang_Up_Call(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to hang up an on-going call.                 */
               ret_val = _HFRM_Hang_Up_Call(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for querying the current    */
   /* call list of the remote Audio Gateway device.  This function may  */
   /* only be performed by a Hands Free device with a valid service     */
   /* level connection to a connected Audio Gateway.  This function     */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Query_Remote_Current_Calls_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query the current calls list.             */
               ret_val = _HFRM_Query_Remote_Current_Calls_List(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for setting the network     */
   /* operator format to long alphanumeric.  This function may only be  */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected Audio Gateway.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Set_Network_Operator_Selection_Format(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set the network operator selection format.*/
               ret_val = _HFRM_Set_Network_Operator_Selection_Format(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for reading the network     */
   /* operator.  This function may only be performed by a Hands Free    */
   /* device with a valid service level connection.  This function      */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The network operator format must be set before querying  */
   /*          the current network operator.                            */
int BTPSAPI HFRM_Query_Remote_Network_Operator_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query the network operator selection.     */
               ret_val = _HFRM_Query_Remote_Network_Operator_Selection(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* extended error results reporting.  This function may only be      */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* accepts as its input parameter a BOOLEAN flag indicating whether  */
   /* the reporting should be enabled (TRUE) or disabled (FALSE).  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Enable_Remote_Extended_Error_Result(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableExtendedErrorResults)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable extended error result reporting.   */
               ret_val = _HFRM_Enable_Remote_Extended_Error_Result(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, EnableExtendedErrorResults);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for retrieving the          */
   /* subscriber number information.  This function may only be         */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Query_Subscriber_Number_Information(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query the subscriber number information.  */
               ret_val = _HFRM_Query_Subscriber_Number_Information(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for retrieving the current  */
   /* response and hold status.  This function may only be performed by */
   /* a Hands Free device with a valid service level connection to a    */
   /* connected Audio Gateway.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Query_Response_Hold_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query the response/hold status.           */
               ret_val = _HFRM_Query_Response_Hold_Status(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for setting the state of an */
   /* incoming call.  This function may only be performed by a Hands    */
   /* Free unit with a valid service level connection to a remote Audio */
   /* Gateway.  This function accepts as its input parameter the call   */
   /* state to set as part of this message.  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Set_Incoming_Call_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set the incoming call state.              */
               ret_val = _HFRM_Set_Incoming_Call_State(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, CallState);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an arbitrary    */
   /* command to the remote Audio Gateway (i.e.  non Bluetooth Hands    */
   /* Free Profile command).  This function may only be performed by a  */
   /* Hands Free with a valid service level connection.  This function  */
   /* accepts as its input parameter a NULL terminated ASCII string that*/
   /* represents the arbitrary command to send.  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The Command string passed to this function *MUST* begin  */
   /*          with AT and *MUST* end with the a carriage return ('\r') */
   /*          if this is the first portion of an arbitrary command     */
   /*          that will span multiple writes.  Subsequent calls (until */
   /*          the actual status reponse is received) can begin with    */
   /*          any character, however, they must end with a carriage    */
   /*          return ('\r').                                           */
int BTPSAPI HFRM_Send_Arbitrary_Command(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryCommand)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ArbitraryCommand) && (BTPS_StringLength(ArbitraryCommand)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified arbitrary command.     */
               ret_val = _HFRM_Send_Arbitrary_Command(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, ArbitraryCommand);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Send the list of supported codecs to a remote audio gateway.      */
   /* The EventCallback ID is returned from a successful call to        */
   /* HFRM_Register_Event_Callback(). RemoteDeviceAddress is the        */
   /* address of the remote audio gateway. NumberSupportedCodecs is the */
   /* number of codecs in the list. AvailableCodecList is the codec     */
   /* list. This function returns zero if successful or a negative code */
   /* in case of error.                                                 */
int BTPSAPI HFRM_Send_Available_Codec_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HandsFree Manager has been          */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* HandsFree Manager has been initialized, let's check the input  */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberSupportedCodecs) && (AvailableCodecList))
      {
         /* Attempt to wait for access to the HandsFree Manager state   */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the codec list.                      */
               ret_val = _HFRM_Send_Available_Codec_List(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, NumberSupportedCodecs, AvailableCodecList);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Audio Gateway Functions.                                          */

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway with a valid service level connection to a       */
   /* connected remote Hands Free device.  This function accepts as its */
   /* input parameters the number of indicators and list of name/value  */
   /* pairs for the indicators to be updated.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Update_Current_Control_Indicator_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberUpdateIndicators, HFRE_Indicator_Update_t *UpdateIndicatorList)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberUpdateIndicators) && (UpdateIndicatorList))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to update the specified indicators.          */
               ret_val = _HFRM_Update_Current_Control_Indicator_Status(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, NumberUpdateIndicators, UpdateIndicatorList);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway.  The function will initially set the specified  */
   /* indicator, then, if a valid service level connection exists and   */
   /* event reporting is activated (via the set remote event indicator  */
   /* event notification function by the remote device) an event        */
   /* notification will be sent to the remote device.  This function    */
   /* accepts as its input parameters the name of the indicator to be   */
   /* updated and the new indicator value.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Update_Current_Control_Indicator_Status_By_Name(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *IndicatorName, unsigned int IndicatorValue)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (IndicatorName) && (BTPS_StringLength(IndicatorName)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to update the specified indicator status (by */
               /* name).                                                */
               ret_val = _HFRM_Update_Current_Control_Indicator_Status_By_Name(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, IndicatorName, IndicatorValue);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a call waiting           */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call waiting       */
   /* notification enabled and have a valid service level connection to */
   /* a connected remote Hands Free device.  This function accepts as   */
   /* its input parameter the phone number of the incoming call, if a   */
   /* number is available.  This parameter should be a pointer to a NULL*/
   /* terminated ASCII string (if specified) and must have a length less*/
   /* than:                                                             */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * It is valid to either pass a NULL for the PhoneNumber    */
   /*          parameter or a blank string to specify that there is no  */
   /*          phone number present.                                    */
int BTPSAPI HFRM_Send_Call_Waiting_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!PhoneNumber) || ((PhoneNumber) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the call waiting notification.       */
               ret_val = _HFRM_Send_Call_Waiting_Notification(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, PhoneNumber);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending call line identification */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call line          */
   /* identification notification enabled and have a valid service level*/
   /* connection to a connected remote Hands Free device.  This function*/
   /* accepts as its input parameters the phone number of the incoming  */
   /* call.  This parameter should be a pointer to a NULL terminated    */
   /* string and its length *MUST* be between the values of:            */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function return zero if successful or a negative return error*/
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Call_Line_Identification_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber) && (BTPS_StringLength(PhoneNumber) >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the call line identification         */
               /* notification.                                         */
               ret_val = _HFRM_Send_Call_Line_Identification_Notification(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, PhoneNumber);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a ring indication to a   */
   /* remote Hands Free unit.  This function may only be performed by   */
   /* Audio Gateways for which a valid service level connection to a    */
   /* connected remote Hands Free device exists.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Ring_Indication(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send a ring indication.                   */
               ret_val = _HFRM_Ring_Indication(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling in-band    */
   /* ring tone capabilities for a connected Hands Free device.  This   */
   /* function may only be performed by Audio Gateways for which a valid*/
   /* service kevel connection exists.  This function may only be used  */
   /* to enable in-band ring tone capabilities if the local Audio       */
   /* Gateway supports this feature.  This function accepts as its input*/
   /* parameter a BOOLEAN flag specifying if this is a call to Enable or*/
   /* Disable this functionality.  This function returns zero if        */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableInBandRing)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable/disable in-band ring tone.         */
               ret_val = _HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, EnableInBandRing);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for responding to a request that was */
   /* received for a phone number to be associated with a unique voice  */
   /* tag by a remote Hands Free device.  This function may only be     */
   /* performed by Audio Gateways that have received a voice tag request*/
   /* Indication.  This function accepts as its input parameter the     */
   /* phone number to be associated with the voice tag.  If the request */
   /* is accepted, the phone number Parameter string length *MUST* be   */
   /* between the values:                                               */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* If the caller wishes to reject the request, the phone number      */
   /* parameter should be set to NULL to indicate this.  This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Voice_Tag_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!PhoneNumber) || ((PhoneNumber) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the voice tag response.              */
               ret_val = _HFRM_Voice_Tag_Response(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, PhoneNumber);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the current     */
   /* calls list entries to a remote Hands Free device.  This function  */
   /* may only be performed by Audio Gateways that have received a      */
   /* request to query the remote current calls list.  This function    */
   /* accepts as its input parameters the list of current call entries  */
   /* to be sent and length of the list.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Current_Calls_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRE_Current_Call_List_Entry_t *CurrentCallListEntryList)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!NumberListEntries) || ((NumberListEntries) && (CurrentCallListEntryList))))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the current call list.               */
               ret_val = _HFRM_Send_Current_Calls_List(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, NumberListEntries, CurrentCallListEntryList);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the network     */
   /* operator.  This function may only be performed by Audio Gateways  */
   /* that have received a request to query the remote network operator */
   /* selection.  This function accepts as input the current network    */
   /* mode and the current network operator.  The network operator      */
   /* should be expressed as a NULL terminated ASCII string (if         */
   /* specified) and must have a length less than:                      */
   /*                                                                   */
   /*    HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM                           */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * It is valid to either pass a NULL for the NetworkOperator*/
   /*          parameter or a blank string to specify that there is no  */
   /*          network operator present.                                */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Network_Operator_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NetworkMode, char *NetworkOperator)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!NetworkOperator) || ((NetworkOperator) && (BTPS_StringLength(NetworkOperator) <= HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM))))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the network operator selection.      */
               ret_val = _HFRM_Send_Network_Operator_Selection(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, NetworkMode, NetworkOperator);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending extended error  */
   /* results.  This function may only be performed by an Audio Gateway */
   /* with a valid service level connection.  This function accepts as  */
   /* its input parameter the result code to send as part of the error  */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Extended_Error_Result(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ResultCode)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the extended error result.           */
               ret_val = _HFRM_Send_Extended_Error_Result(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, ResultCode);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending subscriber      */
   /* number information.  This function may only be performed by an    */
   /* Audio Gateway that has received a request to query the subscriber */
   /* number information.  This function accepts as its input parameters*/
   /* the number of subscribers followed by a list of subscriber        */
   /* numbers.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Subscriber_Number_Information(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRM_Subscriber_Number_Information_t *SubscriberNumberList)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!NumberListEntries) || ((NumberListEntries) && (SubscriberNumberList))))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified subscriber number      */
               /* information.                                          */
               ret_val = _HFRM_Send_Subscriber_Number_Information(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, NumberListEntries, SubscriberNumberList);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending information     */
   /* about the incoming call state.  This function may only be         */
   /* performed by an Audio Gateway that has a valid service level      */
   /* connection to a remote Hands Free device.  This function accepts  */
   /* as its input parameter the call state to set as part of this      */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Incoming_Call_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified incoming call state.   */
               ret_val = _HFRM_Send_Incoming_Call_State(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, CallState);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a terminating   */
   /* response code from an Audio Gateway to a remote Hands Free device.*/
   /* This function may only be performed by an Audio Gateway that has a*/
   /* valid service level connection to a remote Hands Free device.     */
   /* This function can be called in any context where a normal Audio   */
   /* Gateway response function is called if the intention is to        */
   /* generate an error in response to the request.  It also must be    */
   /* called after certain requests that previously automatically       */
   /* generated an OK response.  In general, either this function or an */
   /* explicit response must be called after each request to the Audio  */
   /* Gateway.  This function accepts as its input parameters the type  */
   /* of result to return in the terminating response and, if the result*/
   /* type indicates an extended error code value, the error code.  This*/
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Send_Terminating_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Extended_Result_t ResultType, unsigned int ResultValue)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified terminating response.  */
               ret_val = _HFRM_Send_Terminating_Response(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, ResultType, ResultValue);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling the processing */
   /* of arbitrary commands from a remote Hands Free device.  Once this */
   /* function is called the hetHFRArbitraryCommandIndication event will*/
   /* be dispatched when an arbitrary command is received (i.e. a non   */
   /* Hands Free profile command).  If this function is not called, the */
   /* Audio Gateway will silently respond to any arbitrary commands with*/
   /* an error response ("ERROR").  If support is enabled, then the     */
   /* caller is responsible for responding TO ALL arbitrary command     */
   /* indications (hetHFRArbitraryCommandIndication).  If the arbitrary */
   /* command is not supported, then the caller should simply respond   */
   /* with:                                                             */
   /*                                                                   */
   /*   HFRM_Send_Terminating_Response()                                */
   /*                                                                   */
   /* specifying the erError response. This function returns zero if    */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * Once arbitrary command processing is enabled for an      */
   /*          Audio Gateway it cannot be disabled.                     */
   /* * NOTE * The default value is disabled (i.e. the                  */
   /*          hetHFRArbitraryCommandIndication will NEVER be dispatched*/
   /*          and the Audio Gateway will always respond with an error  */
   /*          response ("ERROR") when an arbitrary command is received.*/
   /* * NOTE * If support is enabled, the caller is guaranteed that a   */
   /*          hetHFRArbitraryCommandIndication will NOT be dispatched  */
   /*          before a service level indication is present. If an      */
   /*          arbitrary command is received, it will be responded with */
   /*          silently with an error response ("ERROR").               */
   /* * NOTE * This function is not applicable to Hands Free devices,   */
   /*          as Hands Free devices will always receive the            */
   /*          hetHFRArbitraryResponseIndication.  No action is required*/
   /*          and the event can simply be ignored.                     */
int BTPSAPI HFRM_Enable_Arbitrary_Command_Processing(unsigned int HandsFreeManagerEventCallbackID)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if(HandsFreeManagerEventCallbackID)
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable arbitrary command processing.      */
               ret_val = _HFRM_Enable_Arbitrary_Command_Processing(HFREEntryInfo->ControlCallbackID);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an arbitrary    */
   /* response to the remote Hands Free device (i.e. non Bluetooth      */
   /* Hands Free Profile response) - either solicited or non-solicited. */
   /* This function may only be performed by an Audio Gateway with a    */
   /* valid service level connection. This function accepts as its      */
   /* input parameter a NULL terminated ASCII string that represents    */
   /* the arbitrary response to send. This function returns zero if     */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The Response string passed to this function *MUST* begin */
   /*          with a carriage return/line feed ("\r\n").               */
int BTPSAPI HFRM_Send_Arbitrary_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryResponse)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ArbitraryResponse) && (BTPS_StringLength(ArbitraryResponse)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified arbitrary response.    */
               ret_val = _HFRM_Send_Arbitrary_Response(HFREEntryInfo->ControlCallbackID, RemoteDeviceAddress, ArbitraryResponse);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Hands Free Manager Audio Connection Management Functions.         */

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Hands Free device for which a valid  */
   /* service level connection Exists.  This function accepts as its    */
   /* input parameter the connection type indicating which connection   */
   /* will process the command.  This function returns zero if          */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Setup_Audio_Connection(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to setup the audio connection.               */
               ret_val = _HFRM_Setup_Audio_Connection(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HFRM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Hands   */
   /* Free device.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int BTPSAPI HFRM_Release_Audio_Connection(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to release the audio connection.             */
               ret_val = _HFRM_Release_Audio_Connection(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Hands Free Manager Data Handler ID           */
   /* (registered via call to the HFRM_Register_Data_Event_Callback()   */
   /* function), followed by the the connection type indicating which   */
   /* connection will transmit the audio data, the length (in Bytes) of */
   /* the audio data to send, and a pointer to the audio data to send to*/
   /* the remote entity.  This function returns zero if successful or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function is only applicable for Bluetooth devices   */
   /*          that are configured to support packetized SCO audio.     */
   /*          This function will have no effect on Bluetooth devices   */
   /*          that are configured to process SCO audio via hardare     */
   /*          codec.                                                   */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to process the audio data themselves (as */
   /*          opposed to having the hardware process the audio data    */
   /*          via a hardware codec.                                    */
   /* * NOTE * The data that is sent *MUST* be formatted in the correct */
   /*          SCO format that is expected by the device.               */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int BTPSAPI HFRM_Send_Audio_Data(unsigned int HandsFreeManagerDataEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerDataEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (AudioDataLength) && (AudioData))
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Data:&HFREEntryInfoList_HF_Data, HandsFreeManagerDataEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified audio data.            */
               ret_val = _HFRM_Send_Audio_Data(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress, AudioDataLength, AudioData);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerDataEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Hands Free Manager Event Callback Registration Functions.         */

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Hands Free       */
   /* Profile Manager Service.  This Callback will be dispatched by the */
   /* Hands Free Manager when various Hands Free Manager events occur.  */
   /* This function accepts the callback function and callback parameter*/
   /* (respectively) to call when a Hands Free Manager event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HFRM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
int BTPSAPI HFRM_Register_Event_Callback(HFRM_Connection_Type_t ConnectionType, Boolean_t ControlCallback, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   HFRE_Entry_Info_t  HFREEntryInfo;
   HFRE_Entry_Info_t *HFREEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HFRE Manager has been initialized.  */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a control event handler for the specified        */
            /* connection type (if control handler was specified).      */
            if(ControlCallback)
            {
               if(ConnectionType == hctAudioGateway)
                  HFREEntryInfoPtr = HFREEntryInfoList_AG_Control;
               else
                  HFREEntryInfoPtr = HFREEntryInfoList_HF_Control;
            }
            else
               HFREEntryInfoPtr = NULL;

            if(!HFREEntryInfoPtr)
            {
               /* First, register the handler locally.                  */
               BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

               HFREEntryInfo.CallbackID        = GetNextCallbackID();
               HFREEntryInfo.EventCallback     = CallbackFunction;
               HFREEntryInfo.CallbackParameter = CallbackParameter;
               HFREEntryInfo.Flags             = HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               /* Note the connection type.                             */
               if(ConnectionType == hctAudioGateway)
                  HFREEntryInfo.Flags |= HFRE_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

               /* Check to see if we need to register a control handler */
               /* or a normal event handler.                            */
               if(ControlCallback)
               {
                  /* Control handler, add it the correct list, and      */
                  /* attempt to register it with the server.            */
                  if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, &HFREEntryInfo)) != NULL)
                  {
                     /* Attempt to register it with the system.         */
                     if((ret_val = _HFRM_Register_Events(ConnectionType, TRUE)) > 0)
                     {
                        /* Control handler registered, go ahead and flag*/
                        /* success to the caller.                       */
                        HFREEntryInfoPtr->ControlCallbackID = ret_val;

                        ret_val                             = HFREEntryInfoPtr->CallbackID;
                     }
                     else
                     {
                        /* Error, go ahead and delete the entry we added*/
                        /* locally.                                     */
                        if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HFREEntryInfoPtr->CallbackID)) != NULL)
                           FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
               {
                  if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, &HFREEntryInfo)) != NULL)
                     ret_val = HFREEntryInfoPtr->CallbackID;
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_ALREADY_REGISTERED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Hands Free Manager event      */
   /* Callback (registered via a successful call to the                 */
   /* HFRM_Register_Event_Callback() function.  This function accepts as*/
   /* input the Hands Free Manager event callback ID (return value from */
   /* the HFRM_Register_Event_Callback() function).                     */
void BTPSAPI HFRM_Un_Register_Event_Callback(unsigned int HandsFreeManagerEventCallbackID)
{
   Boolean_t          ControlCallback;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HandsFreeManagerEventCallbackID)
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* We need to determine what type of Callback this (as we   */
            /* process them differently).                               */
            ControlCallback = FALSE;
            if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG, HandsFreeManagerEventCallbackID)) == NULL)
            {
               if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) == NULL)
               {
                  if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF, HandsFreeManagerEventCallbackID)) == NULL)
                  {
                     if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
                        ControlCallback = TRUE;
                  }
               }
               else
                  ControlCallback = TRUE;
            }

            /* Check to see if we found the callback and deleted it.    */
            if(HFREEntryInfo)
            {
               /* Check to see if need to delete it from the server.    */
               if(ControlCallback)
               {
                  /* Handler found, go ahead and delete it from the     */
                  /* server.                                            */
                  _HFRM_Un_Register_Events(HFREEntryInfo->ControlCallbackID);
               }

               /* Free the memory because we are finished with it.      */
               FreeHFREEntryInfoEntryMemory(HFREEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Hands Free*/
   /* Profile Manager service to explicitly process SCO audio data.     */
   /* This callback will be dispatched by the Hands Free Manager when   */
   /* various Hands Free Manager events occur.  This function accepts   */
   /* the connection type which indicates the connection type the data  */
   /* registration callback to register for, and the callback function  */
   /* and callback parameter (respectively) to call when a Hands Free   */
   /* Manager event needs to be dispatched.  This function returns a    */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HFRM_Send_Audio_Data() function to send SCO audio data.  */
   /* * NOTE * There can only be a single data event handler registered */
   /*          for each type of Hands Free Manager connection type.     */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HFRM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HFRM_Register_Data_Event_Callback(HFRM_Connection_Type_t ConnectionType, HFRM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   HFRE_Entry_Info_t  HFREEntryInfo;
   HFRE_Entry_Info_t *HFREEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HFRE Manager has been initialized.  */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a data event handler for the specified connection*/
            /* type.                                                    */
            if(ConnectionType == hctAudioGateway)
               HFREEntryInfoPtr = HFREEntryInfoList_AG_Data;
            else
               HFREEntryInfoPtr = HFREEntryInfoList_HF_Data;

            if(!HFREEntryInfoPtr)
            {
               /* First, register the handler locally.                  */
               BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

               HFREEntryInfo.CallbackID        = GetNextCallbackID();
               HFREEntryInfo.EventCallback     = CallbackFunction;
               HFREEntryInfo.CallbackParameter = CallbackParameter;
               HFREEntryInfo.Flags             = HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               /* Note the connection type.                             */
               if(ConnectionType == hctAudioGateway)
                  HFREEntryInfo.Flags |= HFRE_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

               if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Data:&HFREEntryInfoList_HF_Data, &HFREEntryInfo)) != NULL)
               {
                  /* Attempt to register it with the system.            */
                  if((ret_val = _HFRM_Register_Data_Events(ConnectionType)) > 0)
                  {
                     /* Data handler registered, go ahead and flag      */
                     /* success to the caller.                          */
                     HFREEntryInfoPtr->DataCallbackID = ret_val;

                     ret_val                          = HFREEntryInfoPtr->CallbackID;
                  }
                  else
                  {
                     /* Error, go ahead and delete the entry we added   */
                     /* locally.                                        */
                     if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Data:&HFREEntryInfoList_HF_Data, HFREEntryInfoPtr->CallbackID)) != NULL)
                        FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_ALREADY_REGISTERED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Hands Free Manager data event */
   /* callback (registered via a successful call to the                 */
   /* HFRM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Hands Free Manager data event callback ID    */
   /* (return value from HFRM_Register_Data_Event_Callback() function). */
void BTPSAPI HFRM_Un_Register_Data_Event_Callback(unsigned int HandsFreeManagerDataCallbackID)
{
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if(HandsFreeManagerDataCallbackID)
      {
         /* Attempt to wait for access to the Hands Free Manager state  */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the local handler.                                */
            if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG_Data, HandsFreeManagerDataCallbackID)) == NULL)
               HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF_Data, HandsFreeManagerDataCallbackID);

            if(HFREEntryInfo)
            {
               /* Handler found, go ahead and delete it from the server.*/
               _HFRM_Un_Register_Data_Events(HFREEntryInfo->DataCallbackID);

               /* All finished with the entry, delete it.               */
               FreeHFREEntryInfoEntryMemory(HFREEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Callback ID that is returned from a        */
   /* successful call to HFRM_Register_Event_Callback().  The second    */
   /* parameter is the local connection type of the SCO connection.  The*/
   /* third parameter is the address of the remote device of the SCO    */
   /* connection.  The fourth parameter is a pointer to the location to */
   /* store the SCO Handle. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int BTPSAPI HFRM_Query_SCO_Connection_Handle(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle)
{
   int                ret_val;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HandsFree Manager has been          */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* HandsFree Manager has been initialized, let's check the input  */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (SCOHandle))
      {
         /* Attempt to wait for access to the HandsFree Manager state   */
         /* information.                                                */
         if(BTPS_WaitMutex(HFREManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, find the local handler.                           */
            if((HFREEntryInfo = SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the specified audio data.            */
               ret_val = _HFRM_Query_SCO_Connection_Handle(HFREEntryInfo->ControlCallbackID, ConnectionType, RemoteDeviceAddress, SCOHandle);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HFREManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


