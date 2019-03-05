/*****< panmgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PANMGR - PAN Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/31/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMPANM.h"            /* BTPM PAN Manager Prototypes/Constants.    */
#include "PANMMSG.h"             /* BTPM PAN Manager Message Formats.         */
#include "PANMGR.h"              /* PAN Manager Impl. Prototypes/Constants.   */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

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
   /* _PANM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variables which hold the initialization information that is to be */
   /* used when the device powers on (registered once at                */
   /* initialization).                                                  */
static PANM_Initialization_Info_t *_PANMInitializationInfo;

   /* The following variables hold the PAN SDP Service Record Handles of*/
   /* each of configured server options.                                */
static DWord_t SDPRecordHandle_User;
static DWord_t SDPRecordHandle_AccessPoint;
static DWord_t SDPRecordHandle_GroupAdHoc;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI PAN_Event_Callback(unsigned int BluetoothStackID, PAN_Event_Data_t *PAN_Event_Data, unsigned long CallbackParameter);

   /* The following function is the function that is installed to       */
   /* process PAN Events from the stack.                                */
static void BTPSAPI PAN_Event_Callback(unsigned int BluetoothStackID, PAN_Event_Data_t *PAN_Event_Data, unsigned long CallbackParameter)
{
   PANM_Update_Data_t *PANMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((PAN_Event_Data) && (PAN_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no event to dispatch.                       */
      PANMUpdateData = NULL;

      switch(PAN_Event_Data->Event_Data_Type)
      {
         case etPAN_Open_Request_Indication:
         case etPAN_Open_Indication:
         case etPAN_Open_Confirmation:
         case etPAN_Close_Indication:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((PAN_Event_Data->Event_Data.PAN_Open_Request_Indication_Data) && (PANMUpdateData = (PANM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PANM_Update_Data_t))) != NULL)
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               PANMUpdateData->UpdateType                        = utPANEvent;
               PANMUpdateData->UpdateData.PANEventData.EventType = PAN_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(PANMUpdateData->UpdateData.PANEventData.EventData), PAN_Event_Data->Event_Data.PAN_Open_Request_Indication_Data, PAN_Event_Data->Event_Data_Size);
            }
            break;
         default:
            /* Unknown/Un-Handled event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(PANMUpdateData)
      {
         if(!PANM_NotifyUpdate(PANMUpdateData))
            BTPS_FreeMemory((void *)PANMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PAN Manager Implementation. This function returns  */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAN Manager  */
   /* Implementation.                                                   */
int _PANM_Initialize(PANM_Initialization_Info_t *PANMInitializationInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAN Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */
      if((PANMInitializationInfo) && (PANMInitializationInfo->NumberNetworkPacketTypes) && (PANMInitializationInfo->NetworkPacketTypeList))
      {
         /* Further verify that the data passed appears to be           */
         /* semi-valid.                                                 */
         if(((PANMInitializationInfo->ServiceTypeFlags & PAN_PERSONAL_AREA_NETWORK_USER_SERVICE) && (PANMInitializationInfo->NamingData_User) && (PANMInitializationInfo->NamingData_User->ServiceName)) || ((PANMInitializationInfo->ServiceTypeFlags & PAN_NETWORK_ACCESS_POINT_SERVICE) && (PANMInitializationInfo->NamingData_AccessPoint) && (PANMInitializationInfo->NamingData_AccessPoint->ServiceName)) || ((PANMInitializationInfo->ServiceTypeFlags & PAN_GROUP_ADHOC_NETWORK_SERVICE) && (PANMInitializationInfo->NamingData_GroupAdHoc) && (PANMInitializationInfo->NamingData_GroupAdHoc->ServiceName)))
         {
            /* Data appears to be semi-valid.  Go ahead and not the     */
            /* information so we can use it when the device is powered  */
            /* on.                                                      */
            _PANMInitializationInfo     = PANMInitializationInfo;

            /* Flag that nothing is currently registered.               */
            SDPRecordHandle_User        = 0;
            SDPRecordHandle_AccessPoint = 0;
            SDPRecordHandle_GroupAdHoc  = 0;

            /* Flag that this module is initialized.                    */
            Initialized                 = TRUE;

            /* Flag success.                                            */
            ret_val                     = 0;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PAN_INITIALIZATION_DATA;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PAN_INITIALIZATION_DATA;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* PAN Manager Implementation. After this function is called the     */
   /* PAN Manager Implementation will no longer operate until it is     */
   /* initialized again via a call to the _PANM_Initialize() function.  */
void _PANM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Flag that there is no PAN initialization information.          */
      _PANMInitializationInfo = 0;

      /* Cleanup any servers that may be active.                        */
      PAN_Close_Server(_BluetoothStackID);

      if(SDPRecordHandle_User)
         SDP_Delete_Service_Record(_BluetoothStackID, SDPRecordHandle_User);

      if(SDPRecordHandle_AccessPoint)
         SDP_Delete_Service_Record(_BluetoothStackID, SDPRecordHandle_AccessPoint);

      if(SDPRecordHandle_GroupAdHoc)
         SDP_Delete_Service_Record(_BluetoothStackID, SDPRecordHandle_GroupAdHoc);

      /* Go ahead and clean-up the PAN Module.                          */
      PAN_Cleanup(_BluetoothStackID);

      /* Flag that nothing is currently registered.                     */
      SDPRecordHandle_User        = 0;
      SDPRecordHandle_AccessPoint = 0;
      SDPRecordHandle_GroupAdHoc  = 0;

      /* Regardless if the stack is open, flag that it is not.          */
      _BluetoothStackID           = 0;

      /* Finally flag that this module is no longer initialized.        */
      Initialized                 = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the PAN       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the PAN Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _PANM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int          Result;
   Byte_t       NumberUUIDS;
   Byte_t       EIRData[2 + (UUID_16_SIZE * 3)];
   UUID_16_t    tempUUID;
   unsigned int EIRDataLength;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Stack has been powered up.                                  */

         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAN with Driver %u.\n", _PANMInitializationInfo->VirtualNetworkDriverIndex));

         /* Go ahead and initialize the PAN Module.                     */
         Result = PAN_Initialize(BluetoothStackID, _PANMInitializationInfo->VirtualNetworkDriverIndex);

         if(!Result)
         {
            /* PAN Module intialized successfully.                      */
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN Module Initialized\n"));

            Result = PAN_Open_Server(BluetoothStackID, _PANMInitializationInfo->ServiceTypeFlags & (PAN_PERSONAL_AREA_NETWORK_USER_SERVICE | PAN_NETWORK_ACCESS_POINT_SERVICE | PAN_GROUP_ADHOC_NETWORK_SERVICE), PAN_Event_Callback, 0);

            if(!Result)
            {
               /* Next, go ahead and make sure that we set the incoming */
               /* connection mode to manual.                            */
               PAN_Set_Server_Connection_Mode(BluetoothStackID, psmManualAccept);

               /* Configure the EIR Data based on what is supported.    */
               NumberUUIDS = 0;

               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

               /* Next, initialize all server SDP information.          */
               if(_PANMInitializationInfo->ServiceTypeFlags & PAN_PERSONAL_AREA_NETWORK_USER_SERVICE)
               {
                  PAN_Register_Personal_Area_Network_User_SDP_Record(BluetoothStackID, _PANMInitializationInfo->NumberNetworkPacketTypes, _PANMInitializationInfo->NetworkPacketTypeList, _PANMInitializationInfo->NamingData_User->ServiceName, _PANMInitializationInfo->NamingData_User->ServiceDescription, _PANMInitializationInfo->SecurityDescription, &SDPRecordHandle_User);
                  
                  /* Assign the PAN-U Role UUID (in big-endian format). */
                  SDP_ASSIGN_PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_16(tempUUID);

                  /* Convert the UUID to little endian as required by   */
                  /* EIR data.                                          */
                  CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                  /* Increment the number of UUIDs.                     */
                  NumberUUIDS++;
               }

               if(_PANMInitializationInfo->ServiceTypeFlags & PAN_NETWORK_ACCESS_POINT_SERVICE)
               {
                  PAN_Register_Network_Access_Point_SDP_Record(BluetoothStackID, _PANMInitializationInfo->NumberNetworkPacketTypes, _PANMInitializationInfo->NetworkPacketTypeList, _PANMInitializationInfo->NamingData_AccessPoint->ServiceName, _PANMInitializationInfo->NamingData_AccessPoint->ServiceDescription, _PANMInitializationInfo->SecurityDescription, _PANMInitializationInfo->NetworkAccessType, _PANMInitializationInfo->MaxNetAccessRate, &SDPRecordHandle_AccessPoint);
                  
                  /* Assign the PAN NAP Role UUID (in big-endian        */
                  /* format).                                           */
                  SDP_ASSIGN_NETWORK_ACCESS_POINT_PROFILE_UUID_16(tempUUID);

                  /* Convert the UUID to little endian as required by   */
                  /* EIR data.                                          */
                  CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                  /* Increment the number of UUIDs.                     */
                  NumberUUIDS++;
               }

               if(_PANMInitializationInfo->ServiceTypeFlags & PAN_GROUP_ADHOC_NETWORK_SERVICE)
               {
                  PAN_Register_Group_Adhoc_Network_SDP_Record(BluetoothStackID, _PANMInitializationInfo->NumberNetworkPacketTypes, _PANMInitializationInfo->NetworkPacketTypeList, _PANMInitializationInfo->NamingData_GroupAdHoc->ServiceName, _PANMInitializationInfo->NamingData_GroupAdHoc->ServiceDescription, _PANMInitializationInfo->SecurityDescription, &SDPRecordHandle_AccessPoint);
                  
                  /* Assign the PAN Adhoc Role UUID (in big-endian      */
                  /* format).                                           */
                  SDP_ASSIGN_GROUP_ADHOC_NETWORK_PROFILE_UUID_16(tempUUID);

                  /* Convert the UUID to little endian as required by   */
                  /* EIR data.                                          */
                  CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2 + (NumberUUIDS * UUID_16_SIZE)])), tempUUID);

                  /* Increment the number of UUIDs.                     */
                  NumberUUIDS++;
               }

               /* Note the Bluetooth Stack ID.                          */
               _BluetoothStackID = BluetoothStackID;

               /* Flag success.                                         */
               Result            = 0;

               /* If we have any UUIDs registered then add the UUID     */
               /* data.                                                 */
               if(NumberUUIDS > 0)
               {
                  /* Assign the length byte based on the number of UUIDs*/
                  /* in the list.                                       */
                  EIRDataLength = (NON_ALIGNED_BYTE_SIZE + (NumberUUIDS*UUID_16_SIZE));
   
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
   
                  /* Increment the length we pass to the internal       */
                  /* function to take into account the length byte.     */
                  EIRDataLength += NON_ALIGNED_BYTE_SIZE;
   
                  /* Configure the EIR data.                            */
                  MOD_AddEIRData(EIRDataLength, EIRData);
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
         {
            /* Error initializing PAN Module.                           */
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN Module NOT Initialized: %d.\n", Result));
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN Module Shutdown.\n"));

         /* Cleanup any servers that may be active.                     */
         PAN_Close_Server(_BluetoothStackID);

         if(SDPRecordHandle_User)
            SDP_Delete_Service_Record(_BluetoothStackID, SDPRecordHandle_User);

         if(SDPRecordHandle_AccessPoint)
            SDP_Delete_Service_Record(_BluetoothStackID, SDPRecordHandle_AccessPoint);

         if(SDPRecordHandle_GroupAdHoc)
            SDP_Delete_Service_Record(_BluetoothStackID, SDPRecordHandle_GroupAdHoc);

         /* Stack has been shutdown.                                    */
         PAN_Cleanup(_BluetoothStackID);

         /* Flag that nothing is currently registered.                  */
         SDPRecordHandle_User        = 0;
         SDPRecordHandle_AccessPoint = 0;
         SDPRecordHandle_GroupAdHoc  = 0;

         _BluetoothStackID           = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to accept or reject a request to open a connection to     */
   /* the local Server. This function returns zero if successful and a  */
   /* negative value if there was an error.                             */
int _PANM_Open_Request_Response(unsigned int PANID, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Responding %s to request %u.\n", AcceptConnection?"TRUE":"FALSE", PANID));

      /* Attempt to respond to the request.                             */
      ret_val = PAN_Open_Request_Response(_BluetoothStackID, PANID, AcceptConnection);

      if(ret_val < 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN_Open_Request_Response: %d.\n", ret_val));

         if(ret_val == BTPAN_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_PAN_INVALID_OPERATION;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to opena remote PAN Server. This function returns   */
   /* a positive non-zero value representing the PANID of the opened    */
   /* connection is successful, and a negative value if there was an    */
   /* error.                                                            */
int _PANM_Open_Remote_Server(BD_ADDR_t BD_ADDR, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Opening Remote Server.\n"));

      /* Attempt to open the remote server.                             */
      ret_val = PAN_Open_Remote_Server(_BluetoothStackID, BD_ADDR, LocalServiceType, RemoteServiceType, PAN_Event_Callback, 0);

      if(ret_val < 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN_Open_Remote_Server: %d.\n", ret_val));

         if(ret_val == BTPAN_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_PAN_UNABLE_TO_CONNECT_TO_DEVICE;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to close a previously opened PAN Connection. This function*/
   /* returns zero if successful and a negative value if there was an   */
   /* error.                                                            */
   /* * NOTE * This function does NOT un-register any server. It only   */
   /*          closes any active connections.                           */
int _PANM_Close_Connection(unsigned int PANID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Closing PAN Connection %u.\n", PANID));

      /* Attempt to close PAN Connection.                               */
      ret_val = PAN_Close(_BluetoothStackID, PANID);

      if(ret_val < 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN_Close: %d.\n", ret_val));

         if(ret_val == BTPAN_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_PAN_UNABLE_TO_DISCONNECT_DEVICE;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

