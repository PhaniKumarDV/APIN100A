/*****< hdpmgr.c >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDPMGR - Health Device Profile Manager Implementation for Stonestreet One */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHDPM.h"            /* BTPM HDP Manager Prototypes/Constants.    */
#include "HDPMMSG.h"             /* BTPM HDP Manager Message Formats.         */
#include "HDPMGR.h"              /* HDP Manager Impl. Prototypes/Constants.   */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following constant represents the default service name for the*/
   /* HDP Server Instance.                                              */
#define HDPM_DEFAULT_SERVICE_NAME   "HDP Service"

   /* The following constant represents the default provider name for   */
   /* the HDP Server Instance.                                          */
#define HDPM_DEFAULT_PROVIDER_NAME  "Stonestreet One"

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;
   
   /* Variable which serves to flag if the Source and Sink HDP Roles are*/
   /* supported.                                                        */
static Boolean_t SourceRoleSupported;
static Boolean_t SinkRoleSupported;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _HDPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variable which holds the initialization information that is       */
   /* to be used when the device powers on (registered once at          */
   /* initialization).                                                  */
static HDPM_Initialization_Info_t _HDPInitializationInfo;

   /* The following variables hold the Health Device server IDs.        */
   /* Currently this module only supports a single Health Device server */
   /* instance (in the future multiple server instances could be        */
   /* supported).                                                       */
static unsigned int HealthDeviceInstanceID;
static DWord_t      HealthDeviceInstanceSDPHandle;

   /* Variable which is used to hold the next (unique) MDEP ID.         */
static Byte_t NextMDEPID = 0x01;

   /* Internal Function Prototypes.                                     */
static Byte_t GetNextMDEPID(void);
static void BTPSAPI HDP_Event_Callback(unsigned int BluetoothStackID, HDP_Event_Data_t *HDP_Event_Data, unsigned long CallbackParameter);
static int UpdateSDPRecord(void);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique MDEP ID that can be used to register an endpoint*/
   /* with the local HDP server instance.                               */
static Byte_t GetNextMDEPID(void)
{
   Byte_t ret_val;

   ret_val = NextMDEPID++;

   if((!NextMDEPID) || (NextMDEPID > 0x7F))
      NextMDEPID = 0x01;

   return(ret_val);
}

   /* The following function is the function that is installed to       */
   /* process Health Device Events from the stack.                      */
static void BTPSAPI HDP_Event_Callback(unsigned int BluetoothStackID, HDP_Event_Data_t *HDP_Event_Data, unsigned long CallbackParameter)
{
   HDPM_Update_Data_t *HDPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((HDP_Event_Data) && (HDP_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no event to dispatch.                       */
      HDPMUpdateData = NULL;

      switch(HDP_Event_Data->Event_Data_Type)
      {
         case etHDP_Connect_Request_Indication:
         case etHDP_Control_Connect_Indication:
         case etHDP_Control_Connect_Confirmation:
         case etHDP_Control_Disconnect_Indication:
         case etHDP_Control_Create_Data_Link_Indication:
         case etHDP_Control_Create_Data_Link_Confirmation:
         case etHDP_Control_Abort_Data_Link_Indication:
         case etHDP_Control_Abort_Data_Link_Confirmation:
         case etHDP_Control_Delete_Data_Link_Indication:
         case etHDP_Control_Delete_Data_Link_Confirmation:
         case etHDP_Data_Link_Connect_Indication:
         case etHDP_Data_Link_Connect_Confirmation:
         case etHDP_Data_Link_Disconnect_Indication:
         case etHDP_Sync_Capabilities_Indication:
         case etHDP_Sync_Capabilities_Confirmation:
         case etHDP_Sync_Set_Indication:
         case etHDP_Sync_Set_Confirmation:
         case etHDP_Sync_Info_Indication:
            /* Allocate memory to hold the event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((HDP_Event_Data->Event_Data.HDP_Connect_Request_Indication_Data) && (HDPMUpdateData = (HDPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HDPM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the event data into the  */
               /* notification structure.                               */
               HDPMUpdateData->UpdateType                        = utHDPEvent;
               HDPMUpdateData->UpdateData.HDPEventData.EventType = HDP_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(HDPMUpdateData->UpdateData.HDPEventData.EventData), HDP_Event_Data->Event_Data.HDP_Connect_Request_Indication_Data, HDP_Event_Data->Event_Data_Size);
            }
            break;
         case etHDP_Data_Link_Data_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HDPMUpdateData = (HDPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HDPM_Update_Data_t) + HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data->DataLength)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HDPMUpdateData->UpdateType                        = utHDPEvent;
                  HDPMUpdateData->UpdateData.HDPEventData.EventType = HDP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HDPMUpdateData->UpdateData.HDPEventData.EventData), HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data, HDP_Event_Data->Event_Data_Size);

                  /* Fix up the pointer.                                */
                  if(HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data->DataPtr)
                  {
                     HDPMUpdateData->UpdateData.HDPEventData.EventData.HDP_Data_Link_Data_Indication_Data.DataPtr = ((Byte_t *)HDPMUpdateData) + sizeof(HDPM_Update_Data_t);

                     /* Now copy the arbitrary command/response data.   */
                     BTPS_MemCopy(HDPMUpdateData->UpdateData.HDPEventData.EventData.HDP_Data_Link_Data_Indication_Data.DataPtr, HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data->DataPtr, HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data->DataLength);
                  }
                  else
                     HDPMUpdateData->UpdateData.HDPEventData.EventData.HDP_Data_Link_Data_Indication_Data.DataPtr = NULL;
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(HDPMUpdateData)
      {
         if(!HDPM_NotifyUpdate(HDPMUpdateData))
            BTPS_FreeMemory((void *)HDPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is a utility function that exists to update*/
   /* the SDP record related to the primary HDP server instance managed */
   /* by this module.  This function returns zero on success, or a      */
   /* negative error code.                                              */
static int UpdateSDPRecord(void)
{
   int          ret_val;
   Byte_t       EIRData[2 + (2 * UUID_16_SIZE)];
   Byte_t       NumberUUIDS;
   DWord_t      ServiceRecordHandle;
   UUID_16_t    tempUUID;
   unsigned int EIRDataLength;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ServiceRecordHandle = 0;

   /* If the SDP record already exists, remove it before creating a new */
   /* one.                                                              */
   if(HealthDeviceInstanceSDPHandle)
   {
      HDP_UnRegister_SDP_Record(_BluetoothStackID, HealthDeviceInstanceSDPHandle);
      HealthDeviceInstanceSDPHandle = 0;
   }

   /* Register an SDP Record for the instance.                          */
   ret_val = HDP_Register_SDP_Record(_BluetoothStackID, HealthDeviceInstanceID, _HDPInitializationInfo.ServiceName, _HDPInitializationInfo.ProviderName, &ServiceRecordHandle);

   if((!ret_val) && (ServiceRecordHandle))
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HDP Instance SDP record updated successfully\n"));

      /* Save the new SDP record handle.                                */
      HealthDeviceInstanceSDPHandle = ServiceRecordHandle;

      /* Configure the EIR Data based on what is supported.             */
      NumberUUIDS = 0;

      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

      /* Check to see if the Source role is initialized.                */
      if(SourceRoleSupported)
      {
         /* Assign the Health Device Source UUID (in big-endian format).*/
         SDP_ASSIGN_HEALTH_DEVICE_SOURCE_SERVICE_CLASS_UUID_16(tempUUID);

         /* Convert the UUID to little endian as required by EIR data.  */
         CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

         /* Increment the number of UUIDs.                              */
         NumberUUIDS++;
      }

      /* Check to see if the Sink role is initialized.                  */
      if(SinkRoleSupported)
      {
         /* Assign the Health Device Sink UUID (in big-endian format).  */
         SDP_ASSIGN_HEALTH_DEVICE_SINK_SERVICE_CLASS_UUID_16(tempUUID);

         /* Convert the UUID to little endian as required by EIR data.  */
         CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

         /* Increment the number of UUIDs.                              */
         NumberUUIDS++;
      }

      /* Assign the length byte based on the number of UUIDs in the     */
      /* list.                                                          */
      EIRDataLength  = (NON_ALIGNED_BYTE_SIZE + (NumberUUIDS*UUID_16_SIZE));

      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);

      /* Increment the length we pass to the internal function to take  */
      /* into account the length byte.                                  */
      EIRDataLength += NON_ALIGNED_BYTE_SIZE;

      /* Configure the EIR data.                                        */
      MOD_AddEIRData(EIRDataLength, EIRData);
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error Registering HDP Instance SDP record: %d\n", ret_val));

      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_REGISTER_SDP;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Health Device Manager implementation. This function*/
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Health Device Manager Implementation.                             */
int _HDPM_Initialize(HDPM_Initialization_Info_t *HDPInitializationInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Health Device Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */
      if(HDPInitializationInfo)
      {
         _HDPInitializationInfo = *HDPInitializationInfo;
      }
      else
      {
         _HDPInitializationInfo.ServiceName  = HDPM_DEFAULT_SERVICE_NAME;
         _HDPInitializationInfo.ProviderName = HDPM_DEFAULT_PROVIDER_NAME;
      }

      if((_HDPInitializationInfo.ServiceName) && (_HDPInitializationInfo.ProviderName))
      {
         HealthDeviceInstanceID        = 0;
         HealthDeviceInstanceSDPHandle = 0;

         /* Flag that this module is initialized.                       */
         Initialized = TRUE;

         ret_val     = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_HEALTH_DEVICE_INITIALIZATION_DATA;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the Health*/
   /* Device Manager implementation. After this function is called the  */
   /* Health Device Manager implementation will no longer operate until */
   /* it is initialized again via a call to the _HDPM_Initialize()      */
   /* function.                                                         */
void _HDPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;

      /* Cleanup any servers that may be active.                        */
      if(HealthDeviceInstanceID)
         HDP_UnRegister_Instance(_BluetoothStackID, HealthDeviceInstanceID);

      if(HealthDeviceInstanceSDPHandle)
         HDP_UnRegister_SDP_Record(_BluetoothStackID, HealthDeviceInstanceSDPHandle);

      HealthDeviceInstanceID        = 0;
      HealthDeviceInstanceSDPHandle = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the Health    */
   /* Device Manager Implementation of that Bluetooth Stack ID of the   */
   /* currently opened Bluetooth stack.                                 */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Health Device Manager with  */
   /*          the specified Bluetooth Stack ID.  When this parameter is*/
   /*          set to zero, this function will actually clean up all    */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HDPM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int    Result;
   Word_t ControlPSM;
   Word_t DataPSM;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         if((Result = HDP_Initialize(BluetoothStackID)) == 0)
         {
            /* Stack has been powered up.                               */
            ControlPSM = 0x1001;
            DataPSM    = 0x1003;

            Result = HDP_Register_Instance(BluetoothStackID, ControlPSM, DataPSM, 0, HDP_Event_Callback, 0);

            while(Result == BTHDP_ERROR_PSM_IN_USE)
            {
               ControlPSM += (Word_t)2;
               DataPSM    += (Word_t)2;

               /* Only continue if we are still in the valid range for  */
               /* Dynamic PSM values.                                   */
               if((ControlPSM > (Word_t)0x1000) && (DataPSM > (Word_t)0x1000))
               {
                  if(ControlPSM & (Word_t)0x0100)
                     ControlPSM += (Word_t)0x0100;

                  if(DataPSM & (Word_t)0x0100)
                     DataPSM += (Word_t)0x0100;

                  Result = HDP_Register_Instance(BluetoothStackID, ControlPSM, DataPSM, 0, HDP_Event_Callback, 0);
               }
               else
               {
                  /* All available Dynamic PSM values have been         */
                  /* exhausted.                                         */
                  break;
               }
            }

            if(Result > 0)
            {
               /* HDP initialized successfully.                         */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HDP Initialized\n"));

               /* Note the HDP Instance ID.                             */
               HealthDeviceInstanceID = (unsigned int)Result;

               /* Go ahead and set the incoming connection mode to      */
               /* Automatic Accept.                                     */
               HDP_Set_Connection_Mode(BluetoothStackID, HealthDeviceInstanceID, hcmAutomaticAccept);
            }

            if(Result >= 0)
            {
               /* Health Device Manager initialized successfully.       */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Health Device Manager Initialized\n"));

               _BluetoothStackID = BluetoothStackID;
            }
            else
            {
               /* Error initializing HDP.                               */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Health Device Manager NOT Initialized: %d\n", Result));

               /* Error, clean up anything that might have been         */
               /* initialized.                                          */
               if(HealthDeviceInstanceID)
                  HDP_UnRegister_Instance(BluetoothStackID, HealthDeviceInstanceID);

               if(HealthDeviceInstanceSDPHandle)
                  HDP_UnRegister_SDP_Record(_BluetoothStackID, HealthDeviceInstanceSDPHandle);

               HealthDeviceInstanceID        = 0;
               HealthDeviceInstanceSDPHandle = 0;
            }
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Health Device Profile NOT Initialized: %d\n", Result));
      }
      else
      {
         /* Stack has been shutdown.                                    */
         if(HealthDeviceInstanceID)
            HDP_UnRegister_Instance(_BluetoothStackID, HealthDeviceInstanceID);

         if(HealthDeviceInstanceSDPHandle)
            HDP_UnRegister_SDP_Record(_BluetoothStackID, HealthDeviceInstanceSDPHandle);

         HealthDeviceInstanceID        = 0;
         HealthDeviceInstanceSDPHandle = 0;

         HDP_Cleanup(_BluetoothStackID);

         _BluetoothStackID             = 0;

         SourceRoleSupported           = FALSE;

         SinkRoleSupported             = FALSE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to register an endpoint on the local HDP server instance.  */
   /* This function will return a positive, non-zero value on success,  */
   /* which represents the MDEP_ID of the new endpoint. A negative error*/
   /* code is returned on failure.                                      */
int _HDPM_Register_Endpoint(Word_t DataType, HDP_Device_Role_t Role, char *Description)
{
   int             Result;
   int             ret_val;
   HDP_MDEP_Info_t MDEPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((DataType) && ((Role == drSource) || (Role == drSink)))
      {
         MDEPInfo.MDEP_ID          = GetNextMDEPID();
         MDEPInfo.MDEP_DataType    = DataType;
         MDEPInfo.MDEP_Role        = Role;
         MDEPInfo.MDEP_Description = Description;

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to register HDP endpoint: MDEP:%d Type:%d Role:%d\n", MDEPInfo.MDEP_ID, MDEPInfo.MDEP_DataType, MDEPInfo.MDEP_Role));

         if((Result = HDP_Register_Endpoint(_BluetoothStackID, HealthDeviceInstanceID, &MDEPInfo, HDP_Event_Callback, 0)) == 0)
         {
            /* Based on the role flag that it is supported.             */
            if(Role == drSource)
               SourceRoleSupported = TRUE;
            else
               SinkRoleSupported   = TRUE;

            /* Publish the new endpoint in SDP.                         */
            if((ret_val = UpdateSDPRecord()) == 0)
            {
               /* Endpoint registration successful. Return the MDEP ID  */
               /* of the new endpoint.                                  */
               ret_val = MDEPInfo.MDEP_ID;
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Unable to register HDP endpoint (%d)\n", Result));

            switch(Result)
            {
               case BTHDP_ERROR_INVALID_PARAMETER:
               case BTHDP_ERROR_INVALID_INSTANCE_ID:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_INSUFFICIENT_RESOURCES:
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  break;
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local module to un-register an endpoint on the local HDP server   */
   /* instance. This function returns zero value on success, or a       */
   /* negative error code on failure.                                   */
int _HDPM_Un_Register_Endpoint(Byte_t MDEP_ID, Word_t DataType, HDP_Device_Role_t Role)
{
   int             Result;
   int             ret_val;
   HDP_MDEP_Info_t MDEPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((VALID_MDEP_ID(MDEP_ID)) && (DataType) && ((Role == drSource) || (Role == drSink)))
      {
         MDEPInfo.MDEP_ID          = MDEP_ID;
         MDEPInfo.MDEP_DataType    = DataType;
         MDEPInfo.MDEP_Role        = Role;
         MDEPInfo.MDEP_Description = NULL;

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to un-register HDP endpoint: MDEP:%d Type:%d Role:%d\n", MDEPInfo.MDEP_ID, MDEPInfo.MDEP_DataType, MDEPInfo.MDEP_Role));

         if((Result = HDP_Un_Register_Endpoint(_BluetoothStackID, HealthDeviceInstanceID, &MDEPInfo)) == 0)
         {
            /* Publish the change in SDP.                               */
            ret_val = UpdateSDPRecord();
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Unable to un-register HDP endpoint (%d)\n", Result));

            switch(Result)
            {
               case BTHDP_ERROR_INVALID_PARAMETER:
               case BTHDP_ERROR_INVALID_MDEP_ID:
               case BTHDP_ERROR_INVALID_INSTANCE_ID:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to an incoming Data Channel connection request. */
   /* This function returns zero value on success, or a negative error  */
   /* code on failure.                                                  */
int _HDPM_Data_Connection_Request_Response(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr)
{
   int Result;
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((ResponseCode) || ((ConfigInfoPtr)))
      {
         if(ResponseCode)
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Rejecting Data Connection Request for Link %u: 0x%02X\n", DataLinkID, ResponseCode));
         else
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Accepting Data Connection Request for Link %u: 0x%02X, CM %u, FCS %u, %u, %u, %u\n", DataLinkID, ResponseCode, ChannelMode, ConfigInfoPtr->FCSMode, ConfigInfoPtr->MaxTxPacketSize, ConfigInfoPtr->NumberOfTxSegmentBuffers, ConfigInfoPtr->TxSegmentSize));

         Result = HDP_Create_Data_Channel_Response(_BluetoothStackID, DataLinkID, ResponseCode, ChannelMode, ConfigInfoPtr);

         if(!Result)
         {
            /* Success.                                                 */
            ret_val = Result;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error initiating connection to remote HDP instance (%d)\n", Result));

            switch(Result)
            {
               case BTHDP_ERROR_INVALID_PARAMETER:
               case BTHDP_ERROR_INVALID_INSTANCE_ID:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_INSUFFICIENT_RESOURCES:
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  break;
               case BTHDP_ERROR_INSTANCE_CONNECTION_ALREADY_EXISTS:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INSTANCE_ALREADY_CONNECTED;
                  break;
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local module to initiate a connection to an HDP instance on a     */
   /* remote device. The function returns a positive, non-zero value    */
   /* representing the MCLID of the new connection, if successful, or a */
   /* negative error code.                                              */
int _HDPM_Connect_Remote_Instance(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance)
{
   int Result;
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (VALID_INSTANCE(Instance)))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Attempting to connect to remote HDP instance: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08x\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance));

         Result = HDP_Connect_Remote_Instance(_BluetoothStackID, HealthDeviceInstanceID, RemoteDeviceAddress, (Word_t)GET_CONTROL_PSM(Instance), (Word_t)GET_DATA_PSM(Instance));

         if(Result > 0)
         {
            /* Success.                                                 */
            ret_val = Result;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error initiating connection to remote HDP instance (%d)\n", Result));

            switch(Result)
            {
               case BTHDP_ERROR_INVALID_PARAMETER:
               case BTHDP_ERROR_INVALID_INSTANCE_ID:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_INSUFFICIENT_RESOURCES:
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  break;
               case BTHDP_ERROR_INSTANCE_CONNECTION_ALREADY_EXISTS:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INSTANCE_ALREADY_CONNECTED;
                  break;
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local module to close an existing connection to an HDP instance   */
   /* on a remote device. The function returns zero if successful, or a */
   /* negative error code.                                              */
int _HDPM_Disconnect_Remote_Instance(unsigned int MCLID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(MCLID)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Attempting to disconnect remote HDP instance: %d\n", MCLID));

         if((ret_val = HDP_Close_Connection(_BluetoothStackID, MCLID)) >= 0)
         {
            /* Success.                                                 */
            ret_val = 0;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error disconnecting from remote HDP instance (%d)\n", ret_val));

            switch(ret_val)
            {
               case BTHDP_ERROR_INVALID_PARAMETER:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_CHANNEL_NOT_CONNECTED:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
                  break;
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local module to initiate a data channel connection to an HDP      */
   /* instance on a remote device. The function returns a positive,     */
   /* non-zero value representing the DataLinkID of the new connection, */
   /* if successful, or a negative error code.                          */
int _HDPM_Connect_Data_Channel(unsigned int MCLID, Byte_t MDEP_ID, HDP_Device_Role_t Role, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr)
{
   int Result;
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((MCLID) && (VALID_MDEP_ID(MDEP_ID)) && (ConfigInfoPtr))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Attempting to connect HDP data channel: %d, 0x%02x, 0x%02x, 0x%02x, 0x%02x, %d, %d, %d\n", MCLID, MDEP_ID, Role, ChannelMode, ConfigInfoPtr->FCSMode, ConfigInfoPtr->MaxTxPacketSize, ConfigInfoPtr->TxSegmentSize, ConfigInfoPtr->NumberOfTxSegmentBuffers));

         Result = HDP_Create_Data_Channel_Request(_BluetoothStackID, MCLID, MDEP_ID, Role, ChannelMode, ConfigInfoPtr);

         if(Result > 0)
         {
            /* Success.                                                 */
            ret_val = Result;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error initiating connection to remote HDP instance (%d)\n", Result));

            switch(Result)
            {
               case BTHDP_ERROR_INVALID_PARAMETER:
               case BTHDP_ERROR_INVALID_MDEP_ID:
               case BTHDP_ERROR_INVALID_CONFIGURATION_PARAMETER:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_INVALID_CHANNEL_MODE:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_CHANNEL_MODE;
                  break;
               case BTHDP_ERROR_INSUFFICIENT_RESOURCES:
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  break;
               case BTHDP_ERROR_REQUEST_ALREADY_OUTSTANDING:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_ENDPOINT;
                  break;
               case BTHDP_ERROR_ACTION_NOT_ALLOWED:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
                  break;
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local module to disconnect an existing HDP data channel. The      */
   /* function returns zero if successful, or a negative error code.    */
int _HDPM_Disconnect_Data_Channel(unsigned int MCLID, unsigned int DataLinkID)
{
   int Result;
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((MCLID) && (DataLinkID))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Attempting to disconnect HDP data channel: %d\n", DataLinkID));

         Result = HDP_Delete_Data_Channel(_BluetoothStackID, MCLID, DataLinkID);

         if(Result == BTHDP_ERROR_ACTION_NOT_ALLOWED)
         {
            /* The data channel connection may still be connecting, so  */
            /* abort any pending attempt.                               */
            Result = HDP_Abort_Data_Channel_Request(_BluetoothStackID, MCLID);
         }

         if(Result >= 0)
         {
            /* Success.                                                 */
            ret_val = Result;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error disconnecting HDP data channel (%d)\n", Result));

            switch(Result)
            {
               case BTHDP_ERROR_INVALID_PARAMETER:
               case BTHDP_ERROR_INVALID_MDEP_ID:
               case BTHDP_ERROR_INVALID_CHANNEL_MODE:
               case BTHDP_ERROR_INVALID_CONFIGURATION_PARAMETER:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_INSUFFICIENT_RESOURCES:
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  break;
               case BTHDP_ERROR_REQUEST_ALREADY_OUTSTANDING:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_ENDPOINT;
                  break;
               case BTHDP_ERROR_ACTION_NOT_ALLOWED:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
                  break;
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local module to write data to an existing HDP data channel. The   */
   /* function returns zero if successful, or a negative error code.    */
int _HDPM_Write_Data(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer)
{
   int Result;
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((DataLinkID) && (DataLength) && (DataBuffer))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Writing to data channel: %d, %d\n", DataLength, DataLinkID));

         Result = HDP_Write_Data(_BluetoothStackID, DataLinkID, DataLength, DataBuffer);

         if(Result >= 0)
         {
            /* Success.                                                 */
            ret_val = Result;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error writing data (%d)\n", Result));

            switch(Result)
            {
               case BTHDP_ERROR_NOT_INITIALIZED:
               case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
                  break;
               case BTHDP_ERROR_INVALID_PARAMETER:
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
               case BTHDP_ERROR_INVALID_DATA_LINK_ID:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_DATALINK_ID;
                  break;
               case BTHDP_ERROR_CHANNEL_NOT_IN_OPEN_STATE:
               default:
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
                  break;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local module to send a response to a Sync Capabilities request.   */
   /* The function returns zero if successful, or a negative error code.*/
int _HDPM_Sync_Capabilities_Response(unsigned int MCLID, Byte_t AccessResolution, Word_t SyncLeadTime, Word_t NativeResolution, Word_t NativeAccuracy, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Sending Sync Capabilities response: %u, %u, %u, %u, 0x%02X\n", AccessResolution, SyncLeadTime, NativeResolution, NativeAccuracy, ResponseCode));

      ret_val = HDP_Sync_Capabilities_Response(_BluetoothStackID, MCLID, AccessResolution, SyncLeadTime, NativeResolution, NativeAccuracy, ResponseCode);

      if(ret_val != 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Error writing data (%d)\n", ret_val));

         switch(ret_val)
         {
            case BTHDP_ERROR_NOT_INITIALIZED:
            case BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID:
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;
               break;
            case BTHDP_ERROR_INVALID_PARAMETER:
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            case BTHDP_ERROR_INVALID_DATA_LINK_ID:
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_DATALINK_ID;
            case BTHDP_ERROR_INVALID_MCL_ID:
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
               break;
            case BTHDP_ERROR_CHANNEL_NOT_IN_OPEN_STATE:
            default:
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
               break;
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}
