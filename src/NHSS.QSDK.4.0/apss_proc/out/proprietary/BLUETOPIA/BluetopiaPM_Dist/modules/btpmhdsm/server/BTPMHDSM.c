/*****< btpmhdsm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDSM - Headset Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/17/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHDSM.h"            /* BTPM HDSET Manager Prototypes/Constants.  */
#include "HDSMAPI.h"             /* HDSET Manager Prototypes/Constants.       */
#include "HDSMMSG.h"             /* BTPM HDSET Manager Message Formats.       */
#include "HDSMGR.h"              /* HDSET Manager Impl. Prototypes/Constants. */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following constant represents the timeout (in milli-seconds)  */
   /* to wait for a Serial Port to close when it is closed by the local */
   /* host.                                                             */
#define MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS               (BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_CLOSE_DELAY_TIME_MS * BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_CLOSE_DELAY_RETRIES)

   /* The following constant represents the number of times that this   */
   /* module will attempt to retry waiting for the Port to Disconnect   */
   /* (before attempting to connect to a remote port) if it is          */
   /* connected.                                                        */
#define MAXIMUM_HANDS_FREE_PORT_OPEN_DELAY_RETRY               (BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_RETRIES)

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHDSM_Entry_Info_t
{
   unsigned int                  CallbackID;
   unsigned int                  ClientID;
   unsigned int                  ConnectionStatus;
   BD_ADDR_t                     ConnectionBD_ADDR;
   Event_t                       ConnectionEvent;
   unsigned long                 Flags;
   HDSM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagHDSM_Entry_Info_t *NextHDSETEntryInfoPtr;
} HDSM_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HDSM_Entry_Info_t structure to denote various state information.  */
#define HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY          0x40000000
#define HDSET_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY     0x80000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   HDSM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking connections.                          */
typedef enum
{
   csIdle,
   csAuthorizing,
   csAuthenticating,
   csEncrypting,
   csConnectingWaiting,
   csConnectingDevice,
   csConnecting,
   csConnected
} Connection_State_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagConnection_Entry_t
{
   HDSM_Connection_Type_t         ConnectionType;
   BD_ADDR_t                      BD_ADDR;
   Boolean_t                      Server;
   unsigned int                   HDSETID;
   unsigned int                   CloseTimerID;
   unsigned int                   CloseTimerCount;
   Connection_State_t             ConnectionState;
   unsigned int                   ServerPort;
   unsigned long                  ConnectionFlags;
   Word_t                         SCOHandle;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

   /* The following enumerated type is used with the DEVM_Status_t      */
   /* structure to denote actual type of the status type information.   */
typedef enum
{
   dstAuthentication,
   dstEncryption,
   dstConnection
} DEVM_Status_Type_t;

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

   /* Variable which tracks the state of incoming connections.          */
static Connection_Entry_t *ConnectionEntryList;

   /* Variables which hold a pointer to the first element in the Headset*/
   /* entry information list (which holds all callbacks tracked by this */
   /* module).                                                          */
static HDSM_Entry_Info_t *HDSETEntryInfoList_AG;
static HDSM_Entry_Info_t *HDSETEntryInfoList_HS;

   /* Variables which hold a pointer to the first element in the Headset*/
   /* control entry information list (which holds all callbacks tracked */
   /* by this module).                                                  */
static HDSM_Entry_Info_t *HDSETEntryInfoList_AG_Control;
static HDSM_Entry_Info_t *HDSETEntryInfoList_HS_Control;

   /* Variables which hold a pointer to the first element in the Headset*/
   /* data entry information list (which holds all callbacks tracked by */
   /* this module).                                                     */
static HDSM_Entry_Info_t *HDSETEntryInfoList_AG_Data;
static HDSM_Entry_Info_t *HDSETEntryInfoList_HS_Data;

   /* Variables which hold the current state of the respective profile  */
   /* roles.                                                            */
static Boolean_t                  AudioGatewaySupported;
static HDSM_Initialization_Data_t AudioGatewayInitializationInfo;

static Boolean_t                  HeadsetSupported;
static HDSM_Initialization_Data_t HeadsetInitializationInfo;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HDSM_Entry_Info_t *AddHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, HDSM_Entry_Info_t *EntryToAdd);
static HDSM_Entry_Info_t *SearchHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID);
static HDSM_Entry_Info_t *DeleteHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHDSETEntryInfoEntryMemory(HDSM_Entry_Info_t *EntryToFree);
static void FreeHDSETEntryInfoList(HDSM_Entry_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HDSM_Connection_Type_t ConnectionType);
static Connection_Entry_t *SearchConnectionEntryHDSETID(Connection_Entry_t **ListHead, unsigned int HDSETID);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HDSM_Connection_Type_t ConnectionType);
static Connection_Entry_t *DeleteConnectionEntryHDSETID(Connection_Entry_t **ListHead, unsigned int HDSETID);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static void DispatchHDSETEvent(Boolean_t ControlOnly, HDSM_Connection_Type_t ConnectionType, HDSM_Event_Data_t *HDSMEventData, BTPM_Message_t *Message);
static void DispatchHDSETAudioDataEvent(HDSM_Connection_Type_t ConnectionType, HDSM_Event_Data_t *HDSMEventData, BTPM_Message_t *Message);

static void ProcessConnectionResponseMessage(HDSM_Connection_Request_Response_Request_t *Message);
static void ProcessConnectRemoteDeviceMessage(HDSM_Connect_Remote_Device_Request_t *Message);
static void ProcessDisconnectDeviceMessage(HDSM_Disconnect_Device_Request_t *Message);
static void ProcessQueryConnectedDevicesMessage(HDSM_Query_Connected_Devices_Request_t *Message);
static void ProcessRegisterHeadsetEventsMessage(HDSM_Register_Headset_Events_Request_t *Message);
static void ProcessUnRegisterHeadsetEventsMessage(HDSM_Un_Register_Headset_Events_Request_t *Message);
static void ProcessRegisterHeadsetDataMessage(HDSM_Register_Headset_Data_Events_Request_t *Message);
static void ProcessUnRegisterHeadsetDataMessage(HDSM_Un_Register_Headset_Data_Events_Request_t *Message);
static void ProcessSetupAudioConnectionMessage(HDSM_Setup_Audio_Connection_Request_t *Message);
static void ProcessReleaseAudioConnectionMessage(HDSM_Release_Audio_Connection_Request_t *Message);
static void ProcessSendAudioDataMessage(HDSM_Send_Audio_Data_Request_t *Message);
static void ProcessQueryConfigurationMessage(HDSM_Query_Current_Configuration_Request_t *Message);
static void ProcessChangeIncomingConnectionFlagsMessage(HDSM_Change_Incoming_Connection_Flags_Request_t *Message);
static void ProcessSetSpeakerGainMessage(HDSM_Set_Speaker_Gain_Request_t *Message);
static void ProcessSetMicrophoneGainMessage(HDSM_Set_Microphone_Gain_Request_t *Message);
static void ProcessSendButtonPressMessage(HDSM_Send_Button_Press_Request_t *Message);
static void ProcessRingIndicationMessage(HDSM_Ring_Indication_Request_t *Message);
static void ProcessQuerySCOConnectionHandleMessage(HDSM_Query_SCO_Connection_Handle_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessOpenRequestIndicationEvent(HDSET_Open_Port_Request_Indication_Data_t *OpenPortRequestIndicationData);
static void ProcessOpenIndicationEvent(HDSET_Open_Port_Indication_Data_t *OpenPortIndicationData);
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HDSET_Open_Port_Confirmation_Data_t *OpenPortConfirmationData);
static void ProcessCloseIndicationEvent(HDSET_Close_Port_Indication_Data_t *ClosePortIndicationData);
static void ProcessRingIndicationEvent(HDSET_Ring_Indication_Data_t *RingIndicationData);
static void ProcessButtonPressedIndicationEvent(HDSET_Button_Pressed_Indication_Data_t *AnswerCallIndicationData);
static void ProcessSpeakerGainIndicationEvent(HDSET_Speaker_Gain_Indication_Data_t *SpeakerGainIndicationData);
static void ProcessMicrophoneGainIndicationEvent(HDSET_Microphone_Gain_Indication_Data_t *MicrophoneGainIndicationData);
static void ProcessAudioConnectionIndicationEvent(HDSET_Audio_Connection_Indication_Data_t *AudioConnectionIndicationData);
static void ProcessAudioDisconnectionIndicationEvent(HDSET_Audio_Disconnection_Indication_Data_t *AudioDisconnectionIndicationData);
static void ProcessAudioDataIndicationEvent(HDSET_Audio_Data_Indication_Data_t *AudioDataIndicationData);

static void ProcessHeadsetEvent(HDSM_Headset_Event_Data_t *HeadsetEventData);

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status);

static void BTPSAPI BTPMDispatchCallback_HDSM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_HDSET(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter);

static void BTPSAPI HeadsetManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique callback ID that can be used to add an entry    */
   /* into the Headset entry information list.                          */
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
static HDSM_Entry_Info_t *AddHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, HDSM_Entry_Info_t *EntryToAdd)
{
   HDSM_Entry_Info_t *AddedEntry = NULL;
   HDSM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HDSM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HDSM_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHDSETEntryInfoPtr = NULL;

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
                     FreeHDSETEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHDSETEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHDSETEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHDSETEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified callback ID.  This function returns NULL if either the  */
   /* list head is invalid, the callback ID is invalid, or the specified*/
   /* callback ID was NOT found.                                        */
static HDSM_Entry_Info_t *SearchHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HDSM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHDSETEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Headset entry       */
   /* information list for the specified callback ID and removes it from*/
   /* the List.  This function returns NULL if either the Headset entry */
   /* information list head is invalid, the callback ID is invalid, or  */
   /* the specified callback ID was NOT present in the list.  The entry */
   /* returned will have the next entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHDSETEntryInfoEntryMemory().                 */
static HDSM_Entry_Info_t *DeleteHDSETEntryInfoEntry(HDSM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HDSM_Entry_Info_t *FoundEntry = NULL;
   HDSM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHDSETEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHDSETEntryInfoPtr = FoundEntry->NextHDSETEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHDSETEntryInfoPtr;

         FoundEntry->NextHDSETEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Headset entry information       */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeHDSETEntryInfoEntryMemory(HDSM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Headset entry information list.  Upon    */
   /* return of this function, the head pointer is set to NULL.         */
static void FreeHDSETEntryInfoList(HDSM_Entry_Info_t **ListHead)
{
   HDSM_Entry_Info_t *EntryToFree;
   HDSM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHDSETEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeHDSETEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified entry to the specified  */
   /* list.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the entry passed into this function.   */
   /* This function will return NULL if NO entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* list head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *AddedEntry = NULL;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Format a NULL BD_ADDR to test against.                         */
      ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

      /* Make sure that the element that we are adding seems semi-valid.*/
      if(!COMPARE_BD_ADDR(NULL_BD_ADDR, EntryToAdd->BD_ADDR))
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Connection_Entry_t *)BTPS_AllocateMemory(sizeof(Connection_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                        = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextConnectionEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeConnectionEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextConnectionEntryPtr)
                        tmpEntry = tmpEntry->NextConnectionEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextConnectionEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified connection entry    */
   /* list for the specified connection entry based on the specified    */
   /* Bluetooth device address and connection type.  This function      */
   /* returns NULL if either the connection entry list head is invalid, */
   /* the Bluetooth device address is invalid, or the specified entry   */
   /* was NOT present in the list.                                      */
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HDSM_Connection_Type_t ConnectionType)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (ConnectionType != FoundEntry->ConnectionType)))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified connection entry    */
   /* list for the specified connection entry based on the specified    */
   /* Headset Port ID.  This function returns NULL if either the        */
   /* connection entry list head is invalid, the Headset Port ID is     */
   /* invalid, or the specified entry was NOT present in the list.      */
static Connection_Entry_t *SearchConnectionEntryHDSETID(Connection_Entry_t **ListHead, unsigned int HDSETID)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Headset Port ID to search for appear */
   /* to be valid.                                                      */
   if((ListHead) && (HDSETID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (HDSETID != FoundEntry->HDSETID))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified connection entry    */
   /* list for the connection entry with the specified Bluetooth device */
   /* address and connection type and removes it from the list.  This   */
   /* function returns NULL if either the connection entry list head is */
   /* invalid, the Bluetooth device address is invalid, or the specified*/
   /* entry was NOT present in the list.  The entry returned will have  */
   /* the next entry field set to NULL, and the caller is responsible   */
   /* for deleting the memory associated with this entry by calling     */
   /* FreeConnectionEntryMemory().                                      */
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HDSM_Connection_Type_t ConnectionType)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (ConnectionType != FoundEntry->ConnectionType)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextConnectionEntryPtr = FoundEntry->NextConnectionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextConnectionEntryPtr;

         FoundEntry->NextConnectionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified connection entry    */
   /* list for the connection entry with the specified Headset Port ID  */
   /* and removes it from the list.  This function returns NULL if      */
   /* either the connection entry list head is invalid, the Headset Port*/
   /* ID is invalid, or the specified entry was NOT present in the list.*/
   /* The entry returned will have the next entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeConnectionEntryMemory().                */
static Connection_Entry_t *DeleteConnectionEntryHDSETID(Connection_Entry_t **ListHead, unsigned int HDSETID)
{
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Headset Port ID to search for appear */
   /* to be valid.                                                      */
   if((ListHead) && (HDSETID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->HDSETID != HDSETID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextConnectionEntryPtr = FoundEntry->NextConnectionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextConnectionEntryPtr;

         FoundEntry->NextConnectionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified connection information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified connection information list.  Upon return*/
   /* of this function, the head pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextConnectionEntryPtr;

         if(tmpEntry->CloseTimerID)
            TMR_StopTimer(tmpEntry->CloseTimerID);

         /* Go ahead and delete this device from the SCO connection list*/
         /* (in case it was being tracked).                             */
         SCOM_DeleteConnectionFromIgnoreList(tmpEntry->BD_ADDR);

         FreeConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Headset event to every registered Headset  */
   /* Event Callback.                                                   */
   /* * NOTE * This function should be called with the Headset Manager  */
   /*          Lock held.  Upon exit from this function it will free the*/
   /*          Headset Manager Lock.                                    */
static void DispatchHDSETEvent(Boolean_t ControlOnly, HDSM_Connection_Type_t ConnectionType, HDSM_Event_Data_t *HDSMEventData, BTPM_Message_t *Message)
{
   unsigned int       Index;
   unsigned int       Index1;
   unsigned int       ServerID;
   unsigned int       NumberCallbacks;
   Callback_Info_t    CallbackInfoArray[16];
   Callback_Info_t   *CallbackInfoArrayPtr;
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HDSETEntryInfoList_AG) || (HDSETEntryInfoList_HS) || (HDSETEntryInfoList_AG_Control) || (HDSETEntryInfoList_HS_Control) || (HDSETEntryInfoList_AG_Data) || (HDSETEntryInfoList_HS_Data)) && (HDSMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      ServerID         = MSG_GetServerAddressID();
      NumberCallbacks  = 0;

      /* First, add the default event handlers.                         */
      if(!ControlOnly)
      {
         if(ConnectionType == sctAudioGateway)
            HDSETEntryInfo = HDSETEntryInfoList_AG;
         else
            HDSETEntryInfo = HDSETEntryInfoList_HS;

         while(HDSETEntryInfo)
         {
            if(((HDSETEntryInfo->EventCallback) || (HDSETEntryInfo->ClientID != ServerID)) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
         }
      }

      /* Next, add the control handlers.                                */
      if(ConnectionType == sctAudioGateway)
         HDSETEntryInfo = HDSETEntryInfoList_AG_Control;
      else
         HDSETEntryInfo = HDSETEntryInfoList_HS_Control;

      while(HDSETEntryInfo)
      {
         if(((HDSETEntryInfo->EventCallback) || (HDSETEntryInfo->ClientID != ServerID)) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
      }

      /* Next, add the data handlers.                                   */
      if(!ControlOnly)
      {
         if(ConnectionType == sctAudioGateway)
            HDSETEntryInfo = HDSETEntryInfoList_AG_Data;
         else
            HDSETEntryInfo = HDSETEntryInfoList_HS_Data;

         while(HDSETEntryInfo)
         {
            if(((HDSETEntryInfo->EventCallback) || (HDSETEntryInfo->ClientID != ServerID)) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
         }
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
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            if(!ControlOnly)
            {
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfo = HDSETEntryInfoList_AG;
               else
                  HDSETEntryInfo = HDSETEntryInfoList_HS;

               while(HDSETEntryInfo)
               {
                  if(((HDSETEntryInfo->EventCallback) || (HDSETEntryInfo->ClientID != ServerID)) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HDSETEntryInfo->ClientID;
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDSETEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDSETEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
               }
            }

            /* Next, add the control handlers.                          */
            if(ConnectionType == sctAudioGateway)
               HDSETEntryInfo = HDSETEntryInfoList_AG_Control;
            else
               HDSETEntryInfo = HDSETEntryInfoList_HS_Control;

            while(HDSETEntryInfo)
            {
               if(((HDSETEntryInfo->EventCallback) || (HDSETEntryInfo->ClientID != ServerID)) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HDSETEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDSETEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDSETEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
            }

            /* Next, add the data handlers.                             */
            if(!ControlOnly)
            {
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfo = HDSETEntryInfoList_AG_Data;
               else
                  HDSETEntryInfo = HDSETEntryInfoList_HS_Data;

               while(HDSETEntryInfo)
               {
                  if(((HDSETEntryInfo->EventCallback) || (HDSETEntryInfo->ClientID != ServerID)) && (HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HDSETEntryInfo->ClientID;
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HDSETEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HDSETEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
               }
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
               /*          for Headset events and Data Events.  To avoid*/
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(HDSMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Headset audio data event to the registered */
   /* Headset Data Event Callback.                                      */
   /* * NOTE * This function should be called with the Headset Manager  */
   /*          Lock held.  Upon exit from this function it will free the*/
   /*          Headset Manager Lock.                                    */
static void DispatchHDSETAudioDataEvent(HDSM_Connection_Type_t ConnectionType, HDSM_Event_Data_t *HDSMEventData, BTPM_Message_t *Message)
{
   void                  *CallbackParameter;
   HDSM_Entry_Info_t     *HDSETEntryInfo;
   HDSM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((HDSMEventData) && (Message))
   {
      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if(ConnectionType == sctAudioGateway)
         HDSETEntryInfo = HDSETEntryInfoList_AG_Data;
      else
         HDSETEntryInfo = HDSETEntryInfoList_HS_Data;

      if(HDSETEntryInfo)
      {
         /* Format up the Data.                                         */
         if(HDSETEntryInfo->ClientID != MSG_GetServerAddressID())
         {
            /* Dispatch a Message Callback.                             */

            /* Note the Client (destination) address.                   */
            Message->MessageHeader.AddressID                                     = HDSETEntryInfo->ClientID;

            /* Note the Headset Manager data event callback ID.         */
            /* * NOTE * All messages have this member in the same       */
            /*          location.                                       */
            ((HDSM_Audio_Data_Received_Message_t *)Message)->DataEventsHandlerID = HDSETEntryInfo->CallbackID;

            /* All that is left to do is to dispatch the Event.         */
            MSG_SendMessage(Message);
         }
         else
         {
            /* Dispatch local event callback.                           */
            if(HDSETEntryInfo->EventCallback)
            {
               /* Note the Headset Manager data event callback ID.      */
               /* * NOTE * All events have this member in the same      */
               /*          location.                                    */
               HDSMEventData->EventData.AudioDataEventData.DataEventsHandlerID = HDSETEntryInfo->CallbackID;

               /* Note the Callback Information.                        */
               EventCallback                                                   = HDSETEntryInfo->EventCallback;
               CallbackParameter                                               = HDSETEntryInfo->CallbackParameter;

               /* Release the Lock because we have already noted the    */
               /* callback information.                                 */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  (*EventCallback)(HDSMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified connect response   */
   /* message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the message   */
   /* before calling this function.                                     */
static void ProcessConnectionResponseMessage(HDSM_Connection_Request_Response_Request_t *Message)
{
   int                                          Result;
   Boolean_t                                    Authenticate;
   Boolean_t                                    Encrypt;
   unsigned long                                IncomingConnectionFlags;
   Connection_Entry_t                          *ConnectionEntry;
   HDSM_Connection_Request_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, next, verify that we are already      */
         /* tracking a connection for the specified connection type.    */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL) && (ConnectionEntry->ConnectionState == csAuthorizing))
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Message->Accept));

            /* If the caller has accepted the request then we need to   */
            /* process it differently.                                  */
            if(Message->Accept)
            {
               /* Determine the incoming connection flags based on the  */
               /* connection type.                                      */
               if(Message->ConnectionType == sctAudioGateway)
                  IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
               else
                  IncomingConnectionFlags = HeadsetInitializationInfo.IncomingConnectionFlags;

               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(ConnectionEntry->BD_ADDR, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Authorization not required, and we are already in  */
                  /* the correct state.                                 */
                  Result = _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, TRUE);

                  if(Result)
                  {
                     _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
                  else
                  {
                     /* Update the current connection state.            */
                     ConnectionEntry->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!Result)
                  {
                     if(Encrypt)
                        ConnectionEntry->ConnectionState = csEncrypting;
                     else
                        ConnectionEntry->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
               }
            }
            else
            {
               /* Rejection - Simply respond to the request.            */
               Result = _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);

               /* Go ahead and delete the entry because we are finished */
               /* with tracking it.                                     */
               if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
                  FreeConnectionEntryMemory(ConnectionEntry);
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified connect remote     */
   /* device message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessConnectRemoteDeviceMessage(HDSM_Connect_Remote_Device_Request_t *Message)
{
   int                                    Result;
   BD_ADDR_t                              NULL_BD_ADDR;
   HDSM_Entry_Info_t                      HDSETEntryInfo;
   HDSM_Entry_Info_t                     *HDSETEntryInfoPtr;
   Connection_Entry_t                     ConnectionEntry;
   Connection_Entry_t                    *ConnectionEntryPtr;
   HDSM_Connect_Remote_Device_Response_t  ResponseMessage;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, next, verify that we are not already  */
         /* tracking a connection to the specified device.              */

         /* Next, verify that the input parameters appear to be         */
         /* semi-valid.                                                 */
         if((!COMPARE_BD_ADDR(Message->RemoteDeviceAddress, NULL_BD_ADDR)) && (((Message->ConnectionType == sctAudioGateway) && (AudioGatewaySupported)) || ((Message->ConnectionType == sctHeadset) && (HeadsetSupported))))
         {
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) == NULL)
            {
               /* Entry is not present, go ahead and create a new entry.*/
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

               ConnectionEntry.BD_ADDR         = Message->RemoteDeviceAddress;
               ConnectionEntry.ConnectionType  = Message->ConnectionType;
               ConnectionEntry.HDSETID          = GetNextCallbackID() | 0x80000000;
               ConnectionEntry.Server          = FALSE;
               ConnectionEntry.ServerPort      = Message->RemoteServerPort;
               ConnectionEntry.ConnectionState = csIdle;
               ConnectionEntry.ConnectionFlags = Message->ConnectionFlags;

               if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
               {
                  /* Attempt to add an entry into the Headset entry     */
                  /* list.                                              */
                  BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

                  HDSETEntryInfo.CallbackID        = GetNextCallbackID();
                  HDSETEntryInfo.ClientID          = Message->MessageHeader.AddressID;
                  HDSETEntryInfo.ConnectionBD_ADDR = Message->RemoteDeviceAddress;

                  if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((Message->ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, &HDSETEntryInfo)) != NULL)
                  {
                     /* First, let's wait for the Port to disconnect.   */
                     if(!SPPM_WaitForPortDisconnection(Message->RemoteServerPort, FALSE, Message->RemoteDeviceAddress, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device      */
                        if(Message->ConnectionFlags & HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                           ConnectionEntryPtr->ConnectionState = csEncrypting;
                        else
                        {
                           if(Message->ConnectionFlags & HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                              ConnectionEntryPtr->ConnectionState = csAuthenticating;
                           else
                              ConnectionEntryPtr->ConnectionState = csConnectingDevice;
                        }

                        Result = DEVM_ConnectWithRemoteDevice(Message->RemoteDeviceAddress, (ConnectionEntryPtr->ConnectionState == csConnectingDevice)?0:((ConnectionEntryPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                        if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                        {
                           /* Check to see if we need to actually issue */
                           /* the Remote connection.                    */
                           if(Result < 0)
                           {
                              /* Set the state to connecting remote     */
                              /* device.                                */
                              ConnectionEntryPtr->ConnectionState = csConnecting;

                              if((Result = _HDSM_Connect_Remote_Device(Message->ConnectionType, Message->RemoteDeviceAddress, ConnectionEntryPtr->ServerPort)) <= 0)
                              {
                                 Result = BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE;

                                 /* Error opening device, go ahead and  */
                                 /* delete the entry that was added.    */
                                 if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((Message->ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfoPtr->CallbackID)) != NULL)
                                    FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);
                              }
                              else
                              {
                                 /* Note the Headset Port ID.           */
                                 ConnectionEntryPtr->HDSETID          = (unsigned int)Result;

                                 /* Flag success.                       */
                                 Result                              = 0;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Move the state to the connecting Waiting     */
                        /* state.                                       */
                        ConnectionEntryPtr->ConnectionState = csConnectingWaiting;

                        if((BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS) && (MAXIMUM_HANDS_FREE_PORT_OPEN_DELAY_RETRY))
                        {
                           /* Port is NOT disconnected, go ahead and    */
                           /* start a timer so that we can continue to  */
                           /* check for the Port Disconnection.         */
                           Result = TMR_StartTimer((void *)ConnectionEntry.HDSETID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                           /* If the timer was started, go ahead and    */
                           /* note the Timer ID.                        */
                           if(Result > 0)
                           {
                              ConnectionEntryPtr->CloseTimerID = (unsigned int)Result;

                              Result                           = 0;
                           }
                           else
                              Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
                        }
                        else
                           Result = BTPM_ERROR_CODE_HEADSET_IS_STILL_CONNECTED;
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

                  /* If an error occurred, go ahead and delete the      */
                  /* Connection Information that was added.             */
                  if(Result)
                  {
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
            {
               if(ConnectionEntryPtr->ConnectionState == csConnected)
                  Result = BTPM_ERROR_CODE_HEADSET_ALREADY_CONNECTED;
               else
                  Result = BTPM_ERROR_CODE_HEADSET_CONNECTION_IN_PROGRESS;
            }
         }
         else
         {
            if(COMPARE_BD_ADDR(Message->RemoteDeviceAddress, NULL_BD_ADDR))
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               Result = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified disconnect device  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the message   */
   /* before calling this function.                                     */
static void ProcessDisconnectDeviceMessage(HDSM_Disconnect_Device_Request_t *Message)
{
   int                                 Result;
   Boolean_t                           Server;
   Boolean_t                           PerformDisconnect;
   unsigned int                        ServerPort;
   Connection_Entry_t                 *ConnectionEntry;
   HDSM_Disconnect_Device_Response_t   ResponseMessage;
   HDSET_Close_Port_Indication_Data_t  ClosePortIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Go ahead and process the message request.                   */
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to disconnect Device\n"));

         if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
         {
            switch(ConnectionEntry->ConnectionState)
            {
               case csAuthorizing:
               case csAuthenticating:
               case csEncrypting:
                  /* Should not occur.                                  */
                  PerformDisconnect = FALSE;
                  break;
               case csConnectingWaiting:
                  if(ConnectionEntry->CloseTimerID)
                     TMR_StopTimer(ConnectionEntry->CloseTimerID);

                  PerformDisconnect = FALSE;
                  break;
               case csConnectingDevice:
                  PerformDisconnect = FALSE;
                  break;
               default:
               case csConnecting:
               case csConnected:
                  PerformDisconnect = TRUE;
                  break;
            }

            if(PerformDisconnect)
            {
               /* Nothing really to do other than to disconnect the     */
               /* device (if it is connected, a disconnect will be      */
               /* dispatched from the framework).                       */
               Result = _HDSM_Disconnect_Device(ConnectionEntry->HDSETID);
            }
            else
               Result = 0;

            /* Make sure we are no longer tracking SCO connections for  */
            /* the specified device.                                    */
            if((!Result) && (!COMPARE_NULL_BD_ADDR(ConnectionEntry->BD_ADDR)))
               SCOM_DeleteConnectionFromIgnoreList(ConnectionEntry->BD_ADDR);
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_CONNECTION_IN_PROGRESS;

         ResponseMessage.MessageHeader                = Message->MessageHeader;

         ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

         ResponseMessage.MessageHeader.MessageLength  = HDSM_DISCONNECT_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ResponseMessage.Status                       = Result;

         MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);

         /* If the result was successful, we need to make sure we clean */
         /* up everything and dispatch the event to all registered      */
         /* clients.                                                    */
         if((!Result) && (ConnectionEntry))
         {
            /* Since we are going to free the connection entry, we need */
            /* to note the information we will use to wait for the port */
            /* to disconnect.                                           */
            Server     = ConnectionEntry->Server;
            ServerPort = ConnectionEntry->ServerPort;

            /* Fake a close event to dispatch to all registered clients */
            /* that the device is no longer connected.                  */
            ClosePortIndicationData.HDSETPortID      = ConnectionEntry->HDSETID;
            ClosePortIndicationData.PortCloseStatus = HDSET_CLOSE_PORT_STATUS_SUCCESS;

            ProcessCloseIndicationEvent(&ClosePortIndicationData);

            /* If successfully closed and this was a client port, go    */
            /* ahead and wait for a little bit for the disconnection.   */
            if(ServerPort)
               SPPM_WaitForPortDisconnection(ServerPort, Server, Message->RemoteDeviceAddress, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset query      */
   /* connected devices message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessQueryConnectedDevicesMessage(HDSM_Query_Connected_Devices_Request_t *Message)
{
   unsigned int                             NumberConnected;
   Connection_Entry_t                      *ConnectionEntry;
   HDSM_Query_Connected_Devices_Response_t  ErrorResponseMessage;
   HDSM_Query_Connected_Devices_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Initialize the Error Response Message.                         */
      ErrorResponseMessage.MessageHeader                = Message->MessageHeader;

      ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ErrorResponseMessage.MessageHeader.MessageLength  = HDSM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

      ErrorResponseMessage.Status                       = 0;

      ErrorResponseMessage.NumberDevicesConnected       = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's determine how many       */
         /* devices are connected.                                      */
         NumberConnected = 0;
         ConnectionEntry = ConnectionEntryList;

         while(ConnectionEntry)
         {
            /* Note that we are only counting devices that are counting */
            /* devices that are either in the connected state or the    */
            /* connecting state (i.e. have been authorized OR passed    */
            /* authentication).                                         */
            if((ConnectionEntry->ConnectionType == Message->ConnectionType) && ((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting)))
               NumberConnected++;

            ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
         }

         /* Now let's attempt to allocate memory to hold the entire     */
         /* list.                                                       */
         if((ResponseMessage = (HDSM_Query_Connected_Devices_Response_t *)BTPS_AllocateMemory(HDSM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(NumberConnected))) != NULL)
         {
            /* Memory allocated, now let's build the response message.  */
            /* * NOTE * Error Response has initialized all values to    */
            /*          known values (i.e. zero devices and success).   */
            BTPS_MemCopy(ResponseMessage, &ErrorResponseMessage, HDSM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0));

            ConnectionEntry = ConnectionEntryList;

            while((ConnectionEntry) && (ResponseMessage->NumberDevicesConnected < NumberConnected))
            {
               /* Note that we are only counting devices that are       */
               /* counting devices that either in the connected state or*/
               /* the connecting state (i.e. have been authorized OR    */
               /* passed authentication).                               */
               if((ConnectionEntry->ConnectionType == Message->ConnectionType) && ((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting)))
                  ResponseMessage->DeviceConnectedList[ResponseMessage->NumberDevicesConnected++] = ConnectionEntry->BD_ADDR;

               ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
            }

            /* Now that we are finsished we need to update the length.  */
            ResponseMessage->MessageHeader.MessageLength  = HDSM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(ResponseMessage->NumberDevicesConnected) - BTPM_MESSAGE_HEADER_SIZE;

            /* Response Message built, go ahead and send it.            */
            MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

            /* Free the memory that was allocated because are finished  */
            /* with it.                                                 */
            BTPS_FreeMemory(ResponseMessage);
         }
         else
         {
            ErrorResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

            MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
         }
      }
      else
      {
         ErrorResponseMessage.Status = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset register   */
   /* Headset events message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessRegisterHeadsetEventsMessage(HDSM_Register_Headset_Events_Request_t *Message)
{
   int                                      Result;
   HDSM_Entry_Info_t                        HDSETEntryInfo;
   HDSM_Entry_Info_t                       *HDSETEntryInfoPtr;
   HDSM_Register_Headset_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Go ahead and initialize the Response.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_REGISTER_HEADSET_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Before proceding any further, make sure that there is not      */
      /* already a control event handler for the specified connection   */
      /* type (if control handler was specified).                       */
      if(Message->ControlHandler)
      {
         if(Message->ConnectionType == sctAudioGateway)
            HDSETEntryInfoPtr = HDSETEntryInfoList_AG_Control;
         else
            HDSETEntryInfoPtr = HDSETEntryInfoList_HS_Control;
      }
      else
         HDSETEntryInfoPtr = NULL;

      if(!HDSETEntryInfoPtr)
      {
         /* First, register the handler locally.                        */
         BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

         HDSETEntryInfo.CallbackID = GetNextCallbackID();
         HDSETEntryInfo.ClientID   = Message->MessageHeader.AddressID;
         HDSETEntryInfo.Flags      = HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         /* Note the connection type.                                   */
         if(Message->ConnectionType == sctAudioGateway)
            HDSETEntryInfo.Flags |= HDSET_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

         /* Check to see if we need to register a control handler or a  */
         /* normal event handler.                                       */
         if(Message->ControlHandler)
         {
            /* Control handler, add it the correct list, and attempt to */
            /* register it with the server.                             */
            if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((Message->ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, &HDSETEntryInfo)) != NULL)
            {
               Result = HDSETEntryInfoPtr->CallbackID;

               /* Add the SDP Record.                                   */
               _HDSM_UpdateSDPRecord(Message->ConnectionType, TRUE);
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
         {
            if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((Message->ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, &HDSETEntryInfo)) != NULL)
               Result = HDSETEntryInfoPtr->CallbackID;
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
      }
      else
         Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_ALREADY_REGISTERED;

      if(Result > 0)
      {
         ResponseMessage.EventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status          = 0;
      }
      else
      {
         ResponseMessage.EventsHandlerID = 0;

         ResponseMessage.Status          = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset un-register*/
   /* Headset events message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessUnRegisterHeadsetEventsMessage(HDSM_Un_Register_Headset_Events_Request_t *Message)
{
   int                                         Result;
   Boolean_t                                   ControlEvent = FALSE;
   HDSM_Entry_Info_t                          *HDSETEntryInfo;
   Connection_Entry_t                         *ConnectionEntry;
   Connection_Entry_t                            *CloseConnectionEntry;
   HDSM_Connection_Type_t                      ConnectionType;
   HDSM_Un_Register_Headset_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, determine if the client that is un-registering is the   */
      /* client that actually registered for the events.                */
      if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG, Message->EventsHandlerID)) == NULL)
      {
         if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, Message->EventsHandlerID)) == NULL)
         {
            if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS, Message->EventsHandlerID)) == NULL)
               HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, Message->EventsHandlerID);
         }
      }

      /* Go ahead and process the message request.                      */
      if((HDSETEntryInfo) && (HDSETEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         ConnectionType = sctAudioGateway;
         if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG, Message->EventsHandlerID)) == NULL)
         {
            if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, Message->EventsHandlerID)) == NULL)
            {
               ConnectionType = sctHeadset;
               if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS, Message->EventsHandlerID)) == NULL)
               {
                  HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, Message->EventsHandlerID);
                  if(HDSETEntryInfo)
                     ControlEvent  = TRUE;
               }
            }
            else
               ControlEvent = TRUE;
         }
      }
      else
         HDSETEntryInfo = NULL;

      if(HDSETEntryInfo)
      {
         /* Free the memory because we are finished with it.            */
         FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);

         if(ControlEvent)
         {
            /* Remove the SDP Record.                                   */
            _HDSM_UpdateSDPRecord(ConnectionType, FALSE);

            /* Check to see if there is an open connection.             */
            ConnectionEntry = ConnectionEntryList;
            while(ConnectionEntry)
            {
               /* Check to see if the this type control is being        */
               /* unregistered.                                         */
               if(ConnectionEntry->ConnectionType == ConnectionType)
                  CloseConnectionEntry = ConnectionEntry;
               else
                  CloseConnectionEntry = NULL;

               /* Move on to the next entry since we may be deleting    */
               /* this one.                                             */
               ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;

               /* Now cleanup the connection.                           */
               if(CloseConnectionEntry)
               {
                  _HDSM_Disconnect_Device(CloseConnectionEntry->HDSETID);

                  if((CloseConnectionEntry = DeleteConnectionEntryHDSETID(&ConnectionEntryList, CloseConnectionEntry->HDSETID)) != NULL)
                     FreeConnectionEntryMemory(CloseConnectionEntry);
               }
            }
         }

         /* Flag success.                                               */
         Result = 0;
      }
      else
         Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_UN_REGISTER_HEADSET_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset register   */
   /* Headset data message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessRegisterHeadsetDataMessage(HDSM_Register_Headset_Data_Events_Request_t *Message)
{
   int                                           Result;
   HDSM_Entry_Info_t                             HDSETEntryInfo;
   HDSM_Entry_Info_t                            *HDSETEntryInfoPtr;
   HDSM_Register_Headset_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Go ahead and initialize the Response.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_REGISTER_HEADSET_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Before proceding any further, make sure that there is not      */
      /* already a data event handler for the specified connection type.*/
      if(Message->ConnectionType == sctAudioGateway)
         HDSETEntryInfoPtr = HDSETEntryInfoList_AG_Data;
      else
         HDSETEntryInfoPtr = HDSETEntryInfoList_HS_Data;

      if(!HDSETEntryInfoPtr)
      {
         /* First, register the handler locally.                        */
         BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

         HDSETEntryInfo.CallbackID = GetNextCallbackID();
         HDSETEntryInfo.ClientID   = Message->MessageHeader.AddressID;
         HDSETEntryInfo.Flags      = HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         /* Note the connection type.                                   */
         if(Message->ConnectionType == sctAudioGateway)
            HDSETEntryInfo.Flags |= HDSET_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

         if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((Message->ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Data:&HDSETEntryInfoList_HS_Data, &HDSETEntryInfo)) != NULL)
            Result = HDSETEntryInfoPtr->CallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_HEADSET_DATA_HANDLER_ALREADY_REGISTERED;

      if(Result > 0)
      {
         ResponseMessage.DataEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status              = 0;
      }
      else
      {
         ResponseMessage.DataEventsHandlerID = 0;

         ResponseMessage.Status              = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset un-register*/
   /* Headset data message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessUnRegisterHeadsetDataMessage(HDSM_Un_Register_Headset_Data_Events_Request_t *Message)
{
   int                                              Result;
   HDSM_Entry_Info_t                               *HDSETEntryInfo;
   HDSM_Un_Register_Headset_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, determine if the client that is un-registering is the   */
      /* client that actually registered for the events.                */
      if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Data, Message->DataEventsHandlerID)) == NULL)
         HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Data, Message->DataEventsHandlerID);

      /* Go ahead and process the message request.                      */
      if((HDSETEntryInfo) && (HDSETEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Data, Message->DataEventsHandlerID)) == NULL)
            HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Data, Message->DataEventsHandlerID);
      }
      else
         HDSETEntryInfo = NULL;

      if(HDSETEntryInfo)
      {
         /* Free the memory because we are finished with it.            */
         FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);

         /* Flag success.                                               */
         Result = 0;
      }
      else
         Result = BTPM_ERROR_CODE_HEADSET_DATA_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_UN_REGISTER_HEADSET_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset set-up     */
   /* audio connection message and responds to the message accordingly. */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSetupAudioConnectionMessage(HDSM_Setup_Audio_Connection_Request_t *Message)
{
   int                                     Result;
   HDSM_Entry_Info_t                      *HDSETEntryInfo;
   Connection_Entry_t                     *ConnectionEntry;
   HDSM_Setup_Audio_Connection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, Message->ControlEventsHandlerID);

         if(HDSETEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set-up the audio connection (after we     */
               /* determine if this is for in-band ringing).            */
               if((Message->ConnectionType == sctAudioGateway) && (AudioGatewayInitializationInfo.SupportedFeaturesMask & HDSM_SUPPORTED_FEATURES_MASK_AUDIO_GATEWAY_SUPPORTS_IN_BAND_RING) && (Message->InBandRinging))
                  Result = _HDSM_Setup_Audio_Connection(ConnectionEntry->HDSETID, TRUE);
               else
                  Result = _HDSM_Setup_Audio_Connection(ConnectionEntry->HDSETID, FALSE);
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_SETUP_AUDIO_CONNECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset release    */
   /* audio connection message and responds to the message accordingly. */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessReleaseAudioConnectionMessage(HDSM_Release_Audio_Connection_Request_t *Message)
{
   int                                       Result;
   HDSM_Entry_Info_t                        *HDSETEntryInfo;
   Connection_Entry_t                       *ConnectionEntry;
   HDSM_Release_Audio_Connection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, Message->ControlEventsHandlerID);

         if(HDSETEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to release the audio connection.             */
               Result = _HDSM_Release_Audio_Connection(ConnectionEntry->HDSETID);

               if(!Result)
                  ConnectionEntry->SCOHandle = 0;
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_RELEASE_AUDIO_CONNECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset send audio */
   /* data message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessSendAudioDataMessage(HDSM_Send_Audio_Data_Request_t *Message)
{
   HDSM_Entry_Info_t  *HDSETEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if((Message) && (Message->AudioDataLength))
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Data, Message->DataEventsHandlerID)) == NULL)
            HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Data, Message->DataEventsHandlerID);

         if(HDSETEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the audio data.                      */
               _HDSM_Send_Audio_Data(ConnectionEntry->HDSETID, Message->AudioDataLength, Message->AudioData);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset query      */
   /* configuration message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessQueryConfigurationMessage(HDSM_Query_Current_Configuration_Request_t *Message)
{
   int                                         Result;
   HDSM_Query_Current_Configuration_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Initialize success.                                            */
      Result = 0;

      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      /* Check to see if Audio Gateway or Headset was specified.        */
      if(Message->ConnectionType == sctAudioGateway)
      {
         if(AudioGatewaySupported)
         {
            /* Copy the static information.                             */
            ResponseMessage.IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
            ResponseMessage.SupportedFeaturesMask   = AudioGatewayInitializationInfo.SupportedFeaturesMask;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
      }
      else
      {
         if(HeadsetSupported)
         {
            /* Copy the static information.                             */
            ResponseMessage.IncomingConnectionFlags = HeadsetInitializationInfo.IncomingConnectionFlags;
            ResponseMessage.SupportedFeaturesMask   = HeadsetInitializationInfo.SupportedFeaturesMask;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
      }

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDSM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset change     */
   /* incoming connection flags message and responds to the message     */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the message before calling this function.*/
static void ProcessChangeIncomingConnectionFlagsMessage(HDSM_Change_Incoming_Connection_Flags_Request_t *Message)
{
   int                                              Result;
   HDSM_Change_Incoming_Connection_Flags_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Initialize success.                                            */
      Result = 0;

      /* Check to see if Audio Gateway or Headset was specified.        */
      if(Message->ConnectionType == sctAudioGateway)
      {
         if(AudioGatewaySupported)
            AudioGatewayInitializationInfo.IncomingConnectionFlags = Message->ConnectionFlags;
         else
            Result = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
      }
      else
      {
         if(HeadsetSupported)
            HeadsetInitializationInfo.IncomingConnectionFlags = Message->ConnectionFlags;
         else
            Result = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
      }

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset set speaker*/
   /* gain message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessSetSpeakerGainMessage(HDSM_Set_Speaker_Gain_Request_t *Message)
{
   int                               Result;
   HDSM_Entry_Info_t                *HDSETEntryInfo;
   Connection_Entry_t               *ConnectionEntry;
   HDSM_Set_Speaker_Gain_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, Message->ControlEventsHandlerID);

         if(HDSETEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set speaker gain.                         */
               Result = _HDSM_Set_Remote_Speaker_Gain(ConnectionEntry->HDSETID, Message->SpeakerGain);
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_SET_SPEAKER_GAIN_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset set        */
   /* microphone gain message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSetMicrophoneGainMessage(HDSM_Set_Microphone_Gain_Request_t *Message)
{
   int                                  Result;
   HDSM_Entry_Info_t                   *HDSETEntryInfo;
   Connection_Entry_t                  *ConnectionEntry;
   HDSM_Set_Microphone_Gain_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HDSETEntryInfo = SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, Message->ControlEventsHandlerID);

         if(HDSETEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set microphone gain.                      */
               Result = _HDSM_Set_Remote_Microphone_Gain(ConnectionEntry->HDSETID, Message->MicrophoneGain);
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_SET_MICROPHONE_GAIN_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset answer     */
   /* incoming call message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendButtonPressMessage(HDSM_Send_Button_Press_Request_t *Message)
{
   int                                Result;
   Connection_Entry_t                *ConnectionEntry;
   HDSM_Send_Button_Press_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, sctHeadset)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the button press.                    */
               Result = _HDSM_Send_Button_Press(ConnectionEntry->HDSETID);
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_SEND_BUTTON_PRESS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Headset ring       */
   /* indication message and responds to the message accordingly.  This */
   /* function does not verify the integrity of the message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessRingIndicationMessage(HDSM_Ring_Indication_Request_t *Message)
{
   int                              Result;
   Connection_Entry_t              *ConnectionEntry;
   HDSM_Ring_Indication_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, sctAudioGateway)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the ring indication.                 */
               Result = _HDSM_Ring_Indication(ConnectionEntry->HDSETID);
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_RING_INDICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified query SCO Handle   */
   /* indication message and responds to the message accordingly.  This */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessQuerySCOConnectionHandleMessage(HDSM_Query_SCO_Connection_Handle_Request_t *Message)
{
   int                                          Result;
   Word_t                                       SCOHandle;
   Connection_Entry_t                          *ConnectionEntry;
   HDSM_Query_SCO_Connection_Handle_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      SCOHandle = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHDSETEntryInfoEntry((Message->ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, Message->EventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL) && (ConnectionEntry->ConnectionState == csConnected) && (ConnectionEntry->SCOHandle))
            {
               SCOHandle = ConnectionEntry->SCOHandle;
               Result    = 0;
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HDSM_QUERY_SCO_CONNECTION_HANDLE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      ResponseMessage.SCOHandle                    = SCOHandle;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Headset lock     */
   /*          held.  This function will release the lock before it     */
   /*          exits (i.e.  the caller SHOULD NOT RELEASE THE LOCK).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Headset connect response request.                     */
               ProcessConnectionResponseMessage((HDSM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* connect remote device request.                        */
               ProcessConnectRemoteDeviceMessage((HDSM_Connect_Remote_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_DISCONNECT_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_DISCONNECT_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* disconnect device request.                            */
               ProcessDisconnectDeviceMessage((HDSM_Disconnect_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Connected Devices Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query connected devices request.                      */
               ProcessQueryConnectedDevicesMessage((HDSM_Query_Connected_Devices_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Headset Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_REGISTER_HEADSET_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* register Headset events request.                      */
               ProcessRegisterHeadsetEventsMessage((HDSM_Register_Headset_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Headset Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_UN_REGISTER_HEADSET_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* un-register Headset events request.                   */
               ProcessUnRegisterHeadsetEventsMessage((HDSM_Un_Register_Headset_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Headset Data Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_REGISTER_HEADSET_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* register Headset data request.                        */
               ProcessRegisterHeadsetDataMessage((HDSM_Register_Headset_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Headset Data Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_UN_REGISTER_HEADSET_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* un-register Headset data request.                     */
               ProcessUnRegisterHeadsetDataMessage((HDSM_Un_Register_Headset_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Set-up Audio Connection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_SETUP_AUDIO_CONNECTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* set-up audio connection request.                      */
               ProcessSetupAudioConnectionMessage((HDSM_Setup_Audio_Connection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Release Audio Connection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_RELEASE_AUDIO_CONNECTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* release audio connection request.                     */
               ProcessReleaseAudioConnectionMessage((HDSM_Release_Audio_Connection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_SEND_AUDIO_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Audio Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_SEND_AUDIO_DATA_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_SEND_AUDIO_DATA_REQUEST_SIZE(((HDSM_Send_Audio_Data_Request_t *)Message)->AudioDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* audio data request.                                   */
               ProcessSendAudioDataMessage((HDSM_Send_Audio_Data_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Query SCO Connection Handle Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_QUERY_SCO_CONNECTION_HANDLE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query SCO Connection handle request.                  */
               ProcessQuerySCOConnectionHandleMessage((HDSM_Query_SCO_Connection_Handle_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Configuration Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query configuration request.                          */
               ProcessQueryConfigurationMessage((HDSM_Query_Current_Configuration_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Incoming Connection Flags Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* change incoming connection flags request.             */
               ProcessChangeIncomingConnectionFlagsMessage((HDSM_Change_Incoming_Connection_Flags_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Speaker Gain Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_SET_SPEAKER_GAIN_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* set speaker gain request.                             */
               ProcessSetSpeakerGainMessage((HDSM_Set_Speaker_Gain_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Microphone Gain Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_SET_MICROPHONE_GAIN_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* set microphone gain request.                          */
               ProcessSetMicrophoneGainMessage((HDSM_Set_Microphone_Gain_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_SEND_BUTTON_PRESS:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Button Press Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_SEND_BUTTON_PRESS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* answer incoming call request.                         */
               ProcessSendButtonPressMessage((HDSM_Send_Button_Press_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDSM_MESSAGE_FUNCTION_RING_INDICATION:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Ring Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDSM_RING_INDICATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the ring*/
               /* indication request.                                   */
               ProcessRingIndicationMessage((HDSM_Ring_Indication_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   Boolean_t            LoopCount;
   HDSM_Entry_Info_t   *HDSETEntryInfo;
   HDSM_Entry_Info_t  **_HDSETEntryInfoList;
   HDSM_Entry_Info_t   *tmpHDSETEntryInfo;
   Connection_Entry_t  *ConnectionEntry;
   Connection_Entry_t  *CloseConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* We need to loop through both lists as there could be client    */
      /* registrations in any of the lists.                             */
      LoopCount = 6;
      while(LoopCount--)
      {
         /* Connection Entry wwill be set to a valid entry structure if */
         /* the Control list is being processed and there is a current  */
         /* connection to a remote device.  Initialize this to NULL at  */
         /* the beginning of each loop.                                 */
         ConnectionEntry = NULL;

         if(LoopCount == 5)
         {
            HDSETEntryInfo      = HDSETEntryInfoList_AG;
            _HDSETEntryInfoList = &HDSETEntryInfoList_AG;
         }
         else
         {
            if(LoopCount == 4)
            {
               HDSETEntryInfo      = HDSETEntryInfoList_HS;
               _HDSETEntryInfoList = &HDSETEntryInfoList_HS;
            }
            else
            {
               if(LoopCount == 3)
               {
                  HDSETEntryInfo      = HDSETEntryInfoList_AG_Control;
                  _HDSETEntryInfoList = &HDSETEntryInfoList_AG_Control;
               }
               else
               {
                  if(LoopCount == 2)
                  {
                     HDSETEntryInfo      = HDSETEntryInfoList_HS_Control;
                     _HDSETEntryInfoList = &HDSETEntryInfoList_HS_Control;
                  }
                  else
                  {
                     if(LoopCount)
                     {
                        HDSETEntryInfo      = HDSETEntryInfoList_AG_Data;
                        _HDSETEntryInfoList = &HDSETEntryInfoList_AG_Data;
                     }
                     else
                     {
                        HDSETEntryInfo      = HDSETEntryInfoList_HS_Data;
                        _HDSETEntryInfoList = &HDSETEntryInfoList_HS_Data;
                     }
                  }
               }
            }
         }

         while(HDSETEntryInfo)
         {
            /* Check to see if the current Client Information is the one*/
            /* that is being un-registered.                             */
            if(HDSETEntryInfo->ClientID == ClientID)
            {
               /* Note the next Headset Entry in the list (we are about */
               /* to delete the current entry).                         */
               tmpHDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;

               /* Go ahead and delete the Headset Information Entry and */
               /* clean up the resources.                               */
               if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(_HDSETEntryInfoList, HDSETEntryInfo->CallbackID)) != NULL)
               {
                  /* Check to see if this is a Control endpoint.  There */
                  /* is only one control allowed per mode.              */
                  if((_HDSETEntryInfoList == &HDSETEntryInfoList_AG_Control) || (_HDSETEntryInfoList == &HDSETEntryInfoList_HS_Control))
                  {
                     /* Check to see if there is an open connection.    */
                     if(ConnectionEntryList)
                     {
                        ConnectionEntry = ConnectionEntryList;
                        while(ConnectionEntry)
                        {
                           /* Check to see if the AudioGateway control is  */
                           /* being unregistered.                          */
                           if((ConnectionEntry->ConnectionType == sctAudioGateway) && (_HDSETEntryInfoList == &HDSETEntryInfoList_AG_Control))
                           {
                              CloseConnectionEntry = ConnectionEntry;
                           }
                           else
                           {
                              /* Check to see if the Handsfree control is  */
                              /* being unregistered.                       */
                              if((ConnectionEntry->ConnectionType == sctHeadset) && (_HDSETEntryInfoList == &HDSETEntryInfoList_HS_Control))
                              {
                                 CloseConnectionEntry = ConnectionEntry;
                              }
                              else
                                 CloseConnectionEntry = NULL;
                           }

                           /* Move to the next entry, in case we delete */
                           /* the current one.                          */
                           ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;

                           /* Now Disconnect if we need to.             */
                           if(CloseConnectionEntry)
                           {
                              _HDSM_Disconnect_Device(CloseConnectionEntry->HDSETID);

                              if((CloseConnectionEntry = DeleteConnectionEntryHDSETID(&ConnectionEntryList, CloseConnectionEntry->HDSETID)) != NULL)
                                 FreeConnectionEntryMemory(CloseConnectionEntry);
                           }
                        }
                     }

                     /* Remove the SDP record, since the control        */
                     /* application his gone away.                      */
                     _HDSM_UpdateSDPRecord((_HDSETEntryInfoList == &HDSETEntryInfoList_AG_Control)?sctAudioGateway:sctHeadset, FALSE);
                  }

                  /* Close any events that were allocated.              */
                  if(HDSETEntryInfo->ConnectionEvent)
                     BTPS_CloseEvent(HDSETEntryInfo->ConnectionEvent);

                  /* All finished with the memory so free the entry.    */
                  FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);
               }

               /* Go ahead and set the next Headset Information Entry   */
               /* (past the one we just deleted).                       */
               HDSETEntryInfo = tmpHDSETEntryInfo;
            }
            else
               HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* port open request indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* lock protecting the Headset Manager information held.             */
static void ProcessOpenRequestIndicationEvent(HDSET_Open_Port_Request_Indication_Data_t *OpenPortRequestIndicationData)
{
   int                                Result;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   unsigned int                       ServerPort;
   unsigned long                      IncomingConnectionFlags;
   HDSM_Event_Data_t                  HDSMEventData;
   HDSM_Entry_Info_t                 *HDSETEntryInfo;
   Connection_Entry_t                 ConnectionEntry;
   Connection_Entry_t                *ConnectionEntryPtr;
   HDSM_Connection_Type_t             ConnectionType;
   HDSM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((OpenPortRequestIndicationData) && (_HDSM_QueryIncomingConnectionType(OpenPortRequestIndicationData->HDSETPortID, &ConnectionType, &ServerPort)))
   {
      /* Determine the incoming connection flags based on the connection*/
      /* type.                                                          */
      if(ConnectionType == sctAudioGateway)
      {
         HDSETEntryInfo          = HDSETEntryInfoList_AG_Control;
         IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
      }
      else
      {
         HDSETEntryInfo          = HDSETEntryInfoList_HS_Control;
         IncomingConnectionFlags = HeadsetInitializationInfo.IncomingConnectionFlags;
      }

      /* Verify that a control event callback has been registered.      */
      if(HDSETEntryInfo)
      {
         /* First, let's see if we actually need to do anything, other  */
         /* than simply accept the connection.                          */
         if(!IncomingConnectionFlags)
         {
            /* Simply Accept the connection.                            */
            _HDSM_Connection_Request_Response(OpenPortRequestIndicationData->HDSETPortID, TRUE);
         }
         else
         {
            /* Before proceding any further, let's make sure that there */
            /* doesn't already exist an entry for this device.          */
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenPortRequestIndicationData->BD_ADDR, ConnectionType)) == NULL)
            {
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(ConnectionEntry));

               /* Entry does not exist, go ahead and format a new entry.*/
               ConnectionEntry.BD_ADDR                = OpenPortRequestIndicationData->BD_ADDR;
               ConnectionEntry.ConnectionType         = ConnectionType;
               ConnectionEntry.HDSETID                 = OpenPortRequestIndicationData->HDSETPortID;
               ConnectionEntry.Server                 = TRUE;
               ConnectionEntry.ServerPort             = ServerPort;
               ConnectionEntry.ConnectionState        = csAuthorizing;
               ConnectionEntry.NextConnectionEntryPtr = NULL;

               ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
            }
            else
               ConnectionEntryPtr->HDSETID = OpenPortRequestIndicationData->HDSETPortID;

            /* Check to see if we are tracking this connection.         */
            if(ConnectionEntryPtr)
            {
               if(IncomingConnectionFlags & HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION)
               {
                  /* Authorization (at least) required, go ahead and    */
                  /* dispatch the request.                              */
                  ConnectionEntryPtr->ConnectionState = csAuthorizing;

                  /* Next, format up the Event to dispatch.             */
                  HDSMEventData.EventType                                                        = hetHDSIncomingConnectionRequest;
                  HDSMEventData.EventLength                                                      = HDSM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

                  HDSMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = OpenPortRequestIndicationData->BD_ADDR;
                  HDSMEventData.EventData.IncomingConnectionRequestEventData.ConnectionType      = ConnectionType;

                  /* Next, format up the Message to dispatch.           */
                  BTPS_MemInitialize(&Message, 0, sizeof(Message));

                  Message.MessageHeader.AddressID       = 0;
                  Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
                  Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
                  Message.MessageHeader.MessageLength   = (HDSM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  Message.RemoteDeviceAddress           = OpenPortRequestIndicationData->BD_ADDR;
                  Message.ConnectionType                = ConnectionType;

                  /* Finally dispatch the formatted Event and Message.  */
                  DispatchHDSETEvent(FALSE, ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
               }
               else
               {
                  /* Determine if Authentication and/or Encryption is   */
                  /* required for this link.                            */
                  if(IncomingConnectionFlags & HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                     Authenticate = TRUE;
                  else
                     Authenticate = FALSE;

                  if(IncomingConnectionFlags & HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                     Encrypt = TRUE;
                  else
                     Encrypt = FALSE;

                  if((Authenticate) || (Encrypt))
                  {
                     if(Encrypt)
                        Result = DEVM_EncryptRemoteDevice(ConnectionEntryPtr->BD_ADDR, 0);
                     else
                        Result = DEVM_AuthenticateRemoteDevice(ConnectionEntryPtr->BD_ADDR, 0);
                  }
                  else
                     Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

                  if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                  {
                     Result = _HDSM_Connection_Request_Response(ConnectionEntryPtr->HDSETID, TRUE);

                     if(Result)
                     {
                        _HDSM_Connection_Request_Response(ConnectionEntryPtr->HDSETID, FALSE);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */
                        if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR, ConnectionType)) != NULL)
                           FreeConnectionEntryMemory(ConnectionEntryPtr);
                     }
                     else
                     {
                        /* Update the current connection state.         */
                        ConnectionEntryPtr->ConnectionState = csConnecting;
                     }
                  }
                  else
                  {
                     /* If we were successfully able to Authenticate    */
                     /* and/or Encrypt, then we need to set the correct */
                     /* state.                                          */
                     if(!Result)
                     {
                        if(Encrypt)
                           ConnectionEntryPtr->ConnectionState = csEncrypting;
                        else
                           ConnectionEntryPtr->ConnectionState = csAuthenticating;

                        /* Flag success.                                */
                        Result = 0;
                     }
                     else
                     {
                        /* Error, reject the request.                   */
                        _HDSM_Connection_Request_Response(ConnectionEntryPtr->HDSETID, FALSE);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */
                        if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR, ConnectionType)) != NULL)
                           FreeConnectionEntryMemory(ConnectionEntryPtr);
                     }
                  }
               }
            }
            else
            {
               /* Unable to add entry, go ahead and reject the request. */
               _HDSM_Connection_Request_Response(OpenPortRequestIndicationData->HDSETPortID, FALSE);
            }
         }
      }
      else
      {
         /* There is no Control Event Callback registered, go ahead and */
         /* reject the request.                                         */
         _HDSM_Connection_Request_Response(OpenPortRequestIndicationData->HDSETPortID, FALSE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* port open indication event that has been received with the        */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Headset Manager information held.             */
static void ProcessOpenIndicationEvent(HDSET_Open_Port_Indication_Data_t *OpenPortIndicationData)
{
   unsigned int                       ServerPort;
   HDSM_Event_Data_t                  HDSMEventData;
   Connection_Entry_t                 ConnectionEntry;
   Connection_Entry_t                *ConnectionEntryPtr;
   HDSM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenPortIndicationData)
   {
      if((ConnectionEntryPtr = SearchConnectionEntryHDSETID(&ConnectionEntryList, OpenPortIndicationData->HDSETPortID)) == NULL)
      {
         /* Entry does not exist, go ahead and format a new entry.      */
         _HDSM_QueryIncomingConnectionType(OpenPortIndicationData->HDSETPortID, &(ConnectionEntry.ConnectionType), &ServerPort);

         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(ConnectionEntry));

         ConnectionEntry.BD_ADDR                = OpenPortIndicationData->BD_ADDR;
         ConnectionEntry.HDSETID                 = OpenPortIndicationData->HDSETPortID;
         ConnectionEntry.Server                 = TRUE;
         ConnectionEntry.ServerPort             = ServerPort;
         ConnectionEntry.ConnectionState        = csConnected;
         ConnectionEntry.NextConnectionEntryPtr = NULL;

         ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
      }
      else
      {
         /* Update the connection information.                          */
         _HDSM_QueryIncomingConnectionType(OpenPortIndicationData->HDSETPortID, &(ConnectionEntryPtr->ConnectionType), &ServerPort);

         ConnectionEntryPtr->ConnectionState = csConnected;
         ConnectionEntryPtr->ServerPort      = ServerPort;
         ConnectionEntryPtr->HDSETID          = OpenPortIndicationData->HDSETPortID;
      }

      if(ConnectionEntryPtr)
      {
         /* Flag that we are now watching SCO connections for this      */
         /* device.                                                     */
         SCOM_AddConnectionToIgnoreList(OpenPortIndicationData->BD_ADDR);

         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                        = hetHDSConnected;
         HDSMEventData.EventLength                                      = HDSM_CONNECTED_EVENT_DATA_SIZE;

         HDSMEventData.EventData.ConnectedEventData.ConnectionType      = ConnectionEntryPtr->ConnectionType;
         HDSMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = OpenPortIndicationData->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTED;
         Message.MessageHeader.MessageLength   = (HDSM_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntryPtr->ConnectionType;
         Message.RemoteDeviceAddress           = OpenPortIndicationData->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(FALSE, ConnectionEntryPtr->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
      }
      else
      {
         /* Error, go ahead and disconnect the device.                  */
         _HDSM_Disconnect_Device(OpenPortIndicationData->HDSETPortID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* open confirmation event that has been received with the specified */
   /* information.  This function should be called with the lock        */
   /* protecting the Headset Manager information held.                  */
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HDSET_Open_Port_Confirmation_Data_t *OpenPortConfirmationData)
{
   void                                    *CallbackParameter;
   unsigned int                             ClientID;
   HDSM_Event_Data_t                        HDSMEventData;
   HDSM_Entry_Info_t                       *HDSETEntryInfo;
   Connection_Entry_t                       ConnectionEntry;
   Connection_Entry_t                      *ConnectionEntryPtr;
   HDSM_Event_Callback_t                    EventCallback;
   HDSM_Device_Connected_Message_t          Message;
   HDSM_Device_Connection_Status_Message_t  ConnectionStatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenPortConfirmationData)
   {
      /* First, flag the connected state.                               */
      if(((ConnectionEntryPtr = SearchConnectionEntryHDSETID(&ConnectionEntryList, OpenPortConfirmationData->HDSETPortID)) != NULL) && (!ConnectionEntryPtr->Server))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Found, State: 0x%08X\n", ConnectionEntryPtr->ConnectionState));

         /* Note all the connection information (we will use it later). */
         ConnectionEntry = *ConnectionEntryPtr;

         if((OpenPortConfirmationData->PortOpenStatus) && (ConnectionEntryPtr->ConnectionState != csConnected))
         {
            if((ConnectionEntryPtr = DeleteConnectionEntryHDSETID(&ConnectionEntryList, OpenPortConfirmationData->HDSETPortID)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntryPtr);
         }
         else
            ConnectionEntryPtr->ConnectionState = csConnected;

         /* Dispatch any registered Connection Status Message/Event.    */
         if(ConnectionEntry.ConnectionType == sctAudioGateway)
            HDSETEntryInfo = HDSETEntryInfoList_AG;
         else
            HDSETEntryInfo = HDSETEntryInfoList_HS;

         while(HDSETEntryInfo)
         {
            if((!(HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(ConnectionEntry.BD_ADDR, HDSETEntryInfo->ConnectionBD_ADDR)))
            {
               /* Flag that we are now watching SCO connections for this*/
               /* device.                                               */
               SCOM_AddConnectionToIgnoreList(HDSETEntryInfo->ConnectionBD_ADDR);

               /* Connection status registered, now see if we need to   */
               /* issue a Callack or an event.                          */

               /* Determine if we need to dispatch the event locally or */
               /* remotely.                                             */
               if(HDSETEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Callback.                                          */
                  BTPS_MemInitialize(&HDSMEventData, 0, sizeof(HDSM_Event_Data_t));

                  HDSMEventData.EventType                                               = hetHDSConnectionStatus;
                  HDSMEventData.EventLength                                             = HDSM_CONNECTION_STATUS_EVENT_DATA_SIZE;

                  HDSMEventData.EventData.ConnectionStatusEventData.ConnectionType      = ConnectionEntry.ConnectionType;
                  HDSMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = ConnectionEntry.BD_ADDR;

                  /* Map the open confirmation Error to the correct     */
                  /* Headset Manager error status.                      */
                  switch(OpenPortConfirmationData->PortOpenStatus)
                  {
                     case HDSET_OPEN_PORT_STATUS_SUCCESS:
                        HDSMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_SUCCESS;
                        break;
                     case HDSET_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
                        HDSMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                        break;
                     case HDSET_OPEN_PORT_STATUS_CONNECTION_REFUSED:
                        HDSMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                        break;
                     default:
                        HDSMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        break;
                  }

                  /* If this was a synchronous event we need to set the */
                  /* status and the event.                              */
                  if(HDSETEntryInfo->ConnectionEvent)
                  {
                     /* Synchronous event, go ahead and set the correct */
                     /* status, then set the event.                     */
                     HDSETEntryInfo->ConnectionStatus = HDSMEventData.EventData.ConnectionStatusEventData.ConnectionStatus;

                     BTPS_SetEvent(HDSETEntryInfo->ConnectionEvent);
                  }
                  else
                  {
                     /* Note the Callback information.                  */
                     EventCallback     = HDSETEntryInfo->EventCallback;
                     CallbackParameter = HDSETEntryInfo->CallbackParameter;

                     /* Go ahead and delete the entry (since we are     */
                     /* dispatching the callback).                      */
                     if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry((ConnectionEntry.ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfo->CallbackID)) != NULL)
                        FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);

                     /* Release the Lock so we can make the callback.   */
                     DEVM_ReleaseLock();

                     __BTPSTRY
                     {
                        if(EventCallback)
                           (*EventCallback)(&HDSMEventData, CallbackParameter);
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* Re-acquire the Lock.                            */
                     DEVM_AcquireLock();
                  }
               }
               else
               {
                  /* Remote Event.                                      */

                  /* Note the Client ID.                                */
                  ClientID = HDSETEntryInfo->ClientID;

                  /* Go ahead and delete the entry (since we are        */
                  /* dispatching the event).                            */
                  if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry((ConnectionEntry.ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfo->CallbackID)) != NULL)
                     FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);

                  BTPS_MemInitialize(&ConnectionStatusMessage, 0, sizeof(ConnectionStatusMessage));

                  ConnectionStatusMessage.MessageHeader.AddressID       = ClientID;
                  ConnectionStatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  ConnectionStatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
                  ConnectionStatusMessage.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS;
                  ConnectionStatusMessage.MessageHeader.MessageLength   = (HDSM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  ConnectionStatusMessage.ConnectionType                = ConnectionEntry.ConnectionType;
                  ConnectionStatusMessage.RemoteDeviceAddress           = ConnectionEntry.BD_ADDR;

                  /* Map the open confirmation error to the correct     */
                  /* Headset Manager error status.                      */
                  switch(OpenPortConfirmationData->PortOpenStatus)
                  {
                     case HDSET_OPEN_PORT_STATUS_SUCCESS:
                        ConnectionStatusMessage.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_SUCCESS;
                        break;
                     case HDSET_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
                        ConnectionStatusMessage.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                        break;
                     case HDSET_OPEN_PORT_STATUS_CONNECTION_REFUSED:
                        ConnectionStatusMessage.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                        break;
                     default:
                        ConnectionStatusMessage.ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        break;
                  }

                  /* Finally dispatch the Message.                      */
                  MSG_SendMessage((BTPM_Message_t *)&ConnectionStatusMessage);
               }

               /* Break out of the loop.                                */
               HDSETEntryInfo = NULL;
            }
            else
               HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
         }

         /* Next, format up the Event to dispatch - ONLY if we need to  */
         /* dispatch a connected event.                                 */
         if((!OpenPortConfirmationData->PortOpenStatus) && (DispatchOpen))
         {
            HDSMEventData.EventType                                        = hetHDSConnected;
            HDSMEventData.EventLength                                      = HDSM_CONNECTED_EVENT_DATA_SIZE;

            HDSMEventData.EventData.ConnectedEventData.ConnectionType      = ConnectionEntry.ConnectionType;
            HDSMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry.BD_ADDR;

            /* Next, format up the message to dispatch.                 */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = 0;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
            Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_DEVICE_CONNECTED;
            Message.MessageHeader.MessageLength   = (HDSM_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.ConnectionType                = ConnectionEntry.ConnectionType;
            Message.RemoteDeviceAddress           = ConnectionEntry.BD_ADDR;

            /* Finally dispatch the formatted event and message.        */
            DispatchHDSETEvent(FALSE, ConnectionEntry.ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
         }
      }
      else
      {
         /* We are not tracking this connection, so we have no way to   */
         /* know what the BD_ADDR of the device is.  Let's go ahead and */
         /* disconnect the connection.                                  */
         _HDSM_Disconnect_Device(OpenPortConfirmationData->HDSETPortID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* close port indication event that has been received with the       */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Headset Manager information held.             */
static void ProcessCloseIndicationEvent(HDSET_Close_Port_Indication_Data_t *ClosePortIndicationData)
{
   HDSM_Event_Data_t                   HDSMEventData;
   Connection_Entry_t                 *ConnectionEntry;
   HDSM_Device_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ClosePortIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = DeleteConnectionEntryHDSETID(&ConnectionEntryList, ClosePortIndicationData->HDSETPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                           = hetHDSDisconnected;
         HDSMEventData.EventLength                                         = HDSM_DISCONNECTED_EVENT_DATA_SIZE;

         HDSMEventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HDSMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDSMEventData.EventData.DisconnectedEventData.DisconnectReason    = ClosePortIndicationData->PortCloseStatus;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HDSM_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.DisconnectReason              = ClosePortIndicationData->PortCloseStatus;

         /* Make sure the device is deleted from the SCO connection list*/
         /* (in case it was added).                                     */
         SCOM_DeleteConnectionFromIgnoreList(ConnectionEntry->BD_ADDR);

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(FALSE, ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);

         /* Free the connection information.                            */
         FreeConnectionEntryMemory(ConnectionEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* ring indication event that has been received with the specified   */
   /* information.  This function should be called with the Lock        */
   /* protecting the Headset Manager information held.                  */
static void ProcessRingIndicationEvent(HDSET_Ring_Indication_Data_t *RingIndicationData)
{
   HDSM_Event_Data_t                          HDSMEventData;
   Connection_Entry_t                        *ConnectionEntry;
   HDSM_Ring_Indication_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(RingIndicationData)
   {
      /* Next map the Headset Port ID to a connection entry we are      */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, RingIndicationData->HDSETPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                             = hetHDSRingIndication;
         HDSMEventData.EventLength                                           = HDSM_RING_INDICATION_EVENT_DATA_SIZE;

         HDSMEventData.EventData.RingIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_RING_INDICATION_IND;
         Message.MessageHeader.MessageLength   = (HDSM_RING_INDICATION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(TRUE, ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* button pressed indication event that has been received with the   */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Headset Manager information held.             */
static void ProcessButtonPressedIndicationEvent(HDSET_Button_Pressed_Indication_Data_t *ButtonPressedIndicationData)
{
   HDSM_Event_Data_t                         HDSMEventData;
   Connection_Entry_t                       *ConnectionEntry;
   HDSM_Button_Pressed_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ButtonPressedIndicationData)
   {
      /* Next map the Headset Port ID to a connection entry we are      */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, ButtonPressedIndicationData->HDSETPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                                    = hetHDSButtonPressedIndication;
         HDSMEventData.EventLength                                                  = HDSM_BUTTON_PRESSED_INDICATION_EVENT_DATA_SIZE;

         HDSMEventData.EventData.ButtonPressIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_BUTTON_PRESSED_IND;
         Message.MessageHeader.MessageLength   = (HDSM_BUTTON_PRESSED_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(TRUE, ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* speaker gain indication event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Headset Manager information held.             */
static void ProcessSpeakerGainIndicationEvent(HDSET_Speaker_Gain_Indication_Data_t *SpeakerGainIndicationData)
{
   HDSM_Event_Data_t                       HDSMEventData;
   Connection_Entry_t                     *ConnectionEntry;
   HDSM_Speaker_Gain_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SpeakerGainIndicationData)
   {
      /* Next map the Headset Port ID to a connection entry we are      */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, SpeakerGainIndicationData->HDSETPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                                    = hetHDSSpeakerGainIndication;
         HDSMEventData.EventLength                                                  = HDSM_SPEAKER_GAIN_INDICATION_EVENT_DATA_SIZE;

         HDSMEventData.EventData.SpeakerGainIndicationEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HDSMEventData.EventData.SpeakerGainIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDSMEventData.EventData.SpeakerGainIndicationEventData.SpeakerGain         = SpeakerGainIndicationData->SpeakerGain;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND;
         Message.MessageHeader.MessageLength   = (HDSM_SPEAKER_GAIN_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.SpeakerGain                   = SpeakerGainIndicationData->SpeakerGain;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(TRUE, ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* microphone gain indication event that has been received with the  */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Headset Manager information held.             */
static void ProcessMicrophoneGainIndicationEvent(HDSET_Microphone_Gain_Indication_Data_t *MicrophoneGainIndicationData)
{
   HDSM_Event_Data_t                          HDSMEventData;
   Connection_Entry_t                        *ConnectionEntry;
   HDSM_Microphone_Gain_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(MicrophoneGainIndicationData)
   {
      /* Next map the Headset Port ID to a connection entry we are      */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, MicrophoneGainIndicationData->HDSETPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                                       = hetHDSMicrophoneGainIndication;
         HDSMEventData.EventLength                                                     = HDSM_MICROPHONE_GAIN_INDICATION_EVENT_DATA_SIZE;

         HDSMEventData.EventData.MicrophoneGainIndicationEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HDSMEventData.EventData.MicrophoneGainIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDSMEventData.EventData.MicrophoneGainIndicationEventData.MicrophoneGain      = MicrophoneGainIndicationData->MicrophoneGain;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND;
         Message.MessageHeader.MessageLength   = (HDSM_MICROPHONE_GAIN_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.MicrophoneGain                = MicrophoneGainIndicationData->MicrophoneGain;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(TRUE, ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* audio connection indication event that has been received with the */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Headset Manager information held.             */
static void ProcessAudioConnectionIndicationEvent(HDSET_Audio_Connection_Indication_Data_t *AudioConnectionIndicationData)
{
   HDSM_Event_Data_t               HDSMEventData;
   Connection_Entry_t             *ConnectionEntry;
   HDSM_Audio_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(AudioConnectionIndicationData)
   {
      /* Next map the Headset Port ID to a connection entry we are      */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, AudioConnectionIndicationData->HDSETPortID)) != NULL)
      {
         /* Note the SCO Handle.                                        */
         ConnectionEntry->SCOHandle = AudioConnectionIndicationData->SCO_Connection_Handle;

         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                             = hetHDSAudioConnected;
         HDSMEventData.EventLength                                           = HDSM_AUDIO_CONNECTED_EVENT_DATA_SIZE;

         HDSMEventData.EventData.AudioConnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HDSMEventData.EventData.AudioConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_AUDIO_CONNECTED;
         Message.MessageHeader.MessageLength   = (HDSM_AUDIO_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(FALSE, ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* audio disconnection indication event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Headset Manager information held.         */
static void ProcessAudioDisconnectionIndicationEvent(HDSET_Audio_Disconnection_Indication_Data_t *AudioDisconnectionIndicationData)
{
   HDSM_Event_Data_t                  HDSMEventData;
   Connection_Entry_t                *ConnectionEntry;
   HDSM_Audio_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(AudioDisconnectionIndicationData)
   {
      /* Next map the Headset Port ID to a connection entry we are      */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, AudioDisconnectionIndicationData->HDSETPortID)) != NULL)
      {
         /* Clear the SCO Handle.                                       */
         ConnectionEntry->SCOHandle = 0;

         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                                = hetHDSAudioDisconnected;
         HDSMEventData.EventLength                                              = HDSM_AUDIO_DISCONNECTED_EVENT_DATA_SIZE;

         HDSMEventData.EventData.AudioDisconnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HDSMEventData.EventData.AudioDisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         Message.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HDSM_AUDIO_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDSETEvent(FALSE, ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Headset    */
   /* audio data indication event that has been received with the       */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Headset Manager information held.             */
static void ProcessAudioDataIndicationEvent(HDSET_Audio_Data_Indication_Data_t *AudioDataIndicationData)
{
   HDSM_Event_Data_t                   HDSMEventData;
   Connection_Entry_t                 *ConnectionEntry;
   HDSM_Audio_Data_Received_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((AudioDataIndicationData) && (AudioDataIndicationData->AudioDataLength))
   {
      /* Next map the Headset Port ID to a connection entry we are      */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, AudioDataIndicationData->HDSETPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDSMEventData.EventType                                        = hetHDSAudioData;
         HDSMEventData.EventLength                                      = HDSM_AUDIO_DATA_EVENT_DATA_SIZE;

         HDSMEventData.EventData.AudioDataEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HDSMEventData.EventData.AudioDataEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDSMEventData.EventData.AudioDataEventData.AudioDataLength     = AudioDataIndicationData->AudioDataLength;

         /* Set the appropriate event flags.                            */
         switch(AudioDataIndicationData->PacketStatus & HCI_SCO_FLAGS_PACKET_STATUS_MASK_MASK)
         {
            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_CORRECTLY_RECEIVED_DATA:
               HDSMEventData.EventData.AudioDataEventData.AudioDataFlags = HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_CORRECTLY_RECEIVED_DATA;
               break;

            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_POSSIBLY_INVALID_DATA:
               HDSMEventData.EventData.AudioDataEventData.AudioDataFlags = HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_POSSIBLY_INVALID_DATA;
               break;

            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_NO_DATA_RECEIVED:
               HDSMEventData.EventData.AudioDataEventData.AudioDataFlags = HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_NO_DATA_RECEIVED;
               break;

            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_DATA_PARTIALLY_LOST:
               HDSMEventData.EventData.AudioDataEventData.AudioDataFlags = HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_DATA_PARTIALLY_LOST;
               break;

            default:
               HDSMEventData.EventData.AudioDataEventData.AudioDataFlags = HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_NO_DATA_RECEIVED;
               break;
         }

         /* Format up the message to dispatch.                          */
         if((Message = (HDSM_Audio_Data_Received_Message_t *)BTPS_AllocateMemory(HDSM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(AudioDataIndicationData->AudioDataLength))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HDSM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(AudioDataIndicationData->AudioDataLength));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
            Message->MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED;
            Message->MessageHeader.MessageLength   = (HDSM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(AudioDataIndicationData->AudioDataLength) - BTPM_MESSAGE_HEADER_SIZE);

            Message->ConnectionType                = ConnectionEntry->ConnectionType;
            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->AudioDataFlags                = HDSMEventData.EventData.AudioDataEventData.AudioDataFlags;
            Message->AudioDataLength               = AudioDataIndicationData->AudioDataLength;

            BTPS_MemCopy(Message->AudioData, AudioDataIndicationData->AudioData, AudioDataIndicationData->AudioDataLength);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHDSETAudioDataEvent(ConnectionEntry->ConnectionType, &HDSMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing Headset Events that have been received.  This function */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessHeadsetEvent(HDSM_Headset_Event_Data_t *HeadsetEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(HeadsetEventData)
   {
      /* Process the event based on the event type.                     */
      switch(HeadsetEventData->EventType)
      {
         case etHDSET_Open_Port_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication\n"));

            ProcessOpenRequestIndicationEvent(&(HeadsetEventData->EventData.HDSET_Open_Port_Request_Indication_Data));
            break;
         case etHDSET_Open_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Indication\n"));

            ProcessOpenIndicationEvent(&(HeadsetEventData->EventData.HDSET_Open_Port_Indication_Data));
            break;
         case etHDSET_Open_Port_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Confirmation\n"));

            ProcessOpenConfirmationEvent(TRUE, &(HeadsetEventData->EventData.HDSET_Open_Port_Confirmation_Data));
            break;
         case etHDSET_Ring_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Ring Indication\n"));

            ProcessRingIndicationEvent(&(HeadsetEventData->EventData.HDSET_Ring_Indication_Data));
            break;
         case etHDSET_Button_Pressed_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Button Pressed Indication\n"));

            ProcessButtonPressedIndicationEvent(&(HeadsetEventData->EventData.HDSET_Button_Pressed_Indication_Data));
            break;
         case etHDSET_Speaker_Gain_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Speaker Gain Indication\n"));

            ProcessSpeakerGainIndicationEvent(&(HeadsetEventData->EventData.HDSET_Speaker_Gain_Indication_Data));
            break;
         case etHDSET_Microphone_Gain_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Microphone Gain Indication\n"));

            ProcessMicrophoneGainIndicationEvent(&(HeadsetEventData->EventData.HDSET_Microphone_Gain_Indication_Data));
            break;
         case etHDSET_Audio_Connection_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Connection Indication\n"));

            ProcessAudioConnectionIndicationEvent(&(HeadsetEventData->EventData.HDSET_Audio_Connection_Indication_Data));
            break;
         case etHDSET_Audio_Disconnection_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Disconnection Indication\n"));

            ProcessAudioDisconnectionIndicationEvent(&(HeadsetEventData->EventData.HDSET_Audio_Disconnection_Indication_Data));
            break;
         case etHDSET_Audio_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Data Indication\n"));

            ProcessAudioDataIndicationEvent(&(HeadsetEventData->EventData.HDSET_Audio_Data_Indication_Data));
            break;
         case etHDSET_Close_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Port Indication\n"));

            ProcessCloseIndicationEvent(&(HeadsetEventData->EventData.HDSET_Close_Port_Indication_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown Headset Event Type: %d\n", HeadsetEventData->EventType));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Headset Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* process Device Manager (DEVM) Status Events (for out-going        */
   /* connection management).                                           */
static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status)
{
   int                                  Result;
   HDSM_Entry_Info_t                   *HDSETEntryInfo;
   Connection_Entry_t                  *ConnectionEntry;
   HDSET_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HDSM): 0x%08X, %d\n", StatusType, Status));

   /* First, determine if we are tracking a connection to this device.  */
   /* * NOTE * We do not know at this time which connection type we     */
   /*          are searching for, so we have to search both lists.      */
   if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, BD_ADDR, sctAudioGateway)) == NULL) || (ConnectionEntry->ConnectionState == csConnected))
   {
      if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, BD_ADDR, sctHeadset)) == NULL) || (ConnectionEntry->ConnectionState == csConnected))
         ConnectionEntry = NULL;
   }

   if(ConnectionEntry)
   {
      /* Next, let's loop through the list and see if there is an       */
      /* out-going Event connection being tracked for this event.       */
      if(ConnectionEntry->ConnectionType == sctAudioGateway)
         HDSETEntryInfo = HDSETEntryInfoList_AG;
      else
         HDSETEntryInfo = HDSETEntryInfoList_HS;

      while(HDSETEntryInfo)
      {
         /* Check to see if there is a out-going connection operation.  */
         if((!(HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(BD_ADDR, HDSETEntryInfo->ConnectionBD_ADDR)))
         {
            /* Match found.                                             */
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Outgoing Connection Entry found\n"));
            break;
         }
         else
            HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
      }

      /* See if there is any processing that is required (i.e. match    */
      /* found).                                                        */
      if(HDSETEntryInfo)
      {
         /* Process the status event.                                   */

         /* Initialize common connection event members.                 */
         BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(HDSET_Open_Port_Confirmation_Data_t));

         OpenPortConfirmationData.HDSETPortID = ConnectionEntry->HDSETID;

         if(Status)
         {
            /* Disconnect the device.                                   */
            DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

            /* Connection Failed.                                       */

            /* Map the status to a known status.                        */
            switch(Status)
            {
               case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                  OpenPortConfirmationData.PortOpenStatus = HDSET_OPEN_PORT_STATUS_CONNECTION_REFUSED;
                  break;
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                  OpenPortConfirmationData.PortOpenStatus = HDSET_OPEN_PORT_STATUS_CONNECTION_TIMEOUT;
                  break;
               default:
                  OpenPortConfirmationData.PortOpenStatus = HDSET_OPEN_PORT_STATUS_UNKNOWN_ERROR;
                  break;
            }

            /* * NOTE * This function will delete the Headset entry from*/
            /*          the list.                                       */
            ProcessOpenConfirmationEvent(TRUE, &OpenPortConfirmationData);

            /* Flag that the connection has been deleted.               */
            ConnectionEntry = NULL;
         }
         else
         {
            /* Connection succeeded.                                    */

            /* Move the state to the connecting state.                  */
            ConnectionEntry->ConnectionState = csConnecting;

            if(((Result = _HDSM_Connect_Remote_Device(ConnectionEntry->ConnectionType, BD_ADDR, ConnectionEntry->ServerPort)) <= 0) && (Result != BTPM_ERROR_CODE_HEADSET_ALREADY_CONNECTED))
            {
               /* Error opening device.                                 */
               DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

               OpenPortConfirmationData.PortOpenStatus = HDSET_OPEN_PORT_STATUS_UNKNOWN_ERROR;

               /* * NOTE * This function will delete the Headset entry  */
               /*          from the list.                               */
               ProcessOpenConfirmationEvent(TRUE, &OpenPortConfirmationData);

               /* Flag that the connection has been deleted.            */
               ConnectionEntry = NULL;
            }
            else
            {
               /* If the device is already connected, we will dispach   */
               /* the the Status only (note this case shouldn't really  */
               /* occur, but just to be safe we will clean up our state */
               /* machine).                                             */
               if(Result == BTPM_ERROR_CODE_HEADSET_ALREADY_CONNECTED)
               {
                  ConnectionEntry->ConnectionState         = csConnected;

                  OpenPortConfirmationData.PortOpenStatus  = 0;

                  ProcessOpenConfirmationEvent(FALSE, &OpenPortConfirmationData);
               }
               else
                  ConnectionEntry->HDSETID = (unsigned int)Result;
            }
         }
      }
      else
      {
         /* No out-going connection found.                              */
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("No Outgoing Connection Entry found\n"));
      }

      /* Next, we will check to see if this event is referencing an     */
      /* incoming connection.                                           */
      if((ConnectionEntry) && ((ConnectionEntry->ConnectionState == csAuthenticating) || (ConnectionEntry->ConnectionState == csEncrypting)))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Entry found\n"));

         /* Status references an outgoing connection.                   */
         if(!Status)
         {
            /* Success, accept the connection.                          */
            _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, TRUE);
         }
         else
         {
            /* Failure, reject the connection.                          */
            _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);

            /* First, delete the Connection Entry we are tracking.      */
            if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntry->BD_ADDR, ConnectionEntry->ConnectionType)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HDSM)\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Headset Manager Asynchronous Events.        */
static void BTPSAPI BTPMDispatchCallback_HDSM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Headset Manager Notification Events.        */
static void BTPSAPI BTPMDispatchCallback_HDSET(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is a Headset Event Update.        */
            if(((HDSM_Update_Data_t *)CallbackParameter)->UpdateType == utHeadsetEvent)
            {
               /* Process the Notification.                             */
               ProcessHeadsetEvent(&(((HDSM_Update_Data_t *)CallbackParameter)->UpdateData.HeadsetEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Headset Profile Manager Timer Events.       */
static void BTPSAPI BTPMDispatchCallback_TMR(void *CallbackParameter)
{
   int                                  Result;
   Connection_Entry_t                  *ConnectionEntry;
   HDSET_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HDSM)\n"));

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
            /* Check to see if the connection is still valid.           */
            if((ConnectionEntry = SearchConnectionEntryHDSETID(&ConnectionEntryList, (unsigned int)CallbackParameter)) != NULL)
            {
               /* Check to see if the Timer is still active.            */
               if(ConnectionEntry->CloseTimerID)
               {
                  /* Flag that the Timer is no longer valid (it has been*/
                  /* processed).                                        */
                  ConnectionEntry->CloseTimerID = 0;

                  /* Finally make sure that we are still in the correct */
                  /* state.                                             */
                  if(ConnectionEntry->ConnectionState == csConnectingWaiting)
                  {
                     /* Everything appears to be valid, go ahead and    */
                     /* attempt to check to see if a connection is      */
                     /* possible (if so, attempt it).                   */
                     if(!SPPM_WaitForPortDisconnection(ConnectionEntry->ServerPort, FALSE, ConnectionEntry->BD_ADDR, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS))
                     {
                        /* Port is disconnected, let's attempt to make  */
                        /* the connection.                              */
                        DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device      */
                        if(ConnectionEntry->ConnectionFlags & HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                           ConnectionEntry->ConnectionState = csEncrypting;
                        else
                        {
                           if(ConnectionEntry->ConnectionFlags & HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                              ConnectionEntry->ConnectionState = csAuthenticating;
                           else
                              ConnectionEntry->ConnectionState = csConnectingDevice;
                        }

                        Result = DEVM_ConnectWithRemoteDevice(ConnectionEntry->BD_ADDR, (ConnectionEntry->ConnectionState == csConnectingDevice)?0:((ConnectionEntry->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                        if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                        {
                           /* Check to see if we need to actually issue */
                           /* the Remote connection.                    */
                           if(Result < 0)
                           {
                              /* Set the state to connecting remote     */
                              /* device.                                */
                              ConnectionEntry->ConnectionState = csConnecting;

                              if((Result = _HDSM_Connect_Remote_Device(ConnectionEntry->ConnectionType, ConnectionEntry->BD_ADDR, ConnectionEntry->ServerPort)) <= 0)
                                 Result = BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE;
                              else
                              {
                                 /* Note the Headset Port ID.           */
                                 ConnectionEntry->HDSETID = (unsigned int)Result;

                                 /* Flag success.                       */
                                 Result                  = 0;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Port is not disconnected, check to see if the*/
                        /* count exceedes the maximum count.            */
                        ConnectionEntry->CloseTimerCount++;

                        if(ConnectionEntry->CloseTimerCount >= MAXIMUM_HANDS_FREE_PORT_OPEN_DELAY_RETRY)
                           Result = BTPM_ERROR_CODE_HEADSET_CONNECTION_RETRIES_EXCEEDED;
                        else
                        {
                           /* Port is NOT disconnected, go ahead and    */
                           /* start a timer so that we can continue to  */
                           /* check for the Port Disconnection.         */
                           Result = TMR_StartTimer((void *)ConnectionEntry->HDSETID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                           /* If the timer was started, go ahead and    */
                           /* note the Timer ID.                        */
                           if(Result > 0)
                           {
                              ConnectionEntry->CloseTimerID = (unsigned int)Result;

                              Result                        = 0;
                           }
                           else
                              Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
                        }
                     }

                     /* Error occurred, go ahead and dispatch an error  */
                     /* (as well as to delete the connection entry).    */
                     if(Result < 0)
                     {
                        /* Initialize common connection event members.  */
                        BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(HDSET_Open_Port_Confirmation_Data_t));

                        OpenPortConfirmationData.HDSETPortID = ConnectionEntry->HDSETID;

                        if(Result)
                        {
                           /* Connection Failed.                        */

                           /* Map the status to a known status.         */
                           switch(Result)
                           {
                              case BTPM_ERROR_CODE_HEADSET_CONNECTION_RETRIES_EXCEEDED:
                              case BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER:
                                 OpenPortConfirmationData.PortOpenStatus = HDSET_OPEN_PORT_STATUS_CONNECTION_TIMEOUT;
                                 break;
                              case BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE:
                                 OpenPortConfirmationData.PortOpenStatus = HDSET_OPEN_PORT_STATUS_UNKNOWN_ERROR;
                                 break;
                              case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
                              case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                              default:
                                 OpenPortConfirmationData.PortOpenStatus = HDSET_OPEN_PORT_STATUS_CONNECTION_REFUSED;
                                 break;
                           }

                           /* * NOTE * This function will delete the    */
                           /*          Headset entry from the list.     */
                           ProcessOpenConfirmationEvent(TRUE, &OpenPortConfirmationData);
                        }
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Connection is no longer in the correct state: 0x%08X (%d)\n", (unsigned int)CallbackParameter, ConnectionEntry->ConnectionState));
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Close Timer is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Connection is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HDSM)\n"));
}

   /* The following function is the Timer Callback function that is     */
   /* registered to process Serial Port Disconnection Events (to        */
   /* determine when it is safe to connect to a remote device).         */
static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HDSM)\n"));

   /* Simply queue a Timer Callback Event to process.                   */
   if(BTPM_QueueMailboxCallback(BTPMDispatchCallback_TMR, CallbackParameter))
      ret_val = FALSE;
   else
      ret_val = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HDSM): %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Headset Manager         */
   /* Messages.                                                         */
static void BTPSAPI HeadsetManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HEADSET_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a Headset Manager        */
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
               /* Headset Manager Thread.                               */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDSM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Headset Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Headset Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Headset Manager       */
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
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Headset Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Headset Manager module.  This      */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI HDSM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                         Result;
   HDSM_Initialization_Info_t *InitializationInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      /* Check to see if this module has already been initialized.      */
      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Headset\n"));

         /* Make sure that there is actual Headset/Audio Gateway data   */
         /* specified.                                                  */
         if(((InitializationInfo = (HDSM_Initialization_Info_t *)InitializationData) != NULL) && ((InitializationInfo->AudioGatewayInitializationInfo) || (InitializationInfo->HeadsetInitializationInfo)))
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Headset Manager messages.                     */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEADSET_MANAGER, HeadsetManagerGroupHandler, NULL))
            {
               /* Initialize the actual Headset Manager Implementation  */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the Headset     */
               /* Manager functionality - this module is just the       */
               /* framework shell).                                     */
               if(!(Result = _HDSM_Initialize(InitializationInfo->AudioGatewayInitializationInfo, InitializationInfo->HeadsetInitializationInfo)))
               {
                  /* Make sure the initialization information is set to */
                  /* a known state.                                     */
                  BTPS_MemInitialize(&AudioGatewayInitializationInfo, 0, sizeof(AudioGatewayInitializationInfo));
                  BTPS_MemInitialize(&HeadsetInitializationInfo, 0, sizeof(HeadsetInitializationInfo));

                  /* Note any Audio Gateway/Headset initialization      */
                  /* information.                                       */
                  if(InitializationInfo->AudioGatewayInitializationInfo)
                  {
                     AudioGatewayInitializationInfo = *InitializationInfo->AudioGatewayInitializationInfo;
                     AudioGatewaySupported          = TRUE;
                  }
                  else
                     AudioGatewaySupported = FALSE;

                  if(InitializationInfo->HeadsetInitializationInfo)
                  {
                     HeadsetInitializationInfo = *InitializationInfo->HeadsetInitializationInfo;
                     HeadsetSupported          = TRUE;
                  }
                  else
                     HeadsetSupported = FALSE;

                  /* Finally determine the current Device Power State.  */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting Headset Callback ID. */
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
               _HDSM_Cleanup();

               MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEADSET_MANAGER);
            }
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEADSET_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the Headset Manager Implementation   */
            /* that we are shutting down.                               */
            _HDSM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the Audio Entry Data Information List is  */
            /* empty.                                                   */
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_AG);
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_HS);

            /* Make sure that the Headset entry control information list*/
            /* is empty.                                                */
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_AG_Control);
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_HS_Control);

            /* Make sure that the Headset entry data information list is*/
            /* empty.                                                   */
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_AG_Data);
            FreeHDSETEntryInfoList(&HDSETEntryInfoList_HS_Data);

            /* Flag that the resources are no longer allocated.         */
            CurrentPowerState     = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized           = FALSE;

            AudioGatewaySupported = FALSE;
            HeadsetSupported    = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDSM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                 Result;
   unsigned int        Index;
   HDSM_Entry_Info_t  *tmpHDSETEntryInfo;
   HDSM_Entry_Info_t  *HDSETEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the Headset Manager that it should*/
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  _HDSM_SetBluetoothStackID((unsigned int)Result);

                  /* Make sure SDP records are enabled if we already    */
                  /* have a callback.                                   */
                  if(HDSETEntryInfoList_AG_Control)
                     _HDSM_UpdateSDPRecord(sctAudioGateway, TRUE);

                  if(HDSETEntryInfoList_HS_Control)
                     _HDSM_UpdateSDPRecord(sctHeadset, TRUE);
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Close any active connections.                         */
               ConnectionEntry = ConnectionEntryList;

               while(ConnectionEntry)
               {
                  if(ConnectionEntry->ConnectionState == csAuthorizing)
                     _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);
                  else
                     _HDSM_Disconnect_Device(ConnectionEntry->HDSETID);

                  ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
               }

               /* Inform the Headset Manager that the Stack has been    */
               /* closed.                                               */
               _HDSM_SetBluetoothStackID(0);

               /* Loop through all outgoing connections to determine if */
               /* there are any synchronous connections outstanding.    */
               Index = 2;
               while(Index--)
               {
                  if(Index)
                     HDSETEntryInfo = HDSETEntryInfoList_AG;
                  else
                     HDSETEntryInfo = HDSETEntryInfoList_HS;

                  while(HDSETEntryInfo)
                  {
                     /* Check to see if there is a synchronous open     */
                     /* operation.                                      */
                     if(!(HDSETEntryInfo->Flags & HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                     {
                        if(HDSETEntryInfo->ConnectionEvent)
                        {
                           HDSETEntryInfo->ConnectionStatus = HDSM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                           BTPS_SetEvent(HDSETEntryInfo->ConnectionEvent);

                           HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
                        }
                        else
                        {
                           /* Entry was waiting on a response, but it   */
                           /* was registered as either an Event Callback*/
                           /* or Connection Message.  Regardless we need*/
                           /* to delete it.                             */
                           tmpHDSETEntryInfo = HDSETEntryInfo;

                           HDSETEntryInfo    = HDSETEntryInfo->NextHDSETEntryInfoPtr;

                           if((tmpHDSETEntryInfo = DeleteHDSETEntryInfoEntry((Index)?&HDSETEntryInfoList_HS:&HDSETEntryInfoList_AG, tmpHDSETEntryInfo->CallbackID)) != NULL)
                              FreeHDSETEntryInfoEntryMemory(tmpHDSETEntryInfo);
                        }
                     }
                     else
                        HDSETEntryInfo = HDSETEntryInfo->NextHDSETEntryInfoPtr;
                  }
               }

               /* Finally free all incoming/outgoing connection entries */
               /* (as there cannot be any active connections).          */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDeviceAuthenticationStatus:
               /* Authentication Status, process the Status Event.      */
               ProcessDEVMStatusEvent(dstAuthentication, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status);
               break;
            case detRemoteDeviceEncryptionStatus:
               /* Encryption Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstEncryption, EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status);
               break;
            case detRemoteDeviceConnectionStatus:
               /* Connection Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstConnection, EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Headset Manager of a specific Update Event.  The    */
   /* Headset Manager can then take the correct action to process the   */
   /* update.                                                           */
Boolean_t HDSM_NotifyUpdate(HDSM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utHeadsetEvent:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Headset Event: %d\n", UpdateData->UpdateData.HeadsetEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDSET, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Headset Manager Connection Management Functions.                  */

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened.  A  */
   /*          hetHDSConnected event will notify if the connection is   */
   /*          successful.                                              */
int BTPSAPI HDSM_Connection_Request_Response(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int                 ret_val;
   BD_ADDR_t           NULL_BD_ADDR;
   Boolean_t           Authenticate;
   Boolean_t           Encrypt;
   unsigned long       IncomingConnectionFlags;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, next, verify that we are already*/
               /* tracking a connection for the specified connection    */
               /* type.                                                 */
               if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL) && (ConnectionEntry->ConnectionState == csAuthorizing))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", AcceptConnection));

                  /* Determine the incoming connection flags based on   */
                  /* the connection type.                               */
                  if(ConnectionType == sctAudioGateway)
                     IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
                  else
                     IncomingConnectionFlags = HeadsetInitializationInfo.IncomingConnectionFlags;

                  /* If the caller has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(AcceptConnection)
                  {
                     /* Determine if Authentication and/or Encryption is*/
                     /* required for this link.                         */
                     if(IncomingConnectionFlags & HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                        Authenticate = TRUE;
                     else
                        Authenticate = FALSE;

                     if(IncomingConnectionFlags & HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                        Encrypt = TRUE;
                     else
                        Encrypt = FALSE;

                     if((Authenticate) || (Encrypt))
                     {
                        if(Encrypt)
                           ret_val = DEVM_EncryptRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                        else
                           ret_val = DEVM_AuthenticateRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

                     if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                     {
                        /* Authorization not required, and we are       */
                        /* already in the correct state.                */
                        ret_val = _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, TRUE);

                        if(ret_val)
                        {
                           _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);

                           /* Go ahead and delete the entry because we  */
                           /* are finished with tracking it.            */
                           if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionEntry->ConnectionType)) != NULL)
                              FreeConnectionEntryMemory(ConnectionEntry);
                        }
                        else
                           ConnectionEntry->ConnectionState = csConnecting;
                     }
                     else
                     {
                        /* If we were successfully able to Authenticate */
                        /* and/or Encrypt, then we need to set the      */
                        /* correct state.                               */
                        if(!ret_val)
                        {
                           if(Encrypt)
                              ConnectionEntry->ConnectionState = csEncrypting;
                           else
                              ConnectionEntry->ConnectionState = csAuthenticating;

                           /* Flag success to the caller.               */
                           ret_val = 0;
                        }
                        else
                        {
                           /* Error, reject the request.                */
                           _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);

                           /* Go ahead and delete the entry because we  */
                           /* are finished with tracking it.            */
                           if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionEntry->ConnectionType)) != NULL)
                              FreeConnectionEntryMemory(ConnectionEntry);
                        }
                     }
                  }
                  else
                  {
                     /* Rejection - Simply respond to the request.      */
                     ret_val = _HDSM_Connection_Request_Response(ConnectionEntry->HDSETID, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionEntry->ConnectionType)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

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
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Headset/Audio Gateway device.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function accepts the connection */
   /* type to make as the first parameter.  This parameter specifies the*/
   /* LOCAL connection type (i.e.  if the caller would like to connect  */
   /* the local Headset service to a remote Audio Gateway device, the   */
   /* Headset connection type would be specified for this parameter).   */
   /* This function also accepts the connection information for the     */
   /* remote device (address and server port).  This function accepts   */
   /* the connection flags to apply to control how the connection is    */
   /* made regarding encryption and/or authentication.                  */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Headset Manager Connection Status Event (if specified).  */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHDSConnectionStatus event will be dispatched to denote*/
   /*          the status of the connection.  This is the ONLY way to   */
   /*          receive this event, as an event callack registered with  */
   /*          the HDSM_Register_Event_Callback() will NOT receive      */
   /*          connection status events.                                */
int BTPSAPI HDSM_Connect_Remote_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   BD_ADDR_t           NULL_BD_ADDR;
   Boolean_t           Delete;
   unsigned int        CallbackID;
   HDSM_Entry_Info_t   HDSETEntryInfo;
   HDSM_Entry_Info_t  *HDSETEntryInfoPtr;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)) && (((ConnectionType == sctAudioGateway) && (AudioGatewaySupported)) || ((ConnectionType == sctHeadset) && (HeadsetSupported))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) == NULL)
               {
                  /* Entry is not present, go ahead and create a new    */
                  /* entry.                                             */
                  BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

                  ConnectionEntry.ConnectionType  = ConnectionType;
                  ConnectionEntry.BD_ADDR         = RemoteDeviceAddress;
                  ConnectionEntry.HDSETID         = GetNextCallbackID() | 0x80000000;
                  ConnectionEntry.Server          = FALSE;
                  ConnectionEntry.ServerPort      = RemoteServerPort;
                  ConnectionEntry.ConnectionState = csIdle;
                  ConnectionEntry.ConnectionFlags = ConnectionFlags;

                  if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
                  {
                     /* Attempt to add an entry into the Headset entry  */
                     /* list.                                           */
                     BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

                     HDSETEntryInfo.CallbackID        = GetNextCallbackID();
                     HDSETEntryInfo.ClientID          = MSG_GetServerAddressID();
                     HDSETEntryInfo.ConnectionBD_ADDR = RemoteDeviceAddress;
                     HDSETEntryInfo.EventCallback     = CallbackFunction;
                     HDSETEntryInfo.CallbackParameter = CallbackParameter;

                     if(ConnectionStatus)
                        HDSETEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

                     Delete = FALSE;

                     if((!ConnectionStatus) || ((ConnectionStatus) && (HDSETEntryInfo.ConnectionEvent)))
                     {
                        if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, &HDSETEntryInfo)) != NULL)
                        {
                           /* First, let's wait for the Port to         */
                           /* disconnect.                               */
                           if(!SPPM_WaitForPortDisconnection(RemoteServerPort, FALSE, RemoteDeviceAddress, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS))
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                              /* Next, attempt to open the remote       */
                              /* device.                                */
                              if(ConnectionFlags & HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                                 ConnectionEntryPtr->ConnectionState = csEncrypting;
                              else
                              {
                                 if(ConnectionFlags & HDSM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                                    ConnectionEntryPtr->ConnectionState = csAuthenticating;
                                 else
                                    ConnectionEntryPtr->ConnectionState = csConnectingDevice;
                              }

                              DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Device\n"));

                              ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, (ConnectionEntryPtr->ConnectionState == csConnectingDevice)?0:((ConnectionEntryPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                              if((ret_val >= 0) || (ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                              {
                                 /* Check to see if we need to actually */
                                 /* issue the Remote connection.        */
                                 if(ret_val < 0)
                                 {
                                    /* Set the state to connecting      */
                                    /* remote device.                   */
                                    ConnectionEntryPtr->ConnectionState = csConnecting;

                                    if((ret_val = _HDSM_Connect_Remote_Device(ConnectionEntryPtr->ConnectionType, RemoteDeviceAddress, RemoteServerPort)) <= 0)
                                    {
                                       ret_val = BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE;

                                       /* Error opening device, go ahead*/
                                       /* and delete the entry that was */
                                       /* added.                        */
                                       if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfoPtr->CallbackID)) != NULL)
                                       {
                                          if(HDSETEntryInfoPtr->ConnectionEvent)
                                             BTPS_CloseEvent(HDSETEntryInfoPtr->ConnectionEvent);

                                          FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);
                                       }
                                    }
                                    else
                                    {
                                       /* Note the Headset Port ID.     */
                                       ConnectionEntryPtr->HDSETID = (unsigned int)ret_val;

                                       /* Flag success.                 */
                                       ret_val                    = 0;
                                    }
                                 }
                              }

                           }
                           else
                           {
                              /* Move the state to the connecting       */
                              /* Waiting state.                         */
                              ConnectionEntryPtr->ConnectionState = csConnectingWaiting;

                              if((BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS) && (MAXIMUM_HANDS_FREE_PORT_OPEN_DELAY_RETRY))
                              {
                                 /* Port is NOT disconnected, go ahead  */
                                 /* and start a timer so that we can    */
                                 /* continue to check for the Port      */
                                 /* Disconnection.                      */
                                 ret_val = TMR_StartTimer((void *)ConnectionEntry.HDSETID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                                 /* If the timer was started, go ahead  */
                                 /* and note the Timer ID.              */
                                 if(ret_val > 0)
                                 {
                                    ConnectionEntryPtr->CloseTimerID = (unsigned int)ret_val;

                                    ret_val                          = 0;
                                 }
                                 else
                                    ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_HEADSET_IS_STILL_CONNECTED;
                           }

                           /* Next, determine if the caller has         */
                           /* requested a blocking open.                */
                           if((!ret_val) && (ConnectionStatus))
                           {
                              /* Blocking open, go ahead and wait for   */
                              /* the event.                             */

                              /* Note the Callback ID.                  */
                              CallbackID      = HDSETEntryInfoPtr->CallbackID;

                              /* Note the Open Event.                   */
                              ConnectionEvent = HDSETEntryInfoPtr->ConnectionEvent;

                              /* Release the lock because we are        */
                              /* finished with it.                      */
                              DEVM_ReleaseLock();

                              BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                              /* Re-acquire the Lock.                   */
                              if(DEVM_AcquireLock())
                              {
                                 if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, CallbackID)) != NULL)
                                 {
                                    /* Note the connection status.      */
                                    *ConnectionStatus = HDSETEntryInfoPtr->ConnectionStatus;

                                    BTPS_CloseEvent(HDSETEntryInfoPtr->ConnectionEvent);

                                    FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);

                                    /* Flag success to the caller.      */
                                    ret_val = 0;
                                 }
                                 else
                                    ret_val = BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

                              /* Flag that the entry is to be deleted.  */
                              Delete = TRUE;
                           }
                           else
                           {
                              /* If we are not tracking this connection */
                              /* OR there was an error, go ahead and    */
                              /* delete the entry that was added.       */
                              if((!CallbackFunction) || (ret_val))
                              {
                                 if((HDSETEntryInfoPtr = DeleteHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, HDSETEntryInfo.CallbackID)) != NULL)
                                 {
                                    if(HDSETEntryInfoPtr->ConnectionEvent)
                                       BTPS_CloseEvent(HDSETEntryInfoPtr->ConnectionEvent);

                                    FreeHDSETEntryInfoEntryMemory(HDSETEntryInfoPtr);
                                 }
                              }
                           }
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;

                     /* If an error occurred, go ahead and delete the   */
                     /* Connection Information that was added.          */
                     if((ret_val) || (Delete))
                     {
                        if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                           FreeConnectionEntryMemory(ConnectionEntryPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
               {
                  if(ConnectionEntryPtr->ConnectionState == csConnected)
                     ret_val = BTPM_ERROR_CODE_HEADSET_ALREADY_CONNECTED;
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_CONNECTION_IN_PROGRESS;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Headset or Audio */
   /* Gateway connection that was previously opened by any of the       */
   /* following mechanisms:                                             */
   /*   - Successful call to HDSM_Connect_Remote_Device() function.     */
   /*   - Incoming open request (Headset or Audio Gateway) which was    */
   /*     accepted either automatically or by a call to                 */
   /*     HDSM_Connection_Request_Response().                           */
   /* This function accepts as input the type of the local connection   */
   /* which should close its active connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.  This function does NOT un-register any Headset or Audio   */
   /* Gateway services from the system, it ONLY disconnects any         */
   /* connection that is currently active on the specified service.     */
int BTPSAPI HDSM_Disconnect_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                                 ret_val;
   Boolean_t                           Server;
   Boolean_t                           PerformDisconnect;
   unsigned int                        ServerPort;
   Connection_Entry_t                 *ConnectionEntry;
   HDSET_Close_Port_Indication_Data_t  ClosePortIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
            {
               switch(ConnectionEntry->ConnectionState)
               {
                  case csAuthorizing:
                  case csAuthenticating:
                  case csEncrypting:
                     /* Should not occur.                               */
                     PerformDisconnect = FALSE;
                     break;
                  case csConnectingWaiting:
                     if(ConnectionEntry->CloseTimerID)
                        TMR_StopTimer(ConnectionEntry->CloseTimerID);

                     PerformDisconnect = FALSE;
                     break;
                  case csConnectingDevice:
                     PerformDisconnect = FALSE;
                     break;
                  default:
                  case csConnecting:
                  case csConnected:
                     PerformDisconnect = TRUE;
                     break;
               }

               if(PerformDisconnect)
               {
                  /* Nothing really to do other than to disconnect the  */
                  /* device (if it is connected, a disconnect will be   */
                  /* dispatched from the framework).                    */
                  ret_val = _HDSM_Disconnect_Device(ConnectionEntry->HDSETID);
               }
               else
                  ret_val = 0;

               /* If the result was successful, we need to make sure we */
               /* clean up everything and dispatch the event to all     */
               /* registered clients.                                   */
               if(!ret_val)
               {
                  /* Make sure we are no longer tracking SCO connections*/
                  /* for the specified device.                          */
                  if(!COMPARE_NULL_BD_ADDR(ConnectionEntry->BD_ADDR))
                     SCOM_DeleteConnectionFromIgnoreList(ConnectionEntry->BD_ADDR);

                  /* Since we are going to free the connection entry we */
                  /* need to note some values so we can use them after  */
                  /* we free the structure.                             */
                  Server     = ConnectionEntry->Server;
                  ServerPort = ConnectionEntry->ServerPort;

                  /* Fake a close event to dispatch to all registered   */
                  /* clients that the device is no longer connected.    */
                  ClosePortIndicationData.HDSETPortID      = ConnectionEntry->HDSETID;
                  ClosePortIndicationData.PortCloseStatus = HDSET_CLOSE_PORT_STATUS_SUCCESS;

                  ProcessCloseIndicationEvent(&ClosePortIndicationData);

                  /* Go ahead and give the port some time to disconnect */
                  /* (since it was initiated locally).                  */
                  SPPM_WaitForPortDisconnection(ServerPort, Server, RemoteDeviceAddress, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS);
               }
            }
            else
            {
               if(ConnectionEntry)
                  ret_val = 0;
               else
                  ret_val = BTPM_ERROR_CODE_HEADSET_CONNECTION_IN_PROGRESS;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Headset */
   /* or Audio Gateway Devices (specified by the first parameter).  This*/
   /* function accepts a the local service type to query, followed by   */
   /* buffer information to receive any currently connected device      */
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
int BTPSAPI HDSM_Query_Connected_Devices(HDSM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                 ret_val;
   unsigned int        NumberConnected;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(((ConnectionType == sctAudioGateway) && (AudioGatewaySupported)) || ((ConnectionType == sctHeadset) && (HeadsetSupported)))
      {
         /* Next, check to see if the input parameters appear to be     */
         /* semi-valid.                                                 */
         if(((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)) || ((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)))
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Check to see if the device is powered on.             */
               if(CurrentPowerState)
               {
                  /* Let's determine how many devices are actually      */
                  /* connected.                                         */
                  NumberConnected = 0;
                  ConnectionEntry = ConnectionEntryList;

                  while(ConnectionEntry)
                  {
                     /* Note that we are only counting devices that are */
                     /* counting devices that either in the connected   */
                     /* state or the connecting state (i.e. have been   */
                     /* authorized OR passed authentication).           */
                     if((ConnectionEntry->ConnectionType == ConnectionType) && ((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting)))
                        NumberConnected++;

                     ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
                  }

                  /* We now have the total number of devices that will  */
                  /* satisy the query.                                  */
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = NumberConnected;

                  /* Now that have the total, we need to build the      */
                  /* Connected Device List.                             */

                  /* See if the caller would like to copy some (or all) */
                  /* of the list.                                       */
                  if(MaximumRemoteDeviceListEntries)
                  {
                     /* If there are more entries in the returned list  */
                     /* than the buffer specified, we need to truncate  */
                     /* it.                                             */
                     if(MaximumRemoteDeviceListEntries >= NumberConnected)
                        MaximumRemoteDeviceListEntries = NumberConnected;

                     NumberConnected = 0;

                     ConnectionEntry = ConnectionEntryList;

                     while((ConnectionEntry) && (NumberConnected < MaximumRemoteDeviceListEntries))
                     {
                        /* Note that we are only counting devices that  */
                        /* are counting devices that either in the      */
                        /* connected state or the connecting state (i.e.*/
                        /* have been authorized OR passed               */
                        /* authentication).                             */
                        if((ConnectionEntry->ConnectionType == ConnectionType) && ((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting)))
                           RemoteDeviceAddressList[NumberConnected++] = ConnectionEntry->BD_ADDR;

                        ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
                     }

                     /* Note the total number of devices that were      */
                     /* copied into the array.                          */
                     ret_val = (int)NumberConnected;
                  }
                  else
                     ret_val = 0;
               }
               else
                  ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

               /* Release the Lock because we are finished with it.     */
               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Headset or Audio   */
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
int BTPSAPI HDSM_Query_Current_Configuration(HDSM_Connection_Type_t ConnectionType, HDSM_Current_Configuration_t *CurrentConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(CurrentConfiguration)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Flag success to the caller.                              */
            ret_val = 0;

            /* Check to see if Audio Gateway or Headset was specified.  */
            if(ConnectionType == sctAudioGateway)
            {
               if(AudioGatewaySupported)
               {
                  /* Copy the static information.                       */
                  CurrentConfiguration->IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
                  CurrentConfiguration->SupportedFeaturesMask   = AudioGatewayInitializationInfo.SupportedFeaturesMask;
               }
               else
                  ret_val = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
            }
            else
            {
               if(HeadsetSupported)
               {
                  /* Copy the static information.                       */
                  CurrentConfiguration->IncomingConnectionFlags = HeadsetInitializationInfo.IncomingConnectionFlags;
                  CurrentConfiguration->SupportedFeaturesMask   = HeadsetInitializationInfo.SupportedFeaturesMask;
               }
               else
                  ret_val = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
            }

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
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Headset and   */
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI HDSM_Change_Incoming_Connection_Flags(HDSM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Flag success to the caller.                                 */
         ret_val = 0;

         /* Check to see if Audio Gateway or Headset was specified.     */
         if(ConnectionType == sctAudioGateway)
         {
            if(AudioGatewaySupported)
               AudioGatewayInitializationInfo.IncomingConnectionFlags = ConnectionFlags;
            else
               ret_val = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
         }
         else
         {
            if(HeadsetSupported)
               HeadsetInitializationInfo.IncomingConnectionFlags = ConnectionFlags;
            else
               ret_val = BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Shared Headset/Audio Gateway Functions.                           */

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  When called by a     */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current speaker gain value.  When     */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the speaker gain of the remote Headset   */
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the speaker gain to be sent to the remote device. */
   /* The speaker gain Parameter *MUST* be between the values:          */
   /*                                                                   */
   /*    HDSET_SPEAKER_GAIN_MINIMUM                                     */
   /*    HDSET_SPEAKER_GAIN_MAXIMUM                                     */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int BTPSAPI HDSM_Set_Remote_Speaker_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set speaker gain.                   */
                     ret_val = _HDSM_Set_Remote_Speaker_Gain(ConnectionEntry->HDSETID, SpeakerGain);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  When called by a  */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current microphone gain value.  When  */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the microphone gain of the remote Headset*/
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the microphone gain to be sent to the remote      */
   /* device.  The microphone gain Parameter *MUST* be between the      */
   /* values:                                                           */
   /*                                                                   */
   /*    HDSET_MICROPHONE_GAIN_MINIMUM                                  */
   /*    HDSET_MICROPHONE_GAIN_MAXIMUM                                  */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int BTPSAPI HDSM_Set_Remote_Microphone_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set microphone gain.                */
                     ret_val = _HDSM_Set_Remote_Microphone_Gain(ConnectionEntry->HDSETID, MicrophoneGain);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Headset Functions.                                                */

   /* This function is responsible for sending the command to a remote  */
   /* Audi Gateway to answer an incoming call.  This function may only  */
   /* be performed by Headset devices.  This function return zero if    */
   /* successful or a negative return error code if there was an error. */

   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int BTPSAPI HDSM_Send_Button_Press(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, sctHeadset)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the button press.              */
                     ret_val = _HDSM_Send_Button_Press(ConnectionEntry->HDSETID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Audio Gateway Functions.                                          */

   /* This function is responsible for sending a ring indication to a   */
   /* remote Headset unit.  This function may only be performed by Audio*/
   /* Gateways.  This function returns zero if successful or a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int BTPSAPI HDSM_Ring_Indication(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, HeadsetManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, sctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the ring indication.           */
                     ret_val = _HDSM_Ring_Indication(ConnectionEntry->HDSETID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Headset Manager Audio Connection Management Functions.            */

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Headset devices.  This function      */
   /* accepts as its input parameter the connection type indicating     */
   /* which connection will process the command.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int BTPSAPI HDSM_Setup_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t InBandRinging)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set-up the audio connection (after  */
                     /* we determine if this is for in-band ringing).   */
                     if((ConnectionType == sctAudioGateway) && (AudioGatewayInitializationInfo.SupportedFeaturesMask & HDSM_SUPPORTED_FEATURES_MASK_AUDIO_GATEWAY_SUPPORTS_IN_BAND_RING) && (InBandRinging))
                        ret_val = _HDSM_Setup_Audio_Connection(ConnectionEntry->HDSETID, TRUE);
                     else
                        ret_val = _HDSM_Setup_Audio_Connection(ConnectionEntry->HDSETID, FALSE);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HDSM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Headset */
   /* device.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int BTPSAPI HDSM_Release_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to release the audio connection.       */
                     ret_val = _HDSM_Release_Audio_Connection(ConnectionEntry->HDSETID);

                     if(!ret_val)
                        ConnectionEntry->SCOHandle = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Headset Manager Data Handler ID (registered  */
   /* via call to the HDSM_Register_Data_Event_Callback() function),    */
   /* followed by the the connection type indicating which connection   */
   /* will transmit the audio data, the length (in Bytes) of the audio  */
   /* data to send, and a pointer to the audio data to send to the      */
   /* remote entity.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function is only applicable for Bluetooth devices   */
   /*          that are configured to support packetized SCO audio.     */
   /*          This function will have no effect on Bluetooth devices   */
   /*          that are configured to process SCO audio via hardare     */
   /*          codec.                                                   */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to process the audio data themselves (as */
   /*          opposed to having the hardware process the audio data via*/
   /*          a hardware codec.                                        */
   /* * NOTE * The data that is sent *MUST* be formatted in the correct */
   /*          SCO format that is expected by the device.               */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int BTPSAPI HDSM_Send_Audio_Data(unsigned int HeadsetManagerDataEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerDataEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (AudioDataLength) && (AudioData))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Data:&HDSETEntryInfoList_HS_Data, HeadsetManagerDataEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the audio data.                */
                     ret_val = _HDSM_Send_Audio_Data(ConnectionEntry->HDSETID, AudioDataLength, AudioData);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerDataEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Headset Manager Event Callback Registration Functions.            */

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Headset Profile  */
   /* Manager Service.  This Callback will be dispatched by the Headset */
   /* Manager when various Headset Manager events occur.  This function */
   /* accepts the callback function and callback parameter              */
   /* (respectively) to call when a Headset Manager event needs to be   */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDSM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
int BTPSAPI HDSM_Register_Event_Callback(HDSM_Connection_Type_t ConnectionType, Boolean_t ControlCallback, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   HDSM_Entry_Info_t  HDSETEntryInfo;
   HDSM_Entry_Info_t *HDSETEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a control event handler for the specified        */
            /* connection type (if control handler was specified).      */
            if(ControlCallback)
            {
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfoPtr = HDSETEntryInfoList_AG_Control;
               else
                  HDSETEntryInfoPtr = HDSETEntryInfoList_HS_Control;
            }
            else
               HDSETEntryInfoPtr = NULL;

            if(!HDSETEntryInfoPtr)
            {
               /* First, register the handler locally.                  */
               BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

               HDSETEntryInfo.CallbackID        = GetNextCallbackID();
               HDSETEntryInfo.ClientID          = MSG_GetServerAddressID();
               HDSETEntryInfo.EventCallback     = CallbackFunction;
               HDSETEntryInfo.CallbackParameter = CallbackParameter;
               HDSETEntryInfo.Flags             = HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               /* Note the connection type.                             */
               if(ConnectionType == sctAudioGateway)
                  HDSETEntryInfo.Flags |= HDSET_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

               /* Check to see if we need to register a control handler */
               /* or a normal event handler.                            */
               if(ControlCallback)
               {
                  /* Control handler, add it the correct list.          */
                  if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, &HDSETEntryInfo)) != NULL)
                     ret_val = HDSETEntryInfoPtr->CallbackID;
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
               {
                  if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG:&HDSETEntryInfoList_HS, &HDSETEntryInfo)) != NULL)
                     ret_val = HDSETEntryInfoPtr->CallbackID;
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_ALREADY_REGISTERED;

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
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager event Callback*/
   /* (registered via a successful call to the                          */
   /* HDSM_Register_Event_Callback() function.  This function accepts as*/
   /* input the Headset Manager event callback ID (return value from the*/
   /* HDSM_Register_Event_Callback() function).                         */
void BTPSAPI HDSM_Un_Register_Event_Callback(unsigned int HeadsetManagerEventCallbackID)
{
   Boolean_t               ControlEvent = FALSE;
   HDSM_Entry_Info_t      *HDSETEntryInfo;
   Connection_Entry_t     *ConnectionEntry;
   Connection_Entry_t     *CloseConnectionEntry;
   HDSM_Connection_Type_t  ConnectionType;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HeadsetManagerEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* We need to determine what type of Callback this is (as we*/
            /* process them differently).                               */
            ConnectionType = sctAudioGateway;
            if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG, HeadsetManagerEventCallbackID)) == NULL)
            {
               if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Control, HeadsetManagerEventCallbackID)) == NULL)
               {
                  ConnectionType = sctHeadset;
                  if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS, HeadsetManagerEventCallbackID)) == NULL)
                  {
                     HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID);

                     if(HDSETEntryInfo)
                        ControlEvent = TRUE;
                  }
               }
               else
                  ControlEvent = TRUE;
            }

            /* Check to see if we found the callback and deleted it.    */
            if(HDSETEntryInfo)
            {
               /* Free the memory because we are finished with it.      */
               FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);

               if(ControlEvent)
               {
                  /* Remove the SDP Record.                                   */
                  _HDSM_UpdateSDPRecord(ConnectionType, FALSE);

                  /* Check to see if there is an open connection.             */
                  ConnectionEntry = ConnectionEntryList;
                  while(ConnectionEntry)
                  {
                     /* Check to see if this type control is being      */
                     /* unregistered.                                   */
                     if(ConnectionEntry->ConnectionType == ConnectionType)
                        CloseConnectionEntry = ConnectionEntry;
                     else
                        CloseConnectionEntry = NULL;

                     /* Move on to the next entry since we may be deleting    */
                     /* this one.                                             */
                     ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;

                     /* Now cleanup the connection.                           */
                     if(CloseConnectionEntry)
                     {
                        _HDSM_Disconnect_Device(CloseConnectionEntry->HDSETID);

                        if((CloseConnectionEntry = DeleteConnectionEntryHDSETID(&ConnectionEntryList, CloseConnectionEntry->HDSETID)) != NULL)
                           FreeConnectionEntryMemory(CloseConnectionEntry);
                     }
                  }
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Headset   */
   /* Profile Manager service to explicitly process SCO audio data.     */
   /* This callback will be dispatched by the Headset Manager when      */
   /* various Headset Manager events occur.  This function accepts the  */
   /* connection type which indicates the connection type the data      */
   /* registration callback to register for, and the callback function  */
   /* and callback parameter (respectively) to call when a Headset      */
   /* Manager event needs to be dispatched.  This function returns a    */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HDSM_Send_Audio_Data() function to send SCO audio data.  */
   /* * NOTE * There can only be a single data event handler registered */
   /*          for each type of Headset Manager connection type.        */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDSM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HDSM_Register_Data_Event_Callback(HDSM_Connection_Type_t ConnectionType, HDSM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   HDSM_Entry_Info_t  HDSETEntryInfo;
   HDSM_Entry_Info_t *HDSETEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a data event handler registered.                 */
            if(ConnectionType == sctAudioGateway)
               HDSETEntryInfoPtr = HDSETEntryInfoList_AG_Data;
            else
               HDSETEntryInfoPtr = HDSETEntryInfoList_HS_Data;

            if(!HDSETEntryInfoPtr)
            {
               /* First, register the handler locally.                  */
               BTPS_MemInitialize(&HDSETEntryInfo, 0, sizeof(HDSM_Entry_Info_t));

               HDSETEntryInfo.CallbackID        = GetNextCallbackID();
               HDSETEntryInfo.ClientID          = MSG_GetServerAddressID();
               HDSETEntryInfo.EventCallback     = CallbackFunction;
               HDSETEntryInfo.CallbackParameter = CallbackParameter;
               HDSETEntryInfo.Flags             = HDSET_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               if((HDSETEntryInfoPtr = AddHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Data:&HDSETEntryInfoList_HS_Data, &HDSETEntryInfo)) != NULL)
               {
                  /* Data handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  ret_val = HDSETEntryInfo.CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HEADSET_DATA_HANDLER_ALREADY_REGISTERED;

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
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager data event    */
   /* callback (registered via a successful call to the                 */
   /* HDSM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Headset Manager data event callback ID       */
   /* (return value from HDSM_Register_Data_Event_Callback() function). */
void BTPSAPI HDSM_Un_Register_Data_Event_Callback(unsigned int HeadsetManagerDataCallbackID)
{
   HDSM_Entry_Info_t *HDSETEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if(HeadsetManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Delete the local handler.                                */
            if((HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_AG_Data, HeadsetManagerDataCallbackID)) == NULL)
               HDSETEntryInfo = DeleteHDSETEntryInfoEntry(&HDSETEntryInfoList_HS_Data, HeadsetManagerDataCallbackID);

            if(HDSETEntryInfo)
            {
               /* All finished with the entry, delete it.               */
               FreeHDSETEntryInfoEntryMemory(HDSETEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Callback ID that is returned from a        */
   /* successful call to HDSM_Register_Event_Callback().  The second    */
   /* parameter is the local connection type of the SCO connection.  The*/
   /* third parameter is the address of the remote device of the SCO    */
   /* connection.  The fourth parameter is a pointer to the location to */
   /* store the SCO Handle. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int BTPSAPI HDSM_Query_SCO_Connection_Handle(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (SCOHandle))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHDSETEntryInfoEntry((ConnectionType == sctAudioGateway)?&HDSETEntryInfoList_AG_Control:&HDSETEntryInfoList_HS_Control, HeadsetManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL) && (ConnectionEntry->ConnectionState == csConnected) && (ConnectionEntry->SCOHandle))
                  {
                     *SCOHandle = ConnectionEntry->SCOHandle;
                     ret_val    = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
      {
         if(HeadsetManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

