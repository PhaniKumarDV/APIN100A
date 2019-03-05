/*****< hdsmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDSMGR - Headset Manager Implementation for Stonestreet One Bluetooth     */
/*           Protocol Stack Platform Manager.                                 */
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

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHDSM.h"            /* BTPM HDSET Manager Prototypes/Constants.  */
#include "HDSMMSG.h"             /* BTPM HDSET Manager Message Formats.       */
#include "HDSMGR.h"              /* HDSET Manager Impl. Prototypes/Constants. */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

typedef struct _tagHDSET_Server_Entry_t
{
   unsigned int                    HDSETID;
} HDSET_Server_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _HDSM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variables which hold the initialization information that is to be */
   /* used when the device powers on (registered once at                */
   /* initialization).                                                  */
static Boolean_t                  AudioGatewaySupported;
static HDSM_Initialization_Data_t _AudioGatewayInitializationInfo;

static Boolean_t                  HeadsetSupported;
static HDSM_Initialization_Data_t _HeadsetInitializationInfo;

static DWord_t      HeadsetServerSDPHandle;
static DWord_t      AudioGatewayServerSDPHandle;

static HDSET_Server_Entry_t *HeadsetServerList;
static HDSET_Server_Entry_t *AudioGatewayServerList;

   /* Internal Function Prototypes.                                     */
static Boolean_t IsServerType(HDSM_Connection_Type_t Type, unsigned int HDSETID);
static void CleanupServers(HDSM_Connection_Type_t Type, Boolean_t FreeList);

static void BTPSAPI HDSET_Event_Callback(unsigned int BluetoothStackID, HDSET_Event_Data_t *HDSET_Event_Data, unsigned long CallbackParameter);

static Boolean_t IsServerType(HDSM_Connection_Type_t Type, unsigned int HDSETID)
{
   Boolean_t            Found = FALSE;
   unsigned int         Index;
   unsigned int         Max;
   HDSET_Server_Entry_t *List;

   List = (Type == sctAudioGateway)?AudioGatewayServerList:HeadsetServerList;
   Max  = (Type == sctAudioGateway)?_AudioGatewayInitializationInfo.MaximumNumberServers:_HeadsetInitializationInfo.MaximumNumberServers;

   for(Index=0; Index<Max && !Found; Index++)
   {
      if(List[Index].HDSETID == HDSETID)
         Found = TRUE;
   }

   return(Found);
}

static void CleanupServers(HDSM_Connection_Type_t Type, Boolean_t FreeList)
{
   unsigned int         Index;
   unsigned int         Max;
   HDSET_Server_Entry_t **List;

   /* Note which list to use.                                           */
   List = (Type == sctAudioGateway)?&AudioGatewayServerList:&HeadsetServerList;
   Max  = (Type == sctAudioGateway)?_AudioGatewayInitializationInfo.MaximumNumberServers:_HeadsetInitializationInfo.MaximumNumberServers;

   /* Close all servers.                                                */
   for(Index=0; Index<Max; Index++)
   {
      if((*List)[Index].HDSETID)
      {
         HDSET_Close_Server_Port(_BluetoothStackID, (*List)[Index].HDSETID);
         (*List)[Index].HDSETID = 0;
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
   /* Headset Events from the stack.                                    */
static void BTPSAPI HDSET_Event_Callback(unsigned int BluetoothStackID, HDSET_Event_Data_t *HDSET_Event_Data, unsigned long CallbackParameter)
{
   HDSM_Update_Data_t *HDSMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((HDSET_Event_Data) && (HDSET_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no event to dispatch.                       */
      HDSMUpdateData = NULL;

      switch(HDSET_Event_Data->Event_Data_Type)
      {
         case etHDSET_Open_Port_Indication:
         case etHDSET_Open_Port_Confirmation:
         case etHDSET_Ring_Indication:
         case etHDSET_Button_Pressed_Indication:
         case etHDSET_Audio_Connection_Indication:
         case etHDSET_Audio_Disconnection_Indication:
         case etHDSET_Close_Port_Indication:
         case etHDSET_Open_Port_Request_Indication:
         case etHDSET_Speaker_Gain_Indication:
         case etHDSET_Microphone_Gain_Indication:
            /* Allocate memory to hold the event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((HDSET_Event_Data->Event_Data.HDSET_Open_Port_Request_Indication_Data) && (HDSMUpdateData = (HDSM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HDSM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the event data into the  */
               /* notification structure.                               */
               HDSMUpdateData->UpdateType                              = utHeadsetEvent;
               HDSMUpdateData->UpdateData.HeadsetEventData.EventType = HDSET_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(HDSMUpdateData->UpdateData.HeadsetEventData.EventData), HDSET_Event_Data->Event_Data.HDSET_Open_Port_Request_Indication_Data, HDSET_Event_Data->Event_Data_Size);
            }
            break;
         case etHDSET_Audio_Data_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HDSET_Event_Data->Event_Data.HDSET_Audio_Data_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HDSMUpdateData = (HDSM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HDSM_Update_Data_t) + HDSET_Event_Data->Event_Data.HDSET_Audio_Data_Indication_Data->AudioDataLength)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HDSMUpdateData->UpdateType                            = utHeadsetEvent;
                  HDSMUpdateData->UpdateData.HeadsetEventData.EventType = HDSET_Event_Data->Event_Data_Type;

                  /* Copy the event structure.                          */
                  BTPS_MemCopy(&(HDSMUpdateData->UpdateData.HeadsetEventData.EventData.HDSET_Audio_Data_Indication_Data), HDSET_Event_Data->Event_Data.HDSET_Audio_Data_Indication_Data, HDSET_Event_Data->Event_Data_Size);

                  /* Assign the audio data pointer to the end of the    */
                  /* update data structure, note that we allocated      */
                  /* additional memory for the data just above.         */
                  HDSMUpdateData->UpdateData.HeadsetEventData.EventData.HDSET_Audio_Data_Indication_Data.AudioData = ((unsigned char *)HDSMUpdateData) + sizeof(HDSM_Update_Data_t);

                  /* Copy the data.                                     */
                  BTPS_MemCopy(HDSMUpdateData->UpdateData.HeadsetEventData.EventData.HDSET_Audio_Data_Indication_Data.AudioData, HDSET_Event_Data->Event_Data.HDSET_Audio_Data_Indication_Data->AudioData, HDSMUpdateData->UpdateData.HeadsetEventData.EventData.HDSET_Audio_Data_Indication_Data.AudioDataLength);
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(HDSMUpdateData)
      {
         if(!HDSM_NotifyUpdate(HDSMUpdateData))
            BTPS_FreeMemory((void *)HDSMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Headset Manager implementation.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Headset Manager Implementation.                                   */
int _HDSM_Initialize(HDSM_Initialization_Data_t *AudioGatewayInitializationInfo, HDSM_Initialization_Data_t *HeadsetInitializationInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Headset Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */

      /* First, check to see if Audio Gateway information was specified.*/
      if((AudioGatewayInitializationInfo) && (AudioGatewayInitializationInfo->ServerPort >= SPP_PORT_NUMBER_MINIMUM) && (AudioGatewayInitializationInfo->ServerPort <= SPP_PORT_NUMBER_MAXIMUM))
      {
         /* Attempt to allocate the memory to track the maximum number  */
         /* of connections.                                             */
         if((AudioGatewayServerList = (HDSET_Server_Entry_t *)BTPS_AllocateMemory(sizeof(HDSET_Server_Entry_t) * AudioGatewayInitializationInfo->MaximumNumberServers)) != NULL)
         {
            BTPS_MemInitialize(AudioGatewayServerList, 0, sizeof(HDSET_Server_Entry_t) * (AudioGatewayInitializationInfo->MaximumNumberServers));

            AudioGatewaySupported           = TRUE;
            AudioGatewayServerSDPHandle     = 0;

            _AudioGatewayInitializationInfo = *AudioGatewayInitializationInfo;
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to allocate Audio Gateway Server List"));

      }

      /* Next, check to see if Headset information was specified.    */
      if((HeadsetInitializationInfo) && (HeadsetInitializationInfo->ServerPort >= SPP_PORT_NUMBER_MINIMUM) && (HeadsetInitializationInfo->ServerPort <= SPP_PORT_NUMBER_MAXIMUM) && (HeadsetInitializationInfo->MaximumNumberServers))
      {
         /* Attempt to allocate the memory to track the maximum number  */
         /* of connections.                                             */
         if((HeadsetServerList = (HDSET_Server_Entry_t *)BTPS_AllocateMemory(sizeof(HDSET_Server_Entry_t) * HeadsetInitializationInfo->MaximumNumberServers)) != NULL)
         {
            BTPS_MemInitialize(HeadsetServerList, 0, sizeof(HDSET_Server_Entry_t) * (HeadsetInitializationInfo->MaximumNumberServers));

            HeadsetSupported           = TRUE;
            HeadsetServerSDPHandle     = 0;

            _HeadsetInitializationInfo = *HeadsetInitializationInfo;
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to allocate Headset Server List"));
      }

      /* Only initialize if either Audio Gateway or Headset is valid.   */
      if((AudioGatewaySupported) || (HeadsetSupported))
      {
         /* Flag that this module is initialized.                       */
         Initialized = TRUE;

         ret_val     = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_HEADSET_INITIALIZATION_DATA;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* Headset Manager implementation.  After this function is called the*/
   /* Headset Manager implementation will no longer operate until it is */
   /* initialized again via a call to the _HDSM_Initialize() function.  */
void _HDSM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Flag that neither Audio Gateway nor Headset has been           */
      /* initialized.                                                   */
      AudioGatewaySupported = FALSE;
      HeadsetSupported      = FALSE;

      /* Finally flag that this module is no longer initialized.        */
      Initialized           = FALSE;

      /* Cleanup any servers that may be active.                        */
      if(AudioGatewayServerList)
         CleanupServers(sctAudioGateway, TRUE);

      if(AudioGatewayServerSDPHandle)
         SDP_Delete_Service_Record(_BluetoothStackID, AudioGatewayServerSDPHandle);

      if(HeadsetServerList)
         CleanupServers(sctHeadset, TRUE);

      if(HeadsetServerSDPHandle)
         SDP_Delete_Service_Record(_BluetoothStackID, HeadsetServerSDPHandle);

      AudioGatewayServerSDPHandle = 0;
      HeadsetServerSDPHandle      = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the Headset   */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Headset Manager with the    */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HDSM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int          Result;
   Byte_t       NumberUUIDS;
   Byte_t       EIRData[2 + (UUID_16_SIZE *2)];
   UUID_16_t    tempUUID;
   Boolean_t    AudioGatewayEnabled            = FALSE;
   Boolean_t    HeadsetEnabled                 = FALSE;
   unsigned int EIRDataLength;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

         /* Stack has been powered up.                                  */
         if(AudioGatewaySupported)
         {
            Result = 0;

            for(Index=0; Index<_AudioGatewayInitializationInfo.MaximumNumberServers && Result >= 0; Index++)
            {
               Result = HDSET_Open_Audio_Gateway_Server_Port(BluetoothStackID, _AudioGatewayInitializationInfo.ServerPort, HDSET_Event_Callback, 0);

               if(Result > 0)
               {
                  /* Audio Gateway initialized successfully.               */
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Gateway Server[%u] Initialized\n", Index));

                  /* Note the Audio Gateway Server ID.                     */
                  AudioGatewayServerList[Index].HDSETID = (unsigned int)Result;

                  /* Go ahead and set the incoming connection mode to      */
                  /* Manual Accept.                                        */
                  HDSET_Set_Server_Mode(BluetoothStackID, AudioGatewayServerList[Index].HDSETID, HDSET_SERVER_MODE_MANUAL_ACCEPT_CONNECTION);

               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Gateway Server[%u] Failed to open\n", Index));
            }

            /* Now check that all servers were opened.                  */
            if(Result > 0)
            {
               /* Assign the Headset Audio Gateway UUID (in big-endian*/
               /* format).                                              */
               SDP_ASSIGN_HEADSET_AUDIO_GATEWAY_PROFILE_UUID_16(tempUUID);

               /* Convert the UUID to little endian as required by EIR  */
               /* data.                                                 */
               CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

               /* Increment the number of UUIDs.                        */
               NumberUUIDS++;

               /* Note that we have succesully brought up Audi Gateway. */
               AudioGatewayEnabled = TRUE;
            }
         }

         if(HeadsetSupported)
         {
            Result = 0;

            for(Index=0; Index<_HeadsetInitializationInfo.MaximumNumberServers && Result >= 0; Index++)
            {
               Result = HDSET_Open_Headset_Server_Port(BluetoothStackID, _HeadsetInitializationInfo.ServerPort, (Boolean_t)((_HeadsetInitializationInfo.SupportedFeaturesMask & HDSM_SUPPORTED_FEATURES_MASK_HEADSET_SUPPORTS_REMOTE_AUDIO_VOLUME_CONTROLS)?TRUE:FALSE), HDSET_Event_Callback, 0);

               if(Result > 0)
               {
                  /* Headset initialized successfully.               */
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Server[%u] Initialized\n", Index));

                  /* Note the Headset Server ID.                     */
                  HeadsetServerList[Index].HDSETID = (unsigned int)Result;

                  /* Go ahead and set the incoming connection mode to   */
                  /* Manual Accept.                                     */
                  HDSET_Set_Server_Mode(BluetoothStackID, HeadsetServerList[Index].HDSETID, HDSET_SERVER_MODE_MANUAL_ACCEPT_CONNECTION);

               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Server[%u] Failed to open\n", Index));
            }

            /* Now make sure we opened all server.                      */
            if(Result > 0)
            {
               /* Assign the Headset Role UUID (in big-endian format).  */
               SDP_ASSIGN_HEADSET_PROFILE_UUID_16(tempUUID);

               /* Convert the UUID to little endian as required by EIR  */
               /* data.                                                 */
               CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

               /* Increment the number of UUIDs.                        */
               NumberUUIDS++;

               /* Note that we have succesully brought up Hands Feee.   */
               HeadsetEnabled = TRUE;
            }
         }

         if(((!AudioGatewaySupported) || ((AudioGatewaySupported) && (AudioGatewayEnabled))) && ((!HeadsetSupported) || ((HeadsetSupported) && (HeadsetEnabled))))
         {
            /* Headset Manager initialized successfully.                */
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Headset Manager Initialized, Audio Gateway: %d, Headset: %d\n", AudioGatewaySupported, HeadsetSupported));

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
            DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Framework NOT Initialized: Audio Gateway: %d, Headset: %d\n", AudioGatewaySupported, HeadsetSupported));

            /* Error, clean up anything that might have been            */
            /* initialized.                                             */
            if(AudioGatewayServerList)
               CleanupServers(sctAudioGateway, FALSE);

            if(AudioGatewayServerSDPHandle)
               SDP_Delete_Service_Record(BluetoothStackID, AudioGatewayServerSDPHandle);

            if(HeadsetServerList)
               CleanupServers(sctHeadset, FALSE);

            if(HeadsetServerSDPHandle)
               SDP_Delete_Service_Record(BluetoothStackID, HeadsetServerSDPHandle);

            AudioGatewayServerSDPHandle = 0;
            HeadsetServerSDPHandle    = 0;
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         if(AudioGatewayServerList)
            CleanupServers(sctAudioGateway, FALSE);

         if(HeadsetServerList)
            CleanupServers(sctHeadset, FALSE);

         _BluetoothStackID    = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for installing/removing the */
   /* Headset or AufioGateway SDP record.                               */
int _HDSM_UpdateSDPRecord(HDSM_Connection_Type_t ConnectionType, Boolean_t Install)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(ConnectionType == sctAudioGateway)
      {
         if(Install)
         {
            /* Make sure the server list is initialized. */
            if(AudioGatewayServerList)
            {
               /* Register an SDP Record for the port.                  */
               /* * NOTE * Since the SDP record is not really tied to   */
               /*          specific HDSET ID in any way, just use the   */
               /*          first one in the list.                       */
               Result = HDSET_Register_Audio_Gateway_SDP_Record(_BluetoothStackID, AudioGatewayServerList[0].HDSETID, _AudioGatewayInitializationInfo.ServiceName, &AudioGatewayServerSDPHandle);
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
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
            if(HeadsetServerList)
            {
               /* Register an SDP Record for the port.                  */
               /* * NOTE * Since the SDP record is not really tied to   */
               /*          specific HDSET ID in any way, just use the   */
               /*          first one in the list.                       */
               Result = HDSET_Register_Headset_SDP_Record(_BluetoothStackID, HeadsetServerList[0].HDSETID, _HeadsetInitializationInfo.ServiceName, &HeadsetServerSDPHandle);
            }
            else
               Result = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
         }
         else
         {
            if(HeadsetServerSDPHandle)
            {
               Result = SDP_Delete_Service_Record(_BluetoothStackID, HeadsetServerSDPHandle);
               HeadsetServerSDPHandle = 0;
            }
            else
               Result = 0;
         }
      }
   }
   else
      Result = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(Result);
}

   /* The following function is a utility function that exists to allow */
   /* the caller a mechanism to determine the incoming connection type  */
   /* of the specified incoming connection (based on the Headset Port   */
   /* ID).                                                              */
Boolean_t _HDSM_QueryIncomingConnectionType(unsigned int HDSETID, HDSM_Connection_Type_t *ConnectionType, unsigned int *ServerPort)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (ConnectionType) && (HDSETID))
   {
      /* Check whether this HDSETID matches one of the Audio Gateway    */
      /* Servers.                                                       */
      if(IsServerType(sctAudioGateway, HDSETID))
      {
         *ConnectionType = sctAudioGateway;

         /* If the caller would like the server port, then be sure we   */
         /* note the port.                                              */
         if(ServerPort)
            *ServerPort = _AudioGatewayInitializationInfo.ServerPort;

         ret_val = TRUE;
      }
      else
      {
         /* Check if this HDSETID matches one of the Headset Servers.   */
         if(IsServerType(sctHeadset, HDSETID))
         {
            *ConnectionType = sctHeadset;

            /* If the caller would like the server port, then be sure we*/
            /* note the port.                                           */
            if(ServerPort)
               *ServerPort = _HeadsetInitializationInfo.ServerPort;

            ret_val         = TRUE;
         }
         else
            ret_val = FALSE;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
int _HDSM_Connection_Request_Response(unsigned int HDSETID, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Type: %s\n", (IsServerType(sctAudioGateway, HDSETID))?"Audio Gateway":"Hands Free"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HDSETID))
   {
      /* Next, check to see if the device is connecting.                */
      if(IsServerType(sctAudioGateway, HDSETID) || IsServerType(sctHeadset, HDSETID))
      {
         /* Server entry was found and the specified device is          */
         /* connecting.                                                 */
         if(!HDSET_Open_Port_Request_Response(_BluetoothStackID, HDSETID, AcceptConnection))
            ret_val = 0;
         else
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Headset/Audio Gateway device.  This*/
   /* function returns a non zero value if successful, or a negative    */
   /* return error code if there was an error.  The return value from   */
   /* this function (if successful) represents the Bluetopia Headset ID */
   /* that is used to track this connection.  This function accepts the */
   /* connection type to make as the first parameter.  This parameter   */
   /* specifies the LOCAL connection type (i.e.  if the caller would    */
   /* like to connect the local Headset service to a remote Audio       */
   /* Gateway device, the Headset connection type would be specified for*/
   /* this parameter).  This function also accepts the connection       */
   /* information for the remote device (address and server port).      */
int _HDSM_Connect_Remote_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Local Type: %s, Port: %d\n", (ConnectionType == sctAudioGateway)?"Audio Gateway":"Headset", RemoteServerPort));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort >= SPP_PORT_NUMBER_MINIMUM) && (RemoteServerPort <= SPP_PORT_NUMBER_MAXIMUM))
      {
         /* Attempt to make the outgoing connetion.                     */
         if(ConnectionType == sctAudioGateway)
            ret_val = HDSET_Open_Remote_Headset_Port(_BluetoothStackID, RemoteDeviceAddress, RemoteServerPort, (Boolean_t)((_AudioGatewayInitializationInfo.SupportedFeaturesMask & HDSM_SUPPORTED_FEATURES_MASK_AUDIO_GATEWAY_SUPPORTS_IN_BAND_RING)?TRUE:FALSE), HDSET_Event_Callback, 0);
         else
            ret_val = HDSET_Open_Remote_Audio_Gateway_Port(_BluetoothStackID, RemoteDeviceAddress, RemoteServerPort, (Boolean_t)((_HeadsetInitializationInfo.SupportedFeaturesMask & HDSM_SUPPORTED_FEATURES_MASK_HEADSET_SUPPORTS_REMOTE_AUDIO_VOLUME_CONTROLS)?TRUE:FALSE), HDSET_Event_Callback, 0);

         /* If an error occurred, we need to map it to a valid error    */
         /* code.                                                       */
         if(ret_val <= 0)
            ret_val = BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
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
int _HDSM_Disconnect_Device(unsigned int HDSETID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HDSETID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HDSETID)
      {
         /* Go ahead and disconnect the device.                         */
         ret_val = HDSET_Close_Port(_BluetoothStackID, HDSETID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_UNABLE_TO_DISCONNECT_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  When called by a     */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current speaker gain value.  When     */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the speaker gain of the remote Headset   */
   /* device.  This function accepts as its input parameters the        */
   /* connection ID indicating the local connection which will process  */
   /* the command and the speaker gain to be sent to the remote device. */
   /* The speaker gain Parameter *MUST* be between the values:          */
   /* HDSET_SPEAKER_GAIN_MINIMUM HDSET_SPEAKER_GAIN_MAXIMUM This        */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDSM_Set_Remote_Speaker_Gain(unsigned int HDSETID, unsigned int SpeakerGain)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Gain: %d\n", HDSETID, SpeakerGain));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HDSETID)
      {
         /* Go ahead and issue the command to set the speaker gain.     */
         ret_val = HDSET_Set_Speaker_Gain(_BluetoothStackID, HDSETID, SpeakerGain);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  When called by a  */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current microphone gain value.  When  */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the microphone gain of the remote Headset*/
   /* device.  This function accepts as its input parameters the        */
   /* connection ID indicating the local connection which will process  */
   /* the command and the microphone gain to be sent to the remote      */
   /* device.  The microphone gain Parameter *MUST* be between the      */
   /* values:                                                           */
   /*                                                                   */
   /*    HDSET_MICROPHONE_GAIN_MINIMUM                                  */
   /*    HDSET_MICROPHONE_GAIN_MAXIMUM                                  */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HDSM_Set_Remote_Microphone_Gain(unsigned int HDSETID, unsigned int MicrophoneGain)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d, Gain: %d\n", HDSETID, MicrophoneGain));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HDSETID)
      {
         /* Go ahead and issue the command to set the micropone gain.   */
         ret_val = HDSET_Set_Microphone_Gain(_BluetoothStackID, HDSETID, MicrophoneGain);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for sending a button press to a      */
   /* remote Audi Gateway.  This function return zero if successful or a*/
   /* negative return error code if there was an error.                 */
int _HDSM_Send_Button_Press(unsigned int HDSETID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HDSETID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (HeadsetSupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HDSETID)
      {
         /* Go ahead and issue the command to answer an incoming call.  */
         ret_val = HDSET_Send_Button_Press(_BluetoothStackID, HDSETID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for sending a ring indication to a   */
   /* remote Headset unit.  This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int _HDSM_Ring_Indication(unsigned int HDSETID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HDSETID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (AudioGatewaySupported))
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HDSETID)
      {
         /* Go ahead and issue the command to send the ring indication. */
         ret_val = HDSET_Ring_Indication(_BluetoothStackID, HDSETID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Headset devices.  This function      */
   /* accepts as its input parameter the connection type indicating     */
   /* which connection will process the command.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HDSM_Setup_Audio_Connection(unsigned int HDSETID, Boolean_t InBandRinging)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HDSETID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HDSETID)
      {
         /* Go ahead and setup the audio connection.                    */
         ret_val = HDSET_Setup_Audio_Connection(_BluetoothStackID, HDSETID, InBandRinging);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HDSM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Headset */
   /* device.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
int _HDSM_Release_Audio_Connection(unsigned int HDSETID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HDSETID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(HDSETID)
      {
         /* Go ahead and release the audio connection.                  */
         ret_val = HDSET_Release_Audio_Connection(_BluetoothStackID, HDSETID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Headset ID, followed by the length (in Bytes)*/
   /* of the audio data to send, and a pointer to the audio data to send*/
   /* to the remote dntity.  This function returns zero if successful or*/
   /* a negative return error code if there was an error.               */
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
int _HDSM_Send_Audio_Data(unsigned int HDSETID, unsigned int AudioDataLength, unsigned char *AudioData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", HDSETID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((HDSETID) && (AudioDataLength) && (AudioData))
      {
         /* Go ahead and send the audio data.                           */
         ret_val = HDSET_Send_Audio_Data(_BluetoothStackID, HDSETID, (Byte_t)AudioDataLength, AudioData);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

