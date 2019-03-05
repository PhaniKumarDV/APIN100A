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
typedef struct _tagHFRE_Entry_Info_t
{
   unsigned int                  CallbackID;
   unsigned int                  ClientID;
   unsigned int                  ConnectionStatus;
   BD_ADDR_t                     ConnectionBD_ADDR;
   Event_t                       ConnectionEvent;
   unsigned long                 Flags;
   HFRM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagHFRE_Entry_Info_t *NextHFREEntryInfoPtr;
} HFRE_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HFRE_Entry_Info_t structure to denote various state information.  */
#define HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY          0x40000000
#define HFRE_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY     0x80000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   HFRM_Event_Callback_t  EventCallback;
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
   HFRM_Connection_Type_t         ConnectionType;
   BD_ADDR_t                      BD_ADDR;
   Boolean_t                      Server;
   unsigned int                   HFREID;
   unsigned int                   CloseTimerID;
   unsigned int                   CloseTimerCount;
   Connection_State_t             ConnectionState;
   unsigned int                   ServerPort;
   unsigned long                  ConnectionFlags;
   Word_t                         SCOHandle;
   Boolean_t                      OutgoingSCOAttempt;
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

   /* Variables which hold a pointer to the first element in the Hands  */
   /* Free entry information list (which holds all callbacks tracked by */
   /* this module).                                                     */
static HFRE_Entry_Info_t *HFREEntryInfoList_AG;
static HFRE_Entry_Info_t *HFREEntryInfoList_HF;

   /* Variables which hold a pointer to the first element in the Hands  */
   /* Free control entry information list (which holds all callbacks    */
   /* tracked by this module).                                          */
static HFRE_Entry_Info_t *HFREEntryInfoList_AG_Control;
static HFRE_Entry_Info_t *HFREEntryInfoList_HF_Control;

   /* Variables which hold a pointer to the first element in the Hands  */
   /* Free data entry information list (which holds all callbacks       */
   /* tracked by this module).                                          */
static HFRE_Entry_Info_t *HFREEntryInfoList_AG_Data;
static HFRE_Entry_Info_t *HFREEntryInfoList_HF_Data;

   /* Variables which hold the current state of the respective profile  */
   /* roles.                                                            */
static Boolean_t                  AudioGatewaySupported;
static HFRM_Initialization_Data_t AudioGatewayInitializationInfo;

static Boolean_t                  HandsFreeSupported;
static HFRM_Initialization_Data_t HandsFreeInitializationInfo;

static Boolean_t                  WBS_Support;

   /* Variable which notes if arbitrary commands have been enabled for  */
   /* this module.                                                      */
static Boolean_t ArbitraryCommandsEnabled;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HFRE_Entry_Info_t *AddHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, HFRE_Entry_Info_t *EntryToAdd);
static HFRE_Entry_Info_t *SearchHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, unsigned int CallbackID);
static HFRE_Entry_Info_t *DeleteHFREEntryInfoEntry(HFRE_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHFREEntryInfoEntryMemory(HFRE_Entry_Info_t *EntryToFree);
static void FreeHFREEntryInfoList(HFRE_Entry_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HFRM_Connection_Type_t ConnectionType);
static Connection_Entry_t *SearchConnectionEntryHFREID(Connection_Entry_t **ListHead, unsigned int HFREID);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HFRM_Connection_Type_t ConnectionType);
static Connection_Entry_t *DeleteConnectionEntryHFREID(Connection_Entry_t **ListHead, unsigned int HFREID);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static void DispatchHFREEvent(Boolean_t ControlOnly, HFRM_Connection_Type_t ConnectionType, HFRM_Event_Data_t *HFRMEventData, BTPM_Message_t *Message);
static void DispatchHFREAudioDataEvent(HFRM_Connection_Type_t ConnectionType, HFRM_Event_Data_t *HFRMEventData, BTPM_Message_t *Message);

static void ProcessConnectionResponseMessage(HFRM_Connection_Request_Response_Request_t *Message);
static void ProcessConnectRemoteDeviceMessage(HFRM_Connect_Remote_Device_Request_t *Message);
static void ProcessDisconnectDeviceMessage(HFRM_Disconnect_Device_Request_t *Message);
static void ProcessQueryConnectedDevicesMessage(HFRM_Query_Connected_Devices_Request_t *Message);
static void ProcessRegisterHandsFreeEventsMessage(HFRM_Register_Hands_Free_Events_Request_t *Message);
static void ProcessUnRegisterHandsFreeEventsMessage(HFRM_Un_Register_Hands_Free_Events_Request_t *Message);
static void ProcessRegisterHandsFreeDataMessage(HFRM_Register_Hands_Free_Data_Events_Request_t *Message);
static void ProcessUnRegisterHandsFreeDataMessage(HFRM_Un_Register_Hands_Free_Data_Events_Request_t *Message);
static void ProcessSetupAudioConnectionMessage(HFRM_Setup_Audio_Connection_Request_t *Message);
static void ProcessReleaseAudioConnectionMessage(HFRM_Release_Audio_Connection_Request_t *Message);
static void ProcessSendAudioDataMessage(HFRM_Send_Audio_Data_Request_t *Message);
static void ProcessQueryConfigurationMessage(HFRM_Query_Current_Configuration_Request_t *Message);
static void ProcessChangeIncomingConnectionFlagsMessage(HFRM_Change_Incoming_Connection_Flags_Request_t *Message);
static void ProcessDisableEchoNoiseCancellationMessage(HFRM_Disable_Echo_Noise_Cancellation_Request_t *Message);
static void ProcessSetVoiceRecognitionActivationMessage(HFRM_Set_Voice_Recognition_Activation_Request_t *Message);
static void ProcessSetSpeakerGainMessage(HFRM_Set_Speaker_Gain_Request_t *Message);
static void ProcessSetMicrophoneGainMessage(HFRM_Set_Microphone_Gain_Request_t *Message);
static void ProcessQueryControlIndicatorStatusMessage(HFRM_Query_Control_Indicator_Status_Request_t *Message);
static void ProcessEnableIndicatorNotificationMessage(HFRM_Enable_Indicator_Event_Notification_Request_t *Message);
static void ProcessQueryCallHoldMultipartySupportMessage(HFRM_Query_Call_Holding_Multiparty_Request_t *Message);
static void ProcessSendCallHoldingMultipartySelectionMessage(HFRM_Send_Call_Holding_Multiparty_Selection_Request_t *Message);
static void ProcessEnableCallWaitingNotificationMessage(HFRM_Enable_Call_Waiting_Notification_Request_t *Message);
static void ProcessEnableCallLineIdentificationNotificationMessage(HFRM_Enable_Call_Line_Identification_Notification_Request_t *Message);
static void ProcessDialPhoneNumberMessage(HFRM_Dial_Phone_Number_Request_t *Message);
static void ProcessDialPhoneNumberFromMemoryMessage(HFRM_Dial_Phone_Number_From_Memory_Request_t *Message);
static void ProcessReDialLastPhoneNumberMessage(HFRM_Re_Dial_Last_Phone_Number_Request_t *Message);
static void ProcessAnswerIncomingCallMessage(HFRM_Answer_Incoming_Call_Request_t *Message);
static void ProcessTransmitDTMFCodeMessage(HFRM_Transmit_DTMF_Code_Request_t *Message);
static void ProcessVoiceTagRequestMessage(HFRM_Voice_Tag_Request_Request_t *Message);
static void ProcessHangUpCallMessage(HFRM_Hang_Up_Call_Request_t *Message);
static void ProcessQueryCurrentCallsListMessage(HFRM_Query_Current_Calls_List_Request_t *Message);
static void ProcessSetNetworkOperatorSelectionFormatMessage(HFRM_Set_Network_Operator_Selection_Format_Request_t *Message);
static void ProcessQueryNetworkOperatorSelectionMessage(HFRM_Query_Network_Operator_Selection_Request_t *Message);
static void ProcessEnableExtendedErrorResultMessage(HFRM_Enable_Extended_Error_Result_Request_t *Message);
static void ProcessQuerySubscriberNumberInformationMessage(HFRM_Query_Subscriber_Number_Information_Request_t *Message);
static void ProcessQueryResponseHoldStatusMessage(HFRM_Query_Response_Hold_Status_Request_t *Message);
static void ProcessSetIncomingCallStateMessage(HFRM_Set_Incoming_Call_State_Request_t *Message);
static void ProcessSendArbitraryCommandMessage(HFRM_Send_Arbitrary_Command_Request_t *Message);
static void ProcessSendAvailableCodecListMessage(HFRM_Send_Available_Codec_List_Request_t *Message);
static void ProcessUpdateControlIndicatorStatusMessage(HFRM_Update_Control_Indicator_Status_Request_t *Message);
static void ProcessUpdateControlIndicatorStatusByNameMessage(HFRM_Update_Control_Indicator_Status_By_Name_Request_t *Message);
static void ProcessSendCallWaitingNotificationMessage(HFRM_Send_Call_Waiting_Notification_Request_t *Message);
static void ProcessSendCallLineIdentificationMessage(HFRM_Send_Call_Line_Identification_Notification_Request_t *Message);
static void ProcessRingIndicationMessage(HFRM_Ring_Indication_Request_t *Message);
static void ProcessEnableInBandRingToneSettingMessage(HFRM_Enable_In_Band_Ring_Tone_Setting_Request_t *Message);
static void ProcessVoiceTagResponseMessage(HFRM_Voice_Tag_Response_Request_t *Message);
static void ProcessSendCurrentCallsListMessage_v1(HFRM_Send_Current_Calls_List_Request_v1_t *Message);
static void ProcessSendCurrentCallsListMessage_v2(HFRM_Send_Current_Calls_List_Request_v2_t *Message);
static void ProcessSendExtendedErrorResultMessage(HFRM_Send_Extended_Error_Result_Request_t *Message);
static void ProcessSendNetworkOperatorSelectionMessage(HFRM_Send_Network_Operator_Selection_Request_t *Message);
static void ProcessSendIncomingCallStateMessage(HFRM_Send_Incoming_Call_State_Request_t *Message);
static void ProcessSendSubscriberNumberInformationMessage(HFRM_Send_Subscriber_Number_Information_Request_t *Message);
static void ProcessSendTerminatingResponseMessage(HFRM_Send_Terminating_Response_Request_t *Message);
static void ProcessEnableArbitraryCommandProcessingMessage(HFRM_Enable_Arbitrary_Command_Processing_Request_t *Message);
static void ProcessSendArbitraryResponseMessage(HFRM_Send_Arbitrary_Response_Request_t *Message);
static void ProcessQuerySCOConnectionHandleMessage(HFRM_Query_SCO_Connection_Handle_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessOpenRequestIndicationEvent(HFRE_Open_Port_Request_Indication_Data_t *OpenPortRequestIndicationData);
static void ProcessOpenIndicationEvent(HFRE_Open_Port_Indication_Data_t *OpenPortIndicationData);
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HFRE_Open_Port_Confirmation_Data_t *OpenPortConfirmationData);
static void ProcessCloseIndicationEvent(HFRE_Close_Port_Indication_Data_t *ClosePortIndicationData);
static void ProcessServiceLevelIndicationEvent(HFRE_Open_Service_Level_Connection_Indication_Data_t *ServiceLevelConnectionIndicationData);
static void ProcessControlIndicatorStatusIndicationEvent(HFRE_Control_Indicator_Status_Indication_Data_t *ControlIndicatorStatusIndicationData);
static void ProcessControlIndicatorStatusConfirmationEvent(HFRE_Control_Indicator_Status_Confirmation_Data_t *ControlIndicatorStatusConfirmationData);
static void ProcessCallHoldMultipartySupportConfirmationEvent(HFRE_Call_Hold_Multiparty_Support_Confirmation_Data_t *CallHoldMultipartySupportConfirmationData);
static void ProcessCallHoldMultipartySelectionIndicationEvent(HFRE_Call_Hold_Multiparty_Selection_Indication_Data_t *CallHoldMultipartySelectionIndicationData);
static void ProcessCallWaitingNotificationActivationIndicationEvent(HFRE_Call_Waiting_Notification_Activation_Indication_Data_t *CallWaitingNotificationActivationIndicationData);
static void ProcessCallWaitingNotificationIndicationEvent(HFRE_Call_Waiting_Notification_Indication_Data_t *CallWaitingNotificationIndicationData);
static void ProcessCallLineIdentificationNotificationActivationIndicationEvent(HFRE_Call_Line_Identification_Notification_Activation_Indication_Data_t *CallLineIdentificationNotificationActivationIndicationData);
static void ProcessCallLineIdentificationNotificationIndicationEvent(HFRE_Call_Line_Identification_Notification_Indication_Data_t *CallLineIdentificationNotificationIndicationData);
static void ProcessDisableSoundEnhancementIndicationEvent(HFRE_Disable_Sound_Enhancement_Indication_Data_t *DisableSoundEnhancementIndicationData);
static void ProcessDialPhoneNumberIndicationEvent(HFRE_Dial_Phone_Number_Indication_Data_t *DialPhoneNumberIndicationData);
static void ProcessDialPhoneNumberFromMemoryIndicationEvent(HFRE_Dial_Phone_Number_From_Memory_Indication_Data_t *DialPhoneNumberFromMemoryIndicationData);
static void ProcessReDialLastPhoneNumberIndicationEvent(HFRE_ReDial_Last_Phone_Number_Indication_Data_t *ReDialLastPhoneNumberIndicationData);
static void ProcessRingIndicationEvent(HFRE_Ring_Indication_Data_t *RingIndicationData);
static void ProcessGenerateDTMFToneIndicationEvent(HFRE_Generate_DTMF_Tone_Indication_Data_t *GenerateDTMFToneIndicationData);
static void ProcessAnswerCallIndicationEvent(HFRE_Answer_Call_Indication_Data_t *AnswerCallIndicationData);
static void ProcessInBandRingToneSettingIndicationEvent(HFRE_InBand_Ring_Tone_Setting_Indication_Data_t *InBandRingToneSettingIndicationData);
static void ProcessVoiceRecognitionNotificationIndicationEvent(HFRE_Voice_Recognition_Notification_Indication_Data_t *VoiceRecognitionNotificationIndicationData);
static void ProcessSpeakerGainIndicationEvent(HFRE_Speaker_Gain_Indication_Data_t *SpeakerGainIndicationData);
static void ProcessMicrophoneGainIndicationEvent(HFRE_Microphone_Gain_Indication_Data_t *MicrophoneGainIndicationData);
static void ProcessVoiceTagRequestIndicationEvent(HFRE_Voice_Tag_Request_Indication_Data_t *VoiceTagRequestIndicationData);
static void ProcessVoiceTagRequestConfirmationEvent(HFRE_Voice_Tag_Request_Confirmation_Data_t *VoiceTagRequestConfirmationData);
static void ProcessHangUpIndicationEvent(HFRE_Hang_Up_Indication_Data_t *HangUpIndicationData);
static void ProcessAudioConnectionIndicationEvent(HFRE_Audio_Connection_Indication_Data_t *AudioConnectionIndicationData);
static void ProcessAudioDisconnectionIndicationEvent(HFRE_Audio_Disconnection_Indication_Data_t *AudioDisconnectionIndicationData);
static void ProcessAudioDataIndicationEvent(HFRE_Audio_Data_Indication_Data_t *AudioDataIndicationData);
static void ProcessCurrentCallsListIndicationEvent(HFRE_Current_Calls_List_Indication_Data_t *CurrentCallsListIndicationData);
static void ProcessCurrentCallsListConfirmationEvent(HFRE_Current_Calls_List_Confirmation_Data_t *CurrentCallsListConfirmationData);
static void ProcessNetworkOperatorSelectionFormatIndicationEvent(HFRE_Network_Operator_Selection_Format_Indication_Data_t *NetworkOperatorSelectionFormatIndicationData);
static void ProcessNetworkOperatorSelectionIndicationEvent(HFRE_Network_Operator_Selection_Indication_Data_t *NetworkOperatorSelectionIndicationData);
static void ProcessNetworkOperatorSelectionConfirmationEvent(HFRE_Network_Operator_Selection_Confirmation_Data_t *NetworkOperatorSelectionConfirmationData);
static void ProcessExtendedErrorResultActivationIndicationEvent(HFRE_Extended_Error_Result_Activation_Indication_Data_t *ExtendedErrorResultActivationIndicationData);
static void ProcessSubscriberNumberInformationIndicationEvent(HFRE_Subscriber_Number_Information_Indication_Data_t *SubscriberNumberInformationIndicationData);
static void ProcessSubscriberNumberInformationConfirmationEvent(HFRE_Subscriber_Number_Information_Confirmation_Data_t *SubscriberNumberInformationConfirmationData);
static void ProcessResponseHoldStatusIndicationEvent(HFRE_Response_Hold_Status_Indication_Data_t *ResponseHoldStatusIndicationData);
static void ProcessResponseHoldStatusConfirmationEvent(HFRE_Response_Hold_Status_Confirmation_Data_t *ResponseHoldStatusConfirmationData);
static void ProcessIncomingCallStateIndicationEvent(HFRE_Incoming_Call_State_Indication_Data_t *IncomingCallStateIndicationData);
static void ProcessIncomingCallStateConfirmationEvent(HFRE_Incoming_Call_State_Confirmation_Data_t *IncomingCallStateConfirmationData);
static void ProcessCommandResultEvent(HFRE_Command_Result_Data_t *CommandResultData);
static void ProcessArbitraryCommandIndicationEvent(HFRE_Arbitrary_Command_Indication_Data_t *ArbitraryCommandIndicationData);
static void ProcessArbitraryResponseIndicationEvent(HFRE_Arbitrary_Response_Indication_Data_t *ArbitraryResponseIndicationData);
static void ProcessCodecSelectIndicationEvent(HFRE_Codec_Select_Indication_t *CodecSelectIndication);
static void ProcessCodecSelectConfirmationEvent(HFRE_Codec_Select_Confirmation_t *CodecSelectConfirmation);
static void ProcessCodecConnectionSetupIndicationEvent(HFRE_Codec_Connection_Setup_Indication_Data_t *CodecConnectionSetupIndicationData);
static void ProcessAvailableCodecListIndicationEvent(HFRE_Available_Codec_List_Indication_Data_t *AvailableCodecListIndicationData);

static void ProcessHandsFreeEvent(HFRM_Hands_Free_Event_Data_t *HandsFreeEventData);

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status);

static void BTPSAPI BTPMDispatchCallback_HFRM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_HFRE(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter);

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified connection entry    */
   /* list for the specified connection entry based on the specified    */
   /* Bluetooth device address and connection type.  This function      */
   /* returns NULL if either the connection entry list head is invalid, */
   /* the Bluetooth device address is invalid, or the specified entry   */
   /* was NOT present in the list.                                      */
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HFRM_Connection_Type_t ConnectionType)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:%p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified connection entry    */
   /* list for the specified connection entry based on the specified    */
   /* Hands Free Port ID.  This function returns NULL if either the     */
   /* connection entry list head is invalid, the Hands Free Port ID is  */
   /* invalid, or the specified entry was NOT present in the list.      */
static Connection_Entry_t *SearchConnectionEntryHFREID(Connection_Entry_t **ListHead, unsigned int HFREID)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Hands Free Port ID to search for     */
   /* appear to be valid.                                               */
   if((ListHead) && (HFREID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (HFREID != FoundEntry->HFREID))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, HFRM_Connection_Type_t ConnectionType)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified connection entry    */
   /* list for the connection entry with the specified Hands Free Port  */
   /* ID and removes it from the list.  This function returns NULL if   */
   /* either the connection entry list head is invalid, the Hands Free  */
   /* Port ID is invalid, or the specified entry was NOT present in the */
   /* list.  The entry returned will have the next entry field set to   */
   /* NULL, and the caller is responsible for deleting the memory       */
   /* associated with this entry by calling FreeConnectionEntryMemory().*/
static Connection_Entry_t *DeleteConnectionEntryHFREID(Connection_Entry_t **ListHead, unsigned int HFREID)
{
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Hands Free Port ID to search for     */
   /* appear to be valid.                                               */
   if((ListHead) && (HFREID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->HFREID != HFREID))
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified connection information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified connection information list.  Upon return*/
   /* of this function, the head pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Hands Free event to every registered Hands */
   /* Free Event Callback.                                              */
   /* * NOTE * This function should be called with the Hands Free       */
   /*          Manager Lock held.  Upon exit from this function it will */
   /*          free the Hands Free Manager Lock.                        */
static void DispatchHFREEvent(Boolean_t ControlOnly, HFRM_Connection_Type_t ConnectionType, HFRM_Event_Data_t *HFRMEventData, BTPM_Message_t *Message)
{
   unsigned int       Index;
   unsigned int       Index1;
   unsigned int       ServerID;
   unsigned int       NumberCallbacks;
   Callback_Info_t    CallbackInfoArray[16];
   Callback_Info_t   *CallbackInfoArrayPtr;
   HFRE_Entry_Info_t *HFREEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HFREEntryInfoList_AG) || (HFREEntryInfoList_HF) || (HFREEntryInfoList_AG_Control) || (HFREEntryInfoList_HF_Control) || (HFREEntryInfoList_AG_Data) || (HFREEntryInfoList_HF_Data)) && (HFRMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      ServerID         = MSG_GetServerAddressID();
      NumberCallbacks  = 0;

      /* First, add the default event handlers.                         */
      if(!ControlOnly)
      {
         if(ConnectionType == hctAudioGateway)
            HFREEntryInfo = HFREEntryInfoList_AG;
         else
            HFREEntryInfo = HFREEntryInfoList_HF;

         while(HFREEntryInfo)
         {
            if(((HFREEntryInfo->EventCallback) || (HFREEntryInfo->ClientID != ServerID)) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
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
         if(((HFREEntryInfo->EventCallback) || (HFREEntryInfo->ClientID != ServerID)) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
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
            if(((HFREEntryInfo->EventCallback) || (HFREEntryInfo->ClientID != ServerID)) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
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
               if(ConnectionType == hctAudioGateway)
                  HFREEntryInfo = HFREEntryInfoList_AG;
               else
                  HFREEntryInfo = HFREEntryInfoList_HF;

               while(HFREEntryInfo)
               {
                  if(((HFREEntryInfo->EventCallback) || (HFREEntryInfo->ClientID != ServerID)) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HFREEntryInfo->ClientID;
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
               if(((HFREEntryInfo->EventCallback) || (HFREEntryInfo->ClientID != ServerID)) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HFREEntryInfo->ClientID;
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
                  if(((HFREEntryInfo->EventCallback) || (HFREEntryInfo->ClientID != ServerID)) && (HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HFREEntryInfo->ClientID;
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HFREEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HFREEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
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
               /*          for Hands Free events and Data Events.       */
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(HFRMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Hands Free audio data event to the         */
   /* registered Hands Free Data Event Callback.                        */
   /* * NOTE * This function should be called with the Hands Free       */
   /*          Manager Lock held.  Upon exit from this function it will */
   /*          free the Hands Free Manager Lock.                        */
static void DispatchHFREAudioDataEvent(HFRM_Connection_Type_t ConnectionType, HFRM_Event_Data_t *HFRMEventData, BTPM_Message_t *Message)
{
   void                  *CallbackParameter;
   HFRE_Entry_Info_t     *HFREEntryInfo;
   HFRM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((HFRMEventData) && (Message))
   {
      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if(ConnectionType == hctAudioGateway)
         HFREEntryInfo = HFREEntryInfoList_AG_Data;
      else
         HFREEntryInfo = HFREEntryInfoList_HF_Data;

      if(HFREEntryInfo)
      {
         /* Format up the Data.                                         */
         if(HFREEntryInfo->ClientID != MSG_GetServerAddressID())
         {
            /* Dispatch a Message Callback.                             */

            /* Note the Client (destination) address.                   */
            Message->MessageHeader.AddressID                                     = HFREEntryInfo->ClientID;

            /* Note the Hands Free Manager data event callback ID.      */
            /* * NOTE * All messages have this member in the same       */
            /*          location.                                       */
            ((HFRM_Audio_Data_Received_Message_t *)Message)->DataEventsHandlerID = HFREEntryInfo->CallbackID;

            /* All that is left to do is to dispatch the Event.         */
            MSG_SendMessage(Message);
         }
         else
         {
            /* Dispatch local event callback.                           */
            if(HFREEntryInfo->EventCallback)
            {
               /* Note the Hands Manager data event callback ID.        */
               /* * NOTE * All events have this member in the same      */
               /*          location.                                    */
               HFRMEventData->EventData.AudioDataEventData.DataEventsHandlerID = HFREEntryInfo->CallbackID;

               /* Note the Callback Information.                        */
               EventCallback                                                   = HFREEntryInfo->EventCallback;
               CallbackParameter                                               = HFREEntryInfo->CallbackParameter;

               /* Release the Lock because we have already noted the    */
               /* callback information.                                 */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  (*EventCallback)(HFRMEventData, CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified connect response   */
   /* message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the message   */
   /* before calling this function.                                     */
static void ProcessConnectionResponseMessage(HFRM_Connection_Request_Response_Request_t *Message)
{
   int                                          Result;
   Boolean_t                                    Authenticate;
   Boolean_t                                    Encrypt;
   unsigned long                                IncomingConnectionFlags;
   Connection_Entry_t                          *ConnectionEntry;
   HFRM_Connection_Request_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Message->Accept));

            /* If the caller has accepted the request then we need to   */
            /* process it differently.                                  */
            if(Message->Accept)
            {
               /* Determine the incoming connection flags based on the  */
               /* connection type.                                      */
               if(Message->ConnectionType == hctAudioGateway)
                  IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
               else
                  IncomingConnectionFlags = HandsFreeInitializationInfo.IncomingConnectionFlags;

               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
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
                  Result = _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, TRUE);

                  if(Result)
                  {
                     _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);

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
                     _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);

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
               Result = _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);

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

      ResponseMessage.MessageHeader.MessageLength  = HFRM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified connect remote     */
   /* device message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessConnectRemoteDeviceMessage(HFRM_Connect_Remote_Device_Request_t *Message)
{
   int                                    Result;
   BD_ADDR_t                              NULL_BD_ADDR;
   HFRE_Entry_Info_t                      HFREEntryInfo;
   HFRE_Entry_Info_t                     *HFREEntryInfoPtr;
   Connection_Entry_t                     ConnectionEntry;
   Connection_Entry_t                    *ConnectionEntryPtr;
   HFRM_Connect_Remote_Device_Response_t  ResponseMessage;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
         if((!COMPARE_BD_ADDR(Message->RemoteDeviceAddress, NULL_BD_ADDR)) && (((Message->ConnectionType == hctAudioGateway) && (AudioGatewaySupported)) || ((Message->ConnectionType == hctHandsFree) && (HandsFreeSupported))))
         {
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) == NULL)
            {
               /* Entry is not present, go ahead and create a new entry.*/
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

               ConnectionEntry.BD_ADDR         = Message->RemoteDeviceAddress;
               ConnectionEntry.ConnectionType  = Message->ConnectionType;
               ConnectionEntry.HFREID          = GetNextCallbackID() | 0x80000000;
               ConnectionEntry.Server          = FALSE;
               ConnectionEntry.ServerPort      = Message->RemoteServerPort;
               ConnectionEntry.ConnectionState = csIdle;
               ConnectionEntry.ConnectionFlags = Message->ConnectionFlags;

               if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
               {
                  /* Attempt to add an entry into the Hands Free entry  */
                  /* list.                                              */
                  BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

                  HFREEntryInfo.CallbackID        = GetNextCallbackID();
                  HFREEntryInfo.ClientID          = Message->MessageHeader.AddressID;
                  HFREEntryInfo.ConnectionBD_ADDR = Message->RemoteDeviceAddress;

                  if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((Message->ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, &HFREEntryInfo)) != NULL)
                  {
                     /* First, let's wait for the Port to disconnect.   */
                     if(!SPPM_WaitForPortDisconnection(Message->RemoteServerPort, FALSE, Message->RemoteDeviceAddress, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device      */
                        if(Message->ConnectionFlags & HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                           ConnectionEntryPtr->ConnectionState = csEncrypting;
                        else
                        {
                           if(Message->ConnectionFlags & HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
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

                              if((Result = _HFRM_Connect_Remote_Device(Message->ConnectionType, Message->RemoteDeviceAddress, ConnectionEntryPtr->ServerPort)) <= 0)
                              {
                                 Result = BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE;

                                 /* Error opening device, go ahead and  */
                                 /* delete the entry that was added.    */
                                 if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((Message->ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, HFREEntryInfoPtr->CallbackID)) != NULL)
                                    FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);
                              }
                              else
                              {
                                 /* Note the Hands Free Port ID.        */
                                 ConnectionEntryPtr->HFREID          = (unsigned int)Result;

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
                           Result = TMR_StartTimer((void *)ConnectionEntry.HFREID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

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
                           Result = BTPM_ERROR_CODE_HANDS_FREE_IS_STILL_CONNECTED;
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
                  Result = BTPM_ERROR_CODE_HANDS_FREE_ALREADY_CONNECTED;
               else
                  Result = BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_IN_PROGRESS;
            }
         }
         else
         {
            if(COMPARE_BD_ADDR(Message->RemoteDeviceAddress, NULL_BD_ADDR))
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified disconnect device  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the message   */
   /* before calling this function.                                     */
static void ProcessDisconnectDeviceMessage(HFRM_Disconnect_Device_Request_t *Message)
{
   int                                Result;
   Boolean_t                          Server;
   Boolean_t                          PerformDisconnect;
   unsigned int                       ServerPort;
   Connection_Entry_t                *ConnectionEntry;
   HFRE_Close_Port_Indication_Data_t  ClosePortIndicationData;
   HFRM_Disconnect_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Go ahead and process the message request.                   */
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to disconnect Device\n"));

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
               Result = _HFRM_Disconnect_Device(ConnectionEntry->HFREID);
            }
            else
               Result = 0;

            /* Make sure we are no longer tracking SCO connections for  */
            /* the specified device.                                    */
            if((!Result) && (!COMPARE_NULL_BD_ADDR(ConnectionEntry->BD_ADDR)))
               SCOM_DeleteConnectionFromIgnoreList(ConnectionEntry->BD_ADDR);
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_IN_PROGRESS;

         ResponseMessage.MessageHeader                = Message->MessageHeader;

         ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

         ResponseMessage.MessageHeader.MessageLength  = HFRM_DISCONNECT_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

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
            ClosePortIndicationData.HFREPortID      = ConnectionEntry->HFREID;
            ClosePortIndicationData.PortCloseStatus = HFRE_CLOSE_PORT_STATUS_SUCCESS;

            ProcessCloseIndicationEvent(&ClosePortIndicationData);

            /* If successfully closed and this was a client port, go    */
            /* ahead and wait for a little bit for the disconnection.   */
            if(ServerPort)
               SPPM_WaitForPortDisconnection(ServerPort, Server, Message->RemoteDeviceAddress, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* connected devices message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessQueryConnectedDevicesMessage(HFRM_Query_Connected_Devices_Request_t *Message)
{
   unsigned int                             NumberConnected;
   Connection_Entry_t                      *ConnectionEntry;
   HFRM_Query_Connected_Devices_Response_t  ErrorResponseMessage;
   HFRM_Query_Connected_Devices_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Initialize the Error Response Message.                         */
      ErrorResponseMessage.MessageHeader                = Message->MessageHeader;

      ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ErrorResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

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
         if((ResponseMessage = (HFRM_Query_Connected_Devices_Response_t *)BTPS_AllocateMemory(HFRM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(NumberConnected))) != NULL)
         {
            /* Memory allocated, now let's build the response message.  */
            /* * NOTE * Error Response has initialized all values to    */
            /*          known values (i.e. zero devices and success).   */
            BTPS_MemCopy(ResponseMessage, &ErrorResponseMessage, HFRM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0));

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
            ResponseMessage->MessageHeader.MessageLength  = HFRM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(ResponseMessage->NumberDevicesConnected) - BTPM_MESSAGE_HEADER_SIZE;

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free register*/
   /* Hands Free events message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessRegisterHandsFreeEventsMessage(HFRM_Register_Hands_Free_Events_Request_t *Message)
{
   int                                         Result;
   HFRE_Entry_Info_t                           HFREEntryInfo;
   HFRE_Entry_Info_t                          *HFREEntryInfoPtr;
   HFRM_Register_Hands_Free_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Go ahead and initialize the Response.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_REGISTER_HANDS_FREE_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Before proceding any further, make sure that there is not      */
      /* already a control event handler for the specified connection   */
      /* type (if control handler was specified).                       */
      if(Message->ControlHandler)
      {
         if(Message->ConnectionType == hctAudioGateway)
            HFREEntryInfoPtr = HFREEntryInfoList_AG_Control;
         else
            HFREEntryInfoPtr = HFREEntryInfoList_HF_Control;
      }
      else
         HFREEntryInfoPtr = NULL;

      if(!HFREEntryInfoPtr)
      {
         /* First, register the handler locally.                        */
         BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

         HFREEntryInfo.CallbackID = GetNextCallbackID();
         HFREEntryInfo.ClientID   = Message->MessageHeader.AddressID;
         HFREEntryInfo.Flags      = HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         /* Note the connection type.                                   */
         if(Message->ConnectionType == hctAudioGateway)
            HFREEntryInfo.Flags |= HFRE_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

         /* Check to see if we need to register a control handler or a  */
         /* normal event handler.                                       */
         if(Message->ControlHandler)
         {
            /* Control handler, add it the correct list, and attempt to */
            /* register it with the server.                             */
            if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((Message->ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, &HFREEntryInfo)) != NULL)
            {
               Result = HFREEntryInfoPtr->CallbackID;
               /* Add the SDP Record.                                   */
               _HFRM_UpdateSDPRecord(Message->ConnectionType, TRUE);
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
         {
            if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((Message->ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, &HFREEntryInfo)) != NULL)
               Result = HFREEntryInfoPtr->CallbackID;
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
      }
      else
         Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_ALREADY_REGISTERED;

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free         */
   /* un-register Hands Free events message and responds to the message */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessUnRegisterHandsFreeEventsMessage(HFRM_Un_Register_Hands_Free_Events_Request_t *Message)
{
   int                                            Result;
   Boolean_t                                      ControlEvent = FALSE;
   HFRE_Entry_Info_t                             *HFREEntryInfo;
   Connection_Entry_t                            *ConnectionEntry;
   Connection_Entry_t                            *CloseConnectionEntry;
   HFRM_Connection_Type_t                         ConnectionType;
   HFRM_Un_Register_Hands_Free_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));
   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, determine if the client that is un-registering is the   */
      /* client that actually registered for the events.                */
      if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG, Message->EventsHandlerID)) == NULL)
      {
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->EventsHandlerID)) == NULL)
         {
            if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF, Message->EventsHandlerID)) == NULL)
               HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->EventsHandlerID);
         }
      }


      /* Go ahead and process the message request.                      */
      if((HFREEntryInfo) && (HFREEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         ConnectionType = hctAudioGateway;
         if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG, Message->EventsHandlerID)) == NULL)
         {
            if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->EventsHandlerID)) == NULL)
            {
               ConnectionType = hctHandsFree;
               if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF, Message->EventsHandlerID)) == NULL)
               {
                  HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->EventsHandlerID);
                  if(HFREEntryInfo)
                     ControlEvent  = TRUE;
               }
            }
            else
               ControlEvent = TRUE;
         }
      }
      else
         HFREEntryInfo = NULL;

      if(HFREEntryInfo)
      {
         /* Free the memory because we are finished with it.            */
         FreeHFREEntryInfoEntryMemory(HFREEntryInfo);

         if(ControlEvent)
         {
            /* Remove the SDP Record.                                   */
            _HFRM_UpdateSDPRecord(ConnectionType, FALSE);

            /* Check to see if there is an open connection.             */
            ConnectionEntry = ConnectionEntryList;
            while(ConnectionEntry)
            {
               /* Check to see if the AudioGateway control is being     */
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
                  _HFRM_Disconnect_Device(CloseConnectionEntry->HFREID);

                  if((CloseConnectionEntry = DeleteConnectionEntryHFREID(&ConnectionEntryList, CloseConnectionEntry->HFREID)) != NULL)
                     FreeConnectionEntryMemory(CloseConnectionEntry);
               }
            }
         }

         /* Flag success.                                               */
         Result = 0;
      }
      else
         Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_UN_REGISTER_HANDS_FREE_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free register*/
   /* Hands Free data message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessRegisterHandsFreeDataMessage(HFRM_Register_Hands_Free_Data_Events_Request_t *Message)
{
   int                                              Result;
   HFRE_Entry_Info_t                                HFREEntryInfo;
   HFRE_Entry_Info_t                               *HFREEntryInfoPtr;
   HFRM_Register_Hands_Free_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Go ahead and initialize the Response.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_REGISTER_HANDS_FREE_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Before proceding any further, make sure that there is not      */
      /* already a data event handler for the specified connection type.*/
      if(Message->ConnectionType == hctAudioGateway)
         HFREEntryInfoPtr = HFREEntryInfoList_AG_Data;
      else
         HFREEntryInfoPtr = HFREEntryInfoList_HF_Data;

      if(!HFREEntryInfoPtr)
      {
         /* First, register the handler locally.                        */
         BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

         HFREEntryInfo.CallbackID = GetNextCallbackID();
         HFREEntryInfo.ClientID   = Message->MessageHeader.AddressID;
         HFREEntryInfo.Flags      = HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         /* Note the connection type.                                   */
         if(Message->ConnectionType == hctAudioGateway)
            HFREEntryInfo.Flags |= HFRE_ENTRY_INFO_FLAGS_EVENT_AUDIO_GATEWAY_ENTRY;

         if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((Message->ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Data:&HFREEntryInfoList_HF_Data, &HFREEntryInfo)) != NULL)
            Result = HFREEntryInfoPtr->CallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_ALREADY_REGISTERED;

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free         */
   /* un-register Hands Free data message and responds to the message   */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessUnRegisterHandsFreeDataMessage(HFRM_Un_Register_Hands_Free_Data_Events_Request_t *Message)
{
   int                                                 Result;
   HFRE_Entry_Info_t                                  *HFREEntryInfo;
   HFRM_Un_Register_Hands_Free_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, determine if the client that is un-registering is the   */
      /* client that actually registered for the events.                */
      if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Data, Message->DataEventsHandlerID)) == NULL)
         HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Data, Message->DataEventsHandlerID);

      /* Go ahead and process the message request.                      */
      if((HFREEntryInfo) && (HFREEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG_Data, Message->DataEventsHandlerID)) == NULL)
            HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF_Data, Message->DataEventsHandlerID);
      }
      else
         HFREEntryInfo = NULL;

      if(HFREEntryInfo)
      {
         /* Free the memory because we are finished with it.            */
         FreeHFREEntryInfoEntryMemory(HFREEntryInfo);

         /* Flag success.                                               */
         Result = 0;
      }
      else
         Result = BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_UN_REGISTER_HANDS_FREE_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free set-up  */
   /* audio connection message and responds to the message accordingly. */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSetupAudioConnectionMessage(HFRM_Setup_Audio_Connection_Request_t *Message)
{
   int                                     Result;
   HFRE_Entry_Info_t                      *HFREEntryInfo;
   Connection_Entry_t                     *ConnectionEntry;
   HFRM_Setup_Audio_Connection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID);

         if(HFREEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set-up the audio connection.              */
               Result = _HFRM_Setup_Audio_Connection(ConnectionEntry->HFREID);

               if(!Result)
                  ConnectionEntry->OutgoingSCOAttempt = TRUE;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SETUP_AUDIO_CONNECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free release */
   /* audio connection message and responds to the message accordingly. */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessReleaseAudioConnectionMessage(HFRM_Release_Audio_Connection_Request_t *Message)
{
   int                                       Result;
   HFRE_Entry_Info_t                        *HFREEntryInfo;
   Connection_Entry_t                       *ConnectionEntry;
   HFRM_Release_Audio_Connection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID);

         if(HFREEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to release the audio connection.             */
               Result = _HFRM_Release_Audio_Connection(ConnectionEntry->HFREID);

               if(!Result)
                  ConnectionEntry->SCOHandle = 0;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_RELEASE_AUDIO_CONNECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* audio data message and responds to the message accordingly.  This */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessSendAudioDataMessage(HFRM_Send_Audio_Data_Request_t *Message)
{
   HFRE_Entry_Info_t  *HFREEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if((Message) && (Message->AudioDataLength))
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Data, Message->DataEventsHandlerID)) == NULL)
            HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Data, Message->DataEventsHandlerID);

         if(HFREEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the audio data.                      */
               _HFRM_Send_Audio_Data(ConnectionEntry->HFREID, Message->AudioDataLength, Message->AudioData);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* configuration message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessQueryConfigurationMessage(HFRM_Query_Current_Configuration_Request_t *Message)
{
   int                                          Result;
   unsigned int                                 Temp;
   unsigned int                                 NumberIndicators;
   HFRM_Query_Current_Configuration_Response_t  ErrorResponseMessage;
   HFRM_Query_Current_Configuration_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Initialize success.                                            */
      Result           = 0;
      NumberIndicators = 0;
      ResponseMessage  = &ErrorResponseMessage;

      BTPS_MemInitialize(&ErrorResponseMessage, 0, sizeof(ErrorResponseMessage));

      /* Check to see if Audio Gateway or Hands Free was specified.     */
      if(Message->ConnectionType == hctAudioGateway)
      {
         if(AudioGatewaySupported)
         {
            /* Determine if we need to allocate space for additional    */
            /* indicators (we have space for one by default).           */
            NumberIndicators = AudioGatewayInitializationInfo.NumberAdditionalIndicators;
            if(NumberIndicators > 1)
               ResponseMessage = (HFRM_Query_Current_Configuration_Response_t *)BTPS_AllocateMemory(HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(NumberIndicators));

            if(ResponseMessage)
            {
               BTPS_MemInitialize(ResponseMessage, 0, HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(NumberIndicators));

               /* Build the additional supported indicators list.       */
               /* * NOTE * We will take extra precautions to make sure  */
               /*          that the Indicator description is not too    */
               /*          large (we will truncate it if it is).        */
               while(ResponseMessage->TotalNumberAdditionalIndicators < NumberIndicators)
               {
                  ResponseMessage->AdditionalIndicatorList[ResponseMessage->TotalNumberAdditionalIndicators].ControlIndicatorType = AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].ControlIndicatorType;

                  BTPS_MemCopy(&(ResponseMessage->AdditionalIndicatorList[ResponseMessage->TotalNumberAdditionalIndicators].Control_Indicator_Data), &(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].Control_Indicator_Data), sizeof(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].Control_Indicator_Data));

                  if(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription)
                     Temp = BTPS_StringLength(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription);
                  else
                     Temp = 0;

                  if(Temp > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                     Temp = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

                  BTPS_MemCopy(ResponseMessage->AdditionalIndicatorList[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription, AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription, Temp);

                  ResponseMessage->TotalNumberAdditionalIndicators++;
               }
            }
            else
            {
               /* Return an error response.                             */
               Result           = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

               /* Point the response message pointer back at the error  */
               /* message that is declared on the stack.                */
               ResponseMessage  = &ErrorResponseMessage;
            }

            /* Copy the static information.                             */
            ResponseMessage->IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
            ResponseMessage->SupportedFeaturesMask   = AudioGatewayInitializationInfo.SupportedFeaturesMask;
            ResponseMessage->CallHoldingSupportMask  = AudioGatewayInitializationInfo.CallHoldingSupportMask;
            ResponseMessage->NetworkType             = AudioGatewayInitializationInfo.NetworkType;

            /* Check to see if WBS is supported.                        */
            if(WBS_Support)
               ResponseMessage->SupportedFeaturesMask |= HFRE_AG_CODEC_NEGOTIATION_SUPPORTED_BIT;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
      }
      else
      {
         if(HandsFreeSupported)
         {
            /* Determine if we need to allocate space for additional    */
            /* indicators (we have space for one by default).           */
            NumberIndicators = HandsFreeInitializationInfo.NumberAdditionalIndicators;
            if(NumberIndicators > 1)
               ResponseMessage = (HFRM_Query_Current_Configuration_Response_t *)BTPS_AllocateMemory(HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(NumberIndicators));

            if(ResponseMessage)
            {
               BTPS_MemInitialize(ResponseMessage, 0, HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(NumberIndicators));

               /* Build the additional supported indicators list.       */
               /* * NOTE * We will take extra precautions to make sure  */
               /*          that the Indicator description is not too    */
               /*          large (we will truncate it if it is).        */
               while(ResponseMessage->TotalNumberAdditionalIndicators < NumberIndicators)
               {
                  ResponseMessage->AdditionalIndicatorList[ResponseMessage->TotalNumberAdditionalIndicators].ControlIndicatorType = HandsFreeInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].ControlIndicatorType;

                  BTPS_MemCopy(&(ResponseMessage->AdditionalIndicatorList[ResponseMessage->TotalNumberAdditionalIndicators].Control_Indicator_Data), &(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].Control_Indicator_Data), sizeof(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].Control_Indicator_Data));

                  if(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription)
                     Temp = BTPS_StringLength(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription);
                  else
                     Temp = 0;

                  if(Temp > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                     Temp = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

                  BTPS_MemCopy(ResponseMessage->AdditionalIndicatorList[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription, HandsFreeInitializationInfo.AdditionalSupportedIndicators[ResponseMessage->TotalNumberAdditionalIndicators].IndicatorDescription, Temp);

                  ResponseMessage->TotalNumberAdditionalIndicators++;
               }
            }
            else
            {
               /* Return an error response.                             */
               Result           = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

               /* Point the response message pointer back at the error  */
               /* message that is declared on the stack.                */
               ResponseMessage  = &ErrorResponseMessage;
            }

            /* Copy the static information.                             */
            ResponseMessage->IncomingConnectionFlags = HandsFreeInitializationInfo.IncomingConnectionFlags;
            ResponseMessage->SupportedFeaturesMask   = HandsFreeInitializationInfo.SupportedFeaturesMask;
            ResponseMessage->CallHoldingSupportMask  = HandsFreeInitializationInfo.CallHoldingSupportMask;
            ResponseMessage->NetworkType             = HandsFreeInitializationInfo.NetworkType;

            /* Check to see if WBS is supported.                        */
            if(WBS_Support)
               ResponseMessage->SupportedFeaturesMask |= HFRE_HF_CODEC_NEGOTIATION_SUPPORTED_BIT;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
      }

      ResponseMessage->MessageHeader                = Message->MessageHeader;

      ResponseMessage->MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      if(!Result)
         ResponseMessage->MessageHeader.MessageLength = HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(NumberIndicators) - BTPM_MESSAGE_HEADER_SIZE;
      else
         ResponseMessage->MessageHeader.MessageLength = HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage->Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

      if(ResponseMessage != &ErrorResponseMessage)
         BTPS_FreeMemory(ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free change  */
   /* incoming connection flags message and responds to the message     */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessChangeIncomingConnectionFlagsMessage(HFRM_Change_Incoming_Connection_Flags_Request_t *Message)
{
   int                                              Result;
   HFRM_Change_Incoming_Connection_Flags_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Initialize success.                                            */
      Result = 0;

      /* Check to see if Audio Gateway or Hands Free was specified.     */
      if(Message->ConnectionType == hctAudioGateway)
      {
         if(AudioGatewaySupported)
            AudioGatewayInitializationInfo.IncomingConnectionFlags = Message->ConnectionFlags;
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
      }
      else
      {
         if(HandsFreeSupported)
            HandsFreeInitializationInfo.IncomingConnectionFlags = Message->ConnectionFlags;
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
      }

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free disable */
   /* echo/noise cancellation message and responds to the message       */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessDisableEchoNoiseCancellationMessage(HFRM_Disable_Echo_Noise_Cancellation_Request_t *Message)
{
   int                                              Result;
   HFRE_Entry_Info_t                               *HFREEntryInfo;
   Connection_Entry_t                              *ConnectionEntry;
   HFRM_Disable_Echo_Noise_Cancellation_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID);

         if(HFREEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to disable echo/noise cancellation.          */
               Result = _HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_DISABLE_ECHO_NOISE_CANCELLATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free set     */
   /* voice recognition activation message and responds to the message  */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSetVoiceRecognitionActivationMessage(HFRM_Set_Voice_Recognition_Activation_Request_t *Message)
{
   int                                               Result;
   HFRE_Entry_Info_t                                *HFREEntryInfo;
   Connection_Entry_t                               *ConnectionEntry;
   HFRM_Set_Voice_Recognition_Activation_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID);

         if(HFREEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set remote voice recognition activation.  */
               Result = _HFRM_Set_Remote_Voice_Recognition_Activation(ConnectionEntry->HFREID, Message->VoiceRecognitionActive);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SET_VOICE_RECOGNITION_ACTIVATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free set     */
   /* speaker gain message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSetSpeakerGainMessage(HFRM_Set_Speaker_Gain_Request_t *Message)
{
   int                               Result;
   HFRE_Entry_Info_t                *HFREEntryInfo;
   Connection_Entry_t               *ConnectionEntry;
   HFRM_Set_Speaker_Gain_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID);

         if(HFREEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set speaker gain.                         */
               Result = _HFRM_Set_Remote_Speaker_Gain(ConnectionEntry->HFREID, Message->SpeakerGain);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SET_SPEAKER_GAIN_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free set     */
   /* microphone gain message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSetMicrophoneGainMessage(HFRM_Set_Microphone_Gain_Request_t *Message)
{
   int                                  Result;
   HFRE_Entry_Info_t                   *HFREEntryInfo;
   Connection_Entry_t                  *ConnectionEntry;
   HFRM_Set_Microphone_Gain_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if((HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID)) == NULL)
            HFREEntryInfo = SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID);

         if(HFREEntryInfo)
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set microphone gain.                      */
               Result = _HFRM_Set_Remote_Microphone_Gain(ConnectionEntry->HFREID, Message->MicrophoneGain);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SET_MICROPHONE_GAIN_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the send select codec message from the pm client in       */
   /* either the hands free or audio gateway role. The audio gateway    */
   /* selects a preferred codec. The hands free device confirms the     */
   /* selection.                                                        */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendSelectCodecMessage(HFRM_Send_Select_Codec_Request_t *Message)
{
   int                                Result;
   Connection_Entry_t                *ConnectionEntry;
   HFRM_Send_Select_Codec_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry((Message->ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the codec list.                      */
               Result = _HFRM_Send_Select_Codec(ConnectionEntry->HFREID, Message->CodecID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_SELECT_CODEC_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* control indicator status message and responds to the message      */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessQueryControlIndicatorStatusMessage(HFRM_Query_Control_Indicator_Status_Request_t *Message)
{
   int                                             Result;
   Connection_Entry_t                             *ConnectionEntry;
   HFRM_Query_Control_Indicator_Status_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query remote control indicator status.    */
               Result = _HFRM_Query_Remote_Control_Indicator_Status(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_CONTROL_INDICATOR_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free enable  */
   /* indicator notification message and responds to the message        */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableIndicatorNotificationMessage(HFRM_Enable_Indicator_Event_Notification_Request_t *Message)
{
   int                                                  Result;
   Connection_Entry_t                                  *ConnectionEntry;
   HFRM_Enable_Indicator_Event_Notification_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable remote indicator event             */
               /* notification.                                         */
               Result = _HFRM_Enable_Remote_Indicator_Event_Notification(ConnectionEntry->HFREID, Message->EnableEventNotification);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_ENABLE_INDICATOR_EVENT_NOTIFICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* call hold/multi-party support message and responds to the message */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessQueryCallHoldMultipartySupportMessage(HFRM_Query_Call_Holding_Multiparty_Request_t *Message)
{
   int                                            Result;
   Connection_Entry_t                            *ConnectionEntry;
   HFRM_Query_Call_Holding_Multiparty_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query remote call holding/multi-party     */
               /* service support.                                      */
               Result = _HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_CALL_HOLDING_MULTIPARTY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* call holding/multi-party selection message and responds to the    */
   /* message accordingly.  This function does not verify the integrity */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendCallHoldingMultipartySelectionMessage(HFRM_Send_Call_Holding_Multiparty_Selection_Request_t *Message)
{
   int                                                     Result;
   Connection_Entry_t                                     *ConnectionEntry;
   HFRM_Send_Call_Holding_Multiparty_Selection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send call holding/multi-party selection.  */
               Result = _HFRM_Send_Call_Holding_Multiparty_Selection(ConnectionEntry->HFREID, Message->CallHoldMultipartyHandling, Message->Index);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_CALL_HOLDING_MULTIPARTY_SELECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free enable  */
   /* call waiting notification message and responds to the message     */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableCallWaitingNotificationMessage(HFRM_Enable_Call_Waiting_Notification_Request_t *Message)
{
   int                                               Result;
   Connection_Entry_t                               *ConnectionEntry;
   HFRM_Enable_Call_Waiting_Notification_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable remote call waiting notification.  */
               Result = _HFRM_Enable_Remote_Call_Waiting_Notification(ConnectionEntry->HFREID, Message->EnableNotification);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_ENABLE_CALL_WAITING_NOTIFICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free enable  */
   /* call line identification notification message and responds to the */
   /* message accordingly.  This function does not verify the integrity */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableCallLineIdentificationNotificationMessage(HFRM_Enable_Call_Line_Identification_Notification_Request_t *Message)
{
   int                                                           Result;
   Connection_Entry_t                                           *ConnectionEntry;
   HFRM_Enable_Call_Line_Identification_Notification_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable remote call line identification    */
               /* notification.                                         */
               Result = _HFRM_Enable_Remote_Call_Line_Identification_Notification(ConnectionEntry->HFREID, Message->EnableNotification);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_ENABLE_CALL_LINE_IDENTIFICATION_NOTIFICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free dial    */
   /* phone number message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessDialPhoneNumberMessage(HFRM_Dial_Phone_Number_Request_t *Message)
{
   int                                Result;
   Connection_Entry_t                *ConnectionEntry;
   HFRM_Dial_Phone_Number_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Next, check to make sure a valid phone number was     */
               /* specified.                                            */
               /* * NOTE * The phone number length *MUST* include the   */
               /*          NULL terminator, so we need to account for   */
               /*          this in our calculation.                     */
               if((Message->PhoneNumberLength >= (HFRE_PHONE_NUMBER_LENGTH_MINIMUM + 1)) && (Message->PhoneNumberLength <= (HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1)))
               {
                  /* Make sure the Phone Number is NULL terminated.     */
                  Message->PhoneNumber[Message->PhoneNumberLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to dial the specified phone number.       */
                  Result = _HFRM_Dial_Phone_Number(ConnectionEntry->HFREID, Message->PhoneNumber);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_DIAL_PHONE_NUMBER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free dial    */
   /* phone number (from memory) message and responds to the message    */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessDialPhoneNumberFromMemoryMessage(HFRM_Dial_Phone_Number_From_Memory_Request_t *Message)
{
   int                                            Result;
   Connection_Entry_t                            *ConnectionEntry;
   HFRM_Dial_Phone_Number_From_Memory_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to dial a phone number (from memory).        */
               Result = _HFRM_Dial_Phone_Number_From_Memory(ConnectionEntry->HFREID, Message->MemoryLocation);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free re-dial */
   /* last phone number message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessReDialLastPhoneNumberMessage(HFRM_Re_Dial_Last_Phone_Number_Request_t *Message)
{
   int                                        Result;
   Connection_Entry_t                        *ConnectionEntry;
   HFRM_Re_Dial_Last_Phone_Number_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to re-dial the last dialed phone number.     */
               Result = _HFRM_Redial_Last_Phone_Number(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_RE_DIAL_LAST_PHONE_NUMBER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free answer  */
   /* incoming call message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessAnswerIncomingCallMessage(HFRM_Answer_Incoming_Call_Request_t *Message)
{
   int                                   Result;
   Connection_Entry_t                   *ConnectionEntry;
   HFRM_Answer_Incoming_Call_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to answer an incoming call.                  */
               Result = _HFRM_Answer_Incoming_Call(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_ANSWER_INCOMING_CALL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free transmit*/
   /* DTMF code message and responds to the message accordingly.  This  */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessTransmitDTMFCodeMessage(HFRM_Transmit_DTMF_Code_Request_t *Message)
{
   int                                 Result;
   Connection_Entry_t                 *ConnectionEntry;
   HFRM_Transmit_DTMF_Code_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to transmit a DTMF code.                     */
               Result = _HFRM_Transmit_DTMF_Code(ConnectionEntry->HFREID, Message->DTMFCode);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_TRANSMIT_DTMF_CODE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free voice   */
   /* tag request message and responds to the message accordingly.  This*/
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessVoiceTagRequestMessage(HFRM_Voice_Tag_Request_Request_t *Message)
{
   int                                Result;
   Connection_Entry_t                *ConnectionEntry;
   HFRM_Voice_Tag_Request_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to issue a voice tag request.                */
               Result = _HFRM_Voice_Tag_Request(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_VOICE_TAG_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free hang-up */
   /* call message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessHangUpCallMessage(HFRM_Hang_Up_Call_Request_t *Message)
{
   int                           Result;
   Connection_Entry_t           *ConnectionEntry;
   HFRM_Hang_Up_Call_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to hang-up an on-going call.                 */
               Result = _HFRM_Hang_Up_Call(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_HANG_UP_CALL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* current calls list message and responds to the message            */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessQueryCurrentCallsListMessage(HFRM_Query_Current_Calls_List_Request_t *Message)
{
   int                                       Result;
   Connection_Entry_t                       *ConnectionEntry;
   HFRM_Query_Current_Calls_List_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query the remote current calls list.      */
               Result = _HFRM_Query_Remote_Current_Calls_List(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_CURRENT_CALLS_LIST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free set     */
   /* network operator selection format message and responds to the     */
   /* message accordingly.  This function does not verify the integrity */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSetNetworkOperatorSelectionFormatMessage(HFRM_Set_Network_Operator_Selection_Format_Request_t *Message)
{
   int                                                    Result;
   Connection_Entry_t                                    *ConnectionEntry;
   HFRM_Set_Network_Operator_Selection_Format_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set the network operator selection format.*/
               Result = _HFRM_Set_Network_Operator_Selection_Format(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SET_NETWORK_OPERATOR_SELECTION_FORMAT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* network operator selection message and responds to the message    */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessQueryNetworkOperatorSelectionMessage(HFRM_Query_Network_Operator_Selection_Request_t *Message)
{
   int                                               Result;
   Connection_Entry_t                               *ConnectionEntry;
   HFRM_Query_Network_Operator_Selection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query the remote network operator         */
               /* selection.                                            */
               Result = _HFRM_Query_Remote_Network_Operator_Selection(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_NETWORK_OPERATOR_SELECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free enable  */
   /* extended error result message and responds to the message         */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableExtendedErrorResultMessage(HFRM_Enable_Extended_Error_Result_Request_t *Message)
{
   int                                           Result;
   Connection_Entry_t                           *ConnectionEntry;
   HFRM_Enable_Extended_Error_Result_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable remote extended error results.     */
               Result = _HFRM_Enable_Remote_Extended_Error_Result(ConnectionEntry->HFREID, Message->EnableExtendedErrorResults);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_ENABLE_EXTENDED_ERROR_RESULT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* subscriber number information message and responds to the message */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessQuerySubscriberNumberInformationMessage(HFRM_Query_Subscriber_Number_Information_Request_t *Message)
{
   int                                                  Result;
   Connection_Entry_t                                  *ConnectionEntry;
   HFRM_Query_Subscriber_Number_Information_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query subscriber number information.      */
               Result = _HFRM_Query_Subscriber_Number_Information(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_SUBSCRIBER_NUMBER_INFORMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free query   */
   /* response/hold status message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessQueryResponseHoldStatusMessage(HFRM_Query_Response_Hold_Status_Request_t *Message)
{
   int                                         Result;
   Connection_Entry_t                         *ConnectionEntry;
   HFRM_Query_Response_Hold_Status_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to query response/hold status.               */
               Result = _HFRM_Query_Response_Hold_Status(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_RESPONSE_HOLD_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free set     */
   /* incoming call state message and responds to the message           */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSetIncomingCallStateMessage(HFRM_Set_Incoming_Call_State_Request_t *Message)
{
   int                                      Result;
   Connection_Entry_t                      *ConnectionEntry;
   HFRM_Set_Incoming_Call_State_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to set the incoming call state.              */
               Result = _HFRM_Set_Incoming_Call_State(ConnectionEntry->HFREID, Message->CallState);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SET_INCOMING_CALL_STATE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* arbitrary command message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendArbitraryCommandMessage(HFRM_Send_Arbitrary_Command_Request_t *Message)
{
   int                                     Result;
   Connection_Entry_t                     *ConnectionEntry;
   HFRM_Send_Arbitrary_Command_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Next, check to make sure a valid arbitrary command was*/
               /* specified.                                            */
               /* * NOTE * The command length *MUST* include the        */
               /*          NULL terminator, so we need to account for   */
               /*          this in our calculation.                     */
               if(Message->ArbitraryCommandLength > 1)
               {
                  /* Make sure the command is NULL terminated.          */
                  Message->ArbitraryCommand[Message->ArbitraryCommandLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to send the specified arbitrary command.  */
                  Result = _HFRM_Send_Arbitrary_Command(ConnectionEntry->HFREID, Message->ArbitraryCommand);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_ARBITRARY_COMMAND_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the available codec list message from the pm client in    */
   /* the hands free role. The audio gateway will choose a preferred    */
   /* codec from this list as part of the codec negotiation.            */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessSendAvailableCodecListMessage(HFRM_Send_Available_Codec_List_Request_t *Message)
{
   int                                        Result;
   Connection_Entry_t                        *ConnectionEntry;
   HFRM_Send_Available_Codec_List_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctHandsFree)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the codec list.                      */
               Result = _HFRM_Send_Available_Codec_List(ConnectionEntry->HFREID, Message->NumberSupportedCodecs, Message->AvailableCodecList);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_AVAILABLE_CODEC_LIST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free update  */
   /* control indicator status message and responds to the message      */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessUpdateControlIndicatorStatusMessage(HFRM_Update_Control_Indicator_Status_Request_t *Message)
{
   int                                              Result;
   Connection_Entry_t                              *ConnectionEntry;
   HFRE_Indicator_Update_t                         *UpdateIndicatorList;
   HFRM_Update_Control_Indicator_Status_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Now we need to build the update indicator list.       */
               if(Message->NumberUpdateIndicators)
               {
                  if((UpdateIndicatorList = (HFRE_Indicator_Update_t *)BTPS_AllocateMemory(Message->NumberUpdateIndicators*HFRE_INDICATOR_UPDATE_SIZE)) != NULL)
                  {
                     BTPS_MemInitialize(UpdateIndicatorList, 0, Message->NumberUpdateIndicators*HFRE_INDICATOR_UPDATE_SIZE);

                     Result = 0;
                     while(Result < (int)Message->NumberUpdateIndicators)
                     {
                        UpdateIndicatorList[Result] = Message->UpdateIndicatorsList[Result].IndicatorUpdate;

                        if(Message->UpdateIndicatorsList[Result].IndicatorDescriptionLength > 1)
                        {
                           if(Message->UpdateIndicatorsList[Result].IndicatorDescriptionLength > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                              Message->UpdateIndicatorsList[Result].IndicatorDescriptionLength = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

                           Message->UpdateIndicatorsList[Result].IndicatorDescription[Message->UpdateIndicatorsList[Result].IndicatorDescriptionLength - 1] = '\0';

                           UpdateIndicatorList[Result].IndicatorDescription = Message->UpdateIndicatorsList[Result].IndicatorDescription;

                           Result++;
                        }
                     }

                     /* Nothing to do here other than to call the actual*/
                     /* function to update the specified indicators.    */
                     Result = _HFRM_Update_Current_Control_Indicator_Status(ConnectionEntry->HFREID, Message->NumberUpdateIndicators, UpdateIndicatorList);

                     BTPS_FreeMemory(UpdateIndicatorList);
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_UPDATE_CONTROL_INDICATOR_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free update  */
   /* control indicator status (by name) message and responds to the    */
   /* message accordingly.  This function does not verify the integrity */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessUpdateControlIndicatorStatusByNameMessage(HFRM_Update_Control_Indicator_Status_By_Name_Request_t *Message)
{
   int                                              Result;
   Connection_Entry_t                              *ConnectionEntry;
   HFRM_Update_Control_Indicator_Status_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Now we need to build the Update Indicator List.       */
               if(Message->IndicatorNameLength > 1)
               {
                  /* Make sure the name is NULL terminated.             */
                  if(Message->IndicatorNameLength > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                     Message->IndicatorNameLength = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

                  Message->IndicatorName[Message->IndicatorNameLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to update the specified indicator.        */
                  Result = _HFRM_Update_Current_Control_Indicator_Status_By_Name(ConnectionEntry->HFREID, Message->IndicatorName, Message->IndicatorValue);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_UPDATE_CONTROL_INDICATOR_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* call waiting notification message and responds to the message     */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendCallWaitingNotificationMessage(HFRM_Send_Call_Waiting_Notification_Request_t *Message)
{
   int                                             Result;
   Connection_Entry_t                             *ConnectionEntry;
   HFRM_Send_Call_Waiting_Notification_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Next, check to make sure a valid phone number was     */
               /* specified.                                            */
               /* * NOTE * The phone number length *MUST* include the   */
               /*          NULL terminator, so we need to account for   */
               /*          this in our calculation.                     */
               if((!Message->PhoneNumberLength) || ((Message->PhoneNumberLength >= (HFRE_PHONE_NUMBER_LENGTH_MINIMUM + 1)) && (Message->PhoneNumberLength <= (HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1))))
               {
                  /* Make sure the Phone Number is NULL terminated.     */
                  if(Message->PhoneNumberLength)
                     Message->PhoneNumber[Message->PhoneNumberLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to send the call waiting notification.    */
                  Result = _HFRM_Send_Call_Waiting_Notification(ConnectionEntry->HFREID, Message->PhoneNumber);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_CALL_WAITING_NOTIFICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* call line identification message and responds to the message      */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendCallLineIdentificationMessage(HFRM_Send_Call_Line_Identification_Notification_Request_t *Message)
{
   int                                                         Result;
   Connection_Entry_t                                         *ConnectionEntry;
   HFRM_Send_Call_Line_Identification_Notification_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Next, check to make sure a valid phone number was     */
               /* specified.                                            */
               /* * NOTE * The phone number length *MUST* include the   */
               /*          NULL terminator, so we need to account for   */
               /*          this in our calculation.                     */
               if((Message->PhoneNumberLength >= (HFRE_PHONE_NUMBER_LENGTH_MINIMUM + 1)) && (Message->PhoneNumberLength <= (HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1)))
               {
                  /* Make sure the Phone Number is NULL terminated.     */
                  Message->PhoneNumber[Message->PhoneNumberLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to send the call line indentification     */
                  /* notification.                                      */
                  Result = _HFRM_Send_Call_Line_Identification_Notification(ConnectionEntry->HFREID, Message->PhoneNumber);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free ring    */
   /* indication message and responds to the message accordingly.  This */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessRingIndicationMessage(HFRM_Ring_Indication_Request_t *Message)
{
   int                              Result;
   Connection_Entry_t              *ConnectionEntry;
   HFRM_Ring_Indication_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the ring indication.                 */
               Result = _HFRM_Ring_Indication(ConnectionEntry->HFREID);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_RING_INDICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free enable  */
   /* in-band ring tone setting message and responds to the message     */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableInBandRingToneSettingMessage(HFRM_Enable_In_Band_Ring_Tone_Setting_Request_t *Message)
{
   int                                               Result;
   Connection_Entry_t                               *ConnectionEntry;
   HFRM_Enable_In_Band_Ring_Tone_Setting_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to enable/disable in-band ring tone.         */
               Result = _HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(ConnectionEntry->HFREID, Message->EnableInBandRing);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_ENABLE_IN_BAND_RING_TONE_SETTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free voice   */
   /* tag response message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessVoiceTagResponseMessage(HFRM_Voice_Tag_Response_Request_t *Message)
{
   int                                 Result;
   Connection_Entry_t                 *ConnectionEntry;
   HFRM_Voice_Tag_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Next, check to make sure a valid phone number was     */
               /* specified.                                            */
               /* * NOTE * The phone number length *MUST* include the   */
               /*          NULL terminator, so we need to account for   */
               /*          this in our calculation.                     */
               if((!Message->PhoneNumberLength) || ((Message->PhoneNumberLength >= (HFRE_PHONE_NUMBER_LENGTH_MINIMUM + 1)) && (Message->PhoneNumberLength <= (HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1))))
               {
                  /* Make sure the Phone Number is NULL terminated.     */
                  if(Message->PhoneNumberLength)
                     Message->PhoneNumber[Message->PhoneNumberLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to send voice tag response.               */
                  Result = _HFRM_Voice_Tag_Response(ConnectionEntry->HFREID, Message->PhoneNumber);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_VOICE_TAG_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* current calls list message and responds to the message            */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendCurrentCallsListMessage_v1(HFRM_Send_Current_Calls_List_Request_v1_t *Message)
{
   int                                      Result;
   unsigned int                             Index;
   Connection_Entry_t                      *ConnectionEntry;
   HFRE_Current_Call_List_Entry_t          *CurrentCallList;
   HFRM_Send_Current_Calls_List_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Now we need to build the current call list array.     */
               if(Message->NumberCallListEntries)
               {
                  /* First, let's attempt to allocate memory to hold the*/
                  /* contiguous array.                                  */
                  if((CurrentCallList = (HFRE_Current_Call_List_Entry_t *)BTPS_AllocateMemory(Message->NumberCallListEntries*sizeof(HFRM_Call_List_List_Entry_v1_t))) != NULL)
                  {
                     /* Now loop through the array and build the        */
                     /* structures.                                     */
                     for(Index=0,Result=0;(Index<Message->NumberCallListEntries) && (!Result);Index++)
                     {
                        CurrentCallList[Index] = Message->CallListEntryList[Index].CallListEntry;

                        /* Now we need to fix up the phone number       */
                        /* pointer.                                     */
                        CurrentCallList[Index].PhoneNumber = Message->CallListEntryList[Index].PhoneNumber;

                        /* Next, check to make sure a valid phone number*/
                        /* was specified.                               */
                        /* * NOTE * The phone number length *MUST*      */
                        /*          include the NULL terminator, so we  */
                        /*          need to account for this in our     */
                        /*          calculation.                        */
                        if((Message->CallListEntryList[Index].PhoneNumberLength >= (HFRE_PHONE_NUMBER_LENGTH_MINIMUM + 1)) && (Message->CallListEntryList[Index].PhoneNumberLength <= (HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1)))
                        {
                           /* Make sure the Phone Number is NULL        */
                           /* terminated.                               */
                           Message->CallListEntryList[Index].PhoneNumber[Message->CallListEntryList[Index].PhoneNumberLength - 1] = '\0';
                        }
                        else
                           Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                     }

                     /* If all phone numbers were successfully parsed,  */
                     /* then we need to finally call the function to    */
                     /* actually send the current calls list.           */
                     if(!Result)
                        Result = _HFRM_Send_Current_Calls_List(ConnectionEntry->HFREID, Message->NumberCallListEntries, CurrentCallList);

                     /* Free the memory that was allocated.             */
                     BTPS_FreeMemory(CurrentCallList);
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
               else
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the current calls list.           */
                  Result = _HFRM_Send_Current_Calls_List(ConnectionEntry->HFREID, 0, NULL);
               }
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_CURRENT_CALLS_LIST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* current calls list message and responds to the message            */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendCurrentCallsListMessage_v2(HFRM_Send_Current_Calls_List_Request_v2_t *Message)
{
   int                                      Result;
   unsigned int                             Index;
   Connection_Entry_t                      *ConnectionEntry;
   HFRE_Current_Call_List_Entry_t          *CurrentCallList;
   HFRM_Send_Current_Calls_List_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Now we need to build the current call list array.     */
               if(Message->NumberCallListEntries)
               {
                  /* First, let's attempt to allocate memory to hold the*/
                  /* contiguous array.                                  */
                  if((CurrentCallList = (HFRE_Current_Call_List_Entry_t *)BTPS_AllocateMemory(Message->NumberCallListEntries*sizeof(HFRE_Current_Call_List_Entry_t))) != NULL)
                  {
                     BTPS_MemInitialize(CurrentCallList, 0, (Message->NumberCallListEntries*sizeof(HFRE_Current_Call_List_Entry_t)));

                     /* Now loop through the array and build the        */
                     /* structures.                                     */
                     for(Index=0,Result=0;(Index<Message->NumberCallListEntries) && (!Result);Index++)
                     {
                        CurrentCallList[Index].Index         = Message->CallListEntryList[Index].Index;
                        CurrentCallList[Index].CallDirection = Message->CallListEntryList[Index].CallDirection;
                        CurrentCallList[Index].CallStatus    = Message->CallListEntryList[Index].CallStatus;
                        CurrentCallList[Index].CallMode      = Message->CallListEntryList[Index].CallMode;
                        CurrentCallList[Index].Multiparty    = Message->CallListEntryList[Index].Multiparty;
                        CurrentCallList[Index].NumberFormat  = Message->CallListEntryList[Index].NumberFormat;

                        if(Message->CallListEntryList[Index].Flags & HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONE_NUMBER_VALID)
                        {
                           CurrentCallList[Index].PhoneNumber = Message->CallListEntryList[Index].PhoneNumber;
                           Message->CallListEntryList[Index].PhoneNumber[HFRE_PHONE_NUMBER_LENGTH_MAXIMUM] = '\0';
                        }

                        if(Message->CallListEntryList[Index].Flags & HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONEBOOK_NAME_VALID)
                        {
                           CurrentCallList[Index].PhonebookName = Message->CallListEntryList[Index].PhonebookName;
                           Message->CallListEntryList[Index].PhonebookName[HFRE_PHONEBOOK_NAME_LENGTH_MAXIMUM] = '\0';
                        }
                     }

                     /* If all phone numbers were successfully parsed,  */
                     /* then we need to finally call the function to    */
                     /* actually send the current calls list.           */
                     if(!Result)
                        Result = _HFRM_Send_Current_Calls_List(ConnectionEntry->HFREID, Message->NumberCallListEntries, CurrentCallList);

                     /* Free the memory that was allocated.             */
                     BTPS_FreeMemory(CurrentCallList);
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
               else
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the current calls list.           */
                  Result = _HFRM_Send_Current_Calls_List(ConnectionEntry->HFREID, 0, NULL);
               }
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_CURRENT_CALLS_LIST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* network operator selection message and responds to the message    */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendNetworkOperatorSelectionMessage(HFRM_Send_Network_Operator_Selection_Request_t *Message)
{
   int                                              Result;
   Connection_Entry_t                              *ConnectionEntry;
   HFRM_Send_Network_Operator_Selection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Next, check to make sure a valid network operator was */
               /* specified.                                            */
               /* * NOTE * The network operator length *MUST* include   */
               /*          the NULL terminator, so we need to account   */
               /*          for this in our calculation.                 */
               if((!Message->NetworkOperatorLength) || ((Message->NetworkOperatorLength >= (HFRE_NETWORK_OPERATOR_LENGTH_MINIMUM + 1)) && (Message->NetworkOperatorLength <= (HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM + 1))))
               {
                  /* Make sure the Phone Number is NULL terminated.     */
                  if(Message->NetworkOperatorLength)
                     Message->NetworkOperator[Message->NetworkOperatorLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to send the network operator selection.   */
                  Result = _HFRM_Send_Network_Operator_Selection(ConnectionEntry->HFREID, Message->NetworkMode, Message->NetworkOperator);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_NETWORK_OPERATOR_SELECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* extended error result message and responds to the message         */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendExtendedErrorResultMessage(HFRM_Send_Extended_Error_Result_Request_t *Message)
{
   int                                         Result;
   Connection_Entry_t                         *ConnectionEntry;
   HFRM_Send_Extended_Error_Result_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the extended error result.           */
               Result = _HFRM_Send_Extended_Error_Result(ConnectionEntry->HFREID, Message->ResultCode);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_EXTENDED_ERROR_RESULT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* send subscriber number information message and responds to the    */
   /* message accordingly.  This function does not verify the integrity */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendSubscriberNumberInformationMessage(HFRM_Send_Subscriber_Number_Information_Request_t *Message)
{
   int                                                 Result;
   unsigned int                                        Index;
   Connection_Entry_t                                 *ConnectionEntry;
   HFRM_Subscriber_Number_Information_t               *SubscriberList;
   HFRM_Send_Subscriber_Number_Information_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Now we need to build the current call list array.     */
               if(Message->NumberSubscriberInformationEntries)
               {
                  /* First, let's attempt to allocate memory to hold the*/
                  /* contiguous array.                                  */
                  if((SubscriberList = (HFRM_Subscriber_Number_Information_t *)BTPS_AllocateMemory(Message->NumberSubscriberInformationEntries*sizeof(HFRM_Subscriber_Number_Information_t))) != NULL)
                  {
                     /* Now loop through the array and build the        */
                     /* structures.                                     */
                     for(Index=0,Result=0;(Index<Message->NumberSubscriberInformationEntries) && (!Result);Index++)
                     {
                        SubscriberList[Index] = Message->SubscriberInformationList[Index].SubscriberNumberInformationEntry;

                        /* Now we need to fix up the phone number       */
                        /* pointer.                                     */
                        SubscriberList[Index].PhoneNumber = Message->SubscriberInformationList[Index].PhoneNumber;

                        /* Next, check to make sure a valid phone number*/
                        /* was specified.                               */
                        /* * NOTE * The phone number length *MUST*      */
                        /*          include the NULL terminator, so we  */
                        /*          need to account for this in our     */
                        /*          calculation.                        */
                        if((Message->SubscriberInformationList[Index].PhoneNumberLength >= (HFRE_PHONE_NUMBER_LENGTH_MINIMUM + 1)) && (Message->SubscriberInformationList[Index].PhoneNumberLength <= (HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1)))
                        {
                           /* Make sure the Phone Number is NULL        */
                           /* terminated.                               */
                           Message->SubscriberInformationList[Index].PhoneNumber[Message->SubscriberInformationList[Index].PhoneNumberLength - 1] = '\0';
                        }
                        else
                           Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                     }

                     /* If all phone numbers were successfully parsed,  */
                     /* then we need to finally call the function to    */
                     /* actually send the subscriber information list.  */
                     if(!Result)
                        Result = _HFRM_Send_Subscriber_Number_Information(ConnectionEntry->HFREID, Message->NumberSubscriberInformationEntries, SubscriberList);

                     /* Free the memory that was allocated.             */
                     BTPS_FreeMemory(SubscriberList);
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
               else
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the subscriber number information */
                  /* list.                                              */
                  Result = _HFRM_Send_Subscriber_Number_Information(ConnectionEntry->HFREID, 0, NULL);
               }
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* incoming call state message and responds to the message           */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendIncomingCallStateMessage(HFRM_Send_Incoming_Call_State_Request_t *Message)
{
   int                                       Result;
   Connection_Entry_t                       *ConnectionEntry;
   HFRM_Send_Incoming_Call_State_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the incoming call state.             */
               Result = _HFRM_Send_Incoming_Call_State(ConnectionEntry->HFREID, Message->CallState);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_INCOMING_CALL_STATE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* terminating response message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendTerminatingResponseMessage(HFRM_Send_Terminating_Response_Request_t *Message)
{
   int                                        Result;
   Connection_Entry_t                        *ConnectionEntry;
   HFRM_Send_Terminating_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the terminating response.            */
               Result = _HFRM_Send_Terminating_Response(ConnectionEntry->HFREID, Message->ResultType, Message->ResultValue);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_TERMINATING_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free enable  */
   /* arbitrary command processing message and responds to the message  */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessEnableArbitraryCommandProcessingMessage(HFRM_Enable_Arbitrary_Command_Processing_Request_t *Message)
{
   int                                                  Result;
   Connection_Entry_t                                  *ConnectionEntry;
   HFRM_Enable_Arbitrary_Command_Processing_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Attempt to verify the Callback specified.                      */
      if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Next, determine the connection information.              */
            ConnectionEntry = ConnectionEntryList;
            Result          = 0;
            while(ConnectionEntry)
            {
               if((!Result) && (ConnectionEntry->ConnectionType == hctAudioGateway))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to enable arbitrary command processing.   */
                  Result = _HFRM_Enable_Arbitrary_Command_Processing(ConnectionEntry->HFREID);
               }

               ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
            }
         }
         else
            Result = 0;

         if(!Result)
            ArbitraryCommandsEnabled = TRUE;
      }
      else
         Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_ENABLE_ARBITRARY_COMMAND_PROCESSING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Hands Free send    */
   /* arbitrary response message and responds to the message            */
   /* accordingly.  This function does not verify the integrity of the  */
   /* message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the message before calling this function.*/
static void ProcessSendArbitraryResponseMessage(HFRM_Send_Arbitrary_Response_Request_t *Message)
{
   int                                      Result;
   Connection_Entry_t                      *ConnectionEntry;
   HFRM_Send_Arbitrary_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, Message->ControlEventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, hctAudioGateway)) != NULL)
            {
               /* Next, check to make sure a valid arbitrary response   */
               /* was specified.                                        */
               /* * NOTE * The command length *MUST* include the        */
               /*          NULL terminator, so we need to account for   */
               /*          this in our calculation.                     */
               if(Message->ArbitraryResponseLength > 1)
               {
                  /* Make sure the command is NULL terminated.          */
                  Message->ArbitraryResponse[Message->ArbitraryResponseLength - 1] = '\0';

                  /* Nothing to do here other than to call the actual   */
                  /* function to send the specified arbitrary response. */
                  Result = _HFRM_Send_Arbitrary_Response(ConnectionEntry->HFREID, Message->ArbitraryResponse);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_SEND_ARBITRARY_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified query SCO Handle   */
   /* indication message and responds to the message accordingly.  This */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessQuerySCOConnectionHandleMessage(HFRM_Query_SCO_Connection_Handle_Request_t *Message)
{
   int                                          Result;
   Word_t                                       SCOHandle;
   Connection_Entry_t                          *ConnectionEntry;
   HFRM_Query_SCO_Connection_Handle_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      SCOHandle = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's attempt to verify the    */
         /* Callback specified.                                         */
         if(SearchHFREEntryInfoEntry((Message->ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, Message->EventsHandlerID))
         {
            /* Next, determine the connection information.              */
            if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress, Message->ConnectionType)) != NULL) && (ConnectionEntry->ConnectionState == csConnected) && (ConnectionEntry->SCOHandle))
            {
               SCOHandle = ConnectionEntry->SCOHandle;
               Result    = 0;
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HFRM_QUERY_SCO_CONNECTION_HANDLE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      ResponseMessage.SCOHandle                    = SCOHandle;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Hands Free lock  */
   /*          held.  This function will release the lock before it     */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).     */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Hands Free connect response request.                  */
               ProcessConnectionResponseMessage((HFRM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* connect remote device request.                        */
               ProcessConnectRemoteDeviceMessage((HFRM_Connect_Remote_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DISCONNECT_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DISCONNECT_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* disconnect device request.                            */
               ProcessDisconnectDeviceMessage((HFRM_Disconnect_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Connected Devices Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query connected devices request.                      */
               ProcessQueryConnectedDevicesMessage((HFRM_Query_Connected_Devices_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Hands Free Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_REGISTER_HANDS_FREE_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* register Hands Free events request.                   */
               ProcessRegisterHandsFreeEventsMessage((HFRM_Register_Hands_Free_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Hands Free Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_UN_REGISTER_HANDS_FREE_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* un-register Hands Free events request.                */
               ProcessUnRegisterHandsFreeEventsMessage((HFRM_Un_Register_Hands_Free_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Hands Free Data Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_REGISTER_HANDS_FREE_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* register Hands Free data request.                     */
               ProcessRegisterHandsFreeDataMessage((HFRM_Register_Hands_Free_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Hands Free Data Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_UN_REGISTER_HANDS_FREE_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* un-register Hands Free data request.                  */
               ProcessUnRegisterHandsFreeDataMessage((HFRM_Un_Register_Hands_Free_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set-up Audio Connection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SETUP_AUDIO_CONNECTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* set-up audio connection request.                      */
               ProcessSetupAudioConnectionMessage((HFRM_Setup_Audio_Connection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Release Audio Connection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_RELEASE_AUDIO_CONNECTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* release audio connection request.                     */
               ProcessReleaseAudioConnectionMessage((HFRM_Release_Audio_Connection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_AUDIO_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Audio Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_AUDIO_DATA_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_AUDIO_DATA_REQUEST_SIZE(((HFRM_Send_Audio_Data_Request_t *)Message)->AudioDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* audio data request.                                   */
               ProcessSendAudioDataMessage((HFRM_Send_Audio_Data_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query SCO Connection Handle Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_SCO_CONNECTION_HANDLE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query SCO Connection handle request.                  */
               ProcessQuerySCOConnectionHandleMessage((HFRM_Query_SCO_Connection_Handle_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Configuration Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query configuration request.                          */
               ProcessQueryConfigurationMessage((HFRM_Query_Current_Configuration_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Incoming Connection Flags Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* change incoming connection flags request.             */
               ProcessChangeIncomingConnectionFlagsMessage((HFRM_Change_Incoming_Connection_Flags_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DISABLE_ECHO_NOISE_CANCELLATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable Echo/Noise Cancellation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DISABLE_ECHO_NOISE_CANCELLATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* disable echo/noise cancellation request.              */
               ProcessDisableEchoNoiseCancellationMessage((HFRM_Disable_Echo_Noise_Cancellation_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SET_VOICE_RECOGNITION_ACTIVATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Voice Recognition Activation Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SET_VOICE_RECOGNITION_ACTIVATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the set */
               /* voice recognition activation request.                 */
               ProcessSetVoiceRecognitionActivationMessage((HFRM_Set_Voice_Recognition_Activation_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Speaker Gain Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SET_SPEAKER_GAIN_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* set speaker gain request.                             */
               ProcessSetSpeakerGainMessage((HFRM_Set_Speaker_Gain_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Microphone Gain Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SET_MICROPHONE_GAIN_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* set microphone gain request.                          */
               ProcessSetMicrophoneGainMessage((HFRM_Set_Microphone_Gain_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_SELECT_CODEC:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Select Codec Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_SELECT_CODEC_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* send available codec list request.                    */
               ProcessSendSelectCodecMessage((HFRM_Send_Select_Codec_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CONTROL_INDICATOR_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Control Indicator Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CONTROL_INDICATOR_STATUS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query control indicator status request.               */
               ProcessQueryControlIndicatorStatusMessage((HFRM_Query_Control_Indicator_Status_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ENABLE_INDICATOR_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Indicator Notification Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ENABLE_INDICATOR_EVENT_NOTIFICATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* enable indicator notification request.                */
               ProcessEnableIndicatorNotificationMessage((HFRM_Enable_Indicator_Event_Notification_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CALL_HOLD_MULTI_SUPPORT:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Call Hold/Multi-party Support Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CALL_HOLDING_MULTIPARTY_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query call hold/multi-party support request.          */
               ProcessQueryCallHoldMultipartySupportMessage((HFRM_Query_Call_Holding_Multiparty_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_CALL_HOLD_MULTI_SELECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Call Holding/Multi-party Selection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CALL_HOLDING_MULTIPARTY_SELECTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* send call holding/multi-party selection request.      */
               ProcessSendCallHoldingMultipartySelectionMessage((HFRM_Send_Call_Holding_Multiparty_Selection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ENABLE_CALL_WAIT_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Call Waiting Notification Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ENABLE_CALL_WAITING_NOTIFICATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* enable call waiting notification request.             */
               ProcessEnableCallWaitingNotificationMessage((HFRM_Enable_Call_Waiting_Notification_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ENABLE_CALL_LINE_ID_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Call Line Identification Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ENABLE_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* enable call line identification notification request. */
               ProcessEnableCallLineIdentificationNotificationMessage((HFRM_Enable_Call_Line_Identification_Notification_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Dial Phone Number Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_REQUEST_SIZE(((HFRM_Dial_Phone_Number_Request_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the dial*/
               /* phone number request.                                 */
               ProcessDialPhoneNumberMessage((HFRM_Dial_Phone_Number_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEMORY:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Dial Phone Number (from memory) Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the dial*/
               /* phone number (from memory) request.                   */
               ProcessDialPhoneNumberFromMemoryMessage((HFRM_Dial_Phone_Number_From_Memory_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Re-dial Last Phone Number Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_RE_DIAL_LAST_PHONE_NUMBER_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* re-dial last phone number request.                    */
               ProcessReDialLastPhoneNumberMessage((HFRM_Re_Dial_Last_Phone_Number_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ANSWER_INCOMING_CALL:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Answer Incoming Call Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ANSWER_INCOMING_CALL_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* answer incoming call request.                         */
               ProcessAnswerIncomingCallMessage((HFRM_Answer_Incoming_Call_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_TRANSMIT_DTMF_CODE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Transmit DTMF Code Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_TRANSMIT_DTMF_CODE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* transmit DTMF code request.                           */
               ProcessTransmitDTMFCodeMessage((HFRM_Transmit_DTMF_Code_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Tag Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_REQUEST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* voice tag request request.                            */
               ProcessVoiceTagRequestMessage((HFRM_Voice_Tag_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_HANG_UP_CALL:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hang-up Call Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_HANG_UP_CALL_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* hang-up call request.                                 */
               ProcessHangUpCallMessage((HFRM_Hang_Up_Call_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Calls List Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CALLS_LIST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query current calls list request.                     */
               ProcessQueryCurrentCallsListMessage((HFRM_Query_Current_Calls_List_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SET_NETWORK_OPERATOR_FORMAT:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Network Operator Selection Format Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SET_NETWORK_OPERATOR_SELECTION_FORMAT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the set */
               /* network operator selection format request.            */
               ProcessSetNetworkOperatorSelectionFormatMessage((HFRM_Set_Network_Operator_Selection_Format_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_NETWORK_OPERATOR_SELECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Network Operator Selection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query network operator selection request.             */
               ProcessQueryNetworkOperatorSelectionMessage((HFRM_Query_Network_Operator_Selection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ENABLE_EXTENDED_ERROR_RESULT:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Extended Error Result Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ENABLE_EXTENDED_ERROR_RESULT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* enable extended error result request.                 */
               ProcessEnableExtendedErrorResultMessage((HFRM_Enable_Extended_Error_Result_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_SUBSCRIBER_NUMBER_INFO:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Subscriber Number Information Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query subscriber number information request.          */
               ProcessQuerySubscriberNumberInformationMessage((HFRM_Query_Subscriber_Number_Information_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_QUERY_RESPONSE_HOLD_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Response/Hold Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_QUERY_RESPONSE_HOLD_STATUS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* query response/hold status request.                   */
               ProcessQueryResponseHoldStatusMessage((HFRM_Query_Response_Hold_Status_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SET_INCOMING_CALL_STATE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Incoming Call State Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SET_INCOMING_CALL_STATE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the set */
               /* incoming call state request.                          */
               ProcessSetIncomingCallStateMessage((HFRM_Set_Incoming_Call_State_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_COMMAND:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Arbitrary Command Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_ARBITRARY_COMMAND_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_ARBITRARY_COMMAND_REQUEST_SIZE(((HFRM_Send_Arbitrary_Command_Request_t *)Message)->ArbitraryCommandLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* arbitrary command request.                            */
               ProcessSendArbitraryCommandMessage((HFRM_Send_Arbitrary_Command_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_AVAILABLE_CODEC_LIST:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Available Codec List Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_AVAILABLE_CODEC_LIST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* send available codec list request.                    */
               ProcessSendAvailableCodecListMessage((HFRM_Send_Available_Codec_List_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Control Indicator Status Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_UPDATE_CONTROL_INDICATOR_STATUS_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_UPDATE_CONTROL_INDICATOR_STATUS_REQUEST_SIZE(((HFRM_Update_Control_Indicator_Status_Request_t *)Message)->NumberUpdateIndicators)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* update control indicator status request.              */
               ProcessUpdateControlIndicatorStatusMessage((HFRM_Update_Control_Indicator_Status_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS_BY_NAME:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Control Indicator Status (By Name) Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_REQUEST_SIZE(((HFRM_Update_Control_Indicator_Status_By_Name_Request_t *)Message)->IndicatorNameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* update control indicator status (by name) request.    */
               ProcessUpdateControlIndicatorStatusByNameMessage((HFRM_Update_Control_Indicator_Status_By_Name_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_CALL_WAITING_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Call Waiting Notification Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CALL_WAITING_NOTIFICATION_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CALL_WAITING_NOTIFICATION_REQUEST_SIZE(((HFRM_Send_Call_Waiting_Notification_Request_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* call waiting notification request.                    */
               ProcessSendCallWaitingNotificationMessage((HFRM_Send_Call_Waiting_Notification_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_CALL_LINE_ID_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Call Line Identification Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE(((HFRM_Send_Call_Line_Identification_Notification_Request_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* call line identification request.                     */
               ProcessSendCallLineIdentificationMessage((HFRM_Send_Call_Line_Identification_Notification_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_RING_INDICATION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Ring Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_RING_INDICATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the ring*/
               /* indication request.                                   */
               ProcessRingIndicationMessage((HFRM_Ring_Indication_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ENABLE_IN_BAND_RING_TONE_SETTING:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable In-band Ring Tone Setting Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ENABLE_IN_BAND_RING_TONE_SETTING_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* enable in-band ring ton setting request.              */
               ProcessEnableInBandRingToneSettingMessage((HFRM_Enable_In_Band_Ring_Tone_Setting_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_VOICE_TAG_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Tag Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_RESPONSE_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_RESPONSE_REQUEST_SIZE(((HFRM_Voice_Tag_Response_Request_t *)Message)->PhoneNumberLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* voice tag response request.                           */
               ProcessVoiceTagResponseMessage((HFRM_Voice_Tag_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V1:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Current Calls List Message (V1 - DEPRECATED)\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V1_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V1_SIZE(((HFRM_Send_Current_Calls_List_Request_v1_t *)Message)->NumberCallListEntries)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* current calls list request.                           */
               ProcessSendCurrentCallsListMessage_v1((HFRM_Send_Current_Calls_List_Request_v1_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V2:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Current Calls List Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V2_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V2_SIZE(((HFRM_Send_Current_Calls_List_Request_v2_t *)Message)->NumberCallListEntries)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* current calls list request.                           */
               ProcessSendCurrentCallsListMessage_v2((HFRM_Send_Current_Calls_List_Request_v2_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_NETWORK_OPERATOR_SELECTION:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Network Operator Selection Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE(((HFRM_Send_Network_Operator_Selection_Request_t *)Message)->NetworkOperatorLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* network operator selection request.                   */
               ProcessSendNetworkOperatorSelectionMessage((HFRM_Send_Network_Operator_Selection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_EXTENDED_ERROR_RESULT:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Extended Error Result Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_EXTENDED_ERROR_RESULT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* extended error result request.                        */
               ProcessSendExtendedErrorResultMessage((HFRM_Send_Extended_Error_Result_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_SUBSCRIBER_NUMBER_INFO:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Subscriber Number Information Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE(((HFRM_Send_Subscriber_Number_Information_Request_t *)Message)->NumberSubscriberInformationEntries)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* network operator selection request.                   */
               ProcessSendSubscriberNumberInformationMessage((HFRM_Send_Subscriber_Number_Information_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_INCOMING_CALL_STATE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Incoming Call State Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_INCOMING_CALL_STATE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* incoming call state request.                          */
               ProcessSendIncomingCallStateMessage((HFRM_Send_Incoming_Call_State_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_TERMINATING_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Terminating Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_TERMINATING_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* terminating response request.                         */
               ProcessSendTerminatingResponseMessage((HFRM_Send_Terminating_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_ENABLE_ARBITRARY_CMD_PROCESSING:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Arbitrary Command Processing Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_ENABLE_ARBITRARY_COMMAND_PROCESSING_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* enable arbitrary command processing request.          */
               ProcessEnableArbitraryCommandProcessingMessage((HFRM_Enable_Arbitrary_Command_Processing_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Arbitrary Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_ARBITRARY_RESPONSE_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HFRM_SEND_ARBITRARY_RESPONSE_REQUEST_SIZE(((HFRM_Send_Arbitrary_Response_Request_t *)Message)->ArbitraryResponseLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the send*/
               /* arbitrary response request.                           */
               ProcessSendArbitraryResponseMessage((HFRM_Send_Arbitrary_Response_Request_t *)Message);
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

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   Boolean_t            LoopCount;
   HFRE_Entry_Info_t   *HFREEntryInfo;
   HFRE_Entry_Info_t  **_HFREEntryInfoList;
   HFRE_Entry_Info_t   *tmpHFREEntryInfo;
   Connection_Entry_t  *ConnectionEntry;
   Connection_Entry_t  *CloseConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

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
            HFREEntryInfo      = HFREEntryInfoList_AG;
            _HFREEntryInfoList = &HFREEntryInfoList_AG;
         }
         else
         {
            if(LoopCount == 4)
            {
               HFREEntryInfo      = HFREEntryInfoList_HF;
               _HFREEntryInfoList = &HFREEntryInfoList_HF;
            }
            else
            {
               if(LoopCount == 3)
               {
                  HFREEntryInfo      = HFREEntryInfoList_AG_Control;
                  _HFREEntryInfoList = &HFREEntryInfoList_AG_Control;
               }
               else
               {
                  if(LoopCount == 2)
                  {
                     HFREEntryInfo      = HFREEntryInfoList_HF_Control;
                     _HFREEntryInfoList = &HFREEntryInfoList_HF_Control;
                  }
                  else
                  {
                     if(LoopCount)
                     {
                        HFREEntryInfo      = HFREEntryInfoList_AG_Data;
                        _HFREEntryInfoList = &HFREEntryInfoList_AG_Data;
                     }
                     else
                     {
                        HFREEntryInfo      = HFREEntryInfoList_HF_Data;
                        _HFREEntryInfoList = &HFREEntryInfoList_HF_Data;
                     }
                  }
               }
            }
         }

         while(HFREEntryInfo)
         {
            /* Check to see if the current Client Information is the one*/
            /* that is being un-registered.                             */
            if(HFREEntryInfo->ClientID == ClientID)
            {
               /* Note the next Hands Free Entry in the list (we are    */
               /* about to delete the current entry).                   */
               tmpHFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;

               /* Go ahead and delete the Hands Free Information Entry  */
               /* and clean up the resources.                           */
               if((HFREEntryInfo = DeleteHFREEntryInfoEntry(_HFREEntryInfoList, HFREEntryInfo->CallbackID)) != NULL)
               {
                  /* Check to see if this is a Control endpoint.  There */
                  /* is only one control allowed per mode.              */
                  if((_HFREEntryInfoList == &HFREEntryInfoList_AG_Control) || (_HFREEntryInfoList == &HFREEntryInfoList_HF_Control))
                  {
                     /* Check to see if there is an open connection.    */
                     if(ConnectionEntryList)
                     {
                        ConnectionEntry = ConnectionEntryList;
                        while(ConnectionEntry)
                        {
                           /* Check to see if the AudioGateway control is  */
                           /* being unregistered.                          */
                           if((ConnectionEntry->ConnectionType == hctAudioGateway) && (_HFREEntryInfoList == &HFREEntryInfoList_AG_Control))
                           {
                              CloseConnectionEntry = ConnectionEntry;
                           }
                           else
                           {
                              /* Check to see if the Handsfree control is  */
                              /* being unregistered.                       */
                              if((ConnectionEntry->ConnectionType == hctHandsFree) && (_HFREEntryInfoList == &HFREEntryInfoList_HF_Control))
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
                              _HFRM_Disconnect_Device(CloseConnectionEntry->HFREID);

                              if((CloseConnectionEntry = DeleteConnectionEntryHFREID(&ConnectionEntryList, CloseConnectionEntry->HFREID)) != NULL)
                                 FreeConnectionEntryMemory(CloseConnectionEntry);
                           }
                        }
                     }

                     /* Remove the SDP record, since the control        */
                     /* application his gone away.                      */
                     _HFRM_UpdateSDPRecord((_HFREEntryInfoList == &HFREEntryInfoList_AG_Control)?hctAudioGateway:hctHandsFree, FALSE);
                  }

                  /* Close any events that were allocated.              */
                  if(HFREEntryInfo->ConnectionEvent)
                     BTPS_CloseEvent(HFREEntryInfo->ConnectionEvent);

                  /* All finished with the memory so free the entry.    */
                  FreeHFREEntryInfoEntryMemory(HFREEntryInfo);
               }

               /* Go ahead and set the next Hands Free Information Entry*/
               /* (past the one we just deleted).                       */
               HFREEntryInfo = tmpHFREEntryInfo;
            }
            else
               HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* port open request indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* lock protecting the Hands Free Manager information held.          */
static void ProcessOpenRequestIndicationEvent(HFRE_Open_Port_Request_Indication_Data_t *OpenPortRequestIndicationData)
{
   int                                Result;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   unsigned int                       ServerPort;
   unsigned long                      IncomingConnectionFlags;
   HFRM_Event_Data_t                  HFRMEventData;
   Connection_Entry_t                 ConnectionEntry;
   Connection_Entry_t                *ConnectionEntryPtr;
   HFRE_Entry_Info_t                 *HFREEntryInfo;
   HFRM_Connection_Type_t             ConnectionType;
   HFRM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((OpenPortRequestIndicationData) && (_HFRM_QueryIncomingConnectionType(OpenPortRequestIndicationData->HFREPortID, &ConnectionType, &ServerPort)))
   {
      /* Determine the incoming connection flags based on the connection*/
      /* type.                                                          */
      if(ConnectionType == hctAudioGateway)
      {
         HFREEntryInfo           = HFREEntryInfoList_AG_Control;
         IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
      }
      else
      {
         HFREEntryInfo           = HFREEntryInfoList_HF_Control;
         IncomingConnectionFlags = HandsFreeInitializationInfo.IncomingConnectionFlags;
      }

      /* Verify that a control event callback has been registered.      */
      if(HFREEntryInfo)
      {
         /* First, let's see if we actually need to do anything, other  */
         /* than simply accept the connection.                          */
         if(!IncomingConnectionFlags)
         {
            /* Simply Accept the connection.                            */
            _HFRM_Connection_Request_Response(OpenPortRequestIndicationData->HFREPortID, TRUE);
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
               ConnectionEntry.HFREID                 = OpenPortRequestIndicationData->HFREPortID;
               ConnectionEntry.Server                 = TRUE;
               ConnectionEntry.ServerPort             = ServerPort;
               ConnectionEntry.ConnectionState        = csAuthorizing;
               ConnectionEntry.NextConnectionEntryPtr = NULL;

               ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
            }
            else
               ConnectionEntryPtr->HFREID = OpenPortRequestIndicationData->HFREPortID;

            /* Check to see if we are tracking this connection.         */
            if(ConnectionEntryPtr)
            {
               if(IncomingConnectionFlags & HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION)
               {
                  /* Authorization (at least) required, go ahead and    */
                  /* dispatch the request.                              */
                  ConnectionEntryPtr->ConnectionState = csAuthorizing;

                  /* Next, format up the Event to dispatch.             */
                  HFRMEventData.EventType                                                        = hetHFRIncomingConnectionRequest;
                  HFRMEventData.EventLength                                                      = HFRM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

                  HFRMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = OpenPortRequestIndicationData->BD_ADDR;
                  HFRMEventData.EventData.IncomingConnectionRequestEventData.ConnectionType      = ConnectionType;

                  /* Next, format up the Message to dispatch.           */
                  BTPS_MemInitialize(&Message, 0, sizeof(Message));

                  Message.MessageHeader.AddressID       = 0;
                  Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
                  Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
                  Message.MessageHeader.MessageLength   = (HFRM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  Message.RemoteDeviceAddress           = OpenPortRequestIndicationData->BD_ADDR;
                  Message.ConnectionType                = ConnectionType;

                  /* Finally dispatch the formatted Event and Message.  */
                  DispatchHFREEvent(FALSE, ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
               }
               else
               {
                  /* Determine if Authentication and/or Encryption is   */
                  /* required for this link.                            */
                  if(IncomingConnectionFlags & HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                     Authenticate = TRUE;
                  else
                     Authenticate = FALSE;

                  if(IncomingConnectionFlags & HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
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
                     Result = _HFRM_Connection_Request_Response(ConnectionEntryPtr->HFREID, TRUE);

                     if(Result)
                     {
                        _HFRM_Connection_Request_Response(ConnectionEntryPtr->HFREID, FALSE);

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
                        _HFRM_Connection_Request_Response(ConnectionEntryPtr->HFREID, FALSE);

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
               _HFRM_Connection_Request_Response(OpenPortRequestIndicationData->HFREPortID, FALSE);
            }
         }
      }
      else
      {
         /* There is no Control Event Callback registered, go ahead and */
         /* reject the request.                                         */
         _HFRM_Connection_Request_Response(OpenPortRequestIndicationData->HFREPortID, FALSE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* port open indication event that has been received with the        */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Hands Free Manager information held.          */
static void ProcessOpenIndicationEvent(HFRE_Open_Port_Indication_Data_t *OpenPortIndicationData)
{
   unsigned int                            ServerPort;
   HFRM_Event_Data_t                       HFRMEventData;
   Connection_Entry_t                      ConnectionEntry;
   Connection_Entry_t                     *ConnectionEntryPtr;
   HFRM_Connection_Request_Message_t       Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenPortIndicationData)
   {
      if((ConnectionEntryPtr = SearchConnectionEntryHFREID(&ConnectionEntryList, OpenPortIndicationData->HFREPortID)) == NULL)
      {
         /* Entry does not exist, go ahead and format a new entry.      */
         _HFRM_QueryIncomingConnectionType(OpenPortIndicationData->HFREPortID, &(ConnectionEntry.ConnectionType), &ServerPort);

         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(ConnectionEntry));

         ConnectionEntry.BD_ADDR                = OpenPortIndicationData->BD_ADDR;
         ConnectionEntry.HFREID                 = OpenPortIndicationData->HFREPortID;
         ConnectionEntry.Server                 = TRUE;
         ConnectionEntry.ServerPort             = ServerPort;
         ConnectionEntry.ConnectionState        = csConnected;
         ConnectionEntry.NextConnectionEntryPtr = NULL;

         ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
      }
      else
      {
         /* Update the connection information.                          */
         _HFRM_QueryIncomingConnectionType(OpenPortIndicationData->HFREPortID, &(ConnectionEntryPtr->ConnectionType), &ServerPort);

         ConnectionEntryPtr->ConnectionState = csConnected;
         ConnectionEntryPtr->ServerPort      = ServerPort;
         ConnectionEntryPtr->HFREID          = OpenPortIndicationData->HFREPortID;
      }

      /* Enable Arbitrary Commands if necessary.                        */
      if(ArbitraryCommandsEnabled)
         _HFRM_Enable_Arbitrary_Command_Processing(OpenPortIndicationData->HFREPortID);

      if(ConnectionEntryPtr)
      {
         /* Flag that we are now watching SCO connections for this      */
         /* device.                                                     */
         SCOM_AddConnectionToIgnoreList(OpenPortIndicationData->BD_ADDR);

         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                        = hetHFRConnected;
         HFRMEventData.EventLength                                      = HFRM_CONNECTED_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ConnectedEventData.ConnectionType      = ConnectionEntryPtr->ConnectionType;
         HFRMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = OpenPortIndicationData->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTED;
         Message.MessageHeader.MessageLength   = (HFRM_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntryPtr->ConnectionType;
         Message.RemoteDeviceAddress           = OpenPortIndicationData->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(FALSE, ConnectionEntryPtr->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
      else
      {
         /* Error, go ahead and disconnect the device.                  */
         _HFRM_Disconnect_Device(OpenPortIndicationData->HFREPortID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* open confirmation event that has been received with the specified */
   /* information.  This function should be called with the lock        */
   /* protecting the Hands Free Manager information held.               */
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HFRE_Open_Port_Confirmation_Data_t *OpenPortConfirmationData)
{
   void                                    *CallbackParameter;
   unsigned int                             ClientID;
   HFRM_Event_Data_t                        HFRMEventData;
   HFRE_Entry_Info_t                       *HFREEntryInfo;
   Connection_Entry_t                       ConnectionEntry;
   Connection_Entry_t                      *ConnectionEntryPtr;
   HFRM_Event_Callback_t                    EventCallback;
   HFRM_Device_Connected_Message_t          Message;
   HFRM_Device_Connection_Status_Message_t  ConnectionStatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenPortConfirmationData)
   {
      /* First, flag the connected state.                               */
      if(((ConnectionEntryPtr = SearchConnectionEntryHFREID(&ConnectionEntryList, OpenPortConfirmationData->HFREPortID)) != NULL) && (!ConnectionEntryPtr->Server))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Found, State: 0x%08X\n", ConnectionEntryPtr->ConnectionState));

         /* Note all the connection information (we will use it later). */
         ConnectionEntry = *ConnectionEntryPtr;

         if((OpenPortConfirmationData->PortOpenStatus) && (ConnectionEntryPtr->ConnectionState != csConnected))
         {
            if((ConnectionEntryPtr = DeleteConnectionEntryHFREID(&ConnectionEntryList, OpenPortConfirmationData->HFREPortID)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntryPtr);
         }
         else
         {
            ConnectionEntryPtr->ConnectionState = csConnected;

            /* Enable arbitrary commands if necessary.                  */
            if(ArbitraryCommandsEnabled)
               _HFRM_Enable_Arbitrary_Command_Processing(ConnectionEntryPtr->HFREID);
         }

         /* Dispatch any registered Connection Status Message/Event.    */
         if(ConnectionEntry.ConnectionType == hctAudioGateway)
            HFREEntryInfo = HFREEntryInfoList_AG;
         else
            HFREEntryInfo = HFREEntryInfoList_HF;

         while(HFREEntryInfo)
         {
            if((!(HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(ConnectionEntry.BD_ADDR, HFREEntryInfo->ConnectionBD_ADDR)))
            {
               /* Flag that we are now watching SCO connections for this*/
               /* device.                                               */
               SCOM_AddConnectionToIgnoreList(HFREEntryInfo->ConnectionBD_ADDR);

               /* Connection status registered, now see if we need to   */
               /* issue a Callack or an event.                          */

               /* Determine if we need to dispatch the event locally or */
               /* remotely.                                             */
               if(HFREEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Callback.                                          */
                  BTPS_MemInitialize(&HFRMEventData, 0, sizeof(HFRM_Event_Data_t));

                  HFRMEventData.EventType                                               = hetHFRConnectionStatus;
                  HFRMEventData.EventLength                                             = HFRM_CONNECTION_STATUS_EVENT_DATA_SIZE;

                  HFRMEventData.EventData.ConnectionStatusEventData.ConnectionType      = ConnectionEntry.ConnectionType;
                  HFRMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = ConnectionEntry.BD_ADDR;

                  /* Map the open confirmation Error to the correct     */
                  /* Hands Free Manager error status.                   */
                  switch(OpenPortConfirmationData->PortOpenStatus)
                  {
                     case HFRE_OPEN_PORT_STATUS_SUCCESS:
                        HFRMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_SUCCESS;
                        break;
                     case HFRE_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
                        HFRMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                        break;
                     case HFRE_OPEN_PORT_STATUS_CONNECTION_REFUSED:
                        HFRMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                        break;
                     default:
                        HFRMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        break;
                  }

                  /* If this was a synchronous event we need to set the */
                  /* status and the event.                              */
                  if(HFREEntryInfo->ConnectionEvent)
                  {
                     /* Synchronous event, go ahead and set the correct */
                     /* status, then set the event.                     */
                     HFREEntryInfo->ConnectionStatus = HFRMEventData.EventData.ConnectionStatusEventData.ConnectionStatus;

                     BTPS_SetEvent(HFREEntryInfo->ConnectionEvent);
                  }
                  else
                  {
                     /* Note the Callback information.                  */
                     EventCallback     = HFREEntryInfo->EventCallback;
                     CallbackParameter = HFREEntryInfo->CallbackParameter;

                     /* Go ahead and delete the entry (since we are     */
                     /* dispatching the callback).                      */
                     if((HFREEntryInfo = DeleteHFREEntryInfoEntry((ConnectionEntry.ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, HFREEntryInfo->CallbackID)) != NULL)
                        FreeHFREEntryInfoEntryMemory(HFREEntryInfo);

                     /* Release the Lock so we can make the callback.   */
                     DEVM_ReleaseLock();

                     __BTPSTRY
                     {
                        if(EventCallback)
                           (*EventCallback)(&HFRMEventData, CallbackParameter);
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
                  ClientID = HFREEntryInfo->ClientID;

                  /* Go ahead and delete the entry (since we are        */
                  /* dispatching the event).                            */
                  if((HFREEntryInfo = DeleteHFREEntryInfoEntry((ConnectionEntry.ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, HFREEntryInfo->CallbackID)) != NULL)
                     FreeHFREEntryInfoEntryMemory(HFREEntryInfo);

                  BTPS_MemInitialize(&ConnectionStatusMessage, 0, sizeof(ConnectionStatusMessage));

                  ConnectionStatusMessage.MessageHeader.AddressID       = ClientID;
                  ConnectionStatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  ConnectionStatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
                  ConnectionStatusMessage.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS;
                  ConnectionStatusMessage.MessageHeader.MessageLength   = (HFRM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  ConnectionStatusMessage.ConnectionType                = ConnectionEntry.ConnectionType;
                  ConnectionStatusMessage.RemoteDeviceAddress           = ConnectionEntry.BD_ADDR;

                  /* Map the open confirmation error to the correct     */
                  /* Hands Free Manager error status.                   */
                  switch(OpenPortConfirmationData->PortOpenStatus)
                  {
                     case HFRE_OPEN_PORT_STATUS_SUCCESS:
                        ConnectionStatusMessage.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_SUCCESS;
                        break;
                     case HFRE_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
                        ConnectionStatusMessage.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                        break;
                     case HFRE_OPEN_PORT_STATUS_CONNECTION_REFUSED:
                        ConnectionStatusMessage.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                        break;
                     default:
                        ConnectionStatusMessage.ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        break;
                  }

                  /* Finally dispatch the Message.                      */
                  MSG_SendMessage((BTPM_Message_t *)&ConnectionStatusMessage);
               }

               /* Break out of the loop.                                */
               HFREEntryInfo = NULL;
            }
            else
               HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
         }

         /* Next, format up the Event to dispatch - ONLY if we need to  */
         /* dispatch a connected event.                                 */
         if((!OpenPortConfirmationData->PortOpenStatus) && (DispatchOpen))
         {
            HFRMEventData.EventType                                        = hetHFRConnected;
            HFRMEventData.EventLength                                      = HFRM_CONNECTED_EVENT_DATA_SIZE;

            HFRMEventData.EventData.ConnectedEventData.ConnectionType      = ConnectionEntry.ConnectionType;
            HFRMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry.BD_ADDR;

            /* Next, format up the message to dispatch.                 */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = 0;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DEVICE_CONNECTED;
            Message.MessageHeader.MessageLength   = (HFRM_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.ConnectionType                = ConnectionEntry.ConnectionType;
            Message.RemoteDeviceAddress           = ConnectionEntry.BD_ADDR;

            /* Finally dispatch the formatted event and message.        */
            DispatchHFREEvent(FALSE, ConnectionEntry.ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
         }
      }
      else
      {
         /* We are not tracking this connection, so we have no way to   */
         /* know what the BD_ADDR of the device is.  Let's go ahead and */
         /* disconnect the connection.                                  */
         _HFRM_Disconnect_Device(OpenPortConfirmationData->HFREPortID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* close port indication event that has been received with the       */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessCloseIndicationEvent(HFRE_Close_Port_Indication_Data_t *ClosePortIndicationData)
{
   HFRM_Event_Data_t                   HFRMEventData;
   Connection_Entry_t                 *ConnectionEntry;
   HFRM_Device_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ClosePortIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = DeleteConnectionEntryHFREID(&ConnectionEntryList, ClosePortIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                           = hetHFRDisconnected;
         HFRMEventData.EventLength                                         = HFRM_DISCONNECTED_EVENT_DATA_SIZE;

         HFRMEventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.DisconnectedEventData.DisconnectReason    = ClosePortIndicationData->PortCloseStatus;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HFRM_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.DisconnectReason              = ClosePortIndicationData->PortCloseStatus;

         /* Make sure the device is deleted from the SCO connection list*/
         /* (in case it was added).                                     */
         SCOM_DeleteConnectionFromIgnoreList(ConnectionEntry->BD_ADDR);

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(FALSE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);

         /* Free the connection information.                            */
         FreeConnectionEntryMemory(ConnectionEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* service level connection indication event that has been received  */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessServiceLevelIndicationEvent(HFRE_Open_Service_Level_Connection_Indication_Data_t *ServiceLevelConnectionIndicationData)
{
   HFRM_Event_Data_t                        HFRMEventData;
   Connection_Entry_t                      *ConnectionEntry;
   HFRM_Service_Level_Connection_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ServiceLevelConnectionIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ServiceLevelConnectionIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                            = hetHFRServiceLevelConnectionEstablished;
         HFRMEventData.EventLength                                                                          = HFRM_SERVICE_LEVEL_CONNECTION_ESTABLISHED_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.ConnectionType                  = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteDeviceAddress             = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteSupportedFeaturesValid    = ServiceLevelConnectionIndicationData->RemoteSupportedFeaturesValid;
         HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteSupportedFeatures         = ServiceLevelConnectionIndicationData->RemoteSupportedFeatures;
         HFRMEventData.EventData.ServiceLevelConnectionEstablishedEventData.RemoteCallHoldMultipartySupport = ServiceLevelConnectionIndicationData->RemoteCallHoldMultipartySupport;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID         = 0;
         Message.MessageHeader.MessageID         = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction   = HFRM_MESSAGE_FUNCTION_SERVICE_LEVEL_CONNECTION;
         Message.MessageHeader.MessageLength     = (HFRM_SERVICE_LEVEL_CONNECTION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                  = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress             = ConnectionEntry->BD_ADDR;
         Message.RemoteSupportedFeaturesValid    = ServiceLevelConnectionIndicationData->RemoteSupportedFeaturesValid;
         Message.RemoteSupportedFeatures         = ServiceLevelConnectionIndicationData->RemoteSupportedFeatures;
         Message.RemoteCallHoldMultipartySupport = ServiceLevelConnectionIndicationData->RemoteCallHoldMultipartySupport;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(FALSE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* control indicator status indication event that has been received  */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessControlIndicatorStatusIndicationEvent(HFRE_Control_Indicator_Status_Indication_Data_t *ControlIndicatorStatusIndicationData)
{
   unsigned int                                        Temp;
   HFRM_Event_Data_t                                   HFRMEventData;
   Connection_Entry_t                                 *ConnectionEntry;
   HFRM_Control_Indicator_Status_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ControlIndicatorStatusIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ControlIndicatorStatusIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                                      = hetHFRControlIndicatorStatusIndication;
         HFRMEventData.EventLength                                                                                    = HFRM_CONTROL_INDICATOR_STATUS_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.RemoteDeviceAddress                        = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.IndicatorDescription = ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.IndicatorDescription;
         HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.ControlIndicatorType = ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.ControlIndicatorType;

         /* Copy the control indicator data.                            */
         BTPS_MemCopy(&(HFRMEventData.EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.Control_Indicator_Data), &(ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.Control_Indicator_Data), sizeof(ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.Control_Indicator_Data));

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID                    = 0;
         Message.MessageHeader.MessageID                    = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup                 = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction              = HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_IND;
         Message.MessageHeader.MessageLength                = (HFRM_CONTROL_INDICATOR_STATUS_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress                        = ConnectionEntry->BD_ADDR;
         Message.ControlIndicatorEntry.ControlIndicatorType = ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.ControlIndicatorType;

         /* Copy the control indicator data.                            */
         BTPS_MemCopy(&(Message.ControlIndicatorEntry.Control_Indicator_Data), &(ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.Control_Indicator_Data), sizeof(ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.Control_Indicator_Data));

         /* Copy the Indicator description.                             */
         /* * NOTE * We need to guard against an indicator description  */
         /*          that is larger than our buffer.                    */
         if(ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.IndicatorDescription)
         {
            Temp = BTPS_StringLength(ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.IndicatorDescription);

            if(Temp > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
               Temp = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

            if(Temp)
               BTPS_MemCopy(Message.ControlIndicatorEntry.IndicatorDescription, ControlIndicatorStatusIndicationData->HFREControlIndicatorEntry.IndicatorDescription, Temp);
         }

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* control indicator status confimation event that has been received */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessControlIndicatorStatusConfirmationEvent(HFRE_Control_Indicator_Status_Confirmation_Data_t *ControlIndicatorStatusConfirmationData)
{
   unsigned int                                          Temp;
   HFRM_Event_Data_t                                     HFRMEventData;
   Connection_Entry_t                                   *ConnectionEntry;
   HFRM_Control_Indicator_Status_Confirmation_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ControlIndicatorStatusConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ControlIndicatorStatusConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                                        = hetHFRControlIndicatorStatusConfirmation;
         HFRMEventData.EventLength                                                                                      = HFRM_CONTROL_INDICATOR_STATUS_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.RemoteDeviceAddress                        = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.IndicatorDescription = ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.IndicatorDescription;
         HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.ControlIndicatorType = ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.ControlIndicatorType;

         /* Copy the control indicator data.                            */
         BTPS_MemCopy(&(HFRMEventData.EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.Control_Indicator_Data), &(ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.Control_Indicator_Data), sizeof(ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.Control_Indicator_Data));

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID                    = 0;
         Message.MessageHeader.MessageID                    = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup                 = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction              = HFRM_MESSAGE_FUNCTION_CONTROL_INDICATOR_STATUS_CFM;
         Message.MessageHeader.MessageLength                = (HFRM_CONTROL_INDICATOR_STATUS_CONFIRMATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress                        = ConnectionEntry->BD_ADDR;
         Message.ControlIndicatorEntry.ControlIndicatorType = ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.ControlIndicatorType;

         /* Copy the control indicator data.                            */
         BTPS_MemCopy(&(Message.ControlIndicatorEntry.Control_Indicator_Data), &(ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.Control_Indicator_Data), sizeof(ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.Control_Indicator_Data));

         /* Copy the Indicator description.                             */
         /* * NOTE * We need to guard against an indicator description  */
         /*          that is larger than our buffer.                    */
         if(ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.IndicatorDescription)
         {
            Temp = BTPS_StringLength(ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.IndicatorDescription);

            if(Temp > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
               Temp = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

            if(Temp)
               BTPS_MemCopy(Message.ControlIndicatorEntry.IndicatorDescription, ControlIndicatorStatusConfirmationData->HFREControlIndicatorEntry.IndicatorDescription, Temp);
         }

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* call hold/multi-party support confimation event that has been     */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessCallHoldMultipartySupportConfirmationEvent(HFRE_Call_Hold_Multiparty_Support_Confirmation_Data_t *CallHoldMultipartySupportConfirmationData)
{
   HFRM_Event_Data_t                                         HFRMEventData;
   Connection_Entry_t                                       *ConnectionEntry;
   HFRM_Call_Hold_Multiparty_Support_Confirmation_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CallHoldMultipartySupportConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CallHoldMultipartySupportConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                    = hetHFRCallHoldMultipartySupportConfirmation;
         HFRMEventData.EventLength                                                                  = HFRM_CALL_HOLD_MULTIPARTY_SUPPORT_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         if(CallHoldMultipartySupportConfirmationData->CallHoldSupportMask == HFRE_CALL_HOLD_MULTIPARTY_SUPPORTED_FEATURES_ERROR)
         {
            HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMaskValid = FALSE;
            HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMask      = 0;
         }
         else
         {
            HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMaskValid = TRUE;
            HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMask      = CallHoldMultipartySupportConfirmationData->CallHoldSupportMask;
         }

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SUPPORT_CFM;
         Message.MessageHeader.MessageLength   = (HFRM_CALL_HOLD_MULTIPARTY_SUPPORT_CONFIRMATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.CallHoldSupportMaskValid      = HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMaskValid;
         Message.CallHoldSupportMask           = HFRMEventData.EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMask;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* call hold/multi-party selection indication event that has been    */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessCallHoldMultipartySelectionIndicationEvent(HFRE_Call_Hold_Multiparty_Selection_Indication_Data_t *CallHoldMultipartySelectionIndicationData)
{
   HFRM_Event_Data_t                                         HFRMEventData;
   Connection_Entry_t                                       *ConnectionEntry;
   HFRM_Call_Hold_Multiparty_Selection_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CallHoldMultipartySelectionIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CallHoldMultipartySelectionIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                           = hetHFRCallHoldMultipartySelectionIndication;
         HFRMEventData.EventLength                                                                         = HFRM_CALL_HOLD_MULTIPARTY_SELECTION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CallHoldMultipartySelectionIndicationEventData.RemoteDeviceAddress        = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CallHoldMultipartySelectionIndicationEventData.CallHoldMultipartyHandling = CallHoldMultipartySelectionIndicationData->CallHoldMultipartyHandling;
         HFRMEventData.EventData.CallHoldMultipartySelectionIndicationEventData.Index                      = CallHoldMultipartySelectionIndicationData->Index;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CALL_HOLD_MULTI_SELECTION_IND;
         Message.MessageHeader.MessageLength   = (HFRM_CALL_HOLD_MULTIPARTY_SELECTION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.CallHoldMultipartyHandling    = CallHoldMultipartySelectionIndicationData->CallHoldMultipartyHandling;
         Message.Index                         = CallHoldMultipartySelectionIndicationData->Index;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* call waiting notification activation indication event that has    */
   /* been received with the specified information.  This function      */
   /* should be called with the Lock protecting the Hands Free Manager  */
   /* information held.                                                 */
static void ProcessCallWaitingNotificationActivationIndicationEvent(HFRE_Call_Waiting_Notification_Activation_Indication_Data_t *CallWaitingNotificationActivationIndicationData)
{
   HFRM_Event_Data_t                                               HFRMEventData;
   Connection_Entry_t                                             *ConnectionEntry;
   HFRM_Call_Waiting_Notification_Activation_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CallWaitingNotificationActivationIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CallWaitingNotificationActivationIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                          = hetHFRCallWaitingNotificationActivationIndication;
         HFRMEventData.EventLength                                                                        = HFRM_CALL_WAITING_NOTIFICATION_ACTIVATION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CallWaitingNotificationActivationIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CallWaitingNotificationActivationIndicationEventData.Enabled             = CallWaitingNotificationActivationIndicationData->Enabled;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOT_ACTIVATION_IND;
         Message.MessageHeader.MessageLength   = (HFRM_CALL_WAITING_NOTIFICATION_ACTIVATION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.Enabled                       = CallWaitingNotificationActivationIndicationData->Enabled;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* call waiting notification indication event that has been received */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessCallWaitingNotificationIndicationEvent(HFRE_Call_Waiting_Notification_Indication_Data_t *CallWaitingNotificationIndicationData)
{
   unsigned int                                         Temp;
   HFRM_Event_Data_t                                    HFRMEventData;
   Connection_Entry_t                                  *ConnectionEntry;
   HFRM_Call_Waiting_Notification_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CallWaitingNotificationIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CallWaitingNotificationIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                = hetHFRCallWaitingNotificationIndication;
         HFRMEventData.EventLength                                                              = HFRM_CALL_WAITING_NOTIFICATION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CallWaitingNotificationIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CallWaitingNotificationIndicationEventData.PhoneNumber         = CallWaitingNotificationIndicationData->PhoneNumber;

         /* Next, format up the Message to dispatch.                    */
         /* * NOTE * We will take care to guard against phone numbers   */
         /*          that are longer than we can process.               */
         if(CallWaitingNotificationIndicationData->PhoneNumber)
            Temp = BTPS_StringLength(CallWaitingNotificationIndicationData->PhoneNumber) + 1;
         else
            Temp = 1;

         if(Temp > HFRE_PHONE_NUMBER_LENGTH_MAXIMUM)
            Temp = HFRE_PHONE_NUMBER_LENGTH_MAXIMUM;

         if((Message = (HFRM_Call_Waiting_Notification_Indication_Message_t *)BTPS_AllocateMemory(HFRM_CALL_WAITING_NOTIFICATION_INDICATION_MESSAGE_SIZE(Temp))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_CALL_WAITING_NOTIFICATION_INDICATION_MESSAGE_SIZE(Temp));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CALL_WAIT_NOTIFICATION_IND;
            Message->MessageHeader.MessageLength   = (HFRM_CALL_WAITING_NOTIFICATION_INDICATION_MESSAGE_SIZE(Temp) - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->PhoneNumberLength             = Temp;

            if(Temp > 1)
               BTPS_MemCopy(Message->PhoneNumber, CallWaitingNotificationIndicationData->PhoneNumber, Temp - 1);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* call line identification notification activation indication event */
   /* that has been received with the specified information.  This      */
   /* function should be called with the Lock protecting the Hands Free */
   /* Manager information held.                                         */
static void ProcessCallLineIdentificationNotificationActivationIndicationEvent(HFRE_Call_Line_Identification_Notification_Activation_Indication_Data_t *CallLineIdentificationNotificationActivationIndicationData)
{
   HFRM_Event_Data_t                                                           HFRMEventData;
   Connection_Entry_t                                                         *ConnectionEntry;
   HFRM_Call_Line_Identification_Notification_Activation_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CallLineIdentificationNotificationActivationIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CallLineIdentificationNotificationActivationIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                                     = hetHFRCallLineIdentificationNotificationActivationIndication;
         HFRMEventData.EventLength                                                                                   = HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_ACTIVATION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CallLineIdentificationNotificationActivationIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CallLineIdentificationNotificationActivationIndicationEventData.Enabled             = CallLineIdentificationNotificationActivationIndicationData->Enabled;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOT_ACTIVATION_IND;
         Message.MessageHeader.MessageLength   = (HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_ACTIVATION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.Enabled                       = CallLineIdentificationNotificationActivationIndicationData->Enabled;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* call line identification notification indication event that has   */
   /* been received with the specified information.  This function      */
   /* should be called with the Lock protecting the Hands Free Manager  */
   /* information held.                                                 */
static void ProcessCallLineIdentificationNotificationIndicationEvent(HFRE_Call_Line_Identification_Notification_Indication_Data_t *CallLineIdentificationNotificationIndicationData)
{
   unsigned int                                                     Temp;
   HFRM_Event_Data_t                                                HFRMEventData;
   Connection_Entry_t                                              *ConnectionEntry;
   HFRM_Call_Line_Identification_Notification_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CallLineIdentificationNotificationIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CallLineIdentificationNotificationIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                           = hetHFRCallLineIdentificationNotificationIndication;
         HFRMEventData.EventLength                                                                         = HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CallLineIdentificationNotificationIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CallLineIdentificationNotificationIndicationEventData.PhoneNumber         = CallLineIdentificationNotificationIndicationData->PhoneNumber;

         /* Next, format up the Message to dispatch.                    */
         /* * NOTE * We will take care to guard against phone numbers   */
         /*          that are longer than we can process.               */
         if(CallLineIdentificationNotificationIndicationData->PhoneNumber)
            Temp = BTPS_StringLength(CallLineIdentificationNotificationIndicationData->PhoneNumber) + 1;
         else
            Temp = 1;

         if(Temp > HFRE_PHONE_NUMBER_LENGTH_MAXIMUM)
            Temp = HFRE_PHONE_NUMBER_LENGTH_MAXIMUM;

         if((Message = (HFRM_Call_Line_Identification_Notification_Indication_Message_t *)BTPS_AllocateMemory(HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_MESSAGE_SIZE(Temp))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_MESSAGE_SIZE(Temp));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CALL_LINE_ID_NOTIFICATION_IND;
            Message->MessageHeader.MessageLength   = (HFRM_CALL_LINE_IDENTIFICATION_NOTIFICATION_INDICATION_MESSAGE_SIZE(Temp) - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->PhoneNumberLength             = Temp;

            if(Temp > 1)
               BTPS_MemCopy(Message->PhoneNumber, CallLineIdentificationNotificationIndicationData->PhoneNumber, Temp - 1);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* disable sound enhancement indication event that has been received */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessDisableSoundEnhancementIndicationEvent(HFRE_Disable_Sound_Enhancement_Indication_Data_t *DisableSoundEnhancementIndicationData)
{
   HFRM_Event_Data_t                                    HFRMEventData;
   Connection_Entry_t                                  *ConnectionEntry;
   HFRM_Disable_Sound_Enhancement_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(DisableSoundEnhancementIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, DisableSoundEnhancementIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                = hetHFRDisableSoundEnhancementIndication;
         HFRMEventData.EventLength                                                              = HFRM_DISABLE_SOUND_ENHANCEMENT_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.DisableSoundEnhancementIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DISABLE_SOUND_ENHANCEMENT_IND;
         Message.MessageHeader.MessageLength   = (HFRM_DISABLE_SOUND_ENHANCEMENT_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* dial phone number indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessDialPhoneNumberIndicationEvent(HFRE_Dial_Phone_Number_Indication_Data_t *DialPhoneNumberIndicationData)
{
   unsigned int                                 Temp;
   HFRM_Event_Data_t                            HFRMEventData;
   Connection_Entry_t                          *ConnectionEntry;
   HFRM_Dial_Phone_Number_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(DialPhoneNumberIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, DialPhoneNumberIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                        = hetHFRDialPhoneNumberIndication;
         HFRMEventData.EventLength                                                      = HFRM_DIAL_PHONE_NUMBER_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.DialPhoneNumberIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.DialPhoneNumberIndicationEventData.PhoneNumber         = DialPhoneNumberIndicationData->PhoneNumber;

         /* Next, format up the Message to dispatch.                    */
         /* * NOTE * We will take care to guard against phone numbers   */
         /*          that are longer than we can process.               */
         if(DialPhoneNumberIndicationData->PhoneNumber)
            Temp = BTPS_StringLength(DialPhoneNumberIndicationData->PhoneNumber) + 1;
         else
            Temp = 1;

         if(Temp > HFRE_PHONE_NUMBER_LENGTH_MAXIMUM)
            Temp = HFRE_PHONE_NUMBER_LENGTH_MAXIMUM;

         if((Message = (HFRM_Dial_Phone_Number_Indication_Message_t *)BTPS_AllocateMemory(HFRM_DIAL_PHONE_NUMBER_INDICATION_MESSAGE_SIZE(Temp))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, (Temp));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_IND;
            Message->MessageHeader.MessageLength   = (HFRM_DIAL_PHONE_NUMBER_INDICATION_MESSAGE_SIZE(Temp) - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->PhoneNumberLength             = Temp;

            if(Temp > 1)
               BTPS_MemCopy(Message->PhoneNumber, DialPhoneNumberIndicationData->PhoneNumber, Temp - 1);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* dial phone number (from memory) indication event that has been    */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessDialPhoneNumberFromMemoryIndicationEvent(HFRE_Dial_Phone_Number_From_Memory_Indication_Data_t *DialPhoneNumberFromMemoryIndicationData)
{
   HFRM_Event_Data_t                             HFRMEventData;
   Connection_Entry_t                           *ConnectionEntry;
   HFRM_Dial_Phone_Number_From_Memory_Request_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(DialPhoneNumberFromMemoryIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, DialPhoneNumberFromMemoryIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                  = hetHFRDialPhoneNumberFromMemoryIndication;
         HFRMEventData.EventLength                                                                = HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.DialPhoneNumberFromMemoryIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.DialPhoneNumberFromMemoryIndicationEventData.MemoryLocation      = DialPhoneNumberFromMemoryIndicationData->MemoryLocation;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEM_IND;
         Message.MessageHeader.MessageLength   = (HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.MemoryLocation                = DialPhoneNumberFromMemoryIndicationData->MemoryLocation;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* re-dial last phone number indication event that has been received */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessReDialLastPhoneNumberIndicationEvent(HFRE_ReDial_Last_Phone_Number_Indication_Data_t *ReDialLastPhoneNumberIndicationData)
{
   HFRM_Event_Data_t                                    HFRMEventData;
   Connection_Entry_t                                  *ConnectionEntry;
   HFRM_Re_Dial_Last_Phone_Number_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ReDialLastPhoneNumberIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ReDialLastPhoneNumberIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                              = hetHFRReDialLastPhoneNumberIndication;
         HFRMEventData.EventLength                                                            = HFRM_RE_DIAL_LAST_PHONE_NUMBER_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ReDialLastPhoneNumberIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER_IND;
         Message.MessageHeader.MessageLength   = (HFRM_RE_DIAL_LAST_PHONE_NUMBER_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* ring indication event that has been received with the specified   */
   /* information.  This function should be called with the Lock        */
   /* protecting the Hands Free Manager information held.               */
static void ProcessRingIndicationEvent(HFRE_Ring_Indication_Data_t *RingIndicationData)
{
   HFRM_Event_Data_t                          HFRMEventData;
   Connection_Entry_t                        *ConnectionEntry;
   HFRM_Ring_Indication_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(RingIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, RingIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                             = hetHFRRingIndication;
         HFRMEventData.EventLength                                           = HFRM_RING_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.RingIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_RING_INDICATION_IND;
         Message.MessageHeader.MessageLength   = (HFRM_RING_INDICATION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* generate DTMF tone indication event that has been received with   */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessGenerateDTMFToneIndicationEvent(HFRE_Generate_DTMF_Tone_Indication_Data_t *GenerateDTMFToneIndicationData)
{
   HFRM_Event_Data_t                             HFRMEventData;
   Connection_Entry_t                           *ConnectionEntry;
   HFRM_Generate_DTMF_Code_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(GenerateDTMFToneIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, GenerateDTMFToneIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                         = hetHFRGenerateDTMFCodeIndication;
         HFRMEventData.EventLength                                                       = HFRM_GENERATE_DTMF_CODE_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.GenerateDTMFCodeIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.GenerateDTMFCodeIndicationEventData.DTMFCode            = GenerateDTMFToneIndicationData->DTMFCode;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_GENERATE_DTMF_CODE_IND;
         Message.MessageHeader.MessageLength   = (HFRM_GENERATE_DTMF_CODE_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.DTMFCode                      = GenerateDTMFToneIndicationData->DTMFCode;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* answer call indication event that has been received with the      */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessAnswerCallIndicationEvent(HFRE_Answer_Call_Indication_Data_t *AnswerCallIndicationData)
{
   HFRM_Event_Data_t                      HFRMEventData;
   Connection_Entry_t                    *ConnectionEntry;
   HFRM_Answer_Call_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(AnswerCallIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, AnswerCallIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                   = hetHFRAnswerCallIndication;
         HFRMEventData.EventLength                                                 = HFRM_ANSWER_CALL_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.AnswerCallIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ANSWER_CALL_IND;
         Message.MessageHeader.MessageLength   = (HFRM_ANSWER_CALL_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* in-band ring tone setting indication event that has been received */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessInBandRingToneSettingIndicationEvent(HFRE_InBand_Ring_Tone_Setting_Indication_Data_t *InBandRingToneSettingIndicationData)
{
   HFRM_Event_Data_t                                    HFRMEventData;
   Connection_Entry_t                                  *ConnectionEntry;
   HFRM_In_Band_Ring_Tone_Setting_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(InBandRingToneSettingIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, InBandRingToneSettingIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                              = hetHFRInBandRingToneSettingIndication;
         HFRMEventData.EventLength                                                            = HFRM_IN_BAND_RING_TONE_SETTING_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.InBandRingToneSettingIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.InBandRingToneSettingIndicationEventData.Enabled             = InBandRingToneSettingIndicationData->Enabled;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_IN_BAND_RING_TONE_SETTING_IND;
         Message.MessageHeader.MessageLength   = (HFRM_IN_BAND_RING_TONE_SETTING_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.Enabled                       = InBandRingToneSettingIndicationData->Enabled;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* voice recognition notification indication event that has been     */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessVoiceRecognitionNotificationIndicationEvent(HFRE_Voice_Recognition_Notification_Indication_Data_t *VoiceRecognitionNotificationIndicationData)
{
   HFRM_Event_Data_t                            HFRMEventData;
   Connection_Entry_t                          *ConnectionEntry;
   HFRM_Voice_Recognition_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(VoiceRecognitionNotificationIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, VoiceRecognitionNotificationIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                            = hetHFRVoiceRecognitionIndication;
         HFRMEventData.EventLength                                                          = HFRM_VOICE_RECOGNITION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.VoiceRecognitionIndicationEventData.ConnectionType         = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.VoiceRecognitionIndicationEventData.RemoteDeviceAddress    = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.VoiceRecognitionIndicationEventData.VoiceRecognitionActive = VoiceRecognitionNotificationIndicationData->VoiceRecognitionActive;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_VOICE_RECOGNITION_IND;
         Message.MessageHeader.MessageLength   = (HFRM_VOICE_RECOGNITION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.VoiceRecognitionActive        = VoiceRecognitionNotificationIndicationData->VoiceRecognitionActive;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* speaker gain indication event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessSpeakerGainIndicationEvent(HFRE_Speaker_Gain_Indication_Data_t *SpeakerGainIndicationData)
{
   HFRM_Event_Data_t                       HFRMEventData;
   Connection_Entry_t                     *ConnectionEntry;
   HFRM_Speaker_Gain_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SpeakerGainIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, SpeakerGainIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                    = hetHFRSpeakerGainIndication;
         HFRMEventData.EventLength                                                  = HFRM_SPEAKER_GAIN_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.SpeakerGainIndicationEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.SpeakerGainIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.SpeakerGainIndicationEventData.SpeakerGain         = SpeakerGainIndicationData->SpeakerGain;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SPEAKER_GAIN_IND;
         Message.MessageHeader.MessageLength   = (HFRM_SPEAKER_GAIN_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.SpeakerGain                   = SpeakerGainIndicationData->SpeakerGain;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* microphone gain indication event that has been received with the  */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessMicrophoneGainIndicationEvent(HFRE_Microphone_Gain_Indication_Data_t *MicrophoneGainIndicationData)
{
   HFRM_Event_Data_t                          HFRMEventData;
   Connection_Entry_t                        *ConnectionEntry;
   HFRM_Microphone_Gain_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(MicrophoneGainIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, MicrophoneGainIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                       = hetHFRMicrophoneGainIndication;
         HFRMEventData.EventLength                                                     = HFRM_MICROPHONE_GAIN_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.MicrophoneGainIndicationEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.MicrophoneGainIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.MicrophoneGainIndicationEventData.MicrophoneGain      = MicrophoneGainIndicationData->MicrophoneGain;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_MICROPHONE_GAIN_IND;
         Message.MessageHeader.MessageLength   = (HFRM_MICROPHONE_GAIN_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.MicrophoneGain                = MicrophoneGainIndicationData->MicrophoneGain;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* voice tag request indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessVoiceTagRequestIndicationEvent(HFRE_Voice_Tag_Request_Indication_Data_t *VoiceTagRequestIndicationData)
{
   HFRM_Event_Data_t                            HFRMEventData;
   Connection_Entry_t                          *ConnectionEntry;
   HFRM_Voice_Tag_Request_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(VoiceTagRequestIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, VoiceTagRequestIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                        = hetHFRVoiceTagRequestIndication;
         HFRMEventData.EventLength                                                      = HFRM_VOICE_TAG_REQUEST_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.VoiceTagRequestIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_IND;
         Message.MessageHeader.MessageLength   = (HFRM_VOICE_TAG_REQUEST_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* voice tag request confirmation event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessVoiceTagRequestConfirmationEvent(HFRE_Voice_Tag_Request_Confirmation_Data_t *VoiceTagRequestConfirmationData)
{
   unsigned int                                   Temp;
   HFRM_Event_Data_t                              HFRMEventData;
   Connection_Entry_t                            *ConnectionEntry;
   HFRM_Voice_Tag_Request_Confirmation_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(VoiceTagRequestConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, VoiceTagRequestConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                          = hetHFRVoiceTagRequestConfirmation;
         HFRMEventData.EventLength                                                        = HFRM_VOICE_TAG_REQUEST_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.VoiceTagRequestConfirmationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.VoiceTagRequestConfirmationEventData.PhoneNumber         = VoiceTagRequestConfirmationData->PhoneNumber;

         /* Next, format up the Message to dispatch.                    */
         /* * NOTE * We will take care to guard against phone numbers   */
         /*          that are longer than we can process.               */
         if(VoiceTagRequestConfirmationData->PhoneNumber)
            Temp = BTPS_StringLength(VoiceTagRequestConfirmationData->PhoneNumber) + 1;
         else
            Temp = 1;

         if(Temp > HFRE_PHONE_NUMBER_LENGTH_MAXIMUM)
            Temp = HFRE_PHONE_NUMBER_LENGTH_MAXIMUM;

         if((Message = (HFRM_Voice_Tag_Request_Confirmation_Message_t *)BTPS_AllocateMemory(HFRM_VOICE_TAG_REQUEST_CONFIRMATION_MESSAGE_SIZE(Temp))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, (Temp));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST_CFM;
            Message->MessageHeader.MessageLength   = (HFRM_VOICE_TAG_REQUEST_CONFIRMATION_MESSAGE_SIZE(Temp) - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->PhoneNumberLength             = Temp;

            if(Temp > 1)
               BTPS_MemCopy(Message->PhoneNumber, VoiceTagRequestConfirmationData->PhoneNumber, Temp - 1);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* hang-up indication event that has been received with the specified*/
   /* information.  This function should be called with the Lock        */
   /* protecting the Hands Free Manager information held.               */
static void ProcessHangUpIndicationEvent(HFRE_Hang_Up_Indication_Data_t *HangUpIndicationData)
{
   HFRM_Event_Data_t                  HFRMEventData;
   Connection_Entry_t                *ConnectionEntry;
   HFRM_Hang_Up_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(HangUpIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, HangUpIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                               = hetHFRHangUpIndication;
         HFRMEventData.EventLength                                             = HFRM_HANG_UP_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.HangUpIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_HANG_UP_IND;
         Message.MessageHeader.MessageLength   = (HFRM_HANG_UP_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* audio connection indication event that has been received with the */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessAudioConnectionIndicationEvent(HFRE_Audio_Connection_Indication_Data_t *AudioConnectionIndicationData)
{
   HFRM_Event_Data_t                       HFRMEventData;
   Connection_Entry_t                     *ConnectionEntry;
   HFRM_Audio_Connected_Message_t          Message;
   HFRM_Audio_Connection_Status_Message_t  StatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(AudioConnectionIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, AudioConnectionIndicationData->HFREPortID)) != NULL)
      {
         /* First send the status event if we create the attempt locally */
         if(ConnectionEntry->OutgoingSCOAttempt)
         {
            /* Format the status event.                                 */
            HFRMEventData.EventType                                                    = hetHFRAudioConnectionStatus;
            HFRMEventData.EventLength                                                  = HFRM_AUDIO_CONNECTION_STATUS_EVENT_DATA_SIZE;

            HFRMEventData.EventData.AudioConnectionStatusEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
            HFRMEventData.EventData.AudioConnectionStatusEventData.ConnectionType      = ConnectionEntry->ConnectionType;
            HFRMEventData.EventData.AudioConnectionStatusEventData.Successful          = (AudioConnectionIndicationData->AudioConnectionOpenStatus == HFRE_AUDIO_CONNECTION_STATUS_SUCCESS)?TRUE:FALSE;

            /* Format the status Message.                               */
            StatusMessage.MessageHeader.AddressID       = 0;
            StatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
            StatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            StatusMessage.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTION_STATUS;
            StatusMessage.MessageHeader.MessageLength   = (HFRM_AUDIO_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            StatusMessage.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
            StatusMessage.ConnectionType      = ConnectionEntry->ConnectionType;
            StatusMessage.Successful          = (AudioConnectionIndicationData->AudioConnectionOpenStatus == HFRE_AUDIO_CONNECTION_STATUS_SUCCESS)?TRUE:FALSE;

            /* Note we no longer have an outgoing attempt.              */
            ConnectionEntry->OutgoingSCOAttempt = FALSE;

            /* Finally dispatch the formatted Event and Message to the  */
            /* Control callback, since only the Control can setup the   */
            /* audio connection.                                        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&StatusMessage);
         }
         /* Now dispatch a connected event if we were successful.       */
         if(AudioConnectionIndicationData->AudioConnectionOpenStatus == HFRE_AUDIO_CONNECTION_STATUS_SUCCESS)
         {
            /* Note the SCO Handle.                                     */
            ConnectionEntry->SCOHandle = AudioConnectionIndicationData->SCO_Connection_Handle;

            /* Next, format up the Event to dispatch.                   */
            HFRMEventData.EventType                                             = hetHFRAudioConnected;
            HFRMEventData.EventLength                                           = HFRM_AUDIO_CONNECTED_EVENT_DATA_SIZE;

            HFRMEventData.EventData.AudioConnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
            HFRMEventData.EventData.AudioConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

            /* Next, format up the Message to dispatch.                 */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = 0;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_AUDIO_CONNECTED;
            Message.MessageHeader.MessageLength   = (HFRM_AUDIO_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.ConnectionType                = ConnectionEntry->ConnectionType;
            Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

            /* Finally dispatch the formatted Event and Message.           */
            DispatchHFREEvent(FALSE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* audio disconnection indication event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessAudioDisconnectionIndicationEvent(HFRE_Audio_Disconnection_Indication_Data_t *AudioDisconnectionIndicationData)
{
   HFRM_Event_Data_t                  HFRMEventData;
   Connection_Entry_t                *ConnectionEntry;
   HFRM_Audio_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(AudioDisconnectionIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, AudioDisconnectionIndicationData->HFREPortID)) != NULL)
      {
         /* Clear the SCO Handle.                                       */
         ConnectionEntry->SCOHandle = 0;

         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                = hetHFRAudioDisconnected;
         HFRMEventData.EventLength                                              = HFRM_AUDIO_DISCONNECTED_EVENT_DATA_SIZE;

         HFRMEventData.EventData.AudioDisconnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.AudioDisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_AUDIO_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HFRM_AUDIO_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(FALSE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* audio data indication event that has been received with the       */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessAudioDataIndicationEvent(HFRE_Audio_Data_Indication_Data_t *AudioDataIndicationData)
{
   HFRM_Event_Data_t                   HFRMEventData;
   Connection_Entry_t                 *ConnectionEntry;
   HFRM_Audio_Data_Received_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((AudioDataIndicationData) && (AudioDataIndicationData->AudioDataLength))
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, AudioDataIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                        = hetHFRAudioData;
         HFRMEventData.EventLength                                      = HFRM_AUDIO_DATA_EVENT_DATA_SIZE;

         HFRMEventData.EventData.AudioDataEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.AudioDataEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.AudioDataEventData.AudioDataLength     = AudioDataIndicationData->AudioDataLength;

         /* Set the appropriate event flags.                            */
         switch(AudioDataIndicationData->PacketStatus & HCI_SCO_FLAGS_PACKET_STATUS_MASK_MASK)
         {
            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_CORRECTLY_RECEIVED_DATA:
               HFRMEventData.EventData.AudioDataEventData.AudioDataFlags = HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_CORRECTLY_RECEIVED_DATA;
               break;

            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_POSSIBLY_INVALID_DATA:
               HFRMEventData.EventData.AudioDataEventData.AudioDataFlags = HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_POSSIBLY_INVALID_DATA;
               break;

            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_NO_DATA_RECEIVED:
               HFRMEventData.EventData.AudioDataEventData.AudioDataFlags = HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_NO_DATA_RECEIVED;
               break;

            case HCI_SCO_FLAGS_PACKET_STATUS_MASK_DATA_PARTIALLY_LOST:
               HFRMEventData.EventData.AudioDataEventData.AudioDataFlags = HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_DATA_PARTIALLY_LOST;
               break;

            default:
               HFRMEventData.EventData.AudioDataEventData.AudioDataFlags = HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_NO_DATA_RECEIVED;
               break;
         }

         /* Format up the message to dispatch.                          */
         if((Message = (HFRM_Audio_Data_Received_Message_t *)BTPS_AllocateMemory(HFRM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(AudioDataIndicationData->AudioDataLength))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(AudioDataIndicationData->AudioDataLength));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_AUDIO_DATA_RECEIVED;
            Message->MessageHeader.MessageLength   = (HFRM_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(AudioDataIndicationData->AudioDataLength) - BTPM_MESSAGE_HEADER_SIZE);

            Message->ConnectionType                = ConnectionEntry->ConnectionType;
            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->AudioDataFlags                = HFRMEventData.EventData.AudioDataEventData.AudioDataFlags;
            Message->AudioDataLength               = AudioDataIndicationData->AudioDataLength;

            BTPS_MemCopy(Message->AudioData, AudioDataIndicationData->AudioData, AudioDataIndicationData->AudioDataLength);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREAudioDataEvent(ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* current calls list indication event that has been received with   */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessCurrentCallsListIndicationEvent(HFRE_Current_Calls_List_Indication_Data_t *CurrentCallsListIndicationData)
{
   HFRM_Event_Data_t                                   HFRMEventData;
   Connection_Entry_t                                 *ConnectionEntry;
   HFRM_Query_Current_Calls_List_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CurrentCallsListIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CurrentCallsListIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                         = hetHFRCurrentCallsListIndication;
         HFRMEventData.EventLength                                                       = HFRM_CURRENT_CALLS_LIST_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CurrentCallsListIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_IND;
         Message.MessageHeader.MessageLength   = (HFRM_QUERY_CURRENT_CALLS_LIST_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* current calls list confirmation event that has been received with */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessCurrentCallsListConfirmationEvent(HFRE_Current_Calls_List_Confirmation_Data_t *CurrentCallsListConfirmationData)
{
   unsigned int                                             Temp;
   HFRM_Event_Data_t                                        HFRMEventData;
   Connection_Entry_t                                      *ConnectionEntry;
   union {
      HFRM_Query_Current_Calls_List_Confirmation_Message_v1_t v1;
      HFRM_Query_Current_Calls_List_Confirmation_Message_v2_t v2;
   } Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CurrentCallsListConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CurrentCallsListConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         BTPS_MemInitialize(&HFRMEventData, 0, HFRM_EVENT_DATA_SIZE);

         HFRMEventData.EventType                                                            = hetHFRCurrentCallsListConfirmation;
         HFRMEventData.EventLength                                                          = HFRM_CURRENT_CALLS_LIST_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CurrentCallsListConfirmationEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry = CurrentCallsListConfirmationData->HFRECurrentCallListEntry;

         /* Next, format up the Message to dispatch.                    */
         if(1 /* TODO: Test for v2 message support in client */)
         {
            BTPS_MemInitialize(&Message.v2, 0, HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V2_SIZE);

            Message.v2.MessageHeader.AddressID       = 0;
            Message.v2.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.v2.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message.v2.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V2;
            Message.v2.MessageHeader.MessageLength   = (HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V2_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.v2.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

            Message.v2.CallListEntry.Index         = CurrentCallsListConfirmationData->HFRECurrentCallListEntry.Index;
            Message.v2.CallListEntry.CallDirection = CurrentCallsListConfirmationData->HFRECurrentCallListEntry.CallDirection;
            Message.v2.CallListEntry.CallStatus    = CurrentCallsListConfirmationData->HFRECurrentCallListEntry.CallStatus;
            Message.v2.CallListEntry.CallMode      = CurrentCallsListConfirmationData->HFRECurrentCallListEntry.CallMode;
            Message.v2.CallListEntry.Multiparty    = CurrentCallsListConfirmationData->HFRECurrentCallListEntry.Multiparty;
            Message.v2.CallListEntry.NumberFormat  = CurrentCallsListConfirmationData->HFRECurrentCallListEntry.NumberFormat;

            if(CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhoneNumber != NULL)
            {
               Temp = BTPS_StringLength(CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhoneNumber);

               if(Temp > HFRE_PHONE_NUMBER_LENGTH_MAXIMUM)
                  Temp = HFRE_PHONE_NUMBER_LENGTH_MAXIMUM;

               BTPS_MemCopy(Message.v2.CallListEntry.PhoneNumber, CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhoneNumber, Temp);

               Message.v2.CallListEntry.PhoneNumber[Temp] = '\0';

               Message.v2.CallListEntry.Flags |= HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONE_NUMBER_VALID;
            }

            if(CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhonebookName != NULL)
            {
               Temp = BTPS_StringLength(CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhonebookName);

               if(Temp > HFRE_PHONEBOOK_NAME_LENGTH_MAXIMUM)
                  Temp = HFRE_PHONEBOOK_NAME_LENGTH_MAXIMUM;

               BTPS_MemCopy(Message.v2.CallListEntry.PhonebookName, CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhonebookName, Temp);

               Message.v2.CallListEntry.PhonebookName[Temp] = '\0';

               Message.v2.CallListEntry.Flags |= HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONEBOOK_NAME_VALID;
            }
         }
         else
         {
            BTPS_MemInitialize(&Message.v1, 0, HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V1_SIZE);

            Message.v1.MessageHeader.AddressID       = 0;
            Message.v1.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.v1.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message.v1.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST_CFM_V1;
            Message.v1.MessageHeader.MessageLength   = (HFRM_QUERY_CURRENT_CALLS_LIST_CONFIRMATION_MESSAGE_V1_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            /* * NOTE * We will take care to guard against phone numbers   */
            /*          that are longer than we can process.               */
            if(CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhoneNumber)
               Temp = BTPS_StringLength(CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhoneNumber) + 1;
            else
               Temp = 1;

            if(Temp > HFRE_PHONE_NUMBER_LENGTH_MAXIMUM)
               Temp = HFRE_PHONE_NUMBER_LENGTH_MAXIMUM;

            Message.v1.RemoteDeviceAddress                     = ConnectionEntry->BD_ADDR;
            Message.v1.CallListEntry.CallListEntry             = CurrentCallsListConfirmationData->HFRECurrentCallListEntry;
            Message.v1.CallListEntry.CallListEntry.PhoneNumber = NULL;
            Message.v1.CallListEntry.PhoneNumberLength         = Temp;

            if(Temp > 1)
               BTPS_MemCopy(Message.v1.CallListEntry.PhoneNumber, CurrentCallsListConfirmationData->HFRECurrentCallListEntry.PhoneNumber, Temp - 1);
         }

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* network operator selection format indication event that has been  */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessNetworkOperatorSelectionFormatIndicationEvent(HFRE_Network_Operator_Selection_Format_Indication_Data_t *NetworkOperatorSelectionFormatIndicationData)
{
   HFRM_Event_Data_t                                            HFRMEventData;
   Connection_Entry_t                                          *ConnectionEntry;
   HFRM_Network_Operator_Selection_Format_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(NetworkOperatorSelectionFormatIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, NetworkOperatorSelectionFormatIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                       = hetHFRNetworkOperatorSelectionFormatIndication;
         HFRMEventData.EventLength                                                                     = HFRM_NETWORK_OPERATOR_SELECTION_FORMAT_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.NetworkOperatorSelectionFormatIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.NetworkOperatorSelectionFormatIndicationEventData.Format              = NetworkOperatorSelectionFormatIndicationData->Format;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_FORMAT_IND;
         Message.MessageHeader.MessageLength   = (HFRM_NETWORK_OPERATOR_SELECTION_FORMAT_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.Format                        = NetworkOperatorSelectionFormatIndicationData->Format;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* network operator selection indication event that has been received*/
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessNetworkOperatorSelectionIndicationEvent(HFRE_Network_Operator_Selection_Indication_Data_t *NetworkOperatorSelectionIndicationData)
{
   HFRM_Event_Data_t                                     HFRMEventData;
   Connection_Entry_t                                   *ConnectionEntry;
   HFRM_Network_Operator_Selection_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(NetworkOperatorSelectionIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, NetworkOperatorSelectionIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                 = hetHFRNetworkOperatorSelectionIndication;
         HFRMEventData.EventLength                                                               = HFRM_NETWORK_OPERATOR_SELECTION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.NetworkOperatorSelectionIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_IND;
         Message.MessageHeader.MessageLength   = (HFRM_NETWORK_OPERATOR_SELECTION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* network operator selection confirmation event that has been       */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessNetworkOperatorSelectionConfirmationEvent(HFRE_Network_Operator_Selection_Confirmation_Data_t *NetworkOperatorSelectionConfirmationData)
{
   unsigned int                                            Temp;
   HFRM_Event_Data_t                                       HFRMEventData;
   Connection_Entry_t                                     *ConnectionEntry;
   HFRM_Network_Operator_Selection_Confirmation_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(NetworkOperatorSelectionConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, NetworkOperatorSelectionConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                   = hetHFRNetworkOperatorSelectionConfirmation;
         HFRMEventData.EventLength                                                                 = HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.NetworkOperatorSelectionConfirmationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.NetworkOperatorSelectionConfirmationEventData.NetworkMode         = NetworkOperatorSelectionConfirmationData->NetworkMode;
         HFRMEventData.EventData.NetworkOperatorSelectionConfirmationEventData.NetworkOperator     = NetworkOperatorSelectionConfirmationData->NetworkOperator;

         /* Next, format up the Message to dispatch.                    */
         /* * NOTE * We will take care to guard against phone numbers   */
         /*          that are longer than we can process.               */
         if(NetworkOperatorSelectionConfirmationData->NetworkOperator)
            Temp = BTPS_StringLength(NetworkOperatorSelectionConfirmationData->NetworkOperator) + 1;
         else
            Temp = 1;

         if(Temp > HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM)
            Temp = HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM;

         if((Message = (HFRM_Network_Operator_Selection_Confirmation_Message_t *)BTPS_AllocateMemory(HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_MESSAGE_SIZE(Temp))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, (Temp));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_NETWORK_OPERATOR_SELECTION_CFM;
            Message->MessageHeader.MessageLength   = (HFRM_NETWORK_OPERATOR_SELECTION_CONFIRMATION_MESSAGE_SIZE(Temp) - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->NetworkMode                   = NetworkOperatorSelectionConfirmationData->NetworkMode;
            Message->NetworkOperatorLength         = Temp;

            if(Temp > 1)
               BTPS_MemCopy(Message->NetworkOperator, NetworkOperatorSelectionConfirmationData->NetworkOperator, Temp - 1);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* extended error result activation indication event that has been   */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessExtendedErrorResultActivationIndicationEvent(HFRE_Extended_Error_Result_Activation_Indication_Data_t *ExtendedErrorResultActivationIndicationData)
{
   HFRM_Event_Data_t                                           HFRMEventData;
   Connection_Entry_t                                         *ConnectionEntry;
   HFRM_Extended_Error_Result_Activation_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ExtendedErrorResultActivationIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ExtendedErrorResultActivationIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                      = hetHFRExtendedErrorResultActivationIndication;
         HFRMEventData.EventLength                                                                    = HFRM_EXTENDED_ERROR_RESULT_ACTIVATION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ExtendedErrorResultActivationIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.ExtendedErrorResultActivationIndicationEventData.Enabled             = ExtendedErrorResultActivationIndicationData->Enabled;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_EXTENDED_ERROR_RESULT_ACT_IND;
         Message.MessageHeader.MessageLength   = (HFRM_EXTENDED_ERROR_RESULT_ACTIVATION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.Enabled                       = ExtendedErrorResultActivationIndicationData->Enabled;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* subscriber number information indication event that has been      */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessSubscriberNumberInformationIndicationEvent(HFRE_Subscriber_Number_Information_Indication_Data_t *SubscriberNumberInformationIndicationData)
{
   HFRM_Event_Data_t                                        HFRMEventData;
   Connection_Entry_t                                      *ConnectionEntry;
   HFRM_Subscriber_Number_Information_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SubscriberNumberInformationIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, SubscriberNumberInformationIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                    = hetHFRSubscriberNumberInformationIndication;
         HFRMEventData.EventLength                                                                  = HFRM_SUBSCRIBER_NUMBER_INFORMATION_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.SubscriberNumberInformationIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INF_IND;
         Message.MessageHeader.MessageLength   = (HFRM_SUBSCRIBER_NUMBER_INFORMATION_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* subscriber number information confirmation event that has been    */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Hands Free Manager information*/
   /* held.                                                             */
static void ProcessSubscriberNumberInformationConfirmationEvent(HFRE_Subscriber_Number_Information_Confirmation_Data_t *SubscriberNumberInformationConfirmationData)
{
   unsigned int                                               Temp;
   HFRM_Event_Data_t                                          HFRMEventData;
   Connection_Entry_t                                        *ConnectionEntry;
   HFRM_Subscriber_Number_Information_Confirmation_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SubscriberNumberInformationConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, SubscriberNumberInformationConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                                                           = hetHFRSubscriberNumberInformationConfirmation;
         HFRMEventData.EventLength                                                                                         = HFRM_SUBSCRIBER_NUMBER_INFORMATION_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.RemoteDeviceAddress                      = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.ServiceType  = SubscriberNumberInformationConfirmationData->ServiceType;
         HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.NumberFormat = SubscriberNumberInformationConfirmationData->NumberFormat;
         HFRMEventData.EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.PhoneNumber  = SubscriberNumberInformationConfirmationData->PhoneNumber;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SUBSCRIBER_NUMBER_INFO_CFM;
         Message.MessageHeader.MessageLength   = (HFRM_SUBSCRIBER_NUMBER_INFORMATION_CONFIRMATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         /* * NOTE * We will take care to guard against phone numbers   */
         /*          that are longer than we can process.               */
         if(SubscriberNumberInformationConfirmationData->PhoneNumber)
            Temp = BTPS_StringLength(SubscriberNumberInformationConfirmationData->PhoneNumber) + 1;
         else
            Temp = 1;

         if(Temp > HFRE_PHONE_NUMBER_LENGTH_MAXIMUM)
            Temp = HFRE_PHONE_NUMBER_LENGTH_MAXIMUM;

         Message.RemoteDeviceAddress                                                      = ConnectionEntry->BD_ADDR;
         Message.SubscriberInformationEntry.SubscriberNumberInformationEntry.ServiceType  = SubscriberNumberInformationConfirmationData->ServiceType;
         Message.SubscriberInformationEntry.SubscriberNumberInformationEntry.NumberFormat = SubscriberNumberInformationConfirmationData->NumberFormat;
         Message.SubscriberInformationEntry.SubscriberNumberInformationEntry.PhoneNumber  = NULL;
         Message.SubscriberInformationEntry.PhoneNumberLength                             = Temp;

         if(Temp > 1)
            BTPS_MemCopy(Message.SubscriberInformationEntry.PhoneNumber, SubscriberNumberInformationConfirmationData->PhoneNumber, Temp - 1);

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* response/hold status indication event that has been received with */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessResponseHoldStatusIndicationEvent(HFRE_Response_Hold_Status_Indication_Data_t *ResponseHoldStatusIndicationData)
{
   HFRM_Event_Data_t                               HFRMEventData;
   Connection_Entry_t                             *ConnectionEntry;
   HFRM_Response_Hold_Status_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ResponseHoldStatusIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ResponseHoldStatusIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                           = hetHFRResponseHoldStatusIndication;
         HFRMEventData.EventLength                                                         = HFRM_RESPONSE_HOLD_STATUS_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ResponseHoldStatusIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_IND;
         Message.MessageHeader.MessageLength   = (HFRM_RESPONSE_HOLD_STATUS_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* response/hold status confirmation event that has been received    */
   /* with the specified information.  This function should be called   */
   /* with the Lock protecting the Hands Free Manager information held. */
static void ProcessResponseHoldStatusConfirmationEvent(HFRE_Response_Hold_Status_Confirmation_Data_t *ResponseHoldStatusConfirmationData)
{
   HFRM_Event_Data_t                                 HFRMEventData;
   Connection_Entry_t                               *ConnectionEntry;
   HFRM_Response_Hold_Status_Confirmation_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ResponseHoldStatusConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ResponseHoldStatusConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                             = hetHFRResponseHoldStatusConfirmation;
         HFRMEventData.EventLength                                                           = HFRM_RESPONSE_HOLD_STATUS_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ResponseHoldStatusConfirmationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.ResponseHoldStatusConfirmationEventData.CallState           = ResponseHoldStatusConfirmationData->CallState;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_RESPONSE_HOLD_STATUS_CFM;
         Message.MessageHeader.MessageLength   = (HFRM_RESPONSE_HOLD_STATUS_CONFIRMATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.CallState                     = ResponseHoldStatusConfirmationData->CallState;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* incoming call state indication event that has been received with  */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessIncomingCallStateIndicationEvent(HFRE_Incoming_Call_State_Indication_Data_t *IncomingCallStateIndicationData)
{
   HFRM_Event_Data_t                              HFRMEventData;
   Connection_Entry_t                            *ConnectionEntry;
   HFRM_Incoming_Call_State_Indication_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(IncomingCallStateIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, IncomingCallStateIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                          = hetHFRIncomingCallStateIndication;
         HFRMEventData.EventLength                                                        = HFRM_INCOMING_CALL_STATE_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.IncomingCallStateIndicationEventData.ConnectionType      = ConnectionEntry->ConnectionType;
         HFRMEventData.EventData.IncomingCallStateIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.IncomingCallStateIndicationEventData.CallState           = IncomingCallStateIndicationData->CallState;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_IND;
         Message.MessageHeader.MessageLength   = (HFRM_INCOMING_CALL_STATE_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.CallState                     = IncomingCallStateIndicationData->CallState;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* incoming call state confirmation event that has been received with*/
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessIncomingCallStateConfirmationEvent(HFRE_Incoming_Call_State_Confirmation_Data_t *IncomingCallStateConfirmationData)
{
   HFRM_Event_Data_t                                HFRMEventData;
   Connection_Entry_t                              *ConnectionEntry;
   HFRM_Incoming_Call_State_Confirmation_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(IncomingCallStateConfirmationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, IncomingCallStateConfirmationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                            = hetHFRIncomingCallStateConfirmation;
         HFRMEventData.EventLength                                                          = HFRM_INCOMING_CALL_STATE_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.IncomingCallStateConfirmationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.IncomingCallStateConfirmationEventData.CallState           = IncomingCallStateConfirmationData->CallState;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_INCOMING_CALL_STATE_CFM;
         Message.MessageHeader.MessageLength   = (HFRM_INCOMING_CALL_STATE_CONFIRMATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.CallState                     = IncomingCallStateConfirmationData->CallState;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* command result event that has been received with the specified    */
   /* information.  This function should be called with the Lock        */
   /* protecting the Hands Free Manager information held.               */
static void ProcessCommandResultEvent(HFRE_Command_Result_Data_t *CommandResultData)
{
   HFRM_Event_Data_t              HFRMEventData;
   Connection_Entry_t            *ConnectionEntry;
   HFRM_Command_Result_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CommandResultData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CommandResultData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                            = hetHFRCommandResult;
         HFRMEventData.EventLength                                          = HFRM_COMMAND_RESULT_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CommandResultEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CommandResultEventData.ResultType          = CommandResultData->ResultType;
         HFRMEventData.EventData.CommandResultEventData.ResultValue         = CommandResultData->ResultValue;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         Message.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_COMMAND_RESULT;
         Message.MessageHeader.MessageLength   = (HFRM_COMMAND_RESULT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.ResultType                    = CommandResultData->ResultType;
         Message.ResultValue                   = CommandResultData->ResultValue;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* arbitrary command indication event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Hands Free Manager information held.          */
static void ProcessArbitraryCommandIndicationEvent(HFRE_Arbitrary_Command_Indication_Data_t *ArbitraryCommandIndicationData)
{
   unsigned int                                 Temp;
   HFRM_Event_Data_t                            HFRMEventData;
   Connection_Entry_t                          *ConnectionEntry;
   HFRM_Arbitrary_Command_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((ArbitraryCommandIndicationData) && (ArbitraryCommandIndicationData->HFRECommandData) && (BTPS_StringLength(ArbitraryCommandIndicationData->HFRECommandData)))
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ArbitraryCommandIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                         = hetHFRArbitraryCommandIndication;
         HFRMEventData.EventLength                                                       = HFRM_ARBITRARY_COMMAND_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ArbitraryCommandIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.ArbitraryCommandIndicationEventData.CommandData         = ArbitraryCommandIndicationData->HFRECommandData;

         /* Note the length of the command data.                        */
         Temp = BTPS_StringLength(ArbitraryCommandIndicationData->HFRECommandData);

         /* Format up the message to dispatch.                          */
         if((Message = (HFRM_Arbitrary_Command_Indication_Message_t *)BTPS_AllocateMemory(HFRM_ARBITRARY_COMMAND_INDICATION_MESSAGE_SIZE(Temp + 1))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_ARBITRARY_COMMAND_INDICATION_MESSAGE_SIZE(Temp + 1));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ARBITRARY_COMMAND_IND;
            Message->MessageHeader.MessageLength   = (HFRM_ARBITRARY_COMMAND_INDICATION_MESSAGE_SIZE(Temp + 1) - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->ArbitraryCommandLength        = Temp + 1;

            BTPS_MemCopy(Message->ArbitraryCommand, ArbitraryCommandIndicationData->HFRECommandData, Temp);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Hands Free */
   /* arbitrary response indication event that has been received with   */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the Hands Free Manager information held.      */
static void ProcessArbitraryResponseIndicationEvent(HFRE_Arbitrary_Response_Indication_Data_t *ArbitraryResponseIndicationData)
{
   unsigned int                                  Temp;
   HFRM_Event_Data_t                             HFRMEventData;
   Connection_Entry_t                           *ConnectionEntry;
   HFRM_Arbitrary_Response_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((ArbitraryResponseIndicationData) && (ArbitraryResponseIndicationData->HFREResponseData) && (BTPS_StringLength(ArbitraryResponseIndicationData->HFREResponseData)))
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, ArbitraryResponseIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                          = hetHFRArbitraryResponseIndication;
         HFRMEventData.EventLength                                                        = HFRM_ARBITRARY_RESPONSE_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.ArbitraryResponseIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.ArbitraryResponseIndicationEventData.ResponseData        = ArbitraryResponseIndicationData->HFREResponseData;

         /* Note the length of the command data.                        */
         Temp = BTPS_StringLength(ArbitraryResponseIndicationData->HFREResponseData);

         /* Format up the message to dispatch.                          */
         if((Message = (HFRM_Arbitrary_Response_Indication_Message_t *)BTPS_AllocateMemory(HFRM_ARBITRARY_RESPONSE_INDICATION_MESSAGE_SIZE(Temp + 1))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_ARBITRARY_RESPONSE_INDICATION_MESSAGE_SIZE(Temp + 1));

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ARBITRARY_RESPONSE;
            Message->MessageHeader.MessageLength   = (HFRM_ARBITRARY_RESPONSE_INDICATION_MESSAGE_SIZE(Temp + 1) - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->ArbitraryResponseLength       = Temp + 1;

            BTPS_MemCopy(Message->ArbitraryResponse, ArbitraryResponseIndicationData->HFREResponseData, Temp);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the codec select indication. The hands free device        */
   /* receives this indication when the remote audio gateway sends a    */
   /* a codec as part of the codec negotiation.                         */
static void ProcessCodecSelectIndicationEvent(HFRE_Codec_Select_Indication_t *CodecSelectIndication)
{
   HFRM_Event_Data_t                       HFRMEventData;
   Connection_Entry_t                     *ConnectionEntry;
   HFRM_Codec_Select_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((CodecSelectIndication) && (CodecSelectIndication->CodecID))
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CodecSelectIndication->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                    = hetHFRCodecSelectIndication;
         HFRMEventData.EventLength                                                  = HFRM_CODEC_SELECT_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CodecSelectIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CodecSelectIndicationEventData.CodecID             = CodecSelectIndication->CodecID;

         /* Format up the message to dispatch.                          */
         if((Message = (HFRM_Codec_Select_Indication_Message_t *)BTPS_AllocateMemory(HFRM_CODEC_SELECT_INDICATION_MESSAGE_SIZE)) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_CODEC_SELECT_INDICATION_MESSAGE_SIZE);

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SELECT_CODEC_IND;
            Message->MessageHeader.MessageLength   = (HFRM_CODEC_SELECT_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->CodecID                       = CodecSelectIndication->CodecID;

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the codec select confirmation. The audio gateway receives */
   /* this confirmation when the remote hands free device sends a codec */
   /* to complete the codec negotiation.                                */
static void ProcessCodecSelectConfirmationEvent(HFRE_Codec_Select_Confirmation_t *CodecSelectConfirmation)
{
   HFRM_Event_Data_t                         HFRMEventData;
   Connection_Entry_t                       *ConnectionEntry;
   HFRM_Codec_Select_Confirmation_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((CodecSelectConfirmation) && (CodecSelectConfirmation->AcceptedCodec))
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CodecSelectConfirmation->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                      = hetHFRCodecSelectConfirmation;
         HFRMEventData.EventLength                                                    = HFRM_CODEC_SELECT_CONFIRMATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CodecSelectConfirmationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.CodecSelectConfirmationEventData.AcceptedCodec       = CodecSelectConfirmation->AcceptedCodec;

         /* Format up the message to dispatch.                          */
         if((Message = (HFRM_Codec_Select_Confirmation_Message_t *)BTPS_AllocateMemory(HFRM_CODEC_SELECT_CONFIRMATION_MESSAGE_SIZE)) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_CODEC_SELECT_CONFIRMATION_MESSAGE_SIZE);

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SELECT_CODEC_CFM;
            Message->MessageHeader.MessageLength   = (HFRM_CODEC_SELECT_CONFIRMATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->CodecID                       = CodecSelectConfirmation->AcceptedCodec;

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the codec connection setup indication. The audio gateway  */
   /* receives this indication when the codec connection is completed   */
   /* as part of the audio connection setup.                            */
static void ProcessCodecConnectionSetupIndicationEvent(HFRE_Codec_Connection_Setup_Indication_Data_t *CodecConnectionSetupIndicationData)
{
   HFRM_Event_Data_t                                 HFRMEventData;
   Connection_Entry_t                               *ConnectionEntry;
   HFRM_Codec_Connection_Setup_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CodecConnectionSetupIndicationData)
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, CodecConnectionSetupIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                            = hetHFRCodecConnectionSetupIndication;
         HFRMEventData.EventLength                                                          = HFRM_CODEC_CONNECTION_SETUP_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.CodecConnectionSetupIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Format up the message to dispatch.                          */
         if((Message = (HFRM_Codec_Connection_Setup_Indication_Message_t *)BTPS_AllocateMemory(HFRM_CODEC_CONNECTION_SETUP_INDICATION_MESSAGE_SIZE)) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_CODEC_CONNECTION_SETUP_INDICATION_MESSAGE_SIZE);

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CONNECT_CODEC_IND;
            Message->MessageHeader.MessageLength   = (HFRM_CODEC_CONNECTION_SETUP_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process the available codec list indication. The audio gateway    */
   /* receives this indication when the remote hands free device sends  */
   /* the list of codecs it supports as part of the codec negotiation.  */
   /* Bluetopia will automatically send the 'OK' response to this       */
   /* indication.                                                       */
static void ProcessAvailableCodecListIndicationEvent(HFRE_Available_Codec_List_Indication_Data_t *AvailableCodecListIndicationData)
{
   HFRM_Event_Data_t                               HFRMEventData;
   Connection_Entry_t                             *ConnectionEntry;
   HFRM_Available_Codec_List_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((AvailableCodecListIndicationData) && (AvailableCodecListIndicationData->NumSupportedCodecs) && (AvailableCodecListIndicationData->AvailableCodecList))
   {
      /* Next map the Hands Free Port ID to a connection entry we are   */
      /* tracking.                                                      */
      if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, AvailableCodecListIndicationData->HFREPortID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HFRMEventData.EventType                                                             = hetHFRAvailableCodecListIndication;
         HFRMEventData.EventLength                                                           = HFRM_AVAILABLE_CODEC_LIST_INDICATION_EVENT_DATA_SIZE;

         HFRMEventData.EventData.AvailableCodecListIndicationEventData.RemoteDeviceAddress   = ConnectionEntry->BD_ADDR;
         HFRMEventData.EventData.AvailableCodecListIndicationEventData.NumberSupportedCodecs = AvailableCodecListIndicationData->NumSupportedCodecs;
         HFRMEventData.EventData.AvailableCodecListIndicationEventData.AvailableCodecList    = AvailableCodecListIndicationData->AvailableCodecList;

         /* Format up the message to dispatch.                          */
         if((Message = (HFRM_Available_Codec_List_Indication_Message_t *)BTPS_AllocateMemory(HFRM_AVAILABLE_CODEC_LIST_INDICATION_MESSAGE_SIZE)) != NULL)
         {
            BTPS_MemInitialize(Message, 0, HFRM_CODEC_SELECT_CONFIRMATION_MESSAGE_SIZE);

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            Message->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_AVAILABLE_CODEC_LIST_IND;
            Message->MessageHeader.MessageLength   = (HFRM_AVAILABLE_CODEC_LIST_INDICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->NumberSupportedCodecs         = AvailableCodecListIndicationData->NumSupportedCodecs;

            BTPS_MemCopy(Message->AvailableCodecList, AvailableCodecListIndicationData->AvailableCodecList, HFRE_MAX_SUPPORTED_CODECS);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHFREEvent(TRUE, ConnectionEntry->ConnectionType, &HFRMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing Hands Free Events that have been received.  This       */
   /* function should ONLY be called with the Context locked AND ONLY in*/
   /* the context of an arbitrary processing thread.                    */
static void ProcessHandsFreeEvent(HFRM_Hands_Free_Event_Data_t *HandsFreeEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(HandsFreeEventData)
   {
      /* Process the event based on the event type.                     */
      switch(HandsFreeEventData->EventType)
      {
         case etHFRE_Open_Port_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication\n"));

            ProcessOpenRequestIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Open_Port_Request_Indication_Data));
            break;
         case etHFRE_Open_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Indication\n"));

            ProcessOpenIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Open_Port_Indication_Data));
            break;
         case etHFRE_Open_Port_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Confirmation\n"));

            ProcessOpenConfirmationEvent(TRUE, &(HandsFreeEventData->EventData.HFRE_Open_Port_Confirmation_Data));
            break;
         case etHFRE_Open_Service_Level_Connection_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Service Level Connection Indication\n"));

            ProcessServiceLevelIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Open_Service_Level_Connection_Indication_Data));
            break;
         case etHFRE_Control_Indicator_Status_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Indicator Status Indication\n"));

            ProcessControlIndicatorStatusIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Control_Indicator_Status_Indication_Data));
            break;
         case etHFRE_Control_Indicator_Status_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Indicator Status Confirmation\n"));

            ProcessControlIndicatorStatusConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Control_Indicator_Status_Confirmation_Data));
            break;
         case etHFRE_Call_Hold_Multiparty_Support_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Hold/Multi-party Support Confirmation\n"));

            ProcessCallHoldMultipartySupportConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Call_Hold_Multiparty_Support_Confirmation_Data));
            break;
         case etHFRE_Call_Hold_Multiparty_Selection_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Hold/Multi-party Selection Indication\n"));

            ProcessCallHoldMultipartySelectionIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Call_Hold_Multiparty_Selection_Indication_Data));
            break;
         case etHFRE_Call_Waiting_Notification_Activation_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Waiting Notification Activation Indication\n"));

            ProcessCallWaitingNotificationActivationIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Call_Waiting_Notification_Activation_Indication_Data));
            break;
         case etHFRE_Call_Waiting_Notification_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Waiting Notification Indication\n"));

            ProcessCallWaitingNotificationIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Call_Waiting_Notification_Indication_Data));
            break;
         case etHFRE_Call_Line_Identification_Notification_Activation_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Line Identification Notification Indication\n"));

            ProcessCallLineIdentificationNotificationActivationIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Call_Line_Identification_Notification_Activation_Indication_Data));
            break;
         case etHFRE_Call_Line_Identification_Notification_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Call Line Identification Indication\n"));

            ProcessCallLineIdentificationNotificationIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Call_Line_Identification_Notification_Indication_Data));
            break;
         case etHFRE_Disable_Sound_Enhancement_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable Sound Enhancement Indication\n"));

            ProcessDisableSoundEnhancementIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Disable_Sound_Enhancement_Indication_Data));
            break;
         case etHFRE_Dial_Phone_Number_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Dial Phone Number Indication\n"));

            ProcessDialPhoneNumberIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Dial_Phone_Number_Indication_Data));
            break;
         case etHFRE_Dial_Phone_Number_From_Memory_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Dial Phone Number (from Memory) Indication\n"));

            ProcessDialPhoneNumberFromMemoryIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Dial_Phone_Number_From_Memory_Indication_Data));
            break;
         case etHFRE_ReDial_Last_Phone_Number_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Re-dial Last Phone Number Indication\n"));

            ProcessReDialLastPhoneNumberIndicationEvent(&(HandsFreeEventData->EventData.HFRE_ReDial_Last_Phone_Number_Indication_Data));
            break;
         case etHFRE_Ring_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Ring Indication\n"));

            ProcessRingIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Ring_Indication_Data));
            break;
         case etHFRE_Generate_DTMF_Tone_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Generate DTMF Tone Indication\n"));

            ProcessGenerateDTMFToneIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Generate_DTMF_Tone_Indication_Data));
            break;
         case etHFRE_Answer_Call_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Ring Indication\n"));

            ProcessAnswerCallIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Answer_Call_Indication_Data));
            break;
         case etHFRE_InBand_Ring_Tone_Setting_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("In-band Ring Tone Setting Indication\n"));

            ProcessInBandRingToneSettingIndicationEvent(&(HandsFreeEventData->EventData.HFRE_InBand_Ring_Tone_Setting_Indication_Data));
            break;
         case etHFRE_Voice_Recognition_Notification_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Recognition Notification Indication\n"));

            ProcessVoiceRecognitionNotificationIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Voice_Recognition_Notification_Indication_Data));
            break;
         case etHFRE_Speaker_Gain_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Speaker Gain Indication\n"));

            ProcessSpeakerGainIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Speaker_Gain_Indication_Data));
            break;
         case etHFRE_Microphone_Gain_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Microphone Gain Indication\n"));

            ProcessMicrophoneGainIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Microphone_Gain_Indication_Data));
            break;
         case etHFRE_Voice_Tag_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Tag Request Indication\n"));

            ProcessVoiceTagRequestIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Voice_Tag_Request_Indication_Data));
            break;
         case etHFRE_Voice_Tag_Request_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Voice Tag Request Confirmation\n"));

            ProcessVoiceTagRequestConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Voice_Tag_Request_Confirmation_Data));
            break;
         case etHFRE_Hang_Up_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hang-up Indication\n"));

            ProcessHangUpIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Hang_Up_Indication_Data));
            break;
         case etHFRE_Audio_Connection_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Connection Indication\n"));

            ProcessAudioConnectionIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Audio_Connection_Indication_Data));
            break;
         case etHFRE_Audio_Disconnection_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Disconnection Indication\n"));

            ProcessAudioDisconnectionIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Audio_Disconnection_Indication_Data));
            break;
         case etHFRE_Audio_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Data Indication\n"));

            ProcessAudioDataIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Audio_Data_Indication_Data));
            break;
         case etHFRE_Close_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Port Indication\n"));

            ProcessCloseIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Close_Port_Indication_Data));
            break;
         case etHFRE_Current_Calls_List_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Current Calls List Indication\n"));

            ProcessCurrentCallsListIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Current_Calls_List_Indication_Data));
            break;
         case etHFRE_Current_Calls_List_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Current Calls List Confirmation\n"));

            ProcessCurrentCallsListConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Current_Calls_List_Confirmation_Data));
            break;
         case etHFRE_Network_Operator_Selection_Format_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Network Operator Selection Format Indication\n"));

            ProcessNetworkOperatorSelectionFormatIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Network_Operator_Selection_Format_Indication_Data));
            break;
         case etHFRE_Network_Operator_Selection_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Network Operator Selection Indication\n"));

            ProcessNetworkOperatorSelectionIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Network_Operator_Selection_Indication_Data));
            break;
         case etHFRE_Network_Operator_Selection_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Network Operator Selection Confirmation\n"));

            ProcessNetworkOperatorSelectionConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Network_Operator_Selection_Confirmation_Data));
            break;
         case etHFRE_Extended_Error_Result_Activation_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Extended Error Result Activation Indication\n"));

            ProcessExtendedErrorResultActivationIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Extended_Error_Result_Activation_Indication_Data));
            break;
         case etHFRE_Subscriber_Number_Information_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Subscriber Number Information Indication\n"));

            ProcessSubscriberNumberInformationIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Subscriber_Number_Information_Indication_Data));
            break;
         case etHFRE_Subscriber_Number_Information_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Subscriber Number Information Confirmation\n"));

            ProcessSubscriberNumberInformationConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Subscriber_Number_Information_Confirmation_Data));
            break;
         case etHFRE_Response_Hold_Status_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Response/Hold Status Indication\n"));

            ProcessResponseHoldStatusIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Response_Hold_Status_Indication_Data));
            break;
         case etHFRE_Response_Hold_Status_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Response/Hold Status Confirmation\n"));

            ProcessResponseHoldStatusConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Response_Hold_Status_Confirmation_Data));
            break;
         case etHFRE_Incoming_Call_State_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Call State Indication\n"));

            ProcessIncomingCallStateIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Incoming_Call_State_Indication_Data));
            break;
         case etHFRE_Incoming_Call_State_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Call State Confirmation\n"));

            ProcessIncomingCallStateConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Incoming_Call_State_Confirmation_Data));
            break;
         case etHFRE_Command_Result:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Command Result\n"));

            ProcessCommandResultEvent(&(HandsFreeEventData->EventData.HFRE_Command_Result_Data));
            break;
         case etHFRE_Arbitrary_Command_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Arbitrary Command Indication\n"));

            ProcessArbitraryCommandIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Arbitrary_Command_Indication_Data));
            break;
         case etHFRE_Arbitrary_Response_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Arbitrary Response Indication\n"));

            ProcessArbitraryResponseIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Arbitrary_Response_Indication_Data));
            break;
         case etHFRE_Codec_Select_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Codec Select Indication\n"));

            ProcessCodecSelectIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Codec_Select_Indication));
            break;
         case etHFRE_Codec_Select_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Codec Select Confirmation\n"));

            ProcessCodecSelectConfirmationEvent(&(HandsFreeEventData->EventData.HFRE_Codec_Select_Confirmation));
            break;
         case etHFRE_Codec_Connection_Setup_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Codec Connection Setup Indication\n"));

            ProcessCodecConnectionSetupIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Codec_Connection_Setup_Indication_Data));
            break;
         case etHFRE_Available_Codec_List_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Available Codec List Indication\n"));

            ProcessAvailableCodecListIndicationEvent(&(HandsFreeEventData->EventData.HFRE_Available_Codec_List_Indication_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown Hands Free Event Type: %d\n", HandsFreeEventData->EventType));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Hands Free Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* process Device Manager (DEVM) Status Events (for out-going        */
   /* connection management).                                           */
static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status)
{
   int                                 Result;
   HFRE_Entry_Info_t                  *HFREEntryInfo;
   Connection_Entry_t                 *ConnectionEntry;
   HFRE_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HFRM): 0x%08X, %d\n", StatusType, Status));

   /* First, determine if we are tracking a connection to this device.  */
   /* * NOTE * We do not know at this time which connection type we     */
   /*          are searching for, so we have to search both lists.      */
   if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, BD_ADDR, hctAudioGateway)) == NULL) || (ConnectionEntry->ConnectionState == csConnected))
   {
      if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, BD_ADDR, hctHandsFree)) == NULL) || (ConnectionEntry->ConnectionState == csConnected))
         ConnectionEntry = NULL;
   }

   if(ConnectionEntry)
   {
      /* Next, let's loop through the list and see if there is an       */
      /* out-going Event connection being tracked for this event.       */
      if(ConnectionEntry->ConnectionType == hctAudioGateway)
         HFREEntryInfo = HFREEntryInfoList_AG;
      else
         HFREEntryInfo = HFREEntryInfoList_HF;

      while(HFREEntryInfo)
      {
         /* Check to see if there is a out-going connection operation.  */
         if((!(HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(BD_ADDR, HFREEntryInfo->ConnectionBD_ADDR)))
         {
            /* Match found.                                             */
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Outgoing Connection Entry found\n"));
            break;
         }
         else
            HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
      }

      /* See if there is any processing that is required (i.e. match    */
      /* found).                                                        */
      if(HFREEntryInfo)
      {
         /* Process the status event.                                   */

         /* Initialize common connection event members.                 */
         BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(HFRE_Open_Port_Confirmation_Data_t));

         OpenPortConfirmationData.HFREPortID = ConnectionEntry->HFREID;

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
                  OpenPortConfirmationData.PortOpenStatus = HFRE_OPEN_PORT_STATUS_CONNECTION_REFUSED;
                  break;
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                  OpenPortConfirmationData.PortOpenStatus = HFRE_OPEN_PORT_STATUS_CONNECTION_TIMEOUT;
                  break;
               default:
                  OpenPortConfirmationData.PortOpenStatus = HFRE_OPEN_PORT_STATUS_UNKNOWN_ERROR;
                  break;
            }

            /* * NOTE * This function will delete the Hands Free entry  */
            /*          from the list.                                  */
            ProcessOpenConfirmationEvent(TRUE, &OpenPortConfirmationData);

            /* Flag that the connection has been deleted.               */
            ConnectionEntry = NULL;
         }
         else
         {
            /* Connection succeeded.                                    */

            /* Move the state to the connecting state.                  */
            ConnectionEntry->ConnectionState = csConnecting;

            if(((Result = _HFRM_Connect_Remote_Device(ConnectionEntry->ConnectionType, BD_ADDR, ConnectionEntry->ServerPort)) <= 0) && (Result != BTPM_ERROR_CODE_HANDS_FREE_ALREADY_CONNECTED))
            {
               /* Error opening device.                                 */
               DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

               OpenPortConfirmationData.PortOpenStatus = HFRE_OPEN_PORT_STATUS_UNKNOWN_ERROR;

               /* * NOTE * This function will delete the Hands Free     */
               /*          entry from the list.                         */
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
               if(Result == BTPM_ERROR_CODE_HANDS_FREE_ALREADY_CONNECTED)
               {
                  ConnectionEntry->ConnectionState         = csConnected;

                  OpenPortConfirmationData.PortOpenStatus  = 0;

                  ProcessOpenConfirmationEvent(FALSE, &OpenPortConfirmationData);
               }
               else
                  ConnectionEntry->HFREID = (unsigned int)Result;
            }
         }
      }
      else
      {
         /* No out-going connection found.                              */
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("No Outgoing Connection Entry found\n"));
      }

      /* Next, we will check to see if this event is referencing an     */
      /* incoming connection.                                           */
      if((ConnectionEntry) && ((ConnectionEntry->ConnectionState == csAuthenticating) || (ConnectionEntry->ConnectionState == csEncrypting)))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Entry found\n"));

         /* Status references an outgoing connection.                   */
         if(!Status)
         {
            /* Success, accept the connection.                          */
            _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, TRUE);
         }
         else
         {
            /* Failure, reject the connection.                          */
            _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);

            /* First, delete the Connection Entry we are tracking.      */
            if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntry->BD_ADDR, ConnectionEntry->ConnectionType)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HFRM)\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Hands Free Manager Notification Events.     */
static void BTPSAPI BTPMDispatchCallback_HFRE(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is a Hands Free Event Update.     */
            if(((HFRM_Update_Data_t *)CallbackParameter)->UpdateType == utHandsFreeEvent)
            {
               /* Process the Notification.                             */
               ProcessHandsFreeEvent(&(((HFRM_Update_Data_t *)CallbackParameter)->UpdateData.HandsFreeEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Hands Free Profile Manager Timer Events.    */
static void BTPSAPI BTPMDispatchCallback_TMR(void *CallbackParameter)
{
   int                                 Result;
   Connection_Entry_t                 *ConnectionEntry;
   HFRE_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HFRM)\n"));

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
            if((ConnectionEntry = SearchConnectionEntryHFREID(&ConnectionEntryList, (unsigned int)CallbackParameter)) != NULL)
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
                        DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device      */
                        if(ConnectionEntry->ConnectionFlags & HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                           ConnectionEntry->ConnectionState = csEncrypting;
                        else
                        {
                           if(ConnectionEntry->ConnectionFlags & HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
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

                              if((Result = _HFRM_Connect_Remote_Device(ConnectionEntry->ConnectionType, ConnectionEntry->BD_ADDR, ConnectionEntry->ServerPort)) <= 0)
                                 Result = BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE;
                              else
                              {
                                 /* Note the Hands Free Port ID.        */
                                 ConnectionEntry->HFREID = (unsigned int)Result;

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
                           Result = BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_RETRIES_EXCEEDED;
                        else
                        {
                           /* Port is NOT disconnected, go ahead and    */
                           /* start a timer so that we can continue to  */
                           /* check for the Port Disconnection.         */
                           Result = TMR_StartTimer((void *)ConnectionEntry->HFREID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

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
                        BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(HFRE_Open_Port_Confirmation_Data_t));

                        OpenPortConfirmationData.HFREPortID = ConnectionEntry->HFREID;

                        if(Result)
                        {
                           /* Connection Failed.                        */

                           /* Map the status to a known status.         */
                           switch(Result)
                           {
                              case BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_RETRIES_EXCEEDED:
                              case BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER:
                                 OpenPortConfirmationData.PortOpenStatus = HFRE_OPEN_PORT_STATUS_CONNECTION_TIMEOUT;
                                 break;
                              case BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE:
                                 OpenPortConfirmationData.PortOpenStatus = HFRE_OPEN_PORT_STATUS_UNKNOWN_ERROR;
                                 break;
                              case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
                              case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                              default:
                                 OpenPortConfirmationData.PortOpenStatus = HFRE_OPEN_PORT_STATUS_CONNECTION_REFUSED;
                                 break;
                           }

                           /* * NOTE * This function will delete the    */
                           /*          Hands Free entry from the list.  */
                           ProcessOpenConfirmationEvent(TRUE, &OpenPortConfirmationData);
                        }
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Connection is no longer in the correct state: 0x%08X (%d)\n", (unsigned int)CallbackParameter, ConnectionEntry->ConnectionState));
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Close Timer is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Connection is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HFRM)\n"));
}

   /* The following function is the Timer Callback function that is     */
   /* registered to process Serial Port Disconnection Events (to        */
   /* determine when it is safe to connect to a remote device).         */
static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HFRM)\n"));

   /* Simply queue a Timer Callback Event to process.                   */
   if(BTPM_QueueMailboxCallback(BTPMDispatchCallback_TMR, CallbackParameter))
      ret_val = FALSE;
   else
      ret_val = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HFRM): %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
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
               /* Hands Free Manager Thread.                            */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HFRM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Hands Free Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
   int                         Result;
   HFRM_Initialization_Info_t *InitializationInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      /* Check to see if this module has already been initialized.      */
      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Hands Free\n"));

         /* Make sure that there is actual AG/HFR data specified.       */
         if(((InitializationInfo = (HFRM_Initialization_Info_t *)InitializationData) != NULL) && ((InitializationInfo->AudioGatewayInitializationInfo) || (InitializationInfo->HandsFreeInitializationInfo)))
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Hands Free Manager messages.                  */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER, HandsFreeManagerGroupHandler, NULL))
            {
               /* Initialize the actual Hands Free Manager              */
               /* Implementation Module (this is the module that is     */
               /* actually responsible for actually implementing the    */
               /* Hands Free Manager functionality - this module is just*/
               /* the framework shell).                                 */
               if(!(Result = _HFRM_Initialize(InitializationInfo->AudioGatewayInitializationInfo, InitializationInfo->HandsFreeInitializationInfo)))
               {
                  /* Make sure the initialization information is set to */
                  /* a known state.                                     */
                  BTPS_MemInitialize(&AudioGatewayInitializationInfo, 0, sizeof(AudioGatewayInitializationInfo));
                  BTPS_MemInitialize(&HandsFreeInitializationInfo, 0, sizeof(HandsFreeInitializationInfo));

                  /* Note any Audio Gateway/Hands Free initialization   */
                  /* information.                                       */
                  if(InitializationInfo->AudioGatewayInitializationInfo)
                  {
                     AudioGatewayInitializationInfo = *InitializationInfo->AudioGatewayInitializationInfo;
                     AudioGatewaySupported          = TRUE;
                  }
                  else
                     AudioGatewaySupported = FALSE;

                  if(InitializationInfo->HandsFreeInitializationInfo)
                  {
                     HandsFreeInitializationInfo = *InitializationInfo->HandsFreeInitializationInfo;
                     HandsFreeSupported          = TRUE;
                  }
                  else
                     HandsFreeSupported = FALSE;

                  /* Finally determine the current Device Power State.  */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting Hands Free Callback  */
                  /* ID.                                                */
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
               _HFRM_Cleanup();

               MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER);
            }
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Manager Already initialized\n"));
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

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the Hands Free Manager Implementation*/
            /* that we are shutting down.                               */
            _HFRM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the Audio Entry Data Information List is  */
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
            CurrentPowerState     = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized           = FALSE;

            AudioGatewaySupported = FALSE;
            HandsFreeSupported    = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
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
   int                 Result;
   unsigned int        Index;
   HFRE_Entry_Info_t  *tmpHFREEntryInfo;
   HFRE_Entry_Info_t  *HFREEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the Hands Free Manager that it    */
               /* should initialize.                                    */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  /* If the function returns TRUE, then the controller  */
                  /* supports WBS.                                      */
                  WBS_Support = _HFRM_SetBluetoothStackID((unsigned int)Result);

                  /* Make sure SDP records are enabled if we already    */
                  /* have a callback.                                   */
                  if(HFREEntryInfoList_AG_Control)
                     _HFRM_UpdateSDPRecord(hctAudioGateway, TRUE);

                  if(HFREEntryInfoList_HF_Control)
                     _HFRM_UpdateSDPRecord(hctHandsFree, TRUE);
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Reset the Arbitrary Commands state.                   */
               ArbitraryCommandsEnabled = FALSE;

               /* Close any active connections.                         */
               ConnectionEntry = ConnectionEntryList;

               while(ConnectionEntry)
               {
                  if(ConnectionEntry->ConnectionState == csAuthorizing)
                     _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);
                  else
                     _HFRM_Disconnect_Device(ConnectionEntry->HFREID);

                  ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
               }

               /* Inform the Hands Free Manager that the Stack has been */
               /* closed.                                               */
               _HFRM_SetBluetoothStackID(0);

               /* WBS can only be supported in Power On State.          */
               WBS_Support = FALSE;

               /* Loop through all outgoing connections to determine if */
               /* there are any synchronous connections outstanding.    */
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
                     if(!(HFREEntryInfo->Flags & HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                     {
                        if(HFREEntryInfo->ConnectionEvent)
                        {
                           HFREEntryInfo->ConnectionStatus = HFRM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                           BTPS_SetEvent(HFREEntryInfo->ConnectionEvent);

                           HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
                        }
                        else
                        {
                           /* Entry was waiting on a response, but it   */
                           /* was registered as either an Event Callback*/
                           /* or Connection Message.  Regardless we need*/
                           /* to delete it.                             */
                           tmpHFREEntryInfo = HFREEntryInfo;

                           HFREEntryInfo    = HFREEntryInfo->NextHFREEntryInfoPtr;

                           if((tmpHFREEntryInfo = DeleteHFREEntryInfoEntry((Index)?&HFREEntryInfoList_HF:&HFREEntryInfoList_AG, tmpHFREEntryInfo->CallbackID)) != NULL)
                              FreeHFREEntryInfoEntryMemory(tmpHFREEntryInfo);
                        }
                     }
                     else
                        HFREEntryInfo = HFREEntryInfo->NextHFREEntryInfoPtr;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Hands Free Manager of a specific Update Event.  The */
   /* Hands Free Manager can then take the correct action to process the*/
   /* update.                                                           */
Boolean_t HFRM_NotifyUpdate(HFRM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utHandsFreeEvent:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Hands Free Event: %d\n", UpdateData->UpdateData.HandsFreeEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_HFRE, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
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
   int                 ret_val;
   BD_ADDR_t           NULL_BD_ADDR;
   Boolean_t           Authenticate;
   Boolean_t           Encrypt;
   unsigned long       IncomingConnectionFlags;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* First, check to make sure the Hands Free Manager has been         */
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
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", AcceptConnection));

                  /* Determine the incoming connection flags based on   */
                  /* the connection type.                               */
                  if(ConnectionType == hctAudioGateway)
                     IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
                  else
                     IncomingConnectionFlags = HandsFreeInitializationInfo.IncomingConnectionFlags;

                  /* If the caller has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(AcceptConnection)
                  {
                     /* Determine if Authentication and/or Encryption is*/
                     /* required for this link.                         */
                     if(IncomingConnectionFlags & HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                        Authenticate = TRUE;
                     else
                        Authenticate = FALSE;

                     if(IncomingConnectionFlags & HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
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
                        ret_val = _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, TRUE);

                        if(ret_val)
                        {
                           _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);

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
                           _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);

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
                     ret_val = _HFRM_Connection_Request_Response(ConnectionEntry->HFREID, FALSE);

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
   int                  ret_val;
   Event_t              ConnectionEvent;
   BD_ADDR_t            NULL_BD_ADDR;
   Boolean_t            Delete;
   unsigned int         CallbackID;
   HFRE_Entry_Info_t    HFREEntryInfo;
   HFRE_Entry_Info_t   *HFREEntryInfoPtr;
   Connection_Entry_t   ConnectionEntry;
   Connection_Entry_t  *ConnectionEntryPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)) && (((ConnectionType == hctAudioGateway) && (AudioGatewaySupported)) || ((ConnectionType == hctHandsFree) && (HandsFreeSupported))))
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
                  ConnectionEntry.HFREID          = GetNextCallbackID() | 0x80000000;
                  ConnectionEntry.Server          = FALSE;
                  ConnectionEntry.ServerPort      = RemoteServerPort;
                  ConnectionEntry.ConnectionState = csIdle;
                  ConnectionEntry.ConnectionFlags = ConnectionFlags;

                  if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
                  {
                     /* Attempt to add an entry into the Hands Free     */
                     /* entry list.                                     */
                     BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

                     HFREEntryInfo.CallbackID        = GetNextCallbackID();
                     HFREEntryInfo.ClientID          = MSG_GetServerAddressID();
                     HFREEntryInfo.ConnectionBD_ADDR = RemoteDeviceAddress;
                     HFREEntryInfo.EventCallback     = CallbackFunction;
                     HFREEntryInfo.CallbackParameter = CallbackParameter;

                     if(ConnectionStatus)
                        HFREEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

                     Delete = FALSE;

                     if((!ConnectionStatus) || ((ConnectionStatus) && (HFREEntryInfo.ConnectionEvent)))
                     {
                        if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, &HFREEntryInfo)) != NULL)
                        {
                           /* First, let's wait for the Port to         */
                           /* disconnect.                               */
                           if(!SPPM_WaitForPortDisconnection(RemoteServerPort, FALSE, RemoteDeviceAddress, MAXIMUM_HANDS_FREE_PORT_DELAY_TIMEOUT_MS))
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                              /* Next, attempt to open the remote       */
                              /* device.                                */
                              if(ConnectionFlags & HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                                 ConnectionEntryPtr->ConnectionState = csEncrypting;
                              else
                              {
                                 if(ConnectionFlags & HFRM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                                    ConnectionEntryPtr->ConnectionState = csAuthenticating;
                                 else
                                    ConnectionEntryPtr->ConnectionState = csConnectingDevice;
                              }

                              DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Device\n"));

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

                                    if((ret_val = _HFRM_Connect_Remote_Device(ConnectionEntryPtr->ConnectionType, RemoteDeviceAddress, RemoteServerPort)) <= 0)
                                    {
                                       ret_val = BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE;

                                       /* Error opening device, go ahead*/
                                       /* and delete the entry that was */
                                       /* added.                        */
                                       if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, HFREEntryInfoPtr->CallbackID)) != NULL)
                                       {
                                          if(HFREEntryInfoPtr->ConnectionEvent)
                                             BTPS_CloseEvent(HFREEntryInfoPtr->ConnectionEvent);

                                          FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);
                                       }
                                    }
                                    else
                                    {
                                       /* Note the Hands Free Port ID.  */
                                       ConnectionEntryPtr->HFREID = (unsigned int)ret_val;

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
                                 ret_val = TMR_StartTimer((void *)ConnectionEntry.HFREID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

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
                                 ret_val = BTPM_ERROR_CODE_HANDS_FREE_IS_STILL_CONNECTED;
                           }

                           /* Next, determine if the caller has         */
                           /* requested a blocking open.                */
                           if((!ret_val) && (ConnectionStatus))
                           {
                              /* Blocking open, go ahead and wait for   */
                              /* the event.                             */

                              /* Note the Callback ID.                  */
                              CallbackID      = HFREEntryInfoPtr->CallbackID;

                              /* Note the Open Event.                   */
                              ConnectionEvent = HFREEntryInfoPtr->ConnectionEvent;

                              /* Release the lock because we are        */
                              /* finished with it.                      */
                              DEVM_ReleaseLock();

                              BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                              /* Re-acquire the Lock.                   */
                              if(DEVM_AcquireLock())
                              {
                                 if((HFREEntryInfoPtr = DeleteHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG:&HFREEntryInfoList_HF, CallbackID)) != NULL)
                                 {
                                    /* Note the connection status.      */
                                    *ConnectionStatus = HFREEntryInfoPtr->ConnectionStatus;

                                    BTPS_CloseEvent(HFREEntryInfoPtr->ConnectionEvent);

                                    FreeHFREEntryInfoEntryMemory(HFREEntryInfoPtr);

                                    /* Flag success to the caller.      */
                                    ret_val = 0;
                                 }
                                 else
                                    ret_val = BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE;
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
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_ALREADY_CONNECTED;
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_IN_PROGRESS;
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
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
      }
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
   int                                ret_val;
   Boolean_t                          Server;
   Boolean_t                          PerformDisconnect;
   unsigned int                       ServerPort;
   Connection_Entry_t                *ConnectionEntry;
   HFRE_Close_Port_Indication_Data_t  ClosePortIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
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
                  ret_val = _HFRM_Disconnect_Device(ConnectionEntry->HFREID);
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
                  ClosePortIndicationData.HFREPortID      = ConnectionEntry->HFREID;
                  ClosePortIndicationData.PortCloseStatus = HFRE_CLOSE_PORT_STATUS_SUCCESS;

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
                  ret_val = BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_IN_PROGRESS;
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
   int                 ret_val;
   unsigned int        NumberConnected;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(((ConnectionType == hctAudioGateway) && (AudioGatewaySupported)) || ((ConnectionType == hctHandsFree) && (HandsFreeSupported)))
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
         ret_val = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
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
   int          ret_val;
   unsigned int Temp;
   unsigned int NumberIndicators;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((CurrentConfiguration) && ((!CurrentConfiguration->TotalNumberAdditionalIndicators) || ((CurrentConfiguration->TotalNumberAdditionalIndicators) && (CurrentConfiguration->AdditionalIndicatorList))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Flag success to the caller.                              */
            ret_val = 0;

            /* Check to see if Audio Gateway or Hands Free was          */
            /* specified.                                               */
            if(ConnectionType == hctAudioGateway)
            {
               if(AudioGatewaySupported)
               {
                  /* First, initialize that there are no additional     */
                  /* indicators, and initialize the total.              */
                  NumberIndicators                                      = CurrentConfiguration->TotalNumberAdditionalIndicators;

                  CurrentConfiguration->TotalNumberAdditionalIndicators = AudioGatewayInitializationInfo.NumberAdditionalIndicators;
                  CurrentConfiguration->NumberAdditionalIndicators      = 0;

                  if(NumberIndicators)
                  {
                     BTPS_MemInitialize(CurrentConfiguration->AdditionalIndicatorList, 0, NumberIndicators*HFRM_CONFIGURATION_INDICATOR_ENTRY_SIZE);

                     for(ret_val=0;ret_val<(int)NumberIndicators;ret_val++)
                     {
                        BTPS_MemCopy(&(CurrentConfiguration->AdditionalIndicatorList[ret_val].Control_Indicator_Data), &(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ret_val].Control_Indicator_Data), sizeof(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ret_val].Control_Indicator_Data));

                        if(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ret_val].IndicatorDescription)
                           Temp = BTPS_StringLength(AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ret_val].IndicatorDescription);
                        else
                           Temp = 0;

                        if(Temp > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                           Temp = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

                        BTPS_MemCopy(CurrentConfiguration->AdditionalIndicatorList[ret_val].IndicatorDescription, AudioGatewayInitializationInfo.AdditionalSupportedIndicators[ret_val].IndicatorDescription, Temp);

                        ret_val++;
                     }
                  }

                  /* Copy the static information.                       */
                  CurrentConfiguration->IncomingConnectionFlags = AudioGatewayInitializationInfo.IncomingConnectionFlags;
                  CurrentConfiguration->SupportedFeaturesMask   = AudioGatewayInitializationInfo.SupportedFeaturesMask;
                  CurrentConfiguration->CallHoldingSupportMask  = AudioGatewayInitializationInfo.CallHoldingSupportMask;
                  CurrentConfiguration->NetworkType             = AudioGatewayInitializationInfo.NetworkType;

                  /* Check to see if WBS is supported.                  */
                  if(WBS_Support)
                     CurrentConfiguration->SupportedFeaturesMask |= HFRE_AG_CODEC_NEGOTIATION_SUPPORTED_BIT;
               }
               else
                  ret_val = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
            }
            else
            {
               if(HandsFreeSupported)
               {
                  /* First, initialize that there are no additional     */
                  /* indicators, and initialize the total.              */
                  NumberIndicators                                      = CurrentConfiguration->TotalNumberAdditionalIndicators;

                  CurrentConfiguration->TotalNumberAdditionalIndicators = HandsFreeInitializationInfo.NumberAdditionalIndicators;
                  CurrentConfiguration->NumberAdditionalIndicators      = 0;

                  if(NumberIndicators)
                  {
                     BTPS_MemInitialize(CurrentConfiguration->AdditionalIndicatorList, 0, NumberIndicators*HFRM_CONFIGURATION_INDICATOR_ENTRY_SIZE);

                     for(ret_val=0;ret_val<(int)NumberIndicators;ret_val++)
                     {
                        BTPS_MemCopy(&(CurrentConfiguration->AdditionalIndicatorList[ret_val].Control_Indicator_Data), &(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ret_val].Control_Indicator_Data), sizeof(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ret_val].Control_Indicator_Data));

                        if(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ret_val].IndicatorDescription)
                           Temp = BTPS_StringLength(HandsFreeInitializationInfo.AdditionalSupportedIndicators[ret_val].IndicatorDescription);
                        else
                           Temp = 0;

                        if(Temp > HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                           Temp = HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM;

                        BTPS_MemCopy(CurrentConfiguration->AdditionalIndicatorList[ret_val].IndicatorDescription, HandsFreeInitializationInfo.AdditionalSupportedIndicators[ret_val].IndicatorDescription, Temp);

                        ret_val++;
                     }
                  }

                  /* Copy the static information.                       */
                  CurrentConfiguration->IncomingConnectionFlags = HandsFreeInitializationInfo.IncomingConnectionFlags;
                  CurrentConfiguration->SupportedFeaturesMask   = HandsFreeInitializationInfo.SupportedFeaturesMask;
                  CurrentConfiguration->CallHoldingSupportMask  = HandsFreeInitializationInfo.CallHoldingSupportMask;
                  CurrentConfiguration->NetworkType             = HandsFreeInitializationInfo.NetworkType;

                  /* Check to see if WBS is supported.                  */
                  if(WBS_Support)
                     CurrentConfiguration->SupportedFeaturesMask |= HFRE_HF_CODEC_NEGOTIATION_SUPPORTED_BIT;
               }
               else
                  ret_val = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
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
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Flag success to the caller.                                 */
         ret_val = 0;

         /* Check to see if Audio Gateway or Hands Free was specified.  */
         if(ConnectionType == hctAudioGateway)
         {
            if(AudioGatewaySupported)
               AudioGatewayInitializationInfo.IncomingConnectionFlags = ConnectionFlags;
            else
               ret_val = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
         }
         else
         {
            if(HandsFreeSupported)
               HandsFreeInitializationInfo.IncomingConnectionFlags = ConnectionFlags;
            else
               ret_val = BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to disable remote echo/noise           */
                     /* cancellation.                                   */
                     ret_val = _HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set remote voice recognition        */
                     /* activation .                                    */
                     ret_val = _HFRM_Set_Remote_Voice_Recognition_Activation(ConnectionEntry->HFREID, VoiceRecognitionActive);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set speaker gain.                   */
                     ret_val = _HFRM_Set_Remote_Speaker_Gain(ConnectionEntry->HFREID, SpeakerGain);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set microphone gain.                */
                     ret_val = _HFRM_Set_Remote_Microphone_Gain(ConnectionEntry->HFREID, MicrophoneGain);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Send a codec identifier to a remote device. The audio gateway     */
   /* uses this function to select a codec from the list of codecs      */
   /* supported by the hands free device. The hands free device uses    */
   /* the function to confirm the audio gateway's selection. The first  */
   /* parameter is the Callback ID that is returned from                */
   /* HFRM_Register_Event_Callback(). The second parameter is the       */
   /* address of the remote device. The third parameter identifies the  */
   /* codec. This function returns zero if successful and a negative    */
   /* code if there was an error.                                       */
int BTPSAPI HFRM_Send_Select_Codec(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (CodecID))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set microphone gain.                */
                     ret_val = _HFRM_Send_Select_Codec(ConnectionEntry->HFREID, CodecID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to query remote control indicator      */
                     /* status.                                         */
                     ret_val = _HFRM_Query_Remote_Control_Indicator_Status(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to enable remote indicator event       */
                     /* notification.                                   */
                     ret_val = _HFRM_Enable_Remote_Indicator_Event_Notification(ConnectionEntry->HFREID, EnableEventNotification);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to query remote call                   */
                     /* holding/multi-party service support.            */
                     ret_val = _HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send call holding/multi-party       */
                     /* selection.                                      */
                     ret_val = _HFRM_Send_Call_Holding_Multiparty_Selection(ConnectionEntry->HFREID, CallHoldMultipartyHandling, Index);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to enable remote call waiting          */
                     /* notification.                                   */
                     ret_val = _HFRM_Enable_Remote_Call_Waiting_Notification(ConnectionEntry->HFREID, EnableNotification);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to enable remote call line             */
                     /* identification notification.                    */
                     ret_val = _HFRM_Enable_Remote_Call_Line_Identification_Notification(ConnectionEntry->HFREID, EnableNotification);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to dial the specified phone number.    */
                     ret_val = _HFRM_Dial_Phone_Number(ConnectionEntry->HFREID, PhoneNumber);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to dial a phone number (from memory).  */
                     ret_val = _HFRM_Dial_Phone_Number_From_Memory(ConnectionEntry->HFREID, MemoryLocation);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to re-dial the last dialed phone       */
                     /* number.                                         */
                     ret_val = _HFRM_Redial_Last_Phone_Number(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to answer an incoming call.            */
                     ret_val = _HFRM_Answer_Incoming_Call(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to transmit a DTMF code.               */
                     ret_val = _HFRM_Transmit_DTMF_Code(ConnectionEntry->HFREID, DTMFCode);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to issue a voice tag request.          */
                     ret_val = _HFRM_Voice_Tag_Request(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to hang-up an on-going call.           */
                     ret_val = _HFRM_Hang_Up_Call(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to query the remote current calls list.*/
                     ret_val = _HFRM_Query_Remote_Current_Calls_List(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set the network operator selection  */
                     /* format.                                         */
                     ret_val = _HFRM_Set_Network_Operator_Selection_Format(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to query the remote network operator   */
                     /* selection.                                      */
                     ret_val = _HFRM_Query_Remote_Network_Operator_Selection(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to enable remote extended error        */
                     /* results.                                        */
                     ret_val = _HFRM_Enable_Remote_Extended_Error_Result(ConnectionEntry->HFREID, EnableExtendedErrorResults);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to query subscriber number information.*/
                     ret_val = _HFRM_Query_Subscriber_Number_Information(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to query response/hold status.         */
                     ret_val = _HFRM_Query_Response_Hold_Status(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set the incoming call state.        */
                     ret_val = _HFRM_Set_Incoming_Call_State(ConnectionEntry->HFREID, CallState);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ArbitraryCommand) && (BTPS_StringLength(ArbitraryCommand)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the specified arbitrary        */
                     /* command.                                        */
                     ret_val = _HFRM_Send_Arbitrary_Command(ConnectionEntry->HFREID, ArbitraryCommand);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* a local Hands Free service to send the available supported        */
   /* codecs to a remote Audio Gateway. The first parameter is          */
   /* the Callback ID that is returned from a successful call to        */
   /* HFRM_Register_Event_Callback().  The second parameter is the      */
   /* address of the remote device. The third parameter is the number of*/
   /* supported codecs being set. The fourth parameter is a list of the */
   /* CodecIDs supported. This function returns zero if successful and a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function may only be called by a Hands Free Unit.   */
int BTPSAPI HFRM_Send_Available_Codec_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberSupportedCodecs) && (AvailableCodecList))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctHandsFree)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the specified arbitrary        */
                     /* command.                                        */
                     ret_val = _HFRM_Send_Available_Codec_List(ConnectionEntry->HFREID, NumberSupportedCodecs, AvailableCodecList);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberUpdateIndicators) && (UpdateIndicatorList))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to update the specified indicators.    */
                     ret_val = _HFRM_Update_Current_Control_Indicator_Status(ConnectionEntry->HFREID, NumberUpdateIndicators, UpdateIndicatorList);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (IndicatorName) && (BTPS_StringLength(IndicatorName)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to update the specified indicator.     */
                     ret_val = _HFRM_Update_Current_Control_Indicator_Status_By_Name(ConnectionEntry->HFREID, IndicatorName, IndicatorValue);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!PhoneNumber) || ((PhoneNumber) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the call waiting notification. */
                     ret_val = _HFRM_Send_Call_Waiting_Notification(ConnectionEntry->HFREID, PhoneNumber);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber) && (BTPS_StringLength(PhoneNumber) >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the call line indentification  */
                     /* notification.                                   */
                     ret_val = _HFRM_Send_Call_Line_Identification_Notification(ConnectionEntry->HFREID, PhoneNumber);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the ring indication.           */
                     ret_val = _HFRM_Ring_Indication(ConnectionEntry->HFREID);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to enable/disable in-band ring tone.   */
                     ret_val = _HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(ConnectionEntry->HFREID, EnableInBandRing);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!PhoneNumber) || ((PhoneNumber) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send voice tag response.            */
                     ret_val = _HFRM_Voice_Tag_Response(ConnectionEntry->HFREID, PhoneNumber);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!NumberListEntries) || ((NumberListEntries) && (CurrentCallListEntryList))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the current calls list.        */
                     ret_val = _HFRM_Send_Current_Calls_List(ConnectionEntry->HFREID, NumberListEntries, CurrentCallListEntryList);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!NetworkOperator) || ((NetworkOperator) && (BTPS_StringLength(NetworkOperator) <= HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the network operator selection.*/
                     ret_val = _HFRM_Send_Network_Operator_Selection(ConnectionEntry->HFREID, NetworkMode, NetworkOperator);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the extended error result.     */
                     ret_val = _HFRM_Send_Extended_Error_Result(ConnectionEntry->HFREID, ResultCode);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!NumberListEntries) || ((NumberListEntries) && (SubscriberNumberList))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the subscriber number          */
                     /* information list.                               */
                     ret_val = _HFRM_Send_Subscriber_Number_Information(ConnectionEntry->HFREID, NumberListEntries, SubscriberNumberList);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the incoming call state.       */
                     ret_val = _HFRM_Send_Incoming_Call_State(ConnectionEntry->HFREID, CallState);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the terminating response.      */
                     ret_val = _HFRM_Send_Terminating_Response(ConnectionEntry->HFREID, ResultType, ResultValue);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if(HandsFreeManagerEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
            {
               /* Next, check to see if we are powered up.              */
               if(CurrentPowerState)
               {
                  /* Next, determine the connection information.        */
                  ConnectionEntry = ConnectionEntryList;
                  ret_val         = 0;
                  while(ConnectionEntry)
                  {
                     if((!ret_val) && (ConnectionEntry->ConnectionType == hctAudioGateway))
                     {
                        /* Nothing to do here other than to call the    */
                        /* actual function to enable arbitrary command  */
                        /* processing.                                  */
                        ret_val = _HFRM_Enable_Arbitrary_Command_Processing(ConnectionEntry->HFREID);
                     }

                     ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
                  }
               }
               else
                  ret_val = 0;

               if(!ret_val)
                  ArbitraryCommandsEnabled = TRUE;
            }
            else
               ret_val = BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED;

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
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ArbitraryResponse) && (BTPS_StringLength(ArbitraryResponse)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, hctAudioGateway)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the specified arbitrary        */
                     /* response.                                       */
                     ret_val = _HFRM_Send_Arbitrary_Response(ConnectionEntry->HFREID, ArbitraryResponse);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to set-up the audio connection.        */
                     ret_val = _HFRM_Setup_Audio_Connection(ConnectionEntry->HFREID);

                     if(!ret_val)
                        ConnectionEntry->OutgoingSCOAttempt = TRUE;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to release the audio connection.       */
                     ret_val = _HFRM_Release_Audio_Connection(ConnectionEntry->HFREID);

                     if(!ret_val)
                        ConnectionEntry->SCOHandle = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerDataEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (AudioDataLength) && (AudioData))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Data:&HFREEntryInfoList_HF_Data, HandsFreeManagerDataEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL)
                  {
                     /* Nothing to do here other than to call the actual*/
                     /* function to send the audio data.                */
                     ret_val = _HFRM_Send_Audio_Data(ConnectionEntry->HFREID, AudioDataLength, AudioData);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerDataEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
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
               HFREEntryInfo.ClientID          = MSG_GetServerAddressID();
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
                  /* Control handler, add it the correct list.          */
                  if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, &HFREEntryInfo)) != NULL)
                     ret_val = HFREEntryInfoPtr->CallbackID;
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
   Boolean_t               ControlEvent = FALSE;
   HFRE_Entry_Info_t      *HFREEntryInfo;
   Connection_Entry_t     *ConnectionEntry;
   Connection_Entry_t     *CloseConnectionEntry;
   HFRM_Connection_Type_t  ConnectionType;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HandsFreeManagerEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* We need to determine what type of Callback this is (as we*/
            /* process them differently).                               */
            ConnectionType = hctAudioGateway;
            if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG, HandsFreeManagerEventCallbackID)) == NULL)
            {
               if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG_Control, HandsFreeManagerEventCallbackID)) == NULL)
               {
                  ConnectionType = hctHandsFree;
                  if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF, HandsFreeManagerEventCallbackID)) == NULL)
                  {
                     HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID);
                     if(HFREEntryInfo)
                        ControlEvent  = TRUE;
                  }
               }
               else
                  ControlEvent = TRUE;
            }

            /* Check to see if we found the callback and deleted it.    */
            if(HFREEntryInfo)
            {
               /* Free the memory because we are finished with it.      */
               FreeHFREEntryInfoEntryMemory(HFREEntryInfo);

               if(ControlEvent)
               {
                  /* Remove the SDP Record.                                   */
                  _HFRM_UpdateSDPRecord(ConnectionType, FALSE);

                  /* Check to see if there is an open connection.             */
                  ConnectionEntry = ConnectionEntryList;
                  while(ConnectionEntry)
                  {
                     /* Check to see if the AudioGateway control is being     */
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
                        _HFRM_Disconnect_Device(CloseConnectionEntry->HFREID);

                        if((CloseConnectionEntry = DeleteConnectionEntryHFREID(&ConnectionEntryList, CloseConnectionEntry->HFREID)) != NULL)
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

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Hands Free Manager has been initialized, let's check the input */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a data event handler registered.                 */
            if(ConnectionType == hctAudioGateway)
               HFREEntryInfoPtr = HFREEntryInfoList_AG_Data;
            else
               HFREEntryInfoPtr = HFREEntryInfoList_HF_Data;

            if(!HFREEntryInfoPtr)
            {
               /* First, register the handler locally.                  */
               BTPS_MemInitialize(&HFREEntryInfo, 0, sizeof(HFRE_Entry_Info_t));

               HFREEntryInfo.CallbackID        = GetNextCallbackID();
               HFREEntryInfo.ClientID          = MSG_GetServerAddressID();
               HFREEntryInfo.EventCallback     = CallbackFunction;
               HFREEntryInfo.CallbackParameter = CallbackParameter;
               HFREEntryInfo.Flags             = HFRE_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               if((HFREEntryInfoPtr = AddHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Data:&HFREEntryInfoList_HF_Data, &HFREEntryInfo)) != NULL)
               {
                  /* Data handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  ret_val = HFREEntryInfo.CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_ALREADY_REGISTERED;

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
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Delete the local handler.                                */
            if((HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_AG_Data, HandsFreeManagerDataCallbackID)) == NULL)
               HFREEntryInfo = DeleteHFREEntryInfoEntry(&HFREEntryInfoList_HF_Data, HandsFreeManagerDataCallbackID);

            if(HFREEntryInfo)
            {
               /* All finished with the entry, delete it.               */
               FreeHFREEntryInfoEntryMemory(HFREEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
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
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Headset Manager has been            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Headset Manager has been initialized, let's check the input    */
      /* parameters to see if they are semi-valid.                      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (SCOHandle))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHFREEntryInfoEntry((ConnectionType == hctAudioGateway)?&HFREEntryInfoList_AG_Control:&HFREEntryInfoList_HF_Control, HandsFreeManagerEventCallbackID))
               {
                  /* Next, determine the connection information.        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress, ConnectionType)) != NULL) && (ConnectionEntry->ConnectionState == csConnected) && (ConnectionEntry->SCOHandle))
                  {
                     *SCOHandle = ConnectionEntry->SCOHandle;
                     ret_val    = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
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
         if(HandsFreeManagerEventCallbackID)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}
