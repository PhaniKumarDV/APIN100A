/*****< hfrmgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HFRMGR - Hands Free Manager Implementation for Stonestreet One Bluetooth  */
/*           Protocol Stack Platform Manager.                                 */
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

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHFRM.h"            /* BTPM HFRE Manager Prototypes/Constants.   */
#include "HFRMMSG.h"             /* BTPM HFRE Manager Message Formats.        */
#include "HFRMGR.h"              /* HFR Manager Impl. Prototypes/Constants.   */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

typedef struct _tagHFRE_Server_Entry_t
{
   unsigned int                    HFREID;
} HFRE_Server_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _HFRM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variables which hold the initialization information that is to be */
   /* used when the device powers on (registered once at                */
   /* initialization).                                                  */
static Boolean_t                  AudioGatewaySupported;
static HFRM_Initialization_Data_t _AudioGatewayInitializationInfo;

static Boolean_t                  HandsFreeSupported;
static HFRM_Initialization_Data_t _HandsFreeInitializationInfo;

static DWord_t      HandsFreeServerSDPHandle;
static DWord_t      AudioGatewayServerSDPHandle;

static HFRE_Server_Entry_t *HandsFreeServerList;
static HFRE_Server_Entry_t *AudioGatewayServerList;

   /* Internal Function Prototypes.                                     */
static Boolean_t IsServerType(HFRM_Connection_Type_t Type, unsigned int HFREID);
static void CleanupServers(HFRM_Connection_Type_t Type, Boolean_t FreeList);

static void BTPSAPI HFRE_Event_Callback(unsigned int BluetoothStackID, HFRE_Event_Data_t *HFRE_Event_Data, unsigned long CallbackParameter);

static Boolean_t IsServerType(HFRM_Connection_Type_t Type, unsigned int HFREID)
{
   Boolean_t            Found = FALSE;
   unsigned int         Index;
   unsigned int         Max;
   HFRE_Server_Entry_t *List;

   List = (Type == hctAudioGateway)?AudioGatewayServerList:HandsFreeServerList;
   Max  = (Type == hctAudioGateway)?_AudioGatewayInitializationInfo.MaximumNumberServers:_HandsFreeInitializationInfo.MaximumNumberServers;

   for(Index=0; Index<Max && !Found; Index++)
   {
      if(List[Index].HFREID == HFREID)
         Found = TRUE;
   }

   return(Found);
}

static void CleanupServers(HFRM_Connection_Type_t Type, Boolean_t FreeList)
{
   unsigned int         Index;
   unsigned int         Max;
   HFRE_Server_Entry_t **List;

   /* Note which list to use.                                           */
   List = (Type == hctAudioGateway)?&AudioGatewayServerList:&HandsFreeServerList;
   Max  = (Type == hctAudioGateway)?_AudioGatewayInitializationInfo.MaximumNumberServers:_HandsFreeInitializationInfo.MaximumNumberServers;

   /* Close all servers.                                                */
   for(Index=0; Index<Max; Index++)
   {
      if((*List)[Index].HFREID)
      {
         HFRE_Close_Server_Port(_BluetoothStackID, (*List)[Index].HFREID);
         (*List)[Index].HFREID = 0;
      }
   }

   /* Delete the list's memory.                                         */
   if(FreeList)
   {
      BTPS_FreeMemory(*List);
      *List = NULL;
   }
}

   /* The following function the function that is installed to process  */
   /* Hands Free Events from the stack.                                 */
static void BTPSAPI HFRE_Event_Callback(unsigned int BluetoothStackID, HFRE_Event_Data_t *HFRE_Event_Data, unsigned long CallbackParameter)
{
   HFRM_Update_Data_t *HFRMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((HFRE_Event_Data) && (HFRE_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no event to dispatch.                       */
      HFRMUpdateData = NULL;

      switch(HFRE_Event_Data->Event_Data_Type)
      {
         case etHFRE_Open_Port_Indication:
         case etHFRE_Open_Port_Confirmation:
         case etHFRE_Open_Service_Level_Connection_Indication:
         case etHFRE_Call_Hold_Multiparty_Support_Confirmation:
         case etHFRE_Call_Hold_Multiparty_Selection_Indication:
         case etHFRE_Call_Waiting_Notification_Activation_Indication:
         case etHFRE_Call_Line_Identification_Notification_Activation_Indication:
         case etHFRE_Disable_Sound_Enhancement_Indication:
         case etHFRE_Dial_Phone_Number_From_Memory_Indication:
         case etHFRE_ReDial_Last_Phone_Number_Indication:
         case etHFRE_Ring_Indication:
         case etHFRE_Generate_DTMF_Tone_Indication:
         case etHFRE_Answer_Call_Indication:
         case etHFRE_InBand_Ring_Tone_Setting_Indication:
         case etHFRE_Voice_Recognition_Notification_Indication:
         case etHFRE_Speaker_Gain_Indication:
         case etHFRE_Microphone_Gain_Indication:
         case etHFRE_Voice_Tag_Request_Indication:
         case etHFRE_Hang_Up_Indication:
         case etHFRE_Audio_Connection_Indication:
         case etHFRE_Audio_Disconnection_Indication:
         case etHFRE_Close_Port_Indication:
         case etHFRE_Current_Calls_List_Indication:
         case etHFRE_Network_Operator_Selection_Format_Indication:
         case etHFRE_Network_Operator_Selection_Indication:
         case etHFRE_Extended_Error_Result_Activation_Indication:
         case etHFRE_Subscriber_Number_Information_Indication:
         case etHFRE_Response_Hold_Status_Indication:
         case etHFRE_Response_Hold_Status_Confirmation:
         case etHFRE_Incoming_Call_State_Indication:
         case etHFRE_Incoming_Call_State_Confirmation:
         case etHFRE_Command_Result:
         case etHFRE_Codec_Select_Request_Indication:
         case etHFRE_Codec_Select_Confirmation:
         case etHFRE_Codec_Connection_Setup_Indication:
         case etHFRE_Open_Port_Request_Indication:
            /* Allocate memory to hold the event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((HFRE_Event_Data->Event_Data.HFRE_Open_Port_Request_Indication_Data) && (HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the event data into the  */
               /* notification structure.                               */
               HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
               HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Open_Port_Request_Indication_Data, HFRE_Event_Data->Event_Data_Size);
            }
            break;
         case etHFRE_Control_Indicator_Status_Indication:
         case etHFRE_Control_Indicator_Status_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM + 1)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.IndicatorDescription)
                  {
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Control_Indicator_Status_Indication_Data.HFREControlIndicatorEntry.IndicatorDescription = ((char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                     /* Now copy the indicator descriptor.              */
                     BTPS_StringCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Control_Indicator_Status_Indication_Data.HFREControlIndicatorEntry.IndicatorDescription, HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.IndicatorDescription);
                  }
                  else
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Control_Indicator_Status_Indication_Data.HFREControlIndicatorEntry.IndicatorDescription = NULL;
               }
            }
            break;
         case etHFRE_Call_Waiting_Notification_Indication:
         case etHFRE_Call_Line_Identification_Notification_Indication:
         case etHFRE_Dial_Phone_Number_Indication:
         case etHFRE_Voice_Tag_Request_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Indication_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Indication_Data->PhoneNumber)
                  {
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Call_Waiting_Notification_Indication_Data.PhoneNumber = ((char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                     /* Now copy the phone number.                      */
                     BTPS_StringCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Call_Waiting_Notification_Indication_Data.PhoneNumber, HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Indication_Data->PhoneNumber);
                  }
                  else
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Call_Waiting_Notification_Indication_Data.PhoneNumber = NULL;
               }
            }
            break;
         case etHFRE_Current_Calls_List_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhoneNumber)
                  {
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Current_Calls_List_Confirmation_Data.HFRECurrentCallListEntry.PhoneNumber = ((char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                     /* Now copy the phone number.                      */
                     BTPS_StringCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Current_Calls_List_Confirmation_Data.HFRECurrentCallListEntry.PhoneNumber, HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhoneNumber);
                  }
                  else
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Current_Calls_List_Confirmation_Data.HFRECurrentCallListEntry.PhoneNumber = NULL;
               }
            }
            break;
         case etHFRE_Network_Operator_Selection_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM + 1)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data->NetworkOperator)
                  {
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Network_Operator_Selection_Confirmation_Data.NetworkOperator = ((char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                     /* Now copy the network operator.                  */
                     BTPS_StringCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Network_Operator_Selection_Confirmation_Data.NetworkOperator, HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data->NetworkOperator);
                  }
                  else
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Network_Operator_Selection_Confirmation_Data.NetworkOperator = NULL;
               }
            }
            break;
         case etHFRE_Subscriber_Number_Information_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + HFRE_PHONE_NUMBER_LENGTH_MAXIMUM + 1)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data->PhoneNumber)
                  {
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Subscriber_Number_Information_Confirmation_Data.PhoneNumber = ((char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                     /* Now copy the phone number.                      */
                     BTPS_StringCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Subscriber_Number_Information_Confirmation_Data.PhoneNumber, HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data->PhoneNumber);
                  }
                  else
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Subscriber_Number_Information_Confirmation_Data.PhoneNumber = NULL;
               }
            }
            break;
         case etHFRE_Arbitrary_Command_Indication:
         case etHFRE_Arbitrary_Response_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Arbitrary_Command_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + (HFRE_Event_Data->Event_Data.HFRE_Arbitrary_Command_Indication_Data->HFRECommandData?BTPS_StringLength(HFRE_Event_Data->Event_Data.HFRE_Arbitrary_Command_Indication_Data->HFRECommandData):0) + 1)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Arbitrary_Command_Indication_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HFRE_Event_Data->Event_Data.HFRE_Arbitrary_Command_Indication_Data->HFRECommandData)
                  {
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Arbitrary_Command_Indication_Data.HFRECommandData = ((char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                     /* Now copy the arbitrary command/response data.   */
                     BTPS_StringCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Arbitrary_Command_Indication_Data.HFRECommandData, HFRE_Event_Data->Event_Data.HFRE_Arbitrary_Command_Indication_Data->HFRECommandData);
                  }
                  else
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Arbitrary_Command_Indication_Data.HFRECommandData = NULL;
               }
            }
            break;
         case etHFRE_Audio_Data_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioDataLength)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  /* Copy the event structure.                          */
                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Audio_Data_Indication_Data), HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Assign the audio data pointer to the end of the    */
                  /* update data structure, note that we allocated      */
                  /* additional memory for the data just above.         */
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Audio_Data_Indication_Data.AudioData = ((unsigned char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                  /* Copy the data.                                     */
                  BTPS_MemCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Audio_Data_Indication_Data.AudioData, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioData, HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Audio_Data_Indication_Data.AudioDataLength);
               }
            }
            break;
         case etHFRE_Available_Codec_List_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HFRMUpdateData = (HFRM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HFRM_Update_Data_t) + HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data->NumSupportedCodecs)) != NULL) 
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HFRMUpdateData->UpdateType                              = utHandsFreeEvent;
                  HFRMUpdateData->UpdateData.HandsFreeEventData.EventType = HFRE_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData), HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data, HFRE_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data->AvailableCodecList)
                  {
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Available_Codec_List_Indication_Data.AvailableCodecList = ((unsigned char *)HFRMUpdateData) + sizeof(HFRM_Update_Data_t);

                     /* Now copy the available codec list.                      */
                     BTPS_MemCopy(HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Available_Codec_List_Indication_Data.AvailableCodecList, HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data->AvailableCodecList, HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Available_Codec_List_Indication_Data.NumSupportedCodecs);
                  }
                  else
                     HFRMUpdateData->UpdateData.HandsFreeEventData.EventData.HFRE_Available_Codec_List_Indication_Data.AvailableCodecList = NULL;
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(HFRMUpdateData)
      {
         if(!HFRM_NotifyUpdate(HFRMUpdateData))
            BTPS_FreeMemory((void *)HFRMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Hands Free Manager implementation.  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Hands Free Manager Implementation.                                */
int _HFRM_Initialize(HFRM_Initialization_Data_t *AudioGatewayInitializationInfo, HFRM_Initialization_Data_t *HandsFreeInitializationInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Hands Free Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */

      /* First, check to see if Audio Gateway information was specified.*/
      if((AudioGatewayInitializationInfo) && (AudioGatewayInitializationInfo->ServerPort >= SPP_PORT_NUMBER_MINIMUM) && (AudioGatewayInitializationInfo->ServerPort <= SPP_PORT_NUMBER_MAXIMUM) && ((!AudioGatewayInitializationInfo->NumberAdditionalIndicators) || ((AudioGatewayInitializationInfo->NumberAdditionalIndicators) && (AudioGatewayInitializationInfo->AdditionalSupportedIndicators))) && (AudioGatewayInitializationInfo->MaximumNumberServers))
      {
         /* Attempt to allocate the memory to track the maximum number  */
         /* of connections.                                             */
         if((AudioGatewayServerList = (HFRE_Server_Entry_t *)BTPS_AllocateMemory(sizeof(HFRE_Server_Entry_t) * AudioGatewayInitializationInfo->MaximumNumberServers)) != NULL)
         {
            BTPS_MemInitialize(AudioGatewayServerList, 0, sizeof(HFRE_Server_Entry_t) * (AudioGatewayInitializationInfo->MaximumNumberServers));

            AudioGatewaySupported           = TRUE;
            AudioGatewayServerSDPHandle     = 0;

            _AudioGatewayInitializationInfo = *AudioGatewayInitializationInfo;
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to allocate Audio Gateway Server List"));

      }

      /* Next, check to see if Hands Free information was specified.    */
      if((HandsFreeInitializationInfo) && (HandsFreeInitializationInfo->ServerPort >= SPP_PORT_NUMBER_MINIMUM) && (HandsFreeInitializationInfo->ServerPort <= SPP_PORT_NUMBER_MAXIMUM) && ((!HandsFreeInitializationInfo->NumberAdditionalIndicators) || ((HandsFreeInitializationInfo->NumberAdditionalIndicators) && (HandsFreeInitializationInfo->AdditionalSupportedIndicators))) && (HandsFreeInitializationInfo->MaximumNumberServers))
      {
         /* Attempt to allocate the memory to track the maximum number  */
         /* of connections.                                             */
         if((HandsFreeServerList = (HFRE_Server_Entry_t *)BTPS_AllocateMemory(sizeof(HFRE_Server_Entry_t) * HandsFreeInitializationInfo->MaximumNumberServers)) != NULL)
         {
            BTPS_MemInitialize(HandsFreeServerList, 0, sizeof(HFRE_Server_Entry_t) * (HandsFreeInitializationInfo->MaximumNumberServers));

            HandsFreeSupported           = TRUE;
            HandsFreeServerSDPHandle     = 0;

            _HandsFreeInitializationInfo = *HandsFreeInitializationInfo;
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to allocate HandsFree Server List"));
      }

      /* Only initialize if either Audio Gateway or Hands Free is valid.*/
      if((AudioGatewaySupported) || (HandsFreeSupported))
      {
         /* Flag that this module is initialized.                       */
         Initialized = TRUE;

         ret_val     = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_HANDS_FREE_INITIALIZATION_DATA;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the Hands */
   /* Free Manager implementation.  After this function is called the   */
   /* Hands Free Manager implementation will no longer operate until it */
   /* is initialized again via a call to the _HFRM_Initialize()         */
   /* function.                                                         */
void _HFRM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Flag that neither Audio Gateway nor Hands Free has been        */
      /* initialized.                                                   */
      AudioGatewaySupported = FALSE;
      HandsFreeSupported    = FALSE;

      /* Finally flag that this module is no longer initialized.        */
      Initialized           = FALSE;

      /* Cleanup any servers that may be active.                        */
      if(AudioGatewayServerList)
         CleanupServers(hctAudioGateway, TRUE);

      if(AudioGatewayServerSDPHandle)
         SDP_Delete_Service_Record(_BluetoothStackID, AudioGatewayServerSDPHandle);

      if(HandsFreeServerList)
         CleanupServers(hctHandsFree, TRUE);

      if(HandsFreeServerSDPHandle)
         SDP_Delete_Service_Record(_BluetoothStackID, HandsFreeServerSDPHandle);

      AudioGatewayServerSDPHandle = 0;
      HandsFreeServerSDPHandle    = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the Hands Free*/
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Hands Free Manager with the */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
Boolean_t _HFRM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int                              Result;
   Byte_t                           Status;
   char                           **AdditionalIndicators;
   Byte_t                           NumberUUIDS;
   Byte_t                           EIRData[2 + (UUID_16_SIZE * 2)];
   Boolean_t                        ret_val = FALSE;
   Boolean_t                        AudioGatewayEnabled = FALSE;
   Boolean_t                        HandsFreeEnabled    = FALSE;
   UUID_16_t                        tempUUID;
   unsigned int                     Index;
   unsigned int                     EIRDataLength;
   HCI_Local_Supported_Codec_Info_t Codecs;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Flag that no UUIDs have been added to the EIR data as of    */
         /* now.                                                        */
         NumberUUIDS = 0;

         /* Determine if the controller support WBS.                    */
         Result = HCI_Read_Local_Supported_Codecs(BluetoothStackID, &Status, &Codecs);
         if((!Result) && (!Status) && (Codecs.NumberOfSupportedCodecs) && (Codecs.SupportedCodecs))
         {
            /* Loop until all Codecs have been queried or we reach the  */
            /* mSBC codec.                                              */
            Index = 0;
            while(Index < Codecs.NumberOfSupportedCodecs)
            {
               if(Codecs.SupportedCodecs[Index] == HCI_AIR_MODE_FORMAT_MODIFIED_SBC)
               {
                  _AudioGatewayInitializationInfo.SupportedFeaturesMask |= HFRE_AG_CODEC_NEGOTIATION_SUPPORTED_BIT;
                  _HandsFreeInitializationInfo.SupportedFeaturesMask    |= HFRE_HF_CODEC_NEGOTIATION_SUPPORTED_BIT;
                  ret_val                                                = TRUE;
                  break;
               }

               Index++;
            }

            /* Free the Memory allocated by the Read Function.          */
            HCI_Free_Local_Supported_Codec_Info(&Codecs);
         }

         /* Stack has been powered up.                                  */
         if(AudioGatewaySupported)
         {
            Result = 0;

            for(Index=0; Index<_AudioGatewayInitializationInfo.MaximumNumberServers && Result >= 0; Index++)
            {
               Result = HFRE_Open_Audio_Gateway_Server_Port(BluetoothStackID, _AudioGatewayInitializationInfo.ServerPort, _AudioGatewayInitializationInfo.SupportedFeaturesMask, _AudioGatewayInitializationInfo.CallHoldingSupportMask, _AudioGatewayInitializationInfo.NumberAdditionalIndicators, _AudioGatewayInitializationInfo.AdditionalSupportedIndicators, HFRE_Event_Callback, 0);

               if(Result > 0)
               {
                  /* Audio Gateway initialized successfully.               */
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Gateway Server[%u] Initialized\n", Index));

                  /* Note the Audio Gateway Server ID.                     */
                  AudioGatewayServerList[Index].HFREID = (unsigned int)Result;

                  /* Go ahead and set the incoming connection mode to      */
                  /* Manual Accept.                                        */
                  HFRE_Set_Server_Mode(BluetoothStackID, AudioGatewayServerList[Index].HFREID, HFRE_SERVER_MODE_MANUAL_ACCEPT_CONNECTION);

               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Gateway Server[%u] Failed to open\n", Index));
            }

            /* Now check that all servers were opened.                  */
            if(Result > 0)
            {
               /* Assign the Handsfree Audio Gateway UUID (in big-endian*/
               /* format).                                              */
               SDP_ASSIGN_HANDSFREE_AUDIO_GATEWAY_PROFILE_UUID_16(tempUUID);

               /* Convert the UUID to little endian as required by EIR  */
               /* data.                                                 */
               CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

               /* Increment the number of UUIDs.                        */
               NumberUUIDS++;

               /* Note that we have succesully brought up Audi Gateway. */
               AudioGatewayEnabled = TRUE;
            }
         }

         if(HandsFreeSupported)
         {
            /* The additional indicators for Hands Free is simply a name*/
            /* list, so we will need to format this correctly.          */
            if(_HandsFreeInitializationInfo.NumberAdditionalIndicators)
            {
               if((AdditionalIndicators = (char **)BTPS_AllocateMemory(_HandsFreeInitializationInfo.NumberAdditionalIndicators*sizeof(char *))) != NULL)
               {
                  for(Index=0;Index<_HandsFreeInitializationInfo.NumberAdditionalIndicators;Index++)
                     AdditionalIndicators[Index] = _HandsFreeInitializationInfo.AdditionalSupportedIndicators[Index].IndicatorDescription;
               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               AdditionalIndicators = NULL;

            for(Index=0; Index<_HandsFreeInitializationInfo.MaximumNumberServers && Result >= 0; Index++)
            {
               Result = HFRE_Open_HandsFree_Server_Port(BluetoothStackID, _HandsFreeInitializationInfo.ServerPort, _HandsFreeInitializationInfo.SupportedFeaturesMask, _HandsFreeInitializationInfo.NumberAdditionalIndicators, AdditionalIndicators, HFRE_Event_Callback, 0);

               if(Result > 0)
               {
                  /* Hands Free initialized successfully.               */
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Server[%u] Initialized\n", Index));

                  /* Note the Hands Free Server ID.                     */
                  HandsFreeServerList[Index].HFREID = (unsigned int)Result;

                  /* Go ahead and set the incoming connection mode to   */
                  /* Manual Accept.                                     */
                  HFRE_Set_Server_Mode(BluetoothStackID, HandsFreeServerList[Index].HFREID, HFRE_SERVER_MODE_MANUAL_ACCEPT_CONNECTION);

               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Server[%u] Failed to open\n", Index));
            }

            /* Free the indicators of the were allocated.               */
            if(AdditionalIndicators)
               BTPS_FreeMemory(AdditionalIndicators);

            /* Now make sure we opened all server.                      */
            if(Result > 0)
            {
               /* Assign the Handsfree Role UUID (in big-endian format).*/
               SDP_ASSIGN_HANDSFREE_PROFILE_UUID_16(tempUUID);

               /* Convert the UUID to little endian as required by EIR  */
               /* data.                                                 */
               CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

               /* Increment the number of UUIDs.                        */
               NumberUUIDS++;

               /* Note that we have succesully brought up Hands Feee.   */
               HandsFreeEnabled = TRUE;
            }
         }

         if(((!AudioGatewaySupported) || ((AudioGatewaySupported) && (AudioGatewayEnabled))) && ((!HandsFreeSupported) || ((HandsFreeSupported) && (HandsFreeEnabled))))
         {
            /* Hands Free Manager initialized successfully.             */
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Hands Free Manager Initialized, Audio Gateway: %d, Hands Free: %d\n", AudioGatewaySupported, HandsFreeSupported));

            _BluetoothStackID = BluetoothStackID;

            /* Configure the EIR data if necessary.                     */
            if(NumberUUIDS > 0)
            {
               /* Assign the length byte based on the number of UUIDs in*/
               /* the list.                                             */
               EIRDataLength  = (NON_ALIGNED_BYTE_SIZE + (NumberUUIDS*UUID_16_SIZE));

               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

               /* Increment the length we pass to the internal function */
               /* to take into account the length byte.                 */
               EIRDataLength += NON_ALIGNED_BYTE_SIZE;

               /* Configure the EIR data.                               */
               MOD_AddEIRData(EIRDataLength, EIRData);
            }
         }
         else
         {
            /* Error initializing Audio Framework.                      */
            DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Framework NOT Initialized: Audio Gateway: %d, Hands Free: %d\n", AudioGatewaySupported, HandsFreeSupported));

            /* Error, clean up anything that might have been            */
            /* initialized.                                             */
            if(AudioGatewayServerList)
               CleanupServers(hctAudioGateway, FALSE);

            if(AudioGatewayServerSDPHandle)
               SDP_Delete_Service_Record(BluetoothStackID, AudioGatewayServerSDPHandle);

            if(HandsFreeServerList)
               CleanupServers(hctHandsFree, FALSE);

            if(HandsFreeServerSDPHandle)
               SDP_Delete_Service_Record(BluetoothStackID, HandsFreeServerSDPHandle);

            AudioGatewayServerSDPHandle = 0;
            HandsFreeServerSDPHandle    = 0;
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         if(AudioGatewayServerList)
            CleanupServers(hctAudioGateway, FALSE);

         if(HandsFreeServerList)
            CleanupServers(hctHandsFree, FALSE);

         _BluetoothStackID    = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function is responsible for installing/removing the */
   /* Hands Free or AufioGateway SDP record.                            */
int _HFRM_UpdateSDPRecord(HFRM_Connection_Type_t ConnectionType, Boolean_t Install)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(ConnectionType == hctAudioGateway)
      {
         if(Install)
         {
            /* Make sure the server list is initialized. */
            if(AudioGatewayServerList)
            {
               /* Register an SDP Record for the port.                  */
               /* * NOTE * Since the SDP record is not really tied to   */
               /*          specific HFRE ID in any way, just use the    */
               /*          first one in the list.                       */
               Result = HFRE_Register_Audio_Gateway_SDP_Record(_BluetoothStackID, AudioGatewayServerList[0].HFREID, _AudioGatewayInitializationInfo.NetworkType, _AudioGatewayInitializationInfo.ServiceName, &AudioGatewayServerSDPHandle);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
         }
         else
         {
            if(AudioGatewayServerSDPHandle)
            {
               Result = SDP_Delete_Service_Record(_BluetoothStackID, AudioGatewayServerSDPHandle);
               AudioGatewayServerSDPHandle = 0;
            }
            else
               Result = 0;
         }
      }
      else
      {
         if(Install)
         {
            /* Make sure the server list is initialized. */
            if(HandsFreeServerList)
            {
               /* Register an SDP Record for the port.                  */
               /* * NOTE * Since the SDP record is not really tied to   */
               /*          specific HFRE ID in any way, just use the    */
               /*          first one in the list.                       */
               Result = HFRE_Register_HandsFree_SDP_Record(_BluetoothStackID, HandsFreeServerList[0].HFREID, _HandsFreeInitializationInfo.ServiceName, &HandsFreeServerSDPHandle);
            }
            else
               Result = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
         }
         else
         {
            if(HandsFreeServerSDPHandle)
            {
               Result = SDP_Delete_Service_Record(_BluetoothStackID, HandsFreeServerSDPHandle);
               HandsFreeServerSDPHandle = 0;
            }
            else
               Result = 0;
         }
      }
   }
   else
      Result = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(Result);
}

   /* The following function is a utility function that exists to allow */
   /* the caller a mechanism to determine the incoming connection type  */
   /* of the specified incoming connection (based on the Hands Free Port*/
   /* ID).                                                              */
Boolean_t _HFRM_QueryIncomingConnectionType(unsigned int HFREID, HFRM_Connection_Type_t *ConnectionType, unsigned int *ServerPort)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (ConnectionType) && (HFREID))
   {
      /* Check whether this HFREID matches one of the Audio Gateway     */
      /* Servers.                                                       */
      if(IsServerType(hctAudioGateway, HFREID))
      {
         *ConnectionType = hctAudioGateway;

         /* If the caller would like the server port, then be sure we   */
         /* note the port.                                              */
         if(ServerPort)
            *ServerPort = _AudioGatewayInitializationInfo.ServerPort;

         ret_val = TRUE;
      }
      else
      {
         /* Check if this HFREID matches one of the Hands Free Servers. */
         if(IsServerType(hctHandsFree, HFREID))
         {
            *ConnectionType = hctHandsFree;

            /* If the caller would like the server port, then be sure we*/
            /* note the port.                                           */
            if(ServerPort)
               *ServerPort = _HandsFreeInitializationInfo.ServerPort;

            ret_val         = TRUE;
         }
         else
            ret_val = FALSE;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server. This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened. A   */
   /*          port open indication event will notify of this status.   */
int _HFRM_Connection_Request_Response(unsigned int HFREID, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Type: %s\n", (IsServerType(hctAudioGateway, HFREID))?"Audio Gateway":"Hands Free"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HFREID))
   {
      /* Next, check to see if the device is connecting.                */
      if(IsServerType(hctAudioGateway, HFREID) || IsServerType(hctHandsFree, HFREID))
      {
         /* Server entry was found and the specified device is          */
         /* connecting.                                                 */
         if(!HFRE_Open_Port_Request_Response(_BluetoothStackID, HFREID, AcceptConnection))
            ret_val = 0;
         else
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Hands Free/Audio Gateway device.   */
   /* This function returns a non zero value if successful, or a        */
   /* negative return error code if there was an error.  The return     */
   /* value from this function (if successful) represents the Bluetopia */
   /* Hands Free ID that is used to track this connection.  This        */
   /* function accepts the connection type to make as the first         */
   /* parameter.  This parameter specifies the LOCAL connection type    */
   /* (i.e. if the caller would like to connect the local Hands Free    */
   /* service to a remote Audio Gateway device, the Hands Free          */
   /* connection type would be specified for this parameter).  This     */
   /* function also accepts the connection information for the remote   */
   /* device (address and server port).                                 */
int _HFRM_Connect_Remote_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort)
{
   int            ret_val;
   char         **AdditionalIndicators;
   unsigned int   Index;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Local Type: %s, Port: %d\n", (ConnectionType == hctAudioGateway)?"Audio Gateway":"Hands Free", RemoteServerPort));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort >= SPP_PORT_NUMBER_MINIMUM) && (RemoteServerPort <= SPP_PORT_NUMBER_MAXIMUM))
      {
         /* Attempt to make the outgoing connetion.                     */
         if(ConnectionType == hctAudioGateway)
            ret_val = HFRE_Open_Remote_HandsFree_Port(_BluetoothStackID, RemoteDeviceAddress, RemoteServerPort, _AudioGatewayInitializationInfo.SupportedFeaturesMask, _AudioGatewayInitializationInfo.CallHoldingSupportMask, _AudioGatewayInitializationInfo.NumberAdditionalIndicators, _AudioGatewayInitializationInfo.AdditionalSupportedIndicators, HFRE_Event_Callback, 0);
         else
         {
            /* Hands Free supported indicators are a list of character  */
            /* pointers so we need to build this.                       */
            if(_HandsFreeInitializationInfo.NumberAdditionalIndicators)
            {
               if((AdditionalIndicators = (char **)BTPS_AllocateMemory(_HandsFreeInitializationInfo.NumberAdditionalIndicators*sizeof(char *))) != NULL)
               {
                  for(Index=0;Index<_HandsFreeInitializationInfo.NumberAdditionalIndicators;Index++)
                     AdditionalIndicators[Index] = _HandsFreeInitializationInfo.AdditionalSupportedIndicators[Index].IndicatorDescription;

                  ret_val = HFRE_Open_Remote_Audio_Gateway_Port(_BluetoothStackID, RemoteDeviceAddress, RemoteServerPort, _HandsFreeInitializationInfo.SupportedFeaturesMask, _HandsFreeInitializationInfo.NumberAdditionalIndicators, AdditionalIndicators, HFRE_Event_Callback, 0);

                  BTPS_FreeMemory(AdditionalIndicators);
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = HFRE_Open_Remote_Audio_Gateway_Port(_BluetoothStackID, RemoteDeviceAddress, RemoteServerPort, _HandsFreeInitializationInfo.SupportedFeaturesMask, 0, NULL, HFRE_Event_Callback, 0);
         }

         /* If an error occurred, we need to map it to a valid error    */
         /* code.                                                       */
         if(ret_val <= 0)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Disconnect_Device(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and disconnect the device.                         */
         ret_val = HFRE_Close_Port(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_DISCONNECT_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for disabling echo cancellation and  */
   /* noise reduction on the remote device.  This function may be       */
   /* performed by both the Hands Free and the Audio Gateway connections*/
   /* for which a valid service level connection exists but no audio    */
   /* connection exists.  This function accepts as its input parameter  */
   /* the Hands Free Port ID indicating the connection that is to       */
   /* receive this command.  This function returns zero if successful or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * It is not possible to enable this feature once it has    */
   /*          been disbled because the specification provides no means */
   /*          to re-enable this feature.  This feature will remained   */
   /*          disabled until the current service level connection has  */
   /*          been dropped.                                            */
int _HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to disable the remote echo   */
         /* cancellation/noise reduction.                               */
         ret_val = HFRE_Disable_Remote_Echo_Cancelation_Noise_Reduction(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
   /* connection ID indicating the local connection which will process  */
   /* the command and a BOOLEAN flag specifying the type of request or  */
   /* notification to send.  When active the voice recognition function */
   /* on the Audio Gateway is turned on, when inactive the voice        */
   /* recognition function on the Audio Gateway is turned off.  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HFRM_Set_Remote_Voice_Recognition_Activation(unsigned int HFREID, Boolean_t VoiceRecognitionActive)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Active: %s\n", HFREID, VoiceRecognitionActive?"TRUE":"FALSE"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to set the voice recognition */
         /* activation.                                                 */
         ret_val = HFRE_Set_Remote_Voice_Recognition_Activation(_BluetoothStackID, HFREID, VoiceRecognitionActive);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
   /* input parameters the connection ID indicating the local           */
   /* connection which will process the command and the speaker gain to */
   /* be sent to the remote device.  The speaker gain Parameter *MUST*  */
   /* be between the values:                                            */
   /*                                                                   */
   /*    HFRE_SPEAKER_GAIN_MINIMUM                                      */
   /*    HFRE_SPEAKER_GAIN_MAXIMUM                                      */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HFRM_Set_Remote_Speaker_Gain(unsigned int HFREID, unsigned int SpeakerGain)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Gain: %d\n", HFREID, SpeakerGain));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to set the speaker gain.     */
         ret_val = HFRE_Set_Remote_Speaker_Gain(_BluetoothStackID, HFREID, SpeakerGain);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
   /* its input parameters the connection ID indicating the local       */
   /* connection which will process the command and the microphone gain */
   /* to be sent to the remote device.  The microphone gain Parameter   */
   /* *MUST* be between the values:                                     */
   /*                                                                   */
   /*    HFRE_MICROPHONE_GAIN_MINIMUM                                   */
   /*    HFRE_MICROPHONE_GAIN_MAXIMUM                                   */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HFRM_Set_Remote_Microphone_Gain(unsigned int HFREID, unsigned int MicrophoneGain)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Gain: %d\n", HFREID, MicrophoneGain));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to set the micropone gain.   */
         ret_val = HFRE_Set_Remote_Microphone_Gain(_BluetoothStackID, HFREID, MicrophoneGain);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* Send a codec ID. The audio gateway uses this function to send the */
   /* preferred codec. The hands free device uses the function to       */ 
   /* confirm the audio gateways choice. HFREID is the hands free ID    */
   /* and CodecID identifies the codec.                                 */
int _HFRM_Send_Select_Codec(unsigned int HFREID, unsigned char CodecID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (CodecID))
      {
         /* Send the codec list.                                        */
         ret_val = HFRE_Send_Select_Codec(_BluetoothStackID, HFREID, CodecID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}
   /* The following function is responsible for querying the remote     */
   /* control indicator status.  This function may only be performed by */
   /* a local Hands Free unit with a valid service level connection to a*/
   /* connected remote Audio Gateway.  The results to this query will be*/
   /* returned as part of the control indicator status confirmation     */
   /* event (hetHFRControlIndicatorStatus).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HFRM_Query_Remote_Control_Indicator_Status(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to query the remote control  */
         /* indicator status.                                           */
         ret_val = HFRE_Query_Remote_Control_Indicator_Status(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Enable_Remote_Indicator_Event_Notification(unsigned int HFREID, Boolean_t EnableEventNotification)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Enable: %s\n", HFREID, EnableEventNotification?"TRUE":"FALSE"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to enable the remote control */
         /* indicator event notification.                               */
         ret_val = HFRE_Enable_Remote_Indicator_Event_Notification(_BluetoothStackID, HFREID, EnableEventNotification);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to query the remote call     */
         /* holding/multi-party service support functionality.          */
         ret_val = HFRE_Query_Remote_Call_Holding_Multiparty_Service_Support(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Send_Call_Holding_Multiparty_Selection(unsigned int HFREID, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to issue the send call       */
         /* holding/multi-party selection.                              */
         ret_val = HFRE_Send_Call_Holding_Multiparty_Selection(_BluetoothStackID, HFREID, CallHoldMultipartyHandling, Index);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Enable_Remote_Call_Waiting_Notification(unsigned int HFREID, Boolean_t EnableNotification)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Enable: %s\n", HFREID, EnableNotification?"TRUE":"FALSE"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to issue the enable remote   */
         /* call waiting notification feature.                          */
         ret_val = HFRE_Enable_Remote_Call_Waiting_Notification(_BluetoothStackID, HFREID, EnableNotification);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Enable_Remote_Call_Line_Identification_Notification(unsigned int HFREID, Boolean_t EnableNotification)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Enable: %s\n", HFREID, EnableNotification?"TRUE":"FALSE"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to issue the enable remote   */
         /* call line identification notification feature.              */
         ret_val = HFRE_Enable_Remote_Call_Line_Identification_Notification(_BluetoothStackID, HFREID, EnableNotification);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Dial_Phone_Number(unsigned int HFREID, char *PhoneNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (PhoneNumber) && (BTPS_StringLength(PhoneNumber) <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
      {
         /* Go ahead and issue the command to issue the dial a phone    */
         /* number.                                                     */
         ret_val = HFRE_Dial_Phone_Number(_BluetoothStackID, HFREID, PhoneNumber);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Dial_Phone_Number_From_Memory(unsigned int HFREID, unsigned int MemoryLocation)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Location: %d\n", HFREID, MemoryLocation));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to issue the dial phone      */
         /* number from memory.                                         */
         ret_val = HFRE_Dial_Phone_Number_From_Memory(_BluetoothStackID, HFREID, MemoryLocation);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for re-dialing the last number dialed*/
   /* on a remote Audio Gateway.  This function may only be performed by*/
   /* Hands Free devices for which a valid service level connection to a*/
   /* connected remote Audio Gateway exists.  This function returns zero*/
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HFRM_Redial_Last_Phone_Number(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to issue the re-dial last    */
         /* dialed phone number.                                        */
         ret_val = HFRE_Redial_Last_Phone_Number(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for sending the command to a remote  */
   /* Audi Gateway to answer an incoming call.  This function may only  */
   /* be performed by Hands Free devices for which a valid service level*/
   /* connection to a connected remote Audio Gateway exists.  This      */
   /* function return zero if successful or a negative return error code*/
   /* if there was an error.                                            */
int _HFRM_Answer_Incoming_Call(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to answer an incoming call.  */
         ret_val = HFRE_Answer_Incoming_Call(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Transmit_DTMF_Code(unsigned int HFREID, char DTMFCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Code: 0x%02X\n", HFREID, DTMFCode));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to send a DTMF code.         */
         ret_val = HFRE_Transmit_DTMF_Code(_BluetoothStackID, HFREID, DTMFCode);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
   /* * NOTE * When this function is called no other function may be    */
   /*          called until a voice tag response is received from the   */
   /*          remote Audio Gateway.                                    */
int _HFRM_Voice_Tag_Request(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to issue a voice tag request.*/
         ret_val = HFRE_Voice_Tag_Request(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for sending a hang-up command to a   */
   /* remote Audio Gateway.  This function may be used to reject an     */
   /* incoming call or to terminate an on-going call.  This function may*/
   /* only be performed by Hands Free devices for which a valid service */
   /* level connection exists.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
int _HFRM_Hang_Up_Call(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to hang up an on-going phone */
         /* call.                                                       */
         ret_val = HFRE_Hang_Up_Call(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for querying the current    */
   /* call list of the remote Audio Gateway device.  This function may  */
   /* only be performed by a Hands Free device with a valid service     */
   /* level connection to a connected Audio Gateway.  This function     */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _HFRM_Query_Remote_Current_Calls_List(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to query the current calls   */
         /* list.                                                       */
         ret_val = HFRE_Query_Remote_Current_Calls_List(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for setting the network     */
   /* operator format to long alphanumeric.  This function may only be  */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected Audio Gateway.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Set_Network_Operator_Selection_Format(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to set the network operator  */
         /* selection format.                                           */
         ret_val = HFRE_Set_Network_Operator_Selection_Format(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for reading the network     */
   /* operator.  This function may only be performed by a Hands Free    */
   /* device with a valid service level connection.  This function      */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The network operator format must be set before querying  */
   /*          the current network operator.                            */
int _HFRM_Query_Remote_Network_Operator_Selection(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to query the remote network  */
         /* operator selection.                                         */
         ret_val = HFRE_Query_Remote_Network_Operator_Selection(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Enable_Remote_Extended_Error_Result(unsigned int HFREID, Boolean_t EnableExtendedErrorResults)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Enable: %s\n", HFREID, EnableExtendedErrorResults?"TRUE":"FALSE"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to enable the extended error */
         /* result.                                                     */
         ret_val = HFRE_Enable_Remote_Extended_Error_Result(_BluetoothStackID, HFREID, EnableExtendedErrorResults);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for retrieving the          */
   /* subscriber number information.  This function may only be         */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _HFRM_Query_Subscriber_Number_Information(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to query the subscriber      */
         /* number information.                                         */
         ret_val = HFRE_Query_Subscriber_Number_Information(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for retrieving the current  */
   /* response and hold status.  This function may only be performed by */
   /* a Hands Free device with a valid service level connection to a    */
   /* connected Audio Gateway.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
int _HFRM_Query_Response_Hold_Status(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to query the response hold   */
         /* status.                                                     */
         ret_val = HFRE_Query_Response_Hold_Status(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for setting the state of an */
   /* incoming call.  This function may only be performed by a Hands    */
   /* Free unit with a valid service level connection to a remote Audio */
   /* Gateway.  This function accepts as its input parameter the call   */
   /* state to set as part of this message.  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HFRM_Set_Incoming_Call_State(unsigned int HFREID, HFRE_Call_State_t CallState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, State: %d\n", HFREID, (unsigned int)CallState));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to set the incoming call     */
         /* state.                                                      */
         ret_val = HFRE_Set_Incoming_Call_State(_BluetoothStackID, HFREID, CallState);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
   /* * NOTE * The Command string passed to this function *MUST* begin  */
   /*          with AT and *MUST* end with the a carriage return ('\r') */
   /*          if this is the first portion of an arbitrary command     */
   /*          that will span multiple writes.  Subsequent calls (until */
   /*          the actual status reponse is received) can begin with    */
   /*          any character, however, they must end with a carriage    */
   /*          return ('\r').                                           */
int _HFRM_Send_Arbitrary_Command(unsigned int HFREID, char *ArbitraryCommand)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HandsFreeSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (ArbitraryCommand) && (BTPS_StringLength(ArbitraryCommand)))
      {
         /* Go ahead and issue the arbitrary command request.           */
         ret_val = HFRE_Send_Arbitrary_Command(_BluetoothStackID, HFREID, ArbitraryCommand);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* Send the list of supported codecs to a remote audio gateway.      */
   /* HFREID is the hands free ID. NumberSupportedCodecs is the number  */
   /* of codec ID in the list. AvailableCodecList is the list.          */
int _HFRM_Send_Available_Codec_List(unsigned int HFREID, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (NumberSupportedCodecs) && (AvailableCodecList))
      {
         /* Send the codec list.                                        */
         ret_val = HFRE_Send_Available_Codecs(_BluetoothStackID, HFREID, NumberSupportedCodecs, AvailableCodecList);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway with a valid service level connection to a       */
   /* connected remote Hands Free device.  This function accepts as its */
   /* input parameters the number of indicators and list of name/value  */
   /* pairs for the indicators to be updated.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Update_Current_Control_Indicator_Status(unsigned int HFREID, unsigned int NumberUpdateIndicators, HFRE_Indicator_Update_t *UpdateIndicators)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (NumberUpdateIndicators) && (UpdateIndicators))
      {
         /* Go ahead and issue the command to update current control    */
         /* indicator status.                                           */
         ret_val = HFRE_Update_Current_Control_Indicator_Status(_BluetoothStackID, HFREID, NumberUpdateIndicators, UpdateIndicators);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Update_Current_Control_Indicator_Status_By_Name(unsigned int HFREID, char *IndicatorName, unsigned int IndicatorValue)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (IndicatorName) && (BTPS_StringLength(IndicatorName)))
      {
         /* Go ahead and issue the command to update the specified      */
         /* indicator (by name).                                        */
         ret_val = HFRE_Update_Current_Control_Indicator_Status_By_Name(_BluetoothStackID, HFREID, IndicatorName, IndicatorValue);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
   /* * NOTE * It is valid to either pass a NULL for the PhoneNumber    */
   /*          parameter or a blank string to specify that there is no  */
   /*          phone number present.                                    */
int _HFRM_Send_Call_Waiting_Notification(unsigned int HFREID, char *PhoneNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to send the call waiting     */
         /* notification.                                               */
         ret_val = HFRE_Send_Call_Waiting_Notification(_BluetoothStackID, HFREID, PhoneNumber);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Send_Call_Line_Identification_Notification(unsigned int HFREID, char *PhoneNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (PhoneNumber) && (BTPS_StringLength(PhoneNumber)))
      {
         /* Go ahead and issue the command to send the call line        */
         /* identification information.                                 */
         ret_val = HFRE_Send_Call_Line_Identification_Notification(_BluetoothStackID, HFREID, PhoneNumber);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for sending a ring indication to a   */
   /* remote Hands Free unit.  This function may only be performed by   */
   /* Audio Gateways for which a valid service level connection to a    */
   /* connected remote Hands Free device exists.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Ring_Indication(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to send the ring indication. */
         ret_val = HFRE_Ring_Indication(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(unsigned int HFREID, Boolean_t EnableInBandRing)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Enable: %s\n", HFREID, EnableInBandRing?"TRUE":"FALSE"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to set the in-band ring tone */
         /* setting.                                                    */
         ret_val = HFRE_Enable_Remote_InBand_Ring_Tone_Setting(_BluetoothStackID, HFREID, EnableInBandRing);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Voice_Tag_Response(unsigned int HFREID, char *PhoneNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (PhoneNumber) && (BTPS_StringLength(PhoneNumber)))
      {
         /* Go ahead and issue the command to send the voice tag        */
         /* response.                                                   */
         ret_val = HFRE_Voice_Tag_Response(_BluetoothStackID, HFREID, PhoneNumber);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for sending the current     */
   /* calls list entries to a remote Hands Free device.  This function  */
   /* may only be performed by Audio Gateways that have received a      */
   /* request to query the remote current calls list.  This function    */
   /* accepts as its input parameters the list of current call entries  */
   /* to be sent and length of the list.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
int _HFRM_Send_Current_Calls_List(unsigned int HFREID, unsigned int NumberListEntries, HFRE_Current_Call_List_Entry_t *CurrentCallListEntry)
{
   int          ret_val;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && ((!NumberListEntries) || ((NumberListEntries) && (CurrentCallListEntry))))
      {
         /* Go ahead and issue the command(s) to send the current calls */
         /* list.                                                       */
         if(NumberListEntries)
         {
            for(Index=0;Index<NumberListEntries;Index++)
            {
               if(CurrentCallListEntry[Index].PhonebookName == NULL)
                  ret_val = HFRE_Send_Current_Calls_List(_BluetoothStackID, HFREID, &(CurrentCallListEntry[Index]), FALSE);
               else
                  ret_val = HFRE_Send_Current_Calls_List_With_Phonebook_Name(_BluetoothStackID, HFREID, &(CurrentCallListEntry[Index]), FALSE);
            }
         }
         else
            ret_val = 0;

         if(!ret_val)
            ret_val = HFRE_Send_Current_Calls_List(_BluetoothStackID, HFREID, NULL, TRUE);
         else
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Send_Network_Operator_Selection(unsigned int HFREID, unsigned int NetworkMode, char *NetworkOperator)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to send the network operator */
         /* selection response.                                         */
         ret_val = HFRE_Send_Network_Operator_Selection(_BluetoothStackID, HFREID, NetworkMode, NetworkOperator);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for sending extended error  */
   /* results.  This function may only be performed by an Audio Gateway */
   /* with a valid service level connection.  This function accepts as  */
   /* its input parameter the result code to send as part of the error  */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _HFRM_Send_Extended_Error_Result(unsigned int HFREID, unsigned int ResultCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to send the extended error   */
         /* response.                                                   */
         ret_val = HFRE_Send_Extended_Error_Result(_BluetoothStackID, HFREID, ResultCode);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for sending subscriber      */
   /* number information.  This function may only be performed by an    */
   /* Audio Gateway that has received a request to query the subscriber */
   /* number information.  This function accepts as its input parameters*/
   /* the number of subscribers followed by a list of subscriber        */
   /* numbers.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _HFRM_Send_Subscriber_Number_Information(unsigned int HFREID, unsigned int NumberListEntries, HFRM_Subscriber_Number_Information_t *SubscriberNumberList)
{
   int          ret_val;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && ((!NumberListEntries) || ((NumberListEntries) && (SubscriberNumberList))))
      {
         /* Go ahead and issue the command(s) to send the subscriber    */
         /* number information list.                                    */
         if(NumberListEntries)
         {
            /* Initialize success.                                      */
            ret_val   = 0;

            for(Index = 0;Index<NumberListEntries && !ret_val;Index++)
               ret_val = HFRE_Send_Subscriber_Number_Information(_BluetoothStackID, HFREID, SubscriberNumberList[Index].PhoneNumber, SubscriberNumberList[Index].ServiceType, SubscriberNumberList[Index].NumberFormat, (Boolean_t)((Index == NumberListEntries-1)?TRUE:FALSE));
         }
         else
            ret_val = HFRE_Send_Terminating_Response(_BluetoothStackID, HFREID, erOK, 0);

         if(ret_val < 0)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for sending information     */
   /* about the incoming call state.  This function may only be         */
   /* performed by an Audio Gateway that has a valid service level      */
   /* connection to a remote Hands Free device.  This function accepts  */
   /* as its input parameter the call state to set as part of this      */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _HFRM_Send_Incoming_Call_State(unsigned int HFREID, HFRE_Call_State_t CallState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, State: %d\n", HFREID, (unsigned int)CallState));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to send the incoming call    */
         /* state.                                                      */
         ret_val = HFRE_Send_Incoming_Call_State(_BluetoothStackID, HFREID, CallState);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Send_Terminating_Response(unsigned int HFREID, HFRE_Extended_Result_t ResultType, unsigned int ResultValue)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Type: %d, Value: %d\n", HFREID, (unsigned int)ResultType, ResultValue));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and issue the command to send the terminating      */
         /* response.                                                   */
         ret_val = HFRE_Send_Terminating_Response(_BluetoothStackID, HFREID, ResultType, ResultValue);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HFRM_Enable_Arbitrary_Command_Processing(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and inform the Hands Free module that arbitrary    */
         /* command processing is enabled.                              */
         ret_val = HFRE_Enable_Arbitrary_Command_Processing(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
   /* * NOTE * The Response string passed to this function *MUST* begin */
   /*          with a carriage return/line feed ("\r\n").               */
int _HFRM_Send_Arbitrary_Response(unsigned int HFREID, char *ArbitraryResponse)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (ArbitraryResponse) && (BTPS_StringLength(ArbitraryResponse)))
      {
         /* Go ahead and issue the arbitrary response.                  */
         ret_val = HFRE_Send_Arbitrary_Response(_BluetoothStackID, HFREID, ArbitraryResponse);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Hands Free device for which a valid  */
   /* service level connection Exists.  This function accepts as its    */
   /* input parameter the connection type indicating which connection   */
   /* will process the command.  This function returns zero if          */
   /* successful or a negative return error code if there was an error. */
int _HFRM_Setup_Audio_Connection(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and setup the audio connection.                    */
         ret_val = HFRE_Setup_Audio_Connection(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HFRM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Hands   */
   /* Free device.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
int _HFRM_Release_Audio_Connection(unsigned int HFREID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HFREID)
      {
         /* Go ahead and release the audio connection.                  */
         ret_val = HFRE_Release_Audio_Connection(_BluetoothStackID, HFREID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Hands Free ID, followed by the length (in    */
   /* Bytes) of the audio data to send, and a pointer to the audio data */
   /* to send to the remote dntity.  This function returns zero if      */
   /* successful or a negative return error code if there was an error. */
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
int _HFRM_Send_Audio_Data(unsigned int HFREID, unsigned int AudioDataLength, unsigned char *AudioData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HFREID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HFREID) && (AudioDataLength) && (AudioData))
      {
         /* Go ahead and send the audio data.                           */
         ret_val = HFRE_Send_Audio_Data(_BluetoothStackID, HFREID, (Byte_t)AudioDataLength, AudioData);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

