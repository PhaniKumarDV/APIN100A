/*****< btpmantm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMANTM - ANT Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/30/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMANTM.h"            /* BTPM ANT Manager Prototypes/Constants.    */
#include "ANTMAPI.h"             /* ANT Manager Prototypes/Constants.         */
#include "ANTMMSG.h"             /* BTPM ANT Manager Message Formats.         */
#include "ANTMGR.h"              /* ANT Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagANTM_Event_Callback_Info_t
{
   unsigned int                           EventCallbackID;
   unsigned int                           ClientID;
   ANTM_Event_Callback_t                  EventCallback;
   void                                  *CallbackParameter;
   struct _tagANTM_Event_Callback_Info_t *NextANTMEventCallbackInfoPtr;
} ANTM_Event_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           CallbackID;
   unsigned int           ClientID;
   ANTM_Event_Callback_t  EventCallback;
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

   /* Variable which holds a pointer to the first element in the Generic*/
   /* Attribute Profile Callback Info List (which holds all ANTM Event  */
   /* Callbacks registered with this module).                           */
static ANTM_Event_Callback_Info_t *EventCallbackInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextEventCallbackID(void);

static ANTM_Event_Callback_Info_t *AddEventCallbackInfoEntry(ANTM_Event_Callback_Info_t **ListHead, ANTM_Event_Callback_Info_t *EntryToAdd);
static ANTM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(ANTM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static ANTM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(ANTM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static void FreeEventCallbackInfoEntryMemory(ANTM_Event_Callback_Info_t *EntryToFree);
static void FreeEventCallbackInfoList(ANTM_Event_Callback_Info_t **ListHead);

static void DispatchANTMEvent(ANTM_Event_Data_t *ANTMEventData, BTPM_Message_t *Message, unsigned int *EventCallbackID, unsigned int *MessageEventHandlerID);

static void ProcessRegisterANTEventsRequestMessage(ANTM_Register_ANT_Events_Request_t *Message);
static void ProcessUnRegisterANTEventsRequestMessage(ANTM_Un_Register_ANT_Events_Request_t *Message);

static void ProcessAssignChannelRequestMessage(ANTM_Assign_Channel_Request_t *Message);
static void ProcessUnAssignChannelRequestMessage(ANTM_Un_Assign_Channel_Request_t *Message);
static void ProcessSetChannelIDRequestMessage(ANTM_Set_Channel_ID_Request_t *Message);
static void ProcessSetChannelPeriodRequestMessage(ANTM_Set_Channel_Period_Request_t *Message);
static void ProcessSetChannelSearchTimeoutRequestMessage(ANTM_Set_Channel_Search_Timeout_Request_t *Message);
static void ProcessSetChannelRFFrequencyRequestMessage(ANTM_Set_Channel_RF_Frequency_Request_t *Message);
static void ProcessSetNetworkKeyRequestMessage(ANTM_Set_Network_Key_Request_t *Message);
static void ProcessSetTransmitPowerRequestMessage(ANTM_Set_Transmit_Power_Request_t *Message);
static void ProcessAddChannelIDRequestMessage(ANTM_Add_Channel_ID_Request_t *Message);
static void ProcessConfigureInclExclListRequestMessage(ANTM_Configure_Incl_Excl_List_Request_t *Message);
static void ProcessSetChannelTransmitPowerRequestMessage(ANTM_Set_Channel_Transmit_Power_Request_t *Message);
static void ProcessSetLowPrioScanSearchTimeoutRequestMessage(ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Request_t *Message);
static void ProcessSetSerialNumberChannelIDRequestMessage(ANTM_Set_Serial_Number_Channel_ID_Request_t *Message);
static void ProcessEnableExtendedMessagesRequestMessage(ANTM_Enable_Extended_Messages_Request_t *Message);
static void ProcessEnableLEDRequestMessage(ANTM_Enable_LED_Request_t *Message);
static void ProcessEnableCrystalRequestMessage(ANTM_Enable_Crystal_Request_t *Message);
static void ProcessExtendedMessagesConfigurationRequestMessage(ANTM_Extended_Messages_Configuration_Request_t *Message);
static void ProcessConfigureFrequencyAgilityRequestMessage(ANTM_Configure_Frequency_Agility_Request_t *Message);
static void ProcessSetProximitySearchRequestMessage(ANTM_Set_Proximity_Search_Request_t *Message);
static void ProcessSetChannelSearchPriorityRequestMessage(ANTM_Set_Channel_Search_Priority_Request_t *Message);
static void ProcessSetUSBDescriptorStringRequestMessage(ANTM_Set_USB_Descriptor_String_Request_t *Message);
static void ProcessResetSystemRequestMessage(ANTM_Reset_System_Request_t *Message);
static void ProcessOpenChannelRequestMessage(ANTM_Open_Channel_Request_t *Message);
static void ProcessCloseChannelRequestMessage(ANTM_Close_Channel_Request_t *Message);
static void ProcessRequestChannelMessageRequestMessage(ANTM_Request_Channel_Message_Request_t *Message);
static void ProcessOpenRxScanModeRequestMessage(ANTM_Open_Rx_Scan_Mode_Request_t *Message);
static void ProcessSleepMessageRequestMessage(ANTM_Sleep_Message_Request_t *Message);
static void ProcessSendBroadcastDataRequestMessage(ANTM_Send_Broadcast_Data_Request_t *Message);
static void ProcessSendAcknowledgedDataRequestMessage(ANTM_Send_Acknowledged_Data_Request_t *Message);
static void ProcessSendBurstTransferDataRequestMessage(ANTM_Send_Burst_Transfer_Data_Request_t *Message);
static void ProcessInitializeCWTestModeRequestMessage(ANTM_Initialize_CW_Test_Mode_Request_t *Message);
static void ProcessANTSetCWTestModeRequestMessage(ANTM_Set_CW_Test_Mode_Request_t *Message);
static void ProcessANTSendRawPacketRequestMessage(ANTM_Send_Raw_Packet_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessANTStartupMessageEvent(ANT_Startup_Message_Event_Data_t *ANTEventData);
static void ProcessANTChannelResponseEvent(ANT_Channel_Response_Event_Data_t *ANTEventData);
static void ProcessANTChannelStatusEvent(ANT_Channel_Status_Event_Data_t *ANTEventData);
static void ProcessANTChannelIDEvent(ANT_Channel_ID_Event_Data_t *ANTEventData);
static void ProcessANTVersionEvent(ANT_Version_Event_Data_t *ANTEventData);
static void ProcessANTCapabilitiesEvent(ANT_Capabilities_Event_Data_t *ANTEventData);
static void ProcessANTBroadcastDataEvent(ANT_Packet_Broadcast_Data_Event_Data_t *ANTEventData);
static void ProcessANTAcknowledgedDataEvent(ANT_Packet_Acknowledged_Data_Event_Data_t *ANTEventData);
static void ProcessANTBurstDataEvent(ANT_Packet_Burst_Data_Event_Data_t *ANTEventData);
static void ProcessANTExtendedBroadcastDataEvent(ANT_Packet_Extended_Broadcast_Data_Event_Data_t *ANTEventData);
static void ProcessANTExtendedAcknowledgedDataEvent(ANT_Packet_Extended_Acknowledged_Data_Event_Data_t *ANTEventData);
static void ProcessANTExtendedBurstDataEvent(ANT_Packet_Extended_Burst_Data_Event_Data_t *ANTEventData);
static void ProcessANTRawPacketDataEvent(ANT_Raw_Packet_Data_Event_Data_t *ANTEventData);

static void ProcessANTEvent(ANTM_Update_Event_Data_t *ANTEventData);

static void BTPSAPI BTPMDispatchCallback_ANTM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_ANT(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI ANTManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the ANT Event Callback List.                                 */
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
static ANTM_Event_Callback_Info_t *AddEventCallbackInfoEntry(ANTM_Event_Callback_Info_t **ListHead, ANTM_Event_Callback_Info_t *EntryToAdd)
{
   ANTM_Event_Callback_Info_t *AddedEntry = NULL;
   ANTM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->EventCallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (ANTM_Event_Callback_Info_t *)BTPS_AllocateMemory(sizeof(ANTM_Event_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                              = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextANTMEventCallbackInfoPtr = NULL;

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
                     if(tmpEntry->NextANTMEventCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextANTMEventCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextANTMEventCallbackInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static ANTM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(ANTM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   ANTM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
         FoundEntry = FoundEntry->NextANTMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static ANTM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(ANTM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   ANTM_Event_Callback_Info_t *FoundEntry = NULL;
   ANTM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextANTMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextANTMEventCallbackInfoPtr = FoundEntry->NextANTMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextANTMEventCallbackInfoPtr;

         FoundEntry->NextANTMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Event Callback member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeEventCallbackInfoEntryMemory(ANTM_Event_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Event Callback List.  Upon return of this*/
   /* function, the Head Pointer is set to NULL.                        */
static void FreeEventCallbackInfoList(ANTM_Event_Callback_Info_t **ListHead)
{
   ANTM_Event_Callback_Info_t *EntryToFree;
   ANTM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Event Callback ID: 0x%08X\n", EntryToFree->EventCallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextANTMEventCallbackInfoPtr;

         FreeEventCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified ANT event to every registered ANT Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the ANT Manager Lock */
   /*          held.  Upon exit from this function it will free the ANT */
   /*          Manager Lock.                                            */
static void DispatchANTMEvent(ANTM_Event_Data_t *ANTMEventData, BTPM_Message_t *Message, unsigned int *EventCallbackID, unsigned int *MessageEventHandlerID)
{
   unsigned int                Index;
   unsigned int                Index1;
   unsigned int                ServerID;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   ANTM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((EventCallbackInfoList) && (ANTMEventData) && (Message) && (EventCallbackID) && (MessageEventHandlerID))
   {
      /* Next, let's determine how many callbacks are registered.       */
      CallbackInfoPtr = EventCallbackInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(CallbackInfoPtr)
      {
         if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
            NumberCallbacks++;

         CallbackInfoPtr = CallbackInfoPtr->NextANTMEventCallbackInfoPtr;
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
            CallbackInfoPtr = EventCallbackInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackInfoPtr)
            {
               if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackInfoPtr->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackID        = CallbackInfoPtr->EventCallbackID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfoPtr = CallbackInfoPtr->NextANTMEventCallbackInfoPtr;
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
               /*          for ANT events and Data Events.              */
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
                        *EventCallbackID = CallbackInfoArrayPtr[Index].CallbackID;

                        (*CallbackInfoArrayPtr[Index].EventCallback)(ANTMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
                     *MessageEventHandlerID           = CallbackInfoArrayPtr[Index].CallbackID;
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

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register ANT Events*/
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterANTEventsRequestMessage(ANTM_Register_ANT_Events_Request_t *Message)
{
   int                                  Result;
   ANTM_Event_Callback_Info_t           EventCallbackEntry;
   ANTM_Register_ANT_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that that no callbacks are currently registered (only   */
      /* one client can register and control the ANT+ system).          */
      if(EventCallbackInfoList == NULL)
      {
         /* Attempt to add an entry into the Event Callback Entry list. */
         BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(ANTM_Event_Callback_Info_t));

         EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
         EventCallbackEntry.ClientID          = Message->MessageHeader.AddressID;
         EventCallbackEntry.EventCallback     = NULL;
         EventCallbackEntry.CallbackParameter = 0;

         if(AddEventCallbackInfoEntry(&EventCallbackInfoList, &EventCallbackEntry))
            Result = 0;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_ANT_EVENT_HANDLER_ALREADY_REGISTERED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_REGISTER_ANT_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      if(!Result)
         ResponseMessage.EventHandlerID            = EventCallbackEntry.EventCallbackID;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register ANT    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterANTEventsRequestMessage(ANTM_Un_Register_ANT_Events_Request_t *Message)
{
   int                                     Result;
   ANTM_Event_Callback_Info_t             *EventCallbackPtr;
   ANTM_Un_Register_ANT_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* doing the un-registering.                                      */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this is owned by the Process that has requested */
         /* the un-registration.                                        */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Attempt to delete the callback specified for this device.*/
            if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
            {
               /* Free the memory allocated for this event callback.    */
               FreeEventCallbackInfoEntryMemory(EventCallbackPtr);

               /* Return success.                                       */
               Result = 0;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_UN_REGISTER_ANT_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Assign Channel     */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessAssignChannelRequestMessage(ANTM_Assign_Channel_Request_t *Message)
{
   int                             Result;
   ANTM_Event_Callback_Info_t     *EventCallbackPtr;
   ANTM_Assign_Channel_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Assign_Channel(Message->ChannelNumber, Message->ChannelType, Message->NetworkNumber, Message->ExtendedAssignment);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_ASSIGN_CHANNEL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Assign Channel  */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessUnAssignChannelRequestMessage(ANTM_Un_Assign_Channel_Request_t *Message)
{
   int                                Result;
   ANTM_Event_Callback_Info_t        *EventCallbackPtr;
   ANTM_Un_Assign_Channel_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Un_Assign_Channel(Message->ChannelNumber);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_UN_ASSIGN_CHANNEL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Channel ID     */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetChannelIDRequestMessage(ANTM_Set_Channel_ID_Request_t *Message)
{
   int                             Result;
   ANTM_Event_Callback_Info_t     *EventCallbackPtr;
   ANTM_Set_Channel_ID_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Channel_ID(Message->ChannelNumber, Message->DeviceNumber, Message->DeviceType, Message->TransmissionType);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_CHANNEL_ID_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Channel Period */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetChannelPeriodRequestMessage(ANTM_Set_Channel_Period_Request_t *Message)
{
   int                                 Result;
   ANTM_Event_Callback_Info_t         *EventCallbackPtr;
   ANTM_Set_Channel_Period_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Channel_Period(Message->ChannelNumber, Message->MessagingPeriod);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_CHANNEL_PERIOD_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Channel Search */
   /* Timeout Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetChannelSearchTimeoutRequestMessage(ANTM_Set_Channel_Search_Timeout_Request_t *Message)
{
   int                                         Result;
   ANTM_Event_Callback_Info_t                 *EventCallbackPtr;
   ANTM_Set_Channel_Search_Timeout_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Channel_Search_Timeout(Message->ChannelNumber, Message->SearchTimeout);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_CHANNEL_SEARCH_TIMEOUT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Channel RF     */
   /* Frequency Request Message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetChannelRFFrequencyRequestMessage(ANTM_Set_Channel_RF_Frequency_Request_t *Message)
{
   int                                       Result;
   ANTM_Event_Callback_Info_t               *EventCallbackPtr;
   ANTM_Set_Channel_RF_Frequency_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Channel_RF_Frequency(Message->ChannelNumber, Message->RFFrequency);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_CHANNEL_RF_FREQUENCY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Network Key    */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetNetworkKeyRequestMessage(ANTM_Set_Network_Key_Request_t *Message)
{
   int                              Result;
   ANTM_Event_Callback_Info_t      *EventCallbackPtr;
   ANTM_Set_Network_Key_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Network_Key(Message->NetworkNumber, Message->NetworkKey);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_NETWORK_KEY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Transmit Power */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetTransmitPowerRequestMessage(ANTM_Set_Transmit_Power_Request_t *Message)
{
   int                                 Result;
   ANTM_Event_Callback_Info_t         *EventCallbackPtr;
   ANTM_Set_Transmit_Power_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Transmit_Power(Message->TransmitPower);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_TRANSMIT_POWER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Add Channel ID     */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessAddChannelIDRequestMessage(ANTM_Add_Channel_ID_Request_t *Message)
{
   int                             Result;
   ANTM_Event_Callback_Info_t     *EventCallbackPtr;
   ANTM_Add_Channel_ID_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Add_Channel_ID(Message->ChannelNumber, Message->DeviceNumber, Message->DeviceType, Message->TransmissionType, Message->ListIndex);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_ADD_CHANNEL_ID_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Configure Inclusion*/
   /* / Exclusion List Request Message and responds to the message      */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessConfigureInclExclListRequestMessage(ANTM_Configure_Incl_Excl_List_Request_t *Message)
{
   int                                       Result;
   ANTM_Event_Callback_Info_t               *EventCallbackPtr;
   ANTM_Configure_Incl_Excl_List_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Configure_Inclusion_Exclusion_List(Message->ChannelNumber, Message->ListSize, Message->Exclude);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_CONFIGURE_INCL_EXCL_LIST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Channel        */
   /* Transmit Power Request Message and responds to the message        */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetChannelTransmitPowerRequestMessage(ANTM_Set_Channel_Transmit_Power_Request_t *Message)
{
   int                                         Result;
   ANTM_Event_Callback_Info_t                 *EventCallbackPtr;
   ANTM_Set_Channel_Transmit_Power_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Channel_Transmit_Power(Message->ChannelNumber, Message->TransmitPower);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_CHANNEL_TRANSMIT_POWER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Low Priority   */
   /* Scan Search Timeout Request Message and responds to the message   */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetLowPrioScanSearchTimeoutRequestMessage(ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Request_t *Message)
{
   int                                                           Result;
   ANTM_Event_Callback_Info_t                                   *EventCallbackPtr;
   ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Low_Priority_Channel_Search_Timeout(Message->ChannelNumber, Message->SearchTimeout);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_LOW_PRIORITY_CHANNEL_SCAN_SEARCH_TIMEOUT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Serial Number  */
   /* Channel ID Request Message and responds to the message            */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetSerialNumberChannelIDRequestMessage(ANTM_Set_Serial_Number_Channel_ID_Request_t *Message)
{
   int                                           Result;
   ANTM_Event_Callback_Info_t                   *EventCallbackPtr;
   ANTM_Set_Serial_Number_Channel_ID_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Serial_Number_Channel_ID(Message->ChannelNumber, Message->DeviceType, Message->TransmissionType);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_SERIAL_NUMBER_CHANNEL_ID_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Enable Extended    */
   /* Messages Request Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessEnableExtendedMessagesRequestMessage(ANTM_Enable_Extended_Messages_Request_t *Message)
{
   int                                       Result;
   ANTM_Event_Callback_Info_t               *EventCallbackPtr;
   ANTM_Enable_Extended_Messages_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Enable_Extended_Messages((unsigned int)(Message->Enable?ANT_EXTENDED_MESSAGES_ENABLE:ANT_EXTENDED_MESSAGES_DISABLE));
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_ENABLE_EXTENDED_MESSAGES_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Enable LED Request */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessEnableLEDRequestMessage(ANTM_Enable_LED_Request_t *Message)
{
   int                         Result;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;
   ANTM_Enable_LED_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Enable_LED((unsigned int)(Message->Enable?ANT_LED_ENABLE:ANT_LED_DISABLE));
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_ENABLE_LED_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Enable Crystal     */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessEnableCrystalRequestMessage(ANTM_Enable_Crystal_Request_t *Message)
{
   int                             Result;
   ANTM_Event_Callback_Info_t     *EventCallbackPtr;
   ANTM_Enable_Crystal_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Enable_Crystal();
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_ENABLE_CRYSTAL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Extended Messages  */
   /* Configuration Request Message and responds to the message         */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessExtendedMessagesConfigurationRequestMessage(ANTM_Extended_Messages_Configuration_Request_t *Message)
{
   int                                              Result;
   ANTM_Event_Callback_Info_t                      *EventCallbackPtr;
   ANTM_Extended_Messages_Configuration_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Configure_Extended_Messages(Message->RxMessagesMask);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_EXTENDED_MESSAGES_CONFIGURATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Configure Frequency*/
   /* Agility Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessConfigureFrequencyAgilityRequestMessage(ANTM_Configure_Frequency_Agility_Request_t *Message)
{
   int                                          Result;
   ANTM_Event_Callback_Info_t                  *EventCallbackPtr;
   ANTM_Configure_Frequency_Agility_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Configure_Frequency_Agility(Message->ChannelNumber, Message->FrequencyAgility1, Message->FrequencyAgility2, Message->FrequencyAgility3);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_CONFIGURE_FREQUENCY_AGILITY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Proximity      */
   /* Search Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetProximitySearchRequestMessage(ANTM_Set_Proximity_Search_Request_t *Message)
{
   int                                   Result;
   ANTM_Event_Callback_Info_t           *EventCallbackPtr;
   ANTM_Set_Proximity_Search_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Proximity_Search(Message->ChannelNumber, Message->SearchThreshold);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_PROXIMITY_SEARCH_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Channel Search */
   /* Priority Request Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetChannelSearchPriorityRequestMessage(ANTM_Set_Channel_Search_Priority_Request_t *Message)
{
   int                                          Result;
   ANTM_Event_Callback_Info_t                  *EventCallbackPtr;
   ANTM_Set_Channel_Search_Priority_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_Channel_Search_Priority(Message->ChannelNumber, Message->SearchPriority);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_CHANNEL_SEARCH_PRIORITY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set USB Descriptor */
   /* String Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSetUSBDescriptorStringRequestMessage(ANTM_Set_USB_Descriptor_String_Request_t *Message)
{
   int                                        Result;
   ANTM_Event_Callback_Info_t                *EventCallbackPtr;
   ANTM_Set_USB_Descriptor_String_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_USB_Descriptor_String(Message->StringNumber, Message->DescriptorString);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_USB_DESCRIPTOR_STRING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Reset System       */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessResetSystemRequestMessage(ANTM_Reset_System_Request_t *Message)
{
   int                           Result;
   ANTM_Event_Callback_Info_t   *EventCallbackPtr;
   ANTM_Reset_System_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Reset_System();
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_RESET_SYSTEM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Open Channel       */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessOpenChannelRequestMessage(ANTM_Open_Channel_Request_t *Message)
{
   int                           Result;
   ANTM_Event_Callback_Info_t   *EventCallbackPtr;
   ANTM_Open_Channel_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Open_Channel(Message->ChannelNumber);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_OPEN_CHANNEL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Close Channel      */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessCloseChannelRequestMessage(ANTM_Close_Channel_Request_t *Message)
{
   int                            Result;
   ANTM_Event_Callback_Info_t    *EventCallbackPtr;
   ANTM_Close_Channel_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Close_Channel(Message->ChannelNumber);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_CLOSE_CHANNEL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Request Channel    */
   /* Message Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessRequestChannelMessageRequestMessage(ANTM_Request_Channel_Message_Request_t *Message)
{
   int                                      Result;
   ANTM_Event_Callback_Info_t              *EventCallbackPtr;
   ANTM_Request_Channel_Message_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Request_Message(Message->ChannelNumber, Message->MessageID);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_REQUEST_CHANNEL_MESSAGE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Open Rx Scan Mode  */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessOpenRxScanModeRequestMessage(ANTM_Open_Rx_Scan_Mode_Request_t *Message)
{
   int                                Result;
   ANTM_Event_Callback_Info_t        *EventCallbackPtr;
   ANTM_Open_Rx_Scan_Mode_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Open_Rx_Scan_Mode(Message->ChannelNumber);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_OPEN_RX_SCAN_MODE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Sleep Message      */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSleepMessageRequestMessage(ANTM_Sleep_Message_Request_t *Message)
{
   int                            Result;
   ANTM_Event_Callback_Info_t    *EventCallbackPtr;
   ANTM_Sleep_Message_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Sleep_Message();
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SLEEP_MESSAGE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send Broadcast Data*/
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSendBroadcastDataRequestMessage(ANTM_Send_Broadcast_Data_Request_t *Message)
{
   int                                  Result;
   Byte_t                               TempData[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   unsigned int                         DataLength;
   ANTM_Event_Callback_Info_t          *EventCallbackPtr;
   ANTM_Send_Broadcast_Data_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Verify that the parameters are semi-valid.               */
            if((Message->BroadcastDataLength) && (Message->BroadcastData))
            {
               /* Copy the data into a temporary buffer.                */
               DataLength = Message->BroadcastDataLength;
               DataLength = (DataLength > ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE)?ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE:DataLength;

               BTPS_MemInitialize(TempData, 0, sizeof(TempData));
               BTPS_MemCopy(TempData, Message->BroadcastData, DataLength);

               /* Simply call the Impl. Mgr. to do the actual work.     */
               Result = _ANTM_Send_Broadcast_Data(Message->ChannelNumber, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE, TempData);
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SEND_BROADCAST_DATA_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send Acknowledged  */
   /* Data Request Message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSendAcknowledgedDataRequestMessage(ANTM_Send_Acknowledged_Data_Request_t *Message)
{
   int                                     Result;
   Byte_t                                  TempData[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   unsigned int                            DataLength;
   ANTM_Event_Callback_Info_t             *EventCallbackPtr;
   ANTM_Send_Acknowledged_Data_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Verify that the parameters are semi-valid.               */
            if((Message->AcknowledgedDataLength) && (Message->AcknowledgedData))
            {
               /* Copy the data into a temporary buffer.                */
               DataLength = Message->AcknowledgedDataLength;
               DataLength = (DataLength > ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE)?ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE:DataLength;

               BTPS_MemInitialize(TempData, 0, sizeof(TempData));
               BTPS_MemCopy(TempData, Message->AcknowledgedData, DataLength);

               /* Simply call the Impl. Mgr. to do the actual work.     */
               Result = _ANTM_Send_Acknowledged_Data(Message->ChannelNumber, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE, TempData);
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SEND_ACKNOWLEDGED_DATA_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send Burst Transfer*/
   /* Data Request Message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessSendBurstTransferDataRequestMessage(ANTM_Send_Burst_Transfer_Data_Request_t *Message)
{
   int                                       Result;
   Byte_t                                    TempData[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   unsigned int                              DataLength;
   ANTM_Event_Callback_Info_t               *EventCallbackPtr;
   ANTM_Send_Burst_Transfer_Data_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Verify that the parameters are semi-valid.               */
            if((Message->BurstTransferDataLength) && (Message->BurstTransferData))
            {
               /* Copy the data into a temporary buffer.                */
               DataLength = Message->BurstTransferDataLength;
               DataLength = (DataLength > ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE)?ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE:DataLength;

               BTPS_MemInitialize(TempData, 0, sizeof(TempData));
               BTPS_MemCopy(TempData, Message->BurstTransferData, DataLength);

               /* Simply call the Impl. Mgr. to do the actual work.     */
               Result = _ANTM_Send_Burst_Transfer_Data(Message->SequenceChannelNumber, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE, TempData);
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SEND_BURST_TRANSFER_DATA_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Initialize CW Test */
   /* Mode Request Message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessInitializeCWTestModeRequestMessage(ANTM_Initialize_CW_Test_Mode_Request_t *Message)
{
   int                                      Result;
   ANTM_Event_Callback_Info_t              *EventCallbackPtr;
   ANTM_Initialize_CW_Test_Mode_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Initialize_CW_Test_Mode();
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_INITIALIZE_CW_TEST_MODE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set CW Test Mode   */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessANTSetCWTestModeRequestMessage(ANTM_Set_CW_Test_Mode_Request_t *Message)
{
   int                               Result;
   ANTM_Event_Callback_Info_t       *EventCallbackPtr;
   ANTM_Set_CW_Test_Mode_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the Impl. Mgr. to do the work.               */
            Result = _ANTM_Set_CW_Test_Mode(Message->TxPower, Message->RFFrequency);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SET_CW_TEST_MODE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send Raw Packet    */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANT Manager Lock */
   /*          held.                                                    */
static void ProcessANTSendRawPacketRequestMessage(ANTM_Send_Raw_Packet_Request_t *Message)
{
   int                              Result;
   ANTM_Event_Callback_Info_t      *EventCallbackPtr;
   ANTM_Send_Raw_Packet_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the client who is    */
      /* sending the request.                                           */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Verify that this callback is owned by the Process that has  */
         /* sent the request message.                                   */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Verify that the packet data appears valid.               */
            if(Message->PacketLength >= sizeof(NonAlignedByte_t))
            {
               /* Simply call the Impl. Mgr. to do the work.            */
               Result = _ANTM_Send_Raw_Packet(Message->PacketLength, Message->PacketData);
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANTM_SEND_RAW_PACKET_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the ANT Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
#if 0
         case ANTM_MESSAGE_FUNCTION_SET_NEW_ALERT:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set New Alert Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_NEW_ALERT_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_NEW_ALERT_REQUEST_SIZE(((ANTM_Set_New_Alert_Request_t *)Message)->LastAlertTextLength))
                  ProcessSetNewAlertRequestMessage((ANTM_Set_New_Alert_Request_t *)Message);
               else
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length for specified data\n"));
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
#endif

         case ANTM_MESSAGE_FUNCTION_REGISTER_ANT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Register ANT Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_REGISTER_ANT_EVENTS_REQUEST_SIZE)
               ProcessRegisterANTEventsRequestMessage((ANTM_Register_ANT_Events_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_UN_REGISTER_ANT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register ANT Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_UN_REGISTER_ANT_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterANTEventsRequestMessage((ANTM_Un_Register_ANT_Events_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_ASSIGN_CHANNEL:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Assign Channel Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_ASSIGN_CHANNEL_REQUEST_SIZE)
               ProcessAssignChannelRequestMessage((ANTM_Assign_Channel_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_UN_ASSIGN_CHANNEL:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Assign Channel Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_UN_ASSIGN_CHANNEL_REQUEST_SIZE)
               ProcessUnAssignChannelRequestMessage((ANTM_Un_Assign_Channel_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_CHANNEL_ID:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Channel ID Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_ID_REQUEST_SIZE)
               ProcessSetChannelIDRequestMessage((ANTM_Set_Channel_ID_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_CHANNEL_PERIOD:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Channel Period Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_PERIOD_REQUEST_SIZE)
               ProcessSetChannelPeriodRequestMessage((ANTM_Set_Channel_Period_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_TIMEOUT:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Channel Search Timeout Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_SEARCH_TIMEOUT_REQUEST_SIZE)
               ProcessSetChannelSearchTimeoutRequestMessage((ANTM_Set_Channel_Search_Timeout_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_CHANNEL_RF_FREQUENCY:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Channel RF Frequency Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_RF_FREQUENCY_RESPONSE_SIZE)
               ProcessSetChannelRFFrequencyRequestMessage((ANTM_Set_Channel_RF_Frequency_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_NETWORK_KEY:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Network Key Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_NETWORK_KEY_REQUEST_SIZE)
               ProcessSetNetworkKeyRequestMessage((ANTM_Set_Network_Key_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_TRANSMIT_POWER:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Transmit Power Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_TRANSMIT_POWER_REQUEST_SIZE)
               ProcessSetTransmitPowerRequestMessage((ANTM_Set_Transmit_Power_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_ADD_CHANNEL_ID:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Add Channel ID Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_ADD_CHANNEL_ID_REQUEST_SIZE)
               ProcessAddChannelIDRequestMessage((ANTM_Add_Channel_ID_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_CONFIGURE_INCL_EXCL_LIST:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure Incl / Excl List Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_CONFIGURE_INCL_EXCL_LIST_REQUEST_SIZE)
               ProcessConfigureInclExclListRequestMessage((ANTM_Configure_Incl_Excl_List_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_CHANNEL_TRANSMIT_POWER:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Channel Transmit Power Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_TRANSMIT_POWER_REQUEST_SIZE)
               ProcessSetChannelTransmitPowerRequestMessage((ANTM_Set_Channel_Transmit_Power_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_LOW_PRIORITY_CHANNEL_SEARCH_TIMEOUT:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Low Priority Channel Search Timeout Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_LOW_PRIORITY_CHANNEL_SCAN_SEARCH_TIMEOUT_REQUEST_SIZE)
               ProcessSetLowPrioScanSearchTimeoutRequestMessage((ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_SERIAL_NUMBER_CHANNEL_ID:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Serial Number Channel ID Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_SERIAL_NUMBER_CHANNEL_ID_REQUEST_SIZE)
               ProcessSetSerialNumberChannelIDRequestMessage((ANTM_Set_Serial_Number_Channel_ID_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_ENABLE_EXTENDED_MESSAGES:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Extended Messages Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_ENABLE_EXTENDED_MESSAGES_REQUEST_SIZE)
               ProcessEnableExtendedMessagesRequestMessage((ANTM_Enable_Extended_Messages_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_ENABLE_LED:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable LED Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_ENABLE_LED_REQUEST_SIZE)
               ProcessEnableLEDRequestMessage((ANTM_Enable_LED_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_ENABLE_CRYSTAL:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Crystal Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_ENABLE_CRYSTAL_REQUEST_SIZE)
               ProcessEnableCrystalRequestMessage((ANTM_Enable_Crystal_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_EXTENDED_MESSAGES_CONFIGURATION:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Extended Messages Configuration Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_EXTENDED_MESSAGES_CONFIGURATION_REQUEST_SIZE)
               ProcessExtendedMessagesConfigurationRequestMessage((ANTM_Extended_Messages_Configuration_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_CONFIGURE_FREQUENCY_AGILITY:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure Frequency Agility Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_CONFIGURE_FREQUENCY_AGILITY_REQUEST_SIZE)
               ProcessConfigureFrequencyAgilityRequestMessage((ANTM_Configure_Frequency_Agility_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_PROXIMITY_SEARCH:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Proximity Search Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_PROXIMITY_SEARCH_REQUEST_SIZE)
               ProcessSetProximitySearchRequestMessage((ANTM_Set_Proximity_Search_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_PRIORITY:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Channel Search Priority Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_SEARCH_PRIORITY_REQUEST_SIZE)
               ProcessSetChannelSearchPriorityRequestMessage((ANTM_Set_Channel_Search_Priority_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_USB_DESCRIPTOR_STRING:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set USB Descriptor String Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_USB_DESCRIPTOR_STRING_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_USB_DESCRIPTOR_STRING_REQUEST_SIZE(((ANTM_Set_USB_Descriptor_String_Request_t *)Message)->DescriptorStringLength))
                  ProcessSetUSBDescriptorStringRequestMessage((ANTM_Set_USB_Descriptor_String_Request_t *)Message);
               else
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length for specified data\n"));
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_RESET_SYSTEM:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Reset System Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_RESET_SYSTEM_REQUEST_SIZE)
               ProcessResetSystemRequestMessage((ANTM_Reset_System_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_OPEN_CHANNEL:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Channel Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_OPEN_CHANNEL_REQUEST_SIZE)
               ProcessOpenChannelRequestMessage((ANTM_Open_Channel_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_CLOSE_CHANNEL:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Channel Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_CLOSE_CHANNEL_REQUEST_SIZE)
               ProcessCloseChannelRequestMessage((ANTM_Close_Channel_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_REQUEST_CHANNEL_MESSAGE:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Request Channel Message Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_REQUEST_CHANNEL_MESSAGE_REQUEST_SIZE)
               ProcessRequestChannelMessageRequestMessage((ANTM_Request_Channel_Message_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_OPEN_RX_SCAN_MODE:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Rx Scan Mode Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_OPEN_RX_SCAN_MODE_REQUEST_SIZE)
               ProcessOpenRxScanModeRequestMessage((ANTM_Open_Rx_Scan_Mode_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SLEEP_MESSAGE:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sleep Message Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SLEEP_MESSAGE_REQUEST_SIZE)
               ProcessSleepMessageRequestMessage((ANTM_Sleep_Message_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SEND_BROADCAST_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Broadcast Data Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_BROADCAST_DATA_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_BROADCAST_DATA_REQUEST_SIZE(((ANTM_Send_Broadcast_Data_Request_t *)Message)->BroadcastDataLength))
                  ProcessSendBroadcastDataRequestMessage((ANTM_Send_Broadcast_Data_Request_t *)Message);
               else
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length for specified data\n"));
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SEND_ACKNOWLEDGED_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Acknowledged Data Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_ACKNOWLEDGED_DATA_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_ACKNOWLEDGED_DATA_REQUEST_SIZE(((ANTM_Send_Acknowledged_Data_Request_t *)Message)->AcknowledgedDataLength))
                  ProcessSendAcknowledgedDataRequestMessage((ANTM_Send_Acknowledged_Data_Request_t *)Message);
               else
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length for specified data\n"));
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SEND_BURST_TRANSFER_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Burst Transfer Data Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_BURST_TRANSFER_DATA_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_BURST_TRANSFER_DATA_REQUEST_SIZE(((ANTM_Send_Burst_Transfer_Data_Request_t *)Message)->BurstTransferDataLength))
                  ProcessSendBurstTransferDataRequestMessage((ANTM_Send_Burst_Transfer_Data_Request_t *)Message);
               else
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length for specified data\n"));
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_INITIALIZE_CW_TEST_MODE:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialize CW Test Mode Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_INITIALIZE_CW_TEST_MODE_REQUEST_SIZE)
               ProcessInitializeCWTestModeRequestMessage((ANTM_Initialize_CW_Test_Mode_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SET_CW_TEST_MODE:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set CW Test Mode Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SET_CW_TEST_MODE_REQUEST_SIZE)
               ProcessANTSetCWTestModeRequestMessage((ANTM_Set_CW_Test_Mode_Request_t *)Message);
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         case ANTM_MESSAGE_FUNCTION_SEND_RAW_PACKET:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Raw Packet Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_RAW_PACKET_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANTM_SEND_RAW_PACKET_REQUEST_SIZE(((ANTM_Send_Raw_Packet_Request_t *)Message)->PacketLength))
                  ProcessANTSendRawPacketRequestMessage((ANTM_Send_Raw_Packet_Request_t *)Message);
               else
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length for specified data\n"));
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));

            break;
      }
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   ANTM_Event_Callback_Info_t *EventCallback;
   ANTM_Event_Callback_Info_t *tmpEventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Loop through the event callback list and delete all callbacks  */
      /* registered for this callback.                                  */
      EventCallback = EventCallbackInfoList;
      while(EventCallback)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(EventCallback->ClientID == ClientID)
         {
            /* Note the next Event Callback Entry in the list (we are   */
            /* about to delete the current entry).                      */
            tmpEventCallback = EventCallback->NextANTMEventCallbackInfoPtr;

            /* Go ahead and delete the Event Callback Entry and clean up*/
            /* the resources.                                           */
            if((EventCallback = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, EventCallback->EventCallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreeEventCallbackInfoEntryMemory(EventCallback);
            }

            /* Go ahead and set the next Event Callback Entry (past the */
            /* one we just deleted).                                    */
            EventCallback = tmpEventCallback;
         }
         else
            EventCallback = EventCallback->NextANTMEventCallbackInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Startup Message event.                              */
static void ProcessANTStartupMessageEvent(ANT_Startup_Message_Event_Data_t *ANTEventData)
{
   ANTM_Event_Data_t      EventData;
   ANTM_Startup_Message_t StartupMessageMessage;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = aetANTMStartupMessage;
      EventData.EventLength                                      = ANTM_STARTUP_MESSAGE_EVENT_DATA_SIZE;
      EventData.EventData.StartupMessageEventData.StartupMessage = ANTEventData->Startup_Message;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&StartupMessageMessage, 0, sizeof(StartupMessageMessage));

      StartupMessageMessage.MessageHeader.AddressID       = 0;
      StartupMessageMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
      StartupMessageMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      StartupMessageMessage.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_STARTUP_MESSAGE;
      StartupMessageMessage.MessageHeader.MessageLength   = (ANTM_STARTUP_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      StartupMessageMessage.StartupMessage                = ANTEventData->Startup_Message;

      /* Dispatch the ANT+ event to all registered callbacks.           */
      DispatchANTMEvent(&EventData, (BTPM_Message_t *)&StartupMessageMessage, &(EventData.EventData.StartupMessageEventData.CallbackID), &(StartupMessageMessage.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Channel Response event.                             */
static void ProcessANTChannelResponseEvent(ANT_Channel_Response_Event_Data_t *ANTEventData)
{
   ANTM_Event_Data_t               EventData;
   ANTM_Channel_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = aetANTMChannelResponse;
      EventData.EventLength                                      = ANTM_CHANNEL_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.ChannelResponseEventData.ChannelNumber = ANTEventData->Channel_Number;
      EventData.EventData.ChannelResponseEventData.MessageID     = ANTEventData->Message_ID;
      EventData.EventData.ChannelResponseEventData.MessageCode   = ANTEventData->Message_Code;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      Message.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_CHANNEL_RESPONSE;
      Message.MessageHeader.MessageLength   = (ANTM_CHANNEL_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ChannelNumber                 = ANTEventData->Channel_Number;
      Message.MessageID                     = ANTEventData->Message_ID;
      Message.MessageCode                   = ANTEventData->Message_Code;

      /* Dispatch the ANT+ event to all registered callbacks.           */
      DispatchANTMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.ChannelResponseEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Channel Status event.                               */
static void ProcessANTChannelStatusEvent(ANT_Channel_Status_Event_Data_t *ANTEventData)
{
   ANTM_Event_Data_t             EventData;
   ANTM_Channel_Status_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                      = aetANTMChannelStatus;
      EventData.EventLength                                    = ANTM_CHANNEL_STATUS_EVENT_DATA_SIZE;
      EventData.EventData.ChannelStatusEventData.ChannelNumber = ANTEventData->Channel_Number;
      EventData.EventData.ChannelStatusEventData.ChannelStatus = ANTEventData->Channel_Status;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      Message.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_CHANNEL_STATUS;
      Message.MessageHeader.MessageLength   = (ANTM_CHANNEL_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ChannelNumber                 = ANTEventData->Channel_Number;
      Message.ChannelStatus                 = ANTEventData->Channel_Status;

      /* Dispatch the ANT+ event to all registered callbacks.           */
      DispatchANTMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.ChannelStatusEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Channel ID event.                                   */
static void ProcessANTChannelIDEvent(ANT_Channel_ID_Event_Data_t *ANTEventData)
{
   ANTM_Event_Data_t         EventData;
   ANTM_Channel_ID_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                     = aetANTMChannelID;
      EventData.EventLength                                   = ANTM_CHANNEL_ID_EVENT_DATA_SIZE;
      EventData.EventData.ChannelIDEventData.ChannelNumber    = ANTEventData->Channel_Number;
      EventData.EventData.ChannelIDEventData.DeviceNumber     = ANTEventData->Device_Number;
      EventData.EventData.ChannelIDEventData.DeviceTypeID     = ANTEventData->Device_Type_ID;
      EventData.EventData.ChannelIDEventData.TransmissionType = ANTEventData->Transmission_Type;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      Message.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_CHANNEL_ID;
      Message.MessageHeader.MessageLength   = (ANTM_CHANNEL_ID_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ChannelNumber                 = ANTEventData->Channel_Number;
      Message.DeviceNumber                  = ANTEventData->Device_Number;
      Message.DeviceTypeID                  = ANTEventData->Device_Type_ID;
      Message.TransmissionType              = ANTEventData->Transmission_Type;

      /* Dispatch the ANT+ event to all registered callbacks.           */
      DispatchANTMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.ChannelIDEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Version event.                                      */
static void ProcessANTVersionEvent(ANT_Version_Event_Data_t *ANTEventData)
{
   unsigned long               MemorySize;
   ANTM_Event_Data_t           EventData;
   ANTM_ANT_Version_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_VERSION_MESSAGE_SIZE(sizeof(ANTEventData->Version_Data));
      if((Message = (ANTM_ANT_Version_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                        = aetANTMANTVersion;
         EventData.EventLength                                      = ANTM_ANT_VERSION_EVENT_DATA_SIZE;
         EventData.EventData.ANTVersionEventData.VersionDataLength  = sizeof(ANTEventData->Version_Data);
         EventData.EventData.ANTVersionEventData.VersionData        = (Byte_t *)&(ANTEventData->Version_Data[0]);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_ANT_VERSION;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->VersionDataLength             = sizeof(ANTEventData->Version_Data);
         BTPS_MemCopy(Message->VersionData, ANTEventData->Version_Data, sizeof(ANTEventData->Version_Data));

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.ANTVersionEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Capabilities event.                                 */
static void ProcessANTCapabilitiesEvent(ANT_Capabilities_Event_Data_t *ANTEventData)
{
   ANTM_Event_Data_t           EventData;
   ANTM_Capabilities_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = aetANTMCapabilities;
      EventData.EventLength                                      = ANTM_CAPABILITIES_EVENT_DATA_SIZE;
      EventData.EventData.CapabilitiesEventData.MaxChannels      = ANTEventData->Max_Channels;
      EventData.EventData.CapabilitiesEventData.MaxNetworks      = ANTEventData->Max_Networks;
      EventData.EventData.CapabilitiesEventData.StandardOptions  = ANTEventData->Standard_Options;
      EventData.EventData.CapabilitiesEventData.AdvancedOptions  = ANTEventData->Advanced_Options;
      EventData.EventData.CapabilitiesEventData.AdvancedOptions2 = ANTEventData->Advanced_Options2;
      EventData.EventData.CapabilitiesEventData.Reserved         = ANTEventData->Reserved;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      Message.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_CAPABILITIES;
      Message.MessageHeader.MessageLength   = (ANTM_CAPABILITIES_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.MaxChannels                   = ANTEventData->Max_Channels;
      Message.MaxNetworks                   = ANTEventData->Max_Networks;
      Message.StandardOptions               = ANTEventData->Standard_Options;
      Message.AdvancedOptions               = ANTEventData->Advanced_Options;
      Message.AdvancedOptions2              = ANTEventData->Advanced_Options2;
      Message.Reserved                      = ANTEventData->Reserved;

      /* Dispatch the ANT+ event to all registered callbacks.           */
      DispatchANTMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.CapabilitiesEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Broadcast Data received event.                      */
static void ProcessANTBroadcastDataEvent(ANT_Packet_Broadcast_Data_Event_Data_t *ANTEventData)
{
   unsigned long                         MemorySize;
   ANTM_Event_Data_t                     EventData;
   ANTM_Broadcast_Data_Packet_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_BROADCAST_DATA_PACKET_MESSAGE_SIZE(ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);
      if((Message = (ANTM_Broadcast_Data_Packet_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                            = aetANTMBroadcastDataPacket;
         EventData.EventLength                                          = ANTM_BROADCAST_DATA_PACKET_EVENT_DATA_SIZE;
         EventData.EventData.BroadcastDataPacketEventData.ChannelNumber = ANTEventData->Channel_Number;
         EventData.EventData.BroadcastDataPacketEventData.DataLength    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         EventData.EventData.BroadcastDataPacketEventData.Data          = &(ANTEventData->Data[0]);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_BROADCAST_DATA_PACKET;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->ChannelNumber                 = ANTEventData->Channel_Number;
         Message->DataLength                    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         BTPS_MemCopy(Message->Data, ANTEventData->Data, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.BroadcastDataPacketEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Acknowledged Data received event.                   */
static void ProcessANTAcknowledgedDataEvent(ANT_Packet_Acknowledged_Data_Event_Data_t *ANTEventData)
{
   unsigned long                            MemorySize;
   ANTM_Event_Data_t                        EventData;
   ANTM_Acknowledged_Data_Packet_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_ACKNOWLEDGED_DATA_PACKET_MESSAGE_SIZE(ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);
      if((Message = (ANTM_Acknowledged_Data_Packet_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                               = aetANTMAcknowledgedDataPacket;
         EventData.EventLength                                             = ANTM_ACKNOWLEDGED_DATA_PACKET_EVENT_DATA_SIZE;
         EventData.EventData.AcknowledgedDataPacketEventData.ChannelNumber = ANTEventData->Channel_Number;
         EventData.EventData.AcknowledgedDataPacketEventData.DataLength    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         EventData.EventData.AcknowledgedDataPacketEventData.Data          = &(ANTEventData->Data[0]);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_ACKNOWLEDGED_DATA_PACKET;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->ChannelNumber                 = ANTEventData->Channel_Number;
         Message->DataLength                    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         BTPS_MemCopy(Message->Data, ANTEventData->Data, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.AcknowledgedDataPacketEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Burst Data received event.                          */
static void ProcessANTBurstDataEvent(ANT_Packet_Burst_Data_Event_Data_t *ANTEventData)
{
   unsigned long                     MemorySize;
   ANTM_Event_Data_t                 EventData;
   ANTM_Burst_Data_Packet_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_BURST_DATA_PACKET_MESSAGE_SIZE(ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);
      if((Message = (ANTM_Burst_Data_Packet_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                = aetANTMBurstDataPacket;
         EventData.EventLength                                              = ANTM_BURST_DATA_PACKET_EVENT_DATA_SIZE;
         EventData.EventData.BurstDataPacketEventData.SequenceChannelNumber = ANTEventData->Sequence_Channel_Number;
         EventData.EventData.BurstDataPacketEventData.DataLength            = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         EventData.EventData.BurstDataPacketEventData.Data                  = &(ANTEventData->Data[0]);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_BURST_DATA_PACKET;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->SequenceChannelNumber         = ANTEventData->Sequence_Channel_Number;
         Message->DataLength                    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         BTPS_MemCopy(Message->Data, ANTEventData->Data, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.BurstDataPacketEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Extended Broadcast Data received event.             */
static void ProcessANTExtendedBroadcastDataEvent(ANT_Packet_Extended_Broadcast_Data_Event_Data_t *ANTEventData)
{
   unsigned long                                  MemorySize;
   ANTM_Event_Data_t                              EventData;
   ANTM_Extended_Broadcast_Data_Packet_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_EXTENDED_BROADCAST_DATA_PACKET_MESSAGE_SIZE(ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);
      if((Message = (ANTM_Extended_Broadcast_Data_Packet_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                       = aetANTMExtendedBroadcastDataPacket;
         EventData.EventLength                                                     = ANTM_EXTENDED_BROADCAST_DATA_PACKET_EVENT_DATA_SIZE;
         EventData.EventData.ExtendedBroadcastDataPacketEventData.ChannelNumber    = ANTEventData->Channel_Number;
         EventData.EventData.ExtendedBroadcastDataPacketEventData.DeviceNumber     = ANTEventData->Device_Number;
         EventData.EventData.ExtendedBroadcastDataPacketEventData.DeviceType       = ANTEventData->Device_Type;
         EventData.EventData.ExtendedBroadcastDataPacketEventData.TransmissionType = ANTEventData->Transmission_Type;
         EventData.EventData.ExtendedBroadcastDataPacketEventData.DataLength       = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         EventData.EventData.ExtendedBroadcastDataPacketEventData.Data             = &(ANTEventData->Data[0]);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_EXTENDED_BROADCAST_DATA_PACKET;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->ChannelNumber                 = ANTEventData->Channel_Number;
         Message->DeviceNumber                  = ANTEventData->Device_Number;
         Message->DeviceType                    = ANTEventData->Device_Type;
         Message->TransmissionType              = ANTEventData->Transmission_Type;
         Message->DataLength                    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         BTPS_MemCopy(Message->Data, ANTEventData->Data, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.ExtendedBroadcastDataPacketEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Extended Acknowledged Data received event.          */
static void ProcessANTExtendedAcknowledgedDataEvent(ANT_Packet_Extended_Acknowledged_Data_Event_Data_t *ANTEventData)
{
   unsigned long                                     MemorySize;
   ANTM_Event_Data_t                                 EventData;
   ANTM_Extended_Acknowledged_Data_Packet_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_EXTENDED_ACKNOWLEDGED_DATA_PACKET_MESSAGE_SIZE(ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);
      if((Message = (ANTM_Extended_Acknowledged_Data_Packet_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                          = aetANTMExtendedAcknowledgedDataPacket;
         EventData.EventLength                                                        = ANTM_EXTENDED_ACKNOWLEDGED_DATA_PACKET_EVENT_DATA_SIZE;
         EventData.EventData.ExtendedAcknowledgedDataPacketEventData.ChannelNumber    = ANTEventData->Channel_Number;
         EventData.EventData.ExtendedAcknowledgedDataPacketEventData.DeviceNumber     = ANTEventData->Device_Number;
         EventData.EventData.ExtendedAcknowledgedDataPacketEventData.DeviceType       = ANTEventData->Device_Type;
         EventData.EventData.ExtendedAcknowledgedDataPacketEventData.TransmissionType = ANTEventData->Transmission_Type;
         EventData.EventData.ExtendedAcknowledgedDataPacketEventData.DataLength       = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         EventData.EventData.ExtendedAcknowledgedDataPacketEventData.Data             = &(ANTEventData->Data[0]);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_EXTENDED_ACKNOWLEDGED_DATA_PACKET;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->ChannelNumber                 = ANTEventData->Channel_Number;
         Message->DeviceNumber                  = ANTEventData->Device_Number;
         Message->DeviceType                    = ANTEventData->Device_Type;
         Message->TransmissionType              = ANTEventData->Transmission_Type;
         Message->DataLength                    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         BTPS_MemCopy(Message->Data, ANTEventData->Data, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.ExtendedAcknowledgedDataPacketEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Extended Burst Data received event.                 */
static void ProcessANTExtendedBurstDataEvent(ANT_Packet_Extended_Burst_Data_Event_Data_t *ANTEventData)
{
   unsigned long                              MemorySize;
   ANTM_Event_Data_t                          EventData;
   ANTM_Extended_Burst_Data_Packet_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_EXTENDED_BURST_DATA_PACKET_MESSAGE_SIZE(ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);
      if((Message = (ANTM_Extended_Burst_Data_Packet_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                        = aetANTMExtendedBurstDataPacket;
         EventData.EventLength                                                      = ANTM_EXTENDED_BURST_DATA_PACKET_EVENT_DATA_SIZE;
         EventData.EventData.ExtendedBurstDataPacketEventData.SequenceChannelNumber = ANTEventData->Sequence_Channel_Number;
         EventData.EventData.ExtendedBurstDataPacketEventData.DeviceNumber          = ANTEventData->Device_Number;
         EventData.EventData.ExtendedBurstDataPacketEventData.DeviceType            = ANTEventData->Device_Type;
         EventData.EventData.ExtendedBurstDataPacketEventData.TransmissionType      = ANTEventData->Transmission_Type;
         EventData.EventData.ExtendedBurstDataPacketEventData.DataLength            = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         EventData.EventData.ExtendedBurstDataPacketEventData.Data                  = &(ANTEventData->Data[0]);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_EXTENDED_BURST_DATA_PACKET;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->SequenceChannelNumber         = ANTEventData->Sequence_Channel_Number;
         Message->DeviceNumber                  = ANTEventData->Device_Number;
         Message->DeviceType                    = ANTEventData->Device_Type;
         Message->TransmissionType              = ANTEventData->Transmission_Type;
         Message->DataLength                    = ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE;
         BTPS_MemCopy(Message->Data, ANTEventData->Data, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE);

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.ExtendedBurstDataPacketEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a ANT Raw Packet Data received event.                     */
static void ProcessANTRawPacketDataEvent(ANT_Raw_Packet_Data_Event_Data_t *ANTEventData)
{
   unsigned long                   MemorySize;
   ANTM_Event_Data_t               EventData;
   ANTM_Raw_Data_Packet_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ANTEventData)
   {
      /* Allocate memory for the message.                               */
      MemorySize = ANTM_EXTENDED_BURST_DATA_PACKET_MESSAGE_SIZE(ANTEventData->PacketLength);
      if((Message = (ANTM_Raw_Data_Packet_Message_t *)BTPS_AllocateMemory(MemorySize)) != NULL)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                   = aetANTMRawDataPacket;
         EventData.EventLength                                 = ANTM_RAW_DATA_PACKET_EVENT_DATA_SIZE;
         EventData.EventData.RawDataPacketEventData.DataLength = ANTEventData->PacketLength;
         EventData.EventData.RawDataPacketEventData.Data       = ANTEventData->PacketData;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, MemorySize);

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         Message->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_RAW_DATA_PACKET;
         Message->MessageHeader.MessageLength   = (MemorySize - BTPM_MESSAGE_HEADER_SIZE);

         Message->DataLength                    = ANTEventData->PacketLength;
         BTPS_MemCopy(Message->Data, ANTEventData->PacketData, Message->DataLength);

         /* Dispatch the ANT+ event to all registered callbacks.        */
         DispatchANTMEvent(&EventData, (BTPM_Message_t *)Message, &(EventData.EventData.ExtendedBurstDataPacketEventData.CallbackID), &(Message->EventHandlerID));

         /* Free the previously allocated memory.                       */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing ANT Events that have been received.  This function     */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessANTEvent(ANTM_Update_Event_Data_t *ANTEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(ANTEventData)
   {
      /* Process the event based on the event type.                     */
      switch(ANTEventData->Event_Data_Type)
      {
         case etStartup_Message:
            /* Process the ANT Startup Message event.                   */
            ProcessANTStartupMessageEvent(&(ANTEventData->Event_Data.ANT_Startup_Message_Event_Data));
            break;
         case etChannel_Response:
            /* Process the ANT Channel Response event.                  */
            ProcessANTChannelResponseEvent(&(ANTEventData->Event_Data.ANT_Channel_Response_Event_Data));
            break;
         case etChannel_Status:
            /* Process the ANT Channel Status event.                    */
            ProcessANTChannelStatusEvent(&(ANTEventData->Event_Data.ANT_Channel_Status_Event_Data));
            break;
         case etChannel_ID:
            /* Process the ANT Channel ID event.                        */
            ProcessANTChannelIDEvent(&(ANTEventData->Event_Data.ANT_Channel_ID_Event_Data));
            break;
         case etANT_Version:
            /* Process the ANT Version event.                           */
            ProcessANTVersionEvent(&(ANTEventData->Event_Data.ANT_Version_Event_Data));
            break;
         case etCapabilities:
            /* Process the ANT Capabilities event.                      */
            ProcessANTCapabilitiesEvent(&(ANTEventData->Event_Data.ANT_Capabilities_Event_Data));
            break;
         case etPacket_Broadcast_Data:
            /* Process the ANT Broadcast Data Packet received event.    */
            ProcessANTBroadcastDataEvent(&(ANTEventData->Event_Data.ANT_Packet_Broadcast_Data_Event_Data));
            break;
         case etPacket_Acknowledged_Data:
            /* Process the ANT Acknowledged Data Packet received event. */
            ProcessANTAcknowledgedDataEvent(&(ANTEventData->Event_Data.ANT_Packet_Acknowledged_Data_Event_Data));
            break;
         case etPacket_Burst_Data:
            /* Process the ANT Burst Data Packet received event.        */
            ProcessANTBurstDataEvent(&(ANTEventData->Event_Data.ANT_Packet_Burst_Data_Event_Data));
            break;
         case etPacket_Extended_Broadcast_Data:
            /* Process the ANT Extended Broadcast Data Packet received  */
            /* event.                                                   */
            ProcessANTExtendedBroadcastDataEvent(&(ANTEventData->Event_Data.ANT_Packet_Extended_Broadcast_Data_Event_Data));
            break;
         case etPacket_Extended_Acknowledged_Data:
            /* Process the ANT Extended Acknowledged Data Packet        */
            /* received event.                                          */
            ProcessANTExtendedAcknowledgedDataEvent(&(ANTEventData->Event_Data.ANT_Packet_Extended_Acknowledged_Data_Event_Data));
            break;
         case etPacket_Extended_Burst_Data:
            /* Process the ANT Extended Burst Data Packet received      */
            /* event.                                                   */
            ProcessANTExtendedBurstDataEvent(&(ANTEventData->Event_Data.ANT_Packet_Extended_Burst_Data_Event_Data));
            break;
         case etRaw_Packet_Data:
            /* Process the ANT Raw Packet Data received event.          */
            ProcessANTRawPacketDataEvent(&(ANTEventData->Event_Data.ANT_Raw_Packet_Data_Event_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown ANT Event Type: %d\n", ANTEventData->Event_Data_Type));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid ANT Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process ANT Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_ANTM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process ANT Events.                                 */
static void BTPSAPI BTPMDispatchCallback_ANT(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an ANT Event Update.           */
            if(((ANTM_Update_Data_t *)CallbackParameter)->UpdateType == utANTEvent)
               ProcessANTEvent(&(((ANTM_Update_Data_t *)CallbackParameter)->UpdateData.ANTEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all ANT Manager Messages.   */
static void BTPSAPI ANTManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANT Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a ANT Manager defined    */
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
               /* ANT Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_ANTM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue ANT Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue ANT Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an ANT Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Non ANT Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager ANT Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI ANTM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int           Result;
   unsigned long Flags;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANT Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process ANT Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER, ANTManagerGroupHandler, NULL))
         {
            /* Check whether InitializationData was supplied and note   */
            /* the flags if present.                                    */
            if(InitializationData)
               Flags = ((ANTM_Initialization_Info_t *)InitializationData)->InitializationFlags;
            else
               Flags = 0;

            /* Initialize the actual ANT Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the ANT Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _ANTM_Initialize(Flags)))
            {
               /* Determine the current Device Power State.             */
               CurrentPowerState   = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               /* Initialize a unique, starting ANT Callback ID.        */
               NextEventCallbackID = 0;

               /* Go ahead and flag that this module is initialized.    */
               Initialized         = TRUE;

               /* Flag success.                                         */
               Result              = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _ANTM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANT Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the ANT Manager Implementation that  */
            /* we are shutting down.                                    */
            _ANTM_Cleanup();

            /* Free the Event Callback Info List.                       */
            FreeEventCallbackInfoList(&EventCallbackInfoList);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI ANTM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the ANT Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _ANTM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the ANT Manager that the Stack has been closed.*/
               _ANTM_SetBluetoothStackID(0);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the ANT Manager of a specific Update Event.  The ANT    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t ANTM_NotifyUpdate(ANTM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utANTEvent:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing ANT Event: %d\n", UpdateData->UpdateData.ANTEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_ANT, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a event callback function with the ANT+       */
   /* Manager Service.  This Callback will be dispatched by the ANT+    */
   /* Manager when various ANT+ Manager Events occur.  This function    */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a ANT+ Manager Event needs to be      */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANTM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
int BTPSAPI ANTM_Register_Event_Callback(ANTM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t  EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that that no callbacks are currently registered   */
            /* (only one client can register and control the ANT+       */
            /* system).                                                 */
            if(EventCallbackInfoList == NULL)
            {
               /* Attempt to add an entry into the Event Callback Entry */
               /* list.                                                 */
               BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(ANTM_Event_Callback_Info_t));

               EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
               EventCallbackEntry.ClientID          = MSG_GetServerAddressID();
               EventCallbackEntry.EventCallback     = CallbackFunction;
               EventCallbackEntry.CallbackParameter = CallbackParameter;

               if(AddEventCallbackInfoEntry(&EventCallbackInfoList, &EventCallbackEntry))
                  ret_val = EventCallbackEntry.EventCallbackID;
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANT+ Manager Event Callback   */
   /* (registered via a successful call to the                          */
   /* ANTM_Register_Event_Callback() function).  This function accepts  */
   /* as input the ANT+ Manager Event Callback ID (return value from    */
   /* ANTM_Register_Event_Callback() function).                         */
void BTPSAPI ANTM_Un_Register_Event_Callback(unsigned int ANTManagerCallbackID)
{
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is doing the un-registering.                             */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
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

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* Configuration Message API.                                        */

   /* The following function is responsible for assigning an ANT channel*/
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to register.  This function   */
   /* accepts as it's third argument, the channel type to be assigned to*/
   /* the channel.  This function accepts as it's fourth argument, the  */
   /* network number to be used for the channel.  Zero should be        */
   /* specified for this argument to use the default public network.    */
   /* This function accepts as it's fifth argument, the extended        */
   /* assignment to be used for the channel.  Zero should be specified  */
   /* for this argument if no extended capabilities are to be used.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int BTPSAPI ANTM_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ChannelType, unsigned int NetworkNumber, unsigned int ExtendedAssignment)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Assign_Channel(ChannelNumber, ChannelType, NetworkNumber, ExtendedAssignment);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for un-assigning an ANT     */
   /* channel on the local ANT+ system.  A channel must be unassigned   */
   /* before it can be reassigned using the ANTM_Assign_Channel() API.  */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to un-assign.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
int BTPSAPI ANTM_Un_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Un_Assign_Channel(ChannelNumber);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, the channel number to configure.  The ANT   */
   /* channel must be assigned using ANTM_Assign_Channel() before       */
   /* calling this function.  This function accepts as it's third       */
   /* argument, the device number to search for on the channel.  Zero   */
   /* should be specified for this argument to scan for any device      */
   /* number.  This function accepts as it's fourth argument, the device*/
   /* type to search for on the channel.  Zero should be specified for  */
   /* this argument to scan for any device type.  This function accepts */
   /* as it's fifth argument, the transmission type to search for on the*/
   /* channel.  Zero should be specified for this argument to scan for  */
   /* any transmission type.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
int BTPSAPI ANTM_Set_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Channel_ID(ChannelNumber, DeviceNumber, DeviceType, TransmissionType);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the local ANT+ system.     */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel messaging period to   */
   /* set on the channel.  This function returns zero if successful,    */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
int BTPSAPI ANTM_Set_Channel_Period(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessagingPeriod)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Channel_Period(ChannelNumber, MessagingPeriod);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument the     */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the search timeout to set on the  */
   /* channel.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will ret_val in an infinite search timeout.  Specifying  */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
int BTPSAPI ANTM_Set_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Channel_Search_Timeout(ChannelNumber, SearchTimeout);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel.  This function accepts as it's first*/
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel frequency to set on   */
   /* the channel.  This function returns zero if successful, otherwise */
   /* this function returns a negative error code.                      */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (2400 + RFFrequency) MHz.                        */
int BTPSAPI ANTM_Set_Channel_RF_Frequency(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int RFFrequency)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Channel_RF_Frequency(ChannelNumber, RFFrequency);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the network */
   /* key for an ANT channel.  This function accepts as it's first      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, a pointer to the ANT network key  */
   /* to set on the channel.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
int BTPSAPI ANTM_Set_Network_Key(unsigned int ANTManagerCallbackID, unsigned int NetworkNumber, ANT_Network_Key_t NetworkKey)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Network_Key(NetworkNumber, NetworkKey);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the transmit*/
   /* power on the local ANT system.  This function accepts as it's     */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument the transmit power to set on the device.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int BTPSAPI ANTM_Set_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int TransmitPower)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Transmit_Power(TransmitPower);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to add to the */
   /* list.  This function accepts as it's third argument, the device   */
   /* number to add to the list.  This function accepts as it's fourth  */
   /* argument, the device type to add to the list.  This function      */
   /* accepts as it's fifth argument, the transmission type to add to   */
   /* the list.  This function accepts as it's sixth argument, the the  */
   /* list index to overwrite with the updated entry.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Add_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType, unsigned int ListIndex)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Add_Channel_ID(ChannelNumber, DeviceNumber, DeviceType, TransmissionType, ListIndex);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* on which the list should be configured.  This function accepts as */
   /* it's third argument, the size of the list.  This function accepts */
   /* as it's fourth argument, the list type.  Zero should be specified */
   /* to configure the list for inclusion, and one should be specified  */
   /* to configure the list for exclusion.  This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Configure_Inclusion_Exclusion_List(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ListSize, unsigned int Exclude)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Configure_Inclusion_Exclusion_List(ChannelNumber, ListSize, Exclude);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the transmit power level for the  */
   /* specified channel.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Set_Channel_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int TransmitPower)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Channel_Transmit_Power(ChannelNumber, TransmitPower);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to   */
   /* configure.  This function accepts as it's third argument, the     */
   /* search timeout to set on the channel.  This function returns zero */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will ret_val in an  */
   /*          infinite low priority search timeout.                    */
int BTPSAPI ANTM_Set_Low_Priority_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Low_Priority_Channel_Search_Timeout(ChannelNumber, SearchTimeout);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function configures the   */
   /* channel ID in the same way as ANTM_Set_Channel_ID(), except it    */
   /* uses the two LSB of the device's serial number as the device's    */
   /* number.  This function accepts as it's first argument the Callback*/
   /* ID that was returned from a successful call                       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  The ANT channel*/
   /* must be assigned using ANTM_Assign_Channel() before calling this  */
   /* function.  This function accepts as it's third argument, the      */
   /* device type to search for on the channel.  Zero should be         */
   /* specified for this argument to scan for any device type.  This    */
   /* function accepts as it's fourth argument, the transmission type to*/
   /* search for on the channel.  Zero should be specified for this     */
   /* argument to scan for any transmission type.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Set_Serial_Number_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Serial_Number_Channel_ID(ChannelNumber, DeviceType, TransmissionType);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument whether or not to enable extended Rx messages.    */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Enable_Extended_Messages(unsigned int ANTManagerCallbackID, Boolean_t Enable)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Enable_Extended_Messages((unsigned int)(Enable?ANT_EXTENDED_MESSAGES_ENABLE:ANT_EXTENDED_MESSAGES_DISABLE));
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* the LED on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, whether or not to enable the LED.  This     */
   /* function returns zero if successful, otherwise this function      */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int BTPSAPI ANTM_Enable_LED(unsigned int ANTManagerCallbackID, Boolean_t Enable)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Enable_LED((unsigned int)(Enable?ANT_LED_ENABLE:ANT_LED_DISABLE));
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the local ANT+ system.  This function accepts as */
   /* it's only argument the Callback ID that was returned from a       */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
int BTPSAPI ANTM_Enable_Crystal(unsigned int ANTManagerCallbackID)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Enable_Crystal();
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the bitmask of extended */
   /* Rx messages that shall be enabled or disabled.  This function     */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Configure_Extended_Messages(unsigned int ANTManagerCallbackID, unsigned int EnabledExtendedMessagesMask)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Configure_Extended_Messages(EnabledExtendedMessagesMask);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel.  This function accepts  */
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to configure. */
   /* This function accepts as it's third, fourth, and fifth arguments, */
   /* the three operating agility frequencies to set.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANTM_Assign_Channel.  Frequency agility      */
   /*          should NOT be used with shared, Tx only, or Rx only      */
   /*          channels.                                                */
int BTPSAPI ANTM_Configure_Frequency_Agility(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int FrequencyAgility1, unsigned int FrequencyAgility2, unsigned int FrequencyAgility3)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Configure_Frequency_Agility(ChannelNumber, FrequencyAgility1, FrequencyAgility2, FrequencyAgility3);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the local ANT+ system.  This      */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search threshold to set.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
int BTPSAPI ANTM_Set_Proximity_Search(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchThreshold)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Proximity_Search(ChannelNumber, SearchThreshold);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search priority to set.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Set_Channel_Search_Priority(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchPriority)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_Channel_Search_Priority(ChannelNumber, SearchPriority);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the local ANT+ system.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the descriptor string type to    */
   /* set.  This function accepts as it's third argument, the           */
   /* NULL-terminated descriptor string to be set.  This function       */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
int BTPSAPI ANTM_Set_USB_Descriptor_String(unsigned int ANTManagerCallbackID, unsigned int StringNumber, char *DescriptorString)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ANTManagerCallbackID) && (DescriptorString))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_USB_Descriptor_String(StringNumber, DescriptorString);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Control Message API.                                              */

   /* The following function is responsible for resetting the ANT module*/
   /* on the local ANT+ system.  This function accepts as it's only     */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  A delay of at least 500ms is     */
   /* suggested after calling this function to allow time for the module*/
   /* to reset.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
int BTPSAPI ANTM_Reset_System(unsigned int ANTManagerCallbackID)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Reset_System();
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for opening an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  The channel    */
   /* specified must have been assigned and configured before calling   */
   /* this function.  This function returns zero if successful,         */
   /* otherwise this function returns a negative error code.            */
int BTPSAPI ANTM_Open_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Open_Channel(ChannelNumber);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for closing an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the aetANTMChannelResponse event has been received */
   /*          with the Message_Code member specifying:                 */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
int BTPSAPI ANTM_Close_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Close_Channel(ChannelNumber);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number that the request will be sent */
   /* to.  This function accepts as it's third argument, the message ID */
   /* being requested from the channel.  This function returns zero if  */
   /* successful, otherwise this function returns a negative error code.*/
int BTPSAPI ANTM_Request_Message(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessageID)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Request_Message(ChannelNumber, MessageID);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the local ANT+ system.  This function  */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to be*/
   /* opened.  The channel specified must have been assigned and        */
   /* configured as a SLAVE Rx ONLY channel before calling this         */
   /* function.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
int BTPSAPI ANTM_Open_Rx_Scan_Mode(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Open_Rx_Scan_Mode(ChannelNumber);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for putting the ANT+ system */
   /* in ultra low-power mode.  This function accepts as it's only      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(!MSGREADY) line on the ANT chip to the appropriate*/
   /*          value.                                                   */
int BTPSAPI ANTM_Sleep_Message(unsigned int ANTManagerCallbackID)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Sleep_Message();
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Data Message API.                                                 */

   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the local ANT+ system.  This function      */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be broadcast on.  This function accepts as it's     */
   /* third argument the length of the data to send.  This function     */
   /* accepts as it's fourth argument a pointer to a byte array of the  */
   /* broadcast data to send.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
int BTPSAPI ANTM_Send_Broadcast_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int                         ret_val;
   Byte_t                      TempData[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ANTManagerCallbackID) && (DataLength) && (Data))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Copy the data into a temporary buffer.             */
                  DataLength = (DataLength > ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE)?ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE:DataLength;

                  BTPS_MemInitialize(TempData, 0, sizeof(TempData));
                  BTPS_MemCopy(TempData, Data, DataLength);

                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Send_Broadcast_Data(ChannelNumber, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE, TempData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be sent on.  This function accepts as it's third    */
   /* argument the length of the data to send.  This function accepts as*/
   /* it's fourth argument, a pointer to a byte array of the            */
   /* acknowledged data to send.  This function returns zero if         */
   /* successful, otherwise this function returns a negative error code.*/
int BTPSAPI ANTM_Send_Acknowledged_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int                         ret_val;
   Byte_t                      TempData[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ANTManagerCallbackID) && (DataLength) && (Data))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Copy the data into a temporary buffer.             */
                  DataLength = (DataLength > ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE)?ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE:DataLength;

                  BTPS_MemInitialize(TempData, 0, sizeof(TempData));
                  BTPS_MemCopy(TempData, Data, DataLength);

                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Send_Acknowledged_Data(ChannelNumber, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE, TempData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the sequence / channel  */
   /* number that the data will be sent on.  The upper three bits of    */
   /* this argument are the sequence number, and the lower five bits are*/
   /* the channel number.  This function accepts as it's third argument */
   /* the length of the data to send.  This function accepts as it's    */
   /* fourth argument, a pointer to a byte array of the burst data to   */
   /* send.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
int BTPSAPI ANTM_Send_Burst_Transfer_Data(unsigned int ANTManagerCallbackID, unsigned int SequenceChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int                         ret_val;
   Byte_t                      TempData[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ANTManagerCallbackID) && (DataLength) && (Data))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Copy the data into a temporary buffer.             */
                  DataLength = (DataLength > ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE)?ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE:DataLength;

                  BTPS_MemInitialize(TempData, 0, sizeof(TempData));
                  BTPS_MemCopy(TempData, Data, DataLength);

                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Send_Burst_Transfer_Data(SequenceChannelNumber, ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE, TempData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Test Mode Message API.                                            */

   /* The following function is responsible for putting the ANT+ system */
   /* in CW test mode.  This function accepts as it's only argument the */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
int BTPSAPI ANTM_Initialize_CW_Test_Mode(unsigned int ANTManagerCallbackID)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Initialize_CW_Test_Mode();
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument the      */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the transmit power level to be used.  This       */
   /* function accepts as it's third argument, the RF frequency to be   */
   /* used.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANTM_Initialize_CW_Test_Mode().                  */
int BTPSAPI ANTM_Set_CW_Test_Mode(unsigned int ANTManagerCallbackID, unsigned int TxPower, unsigned int RFFrequency)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANTManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Set_CW_Test_Mode(TxPower, RFFrequency);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a raw ANT       */
   /* packet.  This function accepts as it's first argument, the        */
   /* Callback ID that was returned from a successful call to           */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int BTPSAPI ANTM_Send_Raw_Packet(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData)
{
   int                         ret_val;
   ANTM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANT Manager has been initialized.   */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ANTManagerCallbackID) && (PacketData) && ((DataSize + sizeof(NonAlignedWord_t)) <= (HCI_COMMAND_MAX_SIZE - HCI_COMMAND_HEADER_SIZE)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the client who */
            /* is issuing the command.                                  */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANTManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the Impl. Mgr. to do the actual work.  */
                  ret_val = _ANTM_Send_Raw_Packet(DataSize, PacketData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a raw ANT       */
   /* packet without waiting for the command to be queued for sending   */
   /* to the chip.  This function accepts as it's first argument,       */
   /* the Callback ID that was returned from a successful call to       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int BTPSAPI ANTM_Send_Raw_Packet_Async(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData)
{
   /* On the PM Server, there is no IPC delay, so just send the packet  */
   /* in the normal fashion and return success.                         */
   ANTM_Send_Raw_Packet(ANTManagerCallbackID, DataSize, PacketData);

   return(0);
}
