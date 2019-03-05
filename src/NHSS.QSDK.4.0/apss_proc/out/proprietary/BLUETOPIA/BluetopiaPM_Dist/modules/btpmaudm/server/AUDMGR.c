/*****< audmgr.c >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDMGR - Audio Manager Implementation for Stonestreet One Bluetooth       */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "SS1BTA2D.h"            /* Bluetooth A2DP API Constants.             */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMAUDM.h"            /* BTPM Audio Manager Prototypes/Constants.  */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */
#include "AUDMGR.h"              /* Audio Manager Impl. Prototypes/Constants. */
#include "AUDMUTIL.h"            /* BTPM Audio Manager Utility Functions.     */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _AUDM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variables which hold the initialization information that is to be */
   /* used when the device powers on (registered once at                */
   /* initialization).                                                  */
typedef struct _tagAudioManagerInitializationData_t
{
   unsigned long                            InitializationFlags;

   Boolean_t                                SRCInitializationInfoValid;
   AUD_Stream_Initialization_Info_t         SRCInitializationInfo;

   Boolean_t                                SNKInitializationInfoValid;
   AUD_Stream_Initialization_Info_t         SNKInitializationInfo;

   Boolean_t                                RemoteControlInitializationInfoValid;
   AUD_Remote_Control_Initialization_Info_t RemoteControlInitializationInfo;
   AUD_Remote_Control_Role_Info_t           ControllerRoleInfo;
   AUD_Remote_Control_Role_Info_t           TargetRoleInfo;
} AudioManagerInitializationData_t;

   /* The following variable stores all initialization information used */
   /* by this module.                                                   */
static AudioManagerInitializationData_t AudioManagerInitializationData;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI AUD_Event_Callback(unsigned int BluetoothStackID, AUD_Event_Data_t *AUD_Event_Data, unsigned long CallbackParameter);

   /* The following function the function that is installed to process  */
   /* Audio Events from the stack.                                      */
static void BTPSAPI AUD_Event_Callback(unsigned int BluetoothStackID, AUD_Event_Data_t *AUD_Event_Data, unsigned long CallbackParameter)
{
   AUDM_Update_Data_t *AUDMUpdateData;
   int                 Result;
   unsigned long       AllocationSize;
   unsigned char      *EndPtr;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((AUD_Event_Data) && (AUD_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      AUDMUpdateData = NULL;

      switch(AUD_Event_Data->Event_Data_Type)
      {
         case etAUD_Open_Request_Indication:
         case etAUD_Stream_Open_Indication:
         case etAUD_Stream_Open_Confirmation:
         case etAUD_Stream_Close_Indication:
         case etAUD_Stream_State_Change_Indication:
         case etAUD_Stream_State_Change_Confirmation:
         case etAUD_Stream_Format_Change_Indication:
         case etAUD_Stream_Format_Change_Confirmation:
         case etAUD_Remote_Control_Open_Indication:
         case etAUD_Remote_Control_Open_Confirmation:
         case etAUD_Remote_Control_Close_Indication:
         case etAUD_Browsing_Channel_Open_Indication:
         case etAUD_Browsing_Channel_Open_Confirmation:
         case etAUD_Browsing_Channel_Close_Indication:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data) && (AUDMUpdateData = (AUDM_Update_Data_t *)BTPS_AllocateMemory(sizeof(AUDM_Update_Data_t))) != NULL)
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               AUDMUpdateData->UpdateType                        = utAUDEvent;
               AUDMUpdateData->UpdateData.AUDEventData.EventType = AUD_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(AUDMUpdateData->UpdateData.AUDEventData.EventData), AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data, AUD_Event_Data->Event_Data_Size);
            }
            break;
         case etAUD_Remote_Control_Command_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data)
            {
               /* Determine the size of required to store the event in a*/
               /* data stream.                                          */
               if((Result = ConvertDecodedAVRCPCommandToStream(&(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData), 0, NULL)) > 0)
               {
                  /* Determine how much memory we need to allocate to   */
                  /* store the entire message.                          */
                  AllocationSize = sizeof(AUDM_Update_Data_t) + Result;

                  /* Allocate memory for the message.                   */
                  if((AUDMUpdateData = (AUDM_Update_Data_t *)BTPS_AllocateMemory(AllocationSize)) != NULL)
                  {
                     /* Initialize the message that will be sent over   */
                     /* the IPC link so that memory checking tools, e.g.*/
                     /* valgrind, do not throw false positives when the */
                     /* data, which will contain padding bytes, is sent */
                     /* over the IPC link.                              */
                     BTPS_MemInitialize(&(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandIndicationData.Message), 0x00, sizeof(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandIndicationData.Message));

                     /* Note the Event type and copy the event data into*/
                     /* the Notification structure.                     */
                     AUDMUpdateData->UpdateType                        = utAUDEvent;
                     AUDMUpdateData->UpdateData.AUDEventData.EventType = AUD_Event_Data->Event_Data_Type;

                     /* Copy the Remote Control Command's Event         */
                     /* Structure to our local message's structure.     */
                     BTPS_MemCopy(&(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandIndicationData.RemoteControlCommandIndicationData), AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data, AUD_Event_Data->Event_Data_Size);

                     /* Convert the command to a data stream.           */
                     if(ConvertDecodedAVRCPCommandToStream(&(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandIndicationData.RemoteControlCommandIndicationData.RemoteControlCommandData), (unsigned int)Result, AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandIndicationData.Message.MessageData) > 0)
                     {
                        /* The command was successfully converted to a  */
                        /* stream, save the length of the data stream.  */
                        AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandIndicationData.Message.MessageDataLength = (unsigned int)Result;
                     }
                     else
                     {
                        /* We were unable to convert the command to a   */
                        /* data stream, free the memory we allocated and*/
                        /* set the pointer to null so that the message  */
                        /* is not added to the mailbox queue.           */
                        BTPS_FreeMemory(AUDMUpdateData);
                        AUDMUpdateData = NULL;
                     }
                  }
               }
            }
            break;
         case etAUD_Remote_Control_Command_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data)
            {
               /* Check if this is a successful command confirmation.   */
               if(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->ConfirmationStatus == AUD_REMOTE_CONTROL_COMMAND_CONFIRMATION_STATUS_SUCCESS)
               {
                  /* This is a successful command confirmation,         */
                  /* determine the size required to store the event in a*/
                  /* data stream.                                       */
                  if((Result = ConvertDecodedAVRCPResponseToStream(&(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->RemoteControlResponseData), 0, NULL)) > 0)
                  {
                     /* Determine how much memory we need to allocate to*/
                     /* store the entire message.                       */
                     AllocationSize = sizeof(AUDM_Update_Data_t) + Result;

                     /* Allocate memory for the message.                */
                     if((AUDMUpdateData = (AUDM_Update_Data_t *)BTPS_AllocateMemory(AllocationSize)) != NULL)
                     {
                        /* Initialize the message that will be sent over*/
                        /* the IPC link so that memory checking tools,  */
                        /* e.g. valgrind, do not throw false positives  */
                        /* when the data, which will contain padding    */
                        /* bytes, is sent over the IPC link.            */
                        BTPS_MemInitialize(&(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.Message), 0x00, sizeof(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.Message));

                        /* Note the Event type and copy the event data  */
                        /* into the Notification structure.             */
                        AUDMUpdateData->UpdateType                        = utAUDEvent;
                        AUDMUpdateData->UpdateData.AUDEventData.EventType = AUD_Event_Data->Event_Data_Type;

                        /* Copy the Remote Control Response's Event     */
                        /* Structure to our local message's structure.  */
                        BTPS_MemCopy(&(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.RemoteControlCommandConfirmationData), AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data, AUD_Event_Data->Event_Data_Size);

                        /* Convert the response to a data stream.       */
                        if(ConvertDecodedAVRCPResponseToStream(&(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.RemoteControlCommandConfirmationData.RemoteControlResponseData), (unsigned int)Result, AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.Message.MessageData) > 0)
                        {
                           /* The response was successfully converted to*/
                           /* a stream, save the length of the data     */
                           /* stream.                                   */
                           AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.Message.MessageDataLength = (unsigned int)Result;
                        }
                        else
                        {
                           /* We were unable to convert the response to */
                           /* a data stream, free the memory we         */
                           /* allocated and set the pointer to null so  */
                           /* that the message is not added to the      */
                           /* mailbox queue.                            */
                           BTPS_FreeMemory(AUDMUpdateData);
                           AUDMUpdateData = NULL;
                        }
                     }
                  }
               }
               else
               {
                  /* This is not a successful command confirmation,     */
                  /* allocate memory for the message to pass it to along*/
                  /* to any registered callbacks.                       */
                  if((AUDMUpdateData = (AUDM_Update_Data_t *)BTPS_AllocateMemory(sizeof(AUDM_Update_Data_t))) != NULL)
                  {
                     /* Initialize the message that will be sent over   */
                     /* the IPC link so that memory checking tools, e.g.*/
                     /* valgrind, do not throw false positives when the */
                     /* data, which will contain padding bytes, is sent */
                     /* over the IPC link.                              */
                     BTPS_MemInitialize(&(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.Message), 0x00, sizeof(AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.Message));

                     /* Note the Event type and copy the event data into*/
                     /* the Notification structure.                     */
                     AUDMUpdateData->UpdateType                        = utAUDEvent;
                     AUDMUpdateData->UpdateData.AUDEventData.EventType = AUD_Event_Data->Event_Data_Type;

                     BTPS_MemCopy(&(AUDMUpdateData->UpdateData.AUDEventData.EventData), AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data, AUD_Event_Data->Event_Data_Size);

                     AUDMUpdateData->UpdateData.AUDEventData.EventData.AUDMRemoteControlCommandConfirmationData.Message.MessageDataLength = 0;
                  }
               }
            }
            break;
         case etAUD_Encoded_Audio_Data_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((AUDMUpdateData = (AUDM_Update_Data_t *)BTPS_AllocateMemory(sizeof(AUDM_Update_Data_t) + AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrameLength + (AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RTPHeaderInfo ? sizeof(AUD_RTP_Header_Info_t) : 0))) != NULL)
               {
                  BTPS_MemInitialize(AUDMUpdateData, 0x00, sizeof(AUDM_Update_Data_t));

                  /* Note the Event type and copy the event data into   */
                  /* the Notification structure.                        */
                  AUDMUpdateData->UpdateType                        = utAUDEvent;
                  AUDMUpdateData->UpdateData.AUDEventData.EventType = AUD_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(AUDMUpdateData->UpdateData.AUDEventData.EventData), AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data, AUD_Event_Data->Event_Data_Size);

                  /* Save the pointer to the end of the structure where */
                  /* variable data will be added.                       */
                  EndPtr = ((Byte_t *)(AUDMUpdateData)) + sizeof(AUDM_Update_Data_t);

                  /* Check if the RTP Header Info is valid.             */
                  if(AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RTPHeaderInfo)
                  {
                     /* The RTP Header Info appears to valid, copy the  */
                     /* data to our local memory space.                 */
                     BTPS_MemCopy(EndPtr, AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RTPHeaderInfo, sizeof(AUD_RTP_Header_Info_t));

                     /* Update the RTP Header Info pointer.             */
                     AUDMUpdateData->UpdateData.AUDEventData.EventData.EncodedAudioDataIndicationData.RTPHeaderInfo = (AUD_RTP_Header_Info_t *)EndPtr;

                     /* Increment the end pointer to the location where */
                     /* audio data will be inserted if audio data exists*/
                     /* in the current message.                         */
                     EndPtr += sizeof(AUD_RTP_Header_Info_t);
                  }

                  /* Check if this message contains audio data.         */
                  if(AUDMUpdateData->UpdateData.AUDEventData.EventData.EncodedAudioDataIndicationData.RawAudioDataFrameLength)
                  {
                     /* The message contains audio data, copy the audio */
                     /* data to our local memory space.                 */
                     BTPS_MemCopy(EndPtr, AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrame, AUDMUpdateData->UpdateData.AUDEventData.EventData.EncodedAudioDataIndicationData.RawAudioDataFrameLength);

                     /* Fix up the Pointer.                             */
                     AUDMUpdateData->UpdateData.AUDEventData.EventData.EncodedAudioDataIndicationData.RawAudioDataFrame = EndPtr;
                  }
                  else
                  {
                     /* The message does not contain audio data, set the*/
                     /* audio data's pointer to NULL.                   */
                     AUDMUpdateData->UpdateData.AUDEventData.EventData.EncodedAudioDataIndicationData.RawAudioDataFrame = NULL;
                  }
               }
            }
            break;
         default:
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(AUDMUpdateData)
      {
         if(!AUDM_NotifyUpdate(AUDMUpdateData))
            BTPS_FreeMemory((void *)AUDMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Audio Manager Implementation.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Audio Manager Implementation.                                     */
int _AUDM_Initialize(AUDM_Initialization_Data_t *InitializationInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      if(InitializationInfo)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing A/V Manager (Imp)\n"));

         /* Note the specified information (will be used to initialize  */
         /* the module when the device is powered On).                  */

         /* First, check to see if SRC information was specified.       */
         if((InitializationInfo->SRCInitializationInfo) && (InitializationInfo->SRCInitializationInfo->NumberSupportedStreamFormats))
         {
            AudioManagerInitializationData.SRCInitializationInfo      = *(InitializationInfo->SRCInitializationInfo);

            AudioManagerInitializationData.SRCInitializationInfoValid = TRUE;
         }

         /* Next, check to see if SNK information was specified.        */
         if((InitializationInfo->SNKInitializationInfo) && (InitializationInfo->SNKInitializationInfo->NumberSupportedStreamFormats))
         {
            AudioManagerInitializationData.SNKInitializationInfo      = *(InitializationInfo->SNKInitializationInfo);

            AudioManagerInitializationData.SNKInitializationInfoValid = TRUE;
         }

         /* Check to see if AVRCP Remote Control is supported.          */
         if((InitializationInfo->RemoteControlInitializationInfo) && ((InitializationInfo->RemoteControlInitializationInfo->ControllerRoleInfo) || (InitializationInfo->RemoteControlInitializationInfo->TargetRoleInfo)))
         {
            AudioManagerInitializationData.RemoteControlInitializationInfo = *(InitializationInfo->RemoteControlInitializationInfo);

            if(InitializationInfo->RemoteControlInitializationInfo->ControllerRoleInfo)
            {
               AudioManagerInitializationData.ControllerRoleInfo                                 = *(InitializationInfo->RemoteControlInitializationInfo->ControllerRoleInfo);

               AudioManagerInitializationData.RemoteControlInitializationInfo.ControllerRoleInfo = &(AudioManagerInitializationData.ControllerRoleInfo);
               
               AudioManagerInitializationData.RemoteControlInitializationInfoValid               = TRUE;
            }

            if(InitializationInfo->RemoteControlInitializationInfo->TargetRoleInfo)
            {
               AudioManagerInitializationData.TargetRoleInfo                                 = *(InitializationInfo->RemoteControlInitializationInfo->TargetRoleInfo);

               AudioManagerInitializationData.RemoteControlInitializationInfo.TargetRoleInfo = &(AudioManagerInitializationData.TargetRoleInfo);
               
               AudioManagerInitializationData.RemoteControlInitializationInfoValid           = TRUE;
            }
         }

         /* Only initialize if either SRC or SNK or AVRCP is valid.     */
         if((AudioManagerInitializationData.SRCInitializationInfoValid) || (AudioManagerInitializationData.SNKInitializationInfoValid) || (AudioManagerInitializationData.RemoteControlInitializationInfoValid))
         {
            /* Flag that this module is initialized.                    */
            Initialized = TRUE;

            ret_val     = 0;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_AUDIO_INITIALIZATION_DATA;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_AUDIO_INITIALIZATION_DATA;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the Audio */
   /* Manager Implementation.  After this function is called the Audio  */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _AUDM_Initialize() function.  */
void _AUDM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Reset the state of our initialization information.             */
      BTPS_MemInitialize(&AudioManagerInitializationData, 0x00, sizeof(AudioManagerInitializationData));

      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the Audio     */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Audio Manager with the      */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _AUDM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int                       Result;
   Byte_t                    NumberUUIDS;
   Byte_t                    EIRData[2 + (UUID_16_SIZE * 5)];
   UUID_16_t                 tempUUID;
   unsigned int              EIRDataLength;
   AUD_Initialization_Info_t InitializationInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* We have already built most of the initialization            */
         /* information, all that is left is to finish formatting in the*/
         /* format that AUD expects.                                    */
         BTPS_MemInitialize(&InitializationInfo, 0, sizeof(InitializationInfo));

         if(AudioManagerInitializationData.SRCInitializationInfoValid)
            InitializationInfo.SRCInitializationInfo = &(AudioManagerInitializationData.SRCInitializationInfo);

         if(AudioManagerInitializationData.SNKInitializationInfoValid)
            InitializationInfo.SNKInitializationInfo = &(AudioManagerInitializationData.SNKInitializationInfo);

         if(AudioManagerInitializationData.RemoteControlInitializationInfoValid)
            InitializationInfo.RemoteControlInitializationInfo = &(AudioManagerInitializationData.RemoteControlInitializationInfo);

         /* Stack has been powered up.                                  */
         Result = AUD_Initialize(BluetoothStackID, &InitializationInfo, AUD_Event_Callback, 0);

         if(!Result)
         {
            /* Audio Framework initialized successfully.                */
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Framework Initialized, SRC: %d, SNK: %d\n", AudioManagerInitializationData.SRCInitializationInfoValid, AudioManagerInitializationData.SNKInitializationInfoValid));

            /* Go ahead and set the incoming connection mode to Manual  */
            /* Accept.                                                  */
            Result = AUD_Set_Server_Connection_Mode(BluetoothStackID, ausManualAccept);

            /* Check if any errors occurred.                            */
            if(!Result)
            {
               /* No errors occurred, save the Bluetooth Stack ID.      */
               _BluetoothStackID = BluetoothStackID;

               /* Configure the EIR Data based on what is supported.    */
               NumberUUIDS = 0;

               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

               /* Check to see if the Source role is initialized.       */
               if(AudioManagerInitializationData.SRCInitializationInfoValid)
               {
                  /* Assign the audio source UUID (in big-endian        */
                  /* format).                                           */
                  SDP_ASSIGN_ADVANCED_AUDIO_DISTRIBUTION_AUDIO_SOURCE_UUID_16(tempUUID);

                  /* Convert the UUID to little endian as required by   */
                  /* EIR data.                                          */
                  CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                  /* Increment the number of UUIDs.                     */
                  NumberUUIDS++;
               }

               /* Check to see if the Source role is initialized.       */
               if(AudioManagerInitializationData.SNKInitializationInfoValid)
               {
                  /* Assign the audio sink UUID (in big-endian format). */
                  SDP_ASSIGN_ADVANCED_AUDIO_DISTRIBUTION_AUDIO_SINK_UUID_16(tempUUID);

                  /* Convert the UUID to little endian as required by   */
                  /* EIR data.                                          */
                  CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                  /* Increment the number of UUIDs.                     */
                  NumberUUIDS++;
               }

               /* Next check to see if either AVRCP Controller role is  */
               /* initialized.                                          */
               if(AudioManagerInitializationData.RemoteControlInitializationInfoValid)
               {
                  /* Check to see if the controller role is supported.  */
                  if(AudioManagerInitializationData.RemoteControlInitializationInfo.ControllerRoleInfo)
                  {
                     /* Assign the AVRCP profile UUID.                  */
                     SDP_ASSIGN_AUDIO_VIDEO_REMOTE_CONTROL_PROFILE_UUID_16(tempUUID);

                     /* Convert the UUID to little endian as required by*/
                     /* EIR data.                                       */
                     CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                     /* Increment the number of UUIDs.                  */
                     NumberUUIDS++;

                     /* If the AVRCP version is greater than v1.3 we    */
                     /* should also include the controller role UUID    */
                     if(AudioManagerInitializationData.RemoteControlInitializationInfo.SupportedVersion > apvVersion1_3)
                     {
                        /* Assign the AVRCP Controller Role UUID.       */
                        SDP_ASSIGN_AUDIO_VIDEO_REMOTE_CONTROL_CONTROLLER_UUID_16(tempUUID);

                        /* Convert the UUID to little endian as required*/
                        /* by EIR data.                                 */
                        CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                        /* Increment the number of UUIDs.               */
                        NumberUUIDS++;
                     }
                  }

                  /* Check to see if the target role is supported.      */
                  if(AudioManagerInitializationData.RemoteControlInitializationInfo.TargetRoleInfo)
                  {
                     /* Assign the AVRCP target UUID.                   */
                     SDP_ASSIGN_AUDIO_VIDEO_REMOTE_CONTROL_TARGET_UUID_16(tempUUID);

                     /* Convert the UUID to little endian as required by*/
                     /* EIR data.                                       */
                     CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                     /* Increment the number of UUIDs.                  */
                     NumberUUIDS++;
                  }
               }

               /* If we have any UUIDs registered then add the UUID     */
               /* data.                                                 */
               if(NumberUUIDS > 0)
               {
                  /* Assign the length byte based on the number of UUIDs*/
                  /* in the list.                                       */
                  EIRDataLength  = (NON_ALIGNED_BYTE_SIZE + (NumberUUIDS*UUID_16_SIZE));
   
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
   
                  /* Increment the length we pass to the internal       */
                  /* function to take into account the length byte.     */
                  EIRDataLength += NON_ALIGNED_BYTE_SIZE;
   
                  /* Configure the EIR data.                            */
                  MOD_AddEIRData(EIRDataLength, EIRData);
               }
            }
            else
               AUD_Un_Initialize(_BluetoothStackID);
         }
         else
         {
            /* Error initializing Audio Framework.                      */
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Framework NOT Initialized: %d, SRC: %d, SNK: %d\n", Result, AudioManagerInitializationData.SRCInitializationInfoValid, AudioManagerInitializationData.SNKInitializationInfoValid));
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         AUD_Un_Initialize(_BluetoothStackID);

         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Audio Stream or*/
   /* Remote Control connection.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _AUDM_Connection_Request_Response(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Responding to Request: %s, %d\n", (RequestType == acrStream)?"Audio Stream":"Remote Control", Accept));

      /* Attempt to respond to the request.                             */
      ret_val = AUD_Open_Request_Response(_BluetoothStackID, RemoteDeviceAddress, RequestType, Accept);

      if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
         ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect an Audio Stream to a remote device.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Connect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Opening Stream: %d\n", StreamType));

      /* Attempt to open the remote stream.                             */
      ret_val = AUD_Open_Remote_Stream(_BluetoothStackID, RemoteDeviceAddress, StreamType);

      if(ret_val == BTAUD_ERROR_STREAM_CONNECTION_IN_PROGRESS)
         ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
      else
      {
         if(ret_val == BTAUD_ERROR_STREAM_ALREADY_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ALREADY_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Audio Stream.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Disconnect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Stream: %d\n", StreamType));

      /* Attempt to disconnect the audio stream.                        */
      ret_val = AUD_Close_Stream(_BluetoothStackID, RemoteDeviceAddress, StreamType);

      if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
         ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
      else
      {
         if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream state of the     */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream State of the Audio Stream (if*/
   /* this function is successful).                                     */
int _AUDM_Query_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t *StreamState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(StreamState)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Querying Stream State: %d\n", StreamType));

         /* Attempt to query the audio stream state.                    */
         ret_val = AUD_Query_Stream_State(_BluetoothStackID, RemoteDeviceAddress, StreamType, StreamState);

         if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_STREAM_CONNECTION_IN_PROGRESS)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
            else
            {
               if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream format of the    */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream Format of the Audio Stream   */
   /* (if this function is successful).                                 */
int _AUDM_Query_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(StreamFormat)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Querying Stream Format: %d\n", StreamType));

         /* Attempt to query the audio stream format.                   */
         ret_val = AUD_Query_Stream_Format(_BluetoothStackID, RemoteDeviceAddress, StreamType, StreamFormat);

         if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_STREAM_CONNECTION_IN_PROGRESS)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
            else
            {
               if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to start/suspend the specified Audio Stream.  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Change_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Changing Stream State: %d, %d\n", StreamType, StreamState));

      /* Attempt to start/suspend the audio stream.                     */
      ret_val = AUD_Change_Stream_State(_BluetoothStackID, RemoteDeviceAddress, StreamType, StreamState);

      /* In case the Stream is already in the requested state, go ahead */
      /* and flag the special error codes that denote this.             */
      if(ret_val == BTAUD_ERROR_STREAM_STATE_ALREADY_CURRENT)
      {
         if(StreamState == astStreamStopped)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_STATE_IS_ALREADY_SUSPENDED;
         else
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_STATE_IS_ALREADY_STARTED;
      }
      else
      {
         if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_STREAM_CONNECTION_IN_PROGRESS)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
            else
            {
               if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
            }
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the current format used by the specified Audio  */
   /* Stream.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Change_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Changing Stream Format: %d\n", StreamType));

      /* Attempt to change the Format of the audio stream.              */
      ret_val = AUD_Change_Stream_Format(_BluetoothStackID, RemoteDeviceAddress, StreamType, StreamFormat);

      if(ret_val == BTAUD_ERROR_STREAM_FORMAT_CHANGE_IN_PROGRESS)
         ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_FORMAT_CHANGE_IN_PROGRESS;
      else
      {
         if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_UNSUPPORTED_FORMAT)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_FORMAT_IS_NOT_SUPPORTED;
            else
            {
               if(ret_val == BTAUD_ERROR_STREAM_STATE_CHANGE_IN_PROGRESS)
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_STATE_CHANGE_IN_PROGRESS;
               else
               {
                  if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
                     ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
                  else
                  {
                     if(ret_val < 0)
                        ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
                  }
               }
            }
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream configuration of */
   /* the specified Audio Stream.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* The final parameter will hold the Audio Stream Configuration of   */
   /* the Audio Stream (if this function is successful).                */
int _AUDM_Query_Audio_Stream_Configuration(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Configuration_t *StreamConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(StreamConfiguration)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Querying Stream Configuration: %d\n", StreamType));

         /* Attempt to query the audio stream format.                   */
         ret_val = AUD_Query_Stream_Configuration(_BluetoothStackID, RemoteDeviceAddress, StreamType, StreamConfiguration);

         if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_STREAM_CONNECTION_IN_PROGRESS)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
            else
            {
               if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the number of bytes of raw, encoded, audio frame            */
   /* information, followed by the raw, encoded, Audio Data to send.    */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          _AUDM_Query_Audio_Stream_Configuration() function.       */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _AUDM_Send_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if((RawAudioDataFrameLength) && (RawAudioDataFrame))
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Encoded Audio Data: %d\n", RawAudioDataFrameLength));

         /* Attempt to send the encoded audio data.                     */
         ret_val = AUD_Send_Encoded_Audio_Data(_BluetoothStackID, RemoteDeviceAddress, RawAudioDataFrameLength, RawAudioDataFrame);

         if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_STREAM_CONNECTION_IN_PROGRESS)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
            else
            {
               if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the number of bytes of raw, encoded, audio frame            */
   /* information, followed by the raw, encoded, Audio Data to send,    */
   /* followed by flags which specify the format of the data (currently */
   /* not used, this parameter is reserved for future additions),       */
   /* followed by the RTP Header Information.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          AUDM_Query_Audio_Stream_Configuration() function.        */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
   /* * NOTE * This is a low level function and allows the user to      */
   /*          specify the RTP Header Information for the outgoing data */
   /*          packet.  To use the default values for the RTP Header    */
   /*          Information use AUDM_Send_Encoded_Audio_Data() instead.  */
int _AUDM_Send_RTP_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if((RawAudioDataFrameLength) && (RawAudioDataFrame) && (RTPHeaderInfo))
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending RTP Encoded Audio Data: %d\n", RawAudioDataFrameLength));

         /* Attempt to send the encoded audio data.                     */
         ret_val = AUD_Send_RTP_Encoded_Audio_Data(_BluetoothStackID, RemoteDeviceAddress, RawAudioDataFrameLength, RawAudioDataFrame, Flags, RTPHeaderInfo);

         if(ret_val == BTAUD_ERROR_STREAM_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED;
         else
         {
            if(ret_val == BTAUD_ERROR_STREAM_CONNECTION_IN_PROGRESS)
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
            else
            {
               if(ret_val == BTAUD_ERROR_STREAM_NOT_INITIALIZED)
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control session to a remote device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _AUDM_Connect_Remote_Control(BD_ADDR_t RemoteDeviceAddress)
{
   int       ret_val;
   BD_ADDR_t NULL_BD_ADDR;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Opening Remote Control Connection: %02X%02X%02X%02X%02X%02X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0));

         /* Attempt to open the remote control connection.              */
         ret_val = AUD_Open_Remote_Control(_BluetoothStackID, RemoteDeviceAddress);

         /* Map errors to BTPM values.                                  */
         switch(ret_val)
         {
            case BTAUD_ERROR_REMOTE_CONTROL_CONNECTION_IN_PROGRESS:
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_CONNECTION_IN_PROGRESS;
               break;
            case BTAUD_ERROR_REMOTE_CONTROL_ALREADY_CONNECTED:
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_ALREADY_CONNECTED;
               break;
            case BTAUD_ERROR_REMOTE_CONTROL_NOT_CONNECTED:
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_NOT_CONNECTED;
               break;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* session. This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Disconnect_Remote_Control(BD_ADDR_t RemoteDeviceAddress)
{
   int       ret_val;
   BD_ADDR_t NULL_BD_ADDR;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Closing Remote Control Connection: %02X%02X%02X%02X%02X%02X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0));

         /* Attempt to open the remote control connection.              */
         ret_val = AUD_Close_Remote_Control(_BluetoothStackID, RemoteDeviceAddress);

         /* Map errors to BTPM values.                                  */
         if(ret_val == BTAUD_ERROR_REMOTE_CONTROL_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_NOT_CONNECTED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Remote Control Command to the remote Device.  This function       */
   /* accepts as input the Device Address of the Device to send the     */
   /* command to, followed by the Response Timeout (in milliseconds),   */
   /* followed by a pointer to the actual Remote Control Command Data to*/
   /* send.  This function returns a positive, value if successful or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Transaction ID of the Remote Control Event that was  */
   /*          submitted.                                               */
int _AUDM_Send_Remote_Control_Command(BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData)
{
   int       ret_val;
   BD_ADDR_t NULL_BD_ADDR;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)))
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(CommandData)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Remote Control Command: %d\n", CommandData->MessageType));

         /* Attempt to send the encoded audio data.                     */
         if((ret_val = AUD_Send_Remote_Control_Command(_BluetoothStackID, RemoteDeviceAddress, CommandData, ResponseTimeout)) < 0)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_SEND_REMOTE_CONTROL_COMMAND;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Remote Control Response to the remote Device.  This function      */
   /* accepts as input the Device Address of the Device to send the     */
   /* command to, followed by the Transaction ID of the Remote Control  */
   /* Event, followed by a pointer to the actual Remote Control Message */
   /* to send.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Send_Remote_Control_Response(BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData)
{
   int       ret_val;
   BD_ADDR_t NULL_BD_ADDR;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)))
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(ResponseData)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Remote Control Response: %d\n", ResponseData->MessageType));

         /* Attempt to send the encoded audio data.                     */
         if((ret_val = AUD_Send_Remote_Control_Response(_BluetoothStackID, RemoteDeviceAddress, TransactionID, ResponseData)) < 0)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_SEND_REMOTE_CONTROL_RESPONSE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control session to a remote device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _AUDM_Connect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress)
{
   int       ret_val;
   BD_ADDR_t NULL_BD_ADDR;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Opening Browsing Connection: %02X%02X%02X%02X%02X%02X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0));

         /* Attempt to open the remote control connection.              */
         ret_val = AUD_Open_Browsing_Channel(_BluetoothStackID, RemoteDeviceAddress);

         /* Map errors to BTPM values.                                  */
         switch(ret_val)
         {
            case BTAUD_ERROR_REMOTE_CONTROL_CONNECTION_IN_PROGRESS:
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_CONNECTION_IN_PROGRESS;
               break;
            case BTAUD_ERROR_REMOTE_CONTROL_ALREADY_CONNECTED:
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_ALREADY_CONNECTED;
               break;
            case BTAUD_ERROR_REMOTE_CONTROL_NOT_CONNECTED:
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_NOT_CONNECTED;
               break;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* session. This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Disconnect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress)
{
   int       ret_val;
   BD_ADDR_t NULL_BD_ADDR;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure the input parameters appear to be           */
      /* semi-valid.                                                    */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Closing Browsing Connection: %02X%02X%02X%02X%02X%02X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0));

         /* Attempt to open the remote control connection.              */
         ret_val = AUD_Close_Browsing_Channel(_BluetoothStackID, RemoteDeviceAddress);

         /* Map errors to BTPM values.                                  */
         if(ret_val == BTAUD_ERROR_REMOTE_CONTROL_NOT_CONNECTED)
            ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_NOT_CONNECTED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

