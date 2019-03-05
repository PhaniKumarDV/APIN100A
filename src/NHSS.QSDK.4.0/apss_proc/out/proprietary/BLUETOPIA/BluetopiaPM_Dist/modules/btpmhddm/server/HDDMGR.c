/*****< hddmgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDDMGR - HID Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/28/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHDDM.h"            /* BTPM HID Manager Prototypes/Constants.    */
#include "HDDMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */
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
   /* _HDDM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Holds the initialization data for when the stack is powered up.   */
static HDDM_Initialization_Data_t HDDMInitializationData;

   /* Holds the Record Handle for the HID Device SDP Record.            */
static DWord_t SDPRecordHandle;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI HID_Event_Callback(unsigned int BluetoothStackID, HID_Event_Data_t *HID_Event_Data, unsigned long CallbackParameter);

   /* The following function the function that is installed to process  */
   /* HID Device Events from the stack.                                 */
static void BTPSAPI HID_Event_Callback(unsigned int BluetoothStackID, HID_Event_Data_t *HID_Event_Data, unsigned long CallbackParameter)
{
   unsigned int        BufferLength;
   HDDM_Update_Data_t *HDDMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((HID_Event_Data) && (HID_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      HDDMUpdateData = NULL;

      switch(HID_Event_Data->Event_Data_Type)
      {
         case etHID_Open_Request_Indication:
         case etHID_Open_Indication:
         case etHID_Open_Confirmation:
         case etHID_Close_Indication:
         case etHID_Control_Indication:
         case etHID_Get_Report_Indication:
         case etHID_Get_Protocol_Indication:
         case etHID_Set_Protocol_Indication:
         case etHID_Get_Idle_Indication:
         case etHID_Set_Idle_Indication:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((HID_Event_Data->Event_Data.HID_Open_Request_Indication_Data) && (HDDMUpdateData = (HDDM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HDDM_Update_Data_t))) != NULL)
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               HDDMUpdateData->UpdateType                        = utHIDEvent;
               HDDMUpdateData->UpdateData.HIDEventData.EventType = HID_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(HDDMUpdateData->UpdateData.HIDEventData.EventData), HID_Event_Data->Event_Data.HID_Open_Request_Indication_Data, HID_Event_Data->Event_Data_Size);
            }
            break;
         case etHID_Set_Report_Indication:
         case etHID_Data_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HID_Event_Data->Event_Data.HID_Data_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               switch(HID_Event_Data->Event_Data_Type)
               {
                  case etHID_Data_Indication:
                     BufferLength = HID_Event_Data->Event_Data.HID_Data_Indication_Data->ReportLength;
                     break;
                  case etHID_Set_Report_Indication:
                     BufferLength = HID_Event_Data->Event_Data.HID_Set_Report_Indication_Data->ReportLength;
                     break;
                  default:
                     BufferLength = 0;
                     break;
               }

               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               /* * NOTE * Since both are unions, we do not need to     */
               /*          handle each event case separately (because we*/
               /*          have the length of the event data).          */
               if((HDDMUpdateData = (HDDM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HDDM_Update_Data_t) + BufferLength)) != NULL)
               {
                  /* Note the Event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HDDMUpdateData->UpdateType                         = utHIDEvent;
                  HDDMUpdateData->UpdateData.HIDEventData.EventType = HID_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HDDMUpdateData->UpdateData.HIDEventData.EventData), HID_Event_Data->Event_Data.HID_Data_Indication_Data, HID_Event_Data->Event_Data_Size);

                  /* Fix up the Pointer.                                */
                  switch(HID_Event_Data->Event_Data_Type)
                  {
                     case etHID_Data_Indication:
                        if(BufferLength)
                        {
                           HDDMUpdateData->UpdateData.HIDEventData.EventData.HID_Data_Indication_Data.ReportDataPayload = ((Byte_t *)(HDDMUpdateData)) + sizeof(HDDM_Update_Data_t);

                           BTPS_MemCopy(HDDMUpdateData->UpdateData.HIDEventData.EventData.HID_Data_Indication_Data.ReportDataPayload, HID_Event_Data->Event_Data.HID_Data_Indication_Data->ReportDataPayload, BufferLength);
                        }
                        else
                           HDDMUpdateData->UpdateData.HIDEventData.EventData.HID_Data_Indication_Data.ReportDataPayload = NULL;
                           
                        break;
                     case etHID_Set_Report_Indication:
                        if(BufferLength)
                        {
                           HDDMUpdateData->UpdateData.HIDEventData.EventData.HID_Set_Report_Indication_Data.ReportDataPayload = ((Byte_t *)(HDDMUpdateData)) + sizeof(HDDM_Update_Data_t);

                           BTPS_MemCopy(HDDMUpdateData->UpdateData.HIDEventData.EventData.HID_Set_Report_Indication_Data.ReportDataPayload, HID_Event_Data->Event_Data.HID_Set_Report_Indication_Data->ReportDataPayload, BufferLength);
                        }
                        else
                           HDDMUpdateData->UpdateData.HIDEventData.EventData.HID_Set_Report_Indication_Data.ReportDataPayload = NULL;

                        break;
                     default:
                        break;
                  }
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(HDDMUpdateData)
      {
         if(!HDDM_NotifyUpdate(HDDMUpdateData))
            BTPS_FreeMemory((void *)HDDMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HID Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HID Manager  */
   /* Implementation.                                                   */
int _HDDM_Initialize(HDDM_Initialization_Data_t *InitializationData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Device Manager (Imp)\n"));

      /* Note the initialization data.                                  */
      HDDMInitializationData = *InitializationData;

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the HID   */
   /* Manager Implementation.  After this function is called the HID    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HDDM_Initialize() function.  */
void _HDDM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the HID       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HID Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HDDM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int                 Result;
   Byte_t              EIRData[2 + UUID_16_SIZE];
   DWord_t             RecordHandle;
   UUID_16_t           tempUUID;
   unsigned int        EIRDataLength;
   HID_Configuration_t HIDConfiguration;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Stack has been powered up.                                  */

         /* HID Host Profile has NOT been initialized, go ahead and     */
         /* initialize it.                                              */
         HIDConfiguration.InMTU                           = L2CAP_MAXIMUM_CONNECTION_MTU_SIZE;

         HIDConfiguration.InFlow.MaxFlow.ServiceType      = 0x01;
         HIDConfiguration.InFlow.MaxFlow.TokenRate        = 0x00;
         HIDConfiguration.InFlow.MaxFlow.TokenBucketSize  = 0x00;
         HIDConfiguration.InFlow.MaxFlow.PeakBandwidth    = 0x00;
         HIDConfiguration.InFlow.MaxFlow.Latency          = 0xFFFFFFFF;
         HIDConfiguration.InFlow.MaxFlow.DelayVariation   = 0xFFFFFFFF;

         HIDConfiguration.OutFlow.MaxFlow.ServiceType     = 0x01;
         HIDConfiguration.OutFlow.MaxFlow.TokenRate       = 0x00;
         HIDConfiguration.OutFlow.MaxFlow.TokenBucketSize = 0x00;
         HIDConfiguration.OutFlow.MaxFlow.PeakBandwidth   = 0x00;
         HIDConfiguration.OutFlow.MaxFlow.Latency         = 0xFFFFFFFF;
         HIDConfiguration.OutFlow.MaxFlow.DelayVariation  = 0xFFFFFFFF;

         HIDConfiguration.OutFlow.MinFlow.ServiceType     = 0x01;
         HIDConfiguration.OutFlow.MinFlow.TokenRate       = 0x00;
         HIDConfiguration.OutFlow.MinFlow.TokenBucketSize = 0x00;
         HIDConfiguration.OutFlow.MinFlow.PeakBandwidth   = 0x00;
         HIDConfiguration.OutFlow.MinFlow.Latency         = 0xFFFFFFFF;
         HIDConfiguration.OutFlow.MinFlow.DelayVariation  = 0xFFFFFFFF;

         /* HID Configuration has been initialized, go ahead and        */
         /* Register the Device Server.                                 */
         Result = HID_Register_Device_Server(BluetoothStackID, &HIDConfiguration, HID_Event_Callback, 0);
         if(!Result )
         {
            /* HID Framework initialized successfully.                  */
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Device Server Registered\n"));

            /* Now attempt to register the service record.              */
            if(!(Result = HID_Register_Device_SDP_Record(BluetoothStackID, HDDMInitializationData.DeviceFlags, HDDMInitializationData.DeviceReleaseNumber, HDDMInitializationData.ParserVersion, HDDMInitializationData.DeviceSubclass, HDDMInitializationData.NumberDescriptors, HDDMInitializationData.DescriptorList, HDDMInitializationData.ServiceName, &RecordHandle)))
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Service Record Registered\n"));

               /* Note the RecordHandle.                                */
               SDPRecordHandle = RecordHandle;

               /* Go ahead and set the incoming connection mode to      */
               /* Manual Accept.                                        */
               HID_Set_Server_Connection_Mode(BluetoothStackID, hsmManualAccept);

               /* No errors occurred, save the Bluetooth Stack ID.      */
               _BluetoothStackID = BluetoothStackID;

               /* Configure the EIR Data.                               */
               EIRDataLength = ((2 * NON_ALIGNED_BYTE_SIZE) + UUID_16_SIZE);

               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], (NON_ALIGNED_BYTE_SIZE+UUID_16_SIZE));
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

               /* Assign the HID Device UUID.                           */
               SDP_ASSIGN_HID_PROFILE_UUID_16(tempUUID);

               /* Convert the UUID to little endian as required by EIR  */
               /* data.                                                 */
               CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2])), tempUUID);

               /* Configure the EIR data.                               */
               MOD_AddEIRData(EIRDataLength, EIRData);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Service record failed: %d\n", Result));

               HID_Un_Register_Server(BluetoothStackID);
            }
         }
         else
         {
            /* Error initializing HID Framework.                        */
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Framework NOT Initialized: %d\n", Result));
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         /* * NOTE * This function is simply a MACRO that maps to an    */
         /*          SDP Call, the HIDID parameter is ignored, so we    */
         /*          will just pass 0 since we don't know it.           */
         HID_Un_Register_Device_SDP_Record(BluetoothStackID, 0, SDPRecordHandle);

         HID_Un_Register_Server(_BluetoothStackID);

         SDPRecordHandle   = 0;
         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local HID Server.  The first   */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Stack associated with the HID Server that is responding */
   /* to the request.  The second parameter to this function is the HID */
   /* ID of the HID Server for which an open request was received.  The */
   /* final parameter to this function specifies whether to accept the  */
   /* pending connection request (or to reject the request).  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
int _HDDM_Open_Request_Response(unsigned int HIDID, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to respond to the request.                             */
      ret_val = HID_Open_Request_Response(_BluetoothStackID, HIDID, AcceptConnection);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for opening a connection    */
   /* to a Remote HID Host on the Specified Bluetooth Device.  This     */
   /* function accepts as its first parameter the Bluetooth Stack ID    */
   /* of the Bluetooth Stack which is to open the HID Connection.  The  */
   /* second parameter specifies the Board Address (NON NULL) of the    */
   /* Remote Bluetooth Device to connect with.  The third parameter to  */
   /* this function is the HID Configuration Specification to be used in*/
   /* the negotiation of the L2CAP Channels associated with this Host   */
   /* Client.  The final two parameters specify the HID Event Callback  */
   /* function and Callback Parameter, respectively, of the HID Event   */
   /* Callback that is to process any further events associated with    */
   /* this Host Client.  This function returns a non-zero, positive,    */
   /* value if successful, or a negative return error code if this      */
   /* function is unsuccessful.  If this function is successful,        */
   /* the return value will represent the HID ID that can be passed     */
   /* to all other functions that require it.  Once a Connection is     */
   /* opened to a Remote Host it can only be closed via a call to the   */
   /* _HDDM_Close_Connection() function (passing in the return value    */
   /* from a successful call to this function as the HID ID input       */
   /* parameter).                                                       */
int _HDDM_Connect_Remote_Host(BD_ADDR_t BD_ADDR)
{
   int                 ret_val;
   HID_Configuration_t HIDConfiguration;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to connect the remote host.                            */
      HIDConfiguration.InMTU                           = L2CAP_MAXIMUM_CONNECTION_MTU_SIZE;

      HIDConfiguration.InFlow.MaxFlow.ServiceType      = 0x01;
      HIDConfiguration.InFlow.MaxFlow.TokenRate        = 0x00;
      HIDConfiguration.InFlow.MaxFlow.TokenBucketSize  = 0x00;
      HIDConfiguration.InFlow.MaxFlow.PeakBandwidth    = 0x00;
      HIDConfiguration.InFlow.MaxFlow.Latency          = 0xFFFFFFFF;
      HIDConfiguration.InFlow.MaxFlow.DelayVariation   = 0xFFFFFFFF;

      HIDConfiguration.OutFlow.MaxFlow.ServiceType     = 0x01;
      HIDConfiguration.OutFlow.MaxFlow.TokenRate       = 0x00;
      HIDConfiguration.OutFlow.MaxFlow.TokenBucketSize = 0x00;
      HIDConfiguration.OutFlow.MaxFlow.PeakBandwidth   = 0x00;
      HIDConfiguration.OutFlow.MaxFlow.Latency         = 0xFFFFFFFF;
      HIDConfiguration.OutFlow.MaxFlow.DelayVariation  = 0xFFFFFFFF;

      HIDConfiguration.OutFlow.MinFlow.ServiceType     = 0x01;
      HIDConfiguration.OutFlow.MinFlow.TokenRate       = 0x00;
      HIDConfiguration.OutFlow.MinFlow.TokenBucketSize = 0x00;
      HIDConfiguration.OutFlow.MinFlow.PeakBandwidth   = 0x00;
      HIDConfiguration.OutFlow.MinFlow.Latency         = 0xFFFFFFFF;
      HIDConfiguration.OutFlow.MinFlow.DelayVariation  = 0xFFFFFFFF;

      ret_val = HID_Connect_Remote_Host(_BluetoothStackID, BD_ADDR, &HIDConfiguration, HID_Event_Callback, 0);

      if(ret_val > 0)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for closing a HID           */
   /* connection established through a connection made to a Registered  */
   /* Server or a connection that was made by calling either the        */
   /* _HDDM_Open_Remote_Device() or HID_Open_Remote_Host() functions.   */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the HID ID specified by the Second  */
   /* Parameter is valid for.  This function returns zero if successful,*/
   /* or a negative return error code if an error occurred.  Note that  */
   /* if this function is called with the HID ID of a Local Server, the */
   /* Server will remain registered but the connection associated with  */
   /* the specified HID ID will be closed. SendVirtualCableDisconnect   */
   /* specifies whether to send the virtual cable disconnect control    */
   /* command before disconnecting.                                     */
int _HDDM_Close_Connection(unsigned int HIDID, Boolean_t SendVirtualCableDisconnect)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(SendVirtualCableDisconnect)
         HID_Control_Request(_BluetoothStackID, HIDID, hcVirtualCableUnplug);

      /* Attempt to close the connection.                               */
      ret_val = HID_Close_Connection(_BluetoothStackID, HIDID);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_REPORT transaction.  This function */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The            */
   /* rtSuccessful Result Type is Invalid for use with this function.   */
   /* If the rtNotReady through rtErrFatal Result Statuses are used to  */
   /* respond, a HANDSHAKE response that has a Result Code parameter of */
   /* the specified Error Condition is sent.  If the ResultType         */
   /* specified is rtData, the GET_REPORT transaction is responded to   */
   /* with a DATA Response that has the Report (specified by the final  */
   /* parameter) as its Payload.  The fourth parameter is the type of   */
   /* report being sent.  Note that rtOther is an Invalid Report Type   */
   /* for use with this function.  The final two parameters are the     */
   /* Length of the Report Payload to send and a pointer to the Report  */
   /* Payload that will be sent.  This function returns a zero if       */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Get_Report_Response(unsigned int HIDID, HID_Result_Type_t ResultType, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to respond to the request.                             */
      ret_val = HID_Get_Report_Response(_BluetoothStackID, HIDID, ResultType, ReportType, ReportPayloadSize, ReportDataPayload);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_REPORT transaction.  This function */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtData Result Type is Invalid for use with this    */
   /* function.  If the rtSuccessful through rtErrFatal Result Types are*/
   /* specified, this function responds to the SET_REPORT request with a*/
   /* HANDSHAKE response that has a Result Code parameter that matches  */
   /* the specified Result Type.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Set_Report_Response(unsigned int HIDID, HID_Result_Type_t ResultType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to respond to the request.                             */
      ret_val = HID_Set_Report_Response(_BluetoothStackID, HIDID, ResultType);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_PROTOCOL transaction.  This        */
   /* function accepts the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtSuccessful Result Type is Invalid for use with   */
   /* this function.  If the rtNotReady through rtErrFatal Result Types */
   /* are specified, this function will respond to the GET_PROTOCOL     */
   /* request with a HANDSHAKE response that has a Result Code parameter*/
   /* of the specified Error Condition.  If the ResultType specified is */
   /* rtData, the GET_PROTOCOL transaction is responded to with a DATA  */
   /* Response that has the Protocol type specified as the final        */
   /* parameter as its Payload.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Get_Protocol_Response(unsigned int HIDID, HID_Result_Type_t ResultType, HID_Protocol_Type_t Protocol)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u, %u, %d, %d\n", _BluetoothStackID, HIDID, ResultType, Protocol));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to respond to the request.                             */
      ret_val = HID_Get_Protocol_Response(_BluetoothStackID, HIDID, ResultType, Protocol);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_PROTOCOL transaction.  This        */
   /* function accepts the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtData Result Type is Invalid for use with this    */
   /* function.  If the rtSuccessful through rtErrFatal Result Types are*/
   /* specified then this function will respond to the SET_PROTOCOL     */
   /* Transaction with a HANDSHAKE response that has a Result Code      */
   /* parameter that matches the specified Result Type.  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _HDDM_Set_Protocol_Response(unsigned int HIDID, HID_Result_Type_t ResultType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u, %d\n", HIDID, ResultType));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to respond to the request.                             */
      ret_val = HID_Set_Protocol_Response(_BluetoothStackID, HIDID, ResultType);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_IDLE transaction.  This function   */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The            */
   /* rtSuccessful Result Type is Invalid for use with this function.   */
   /* If the rtNotReady through rtErrFatal Result Types are specified,  */
   /* then this function will respond to the GET_IDLE Transaction with a*/
   /* HANDSHAKE response that has a Result Code parameter of the        */
   /* specified Error Condition.  If the ResultType specified is rtData */
   /* the GET_IDLE transaction is responded to with a DATA Response that*/
   /* has the Idle Rate specified as the final parameter as its Payload.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _HDDM_Get_Idle_Response(unsigned int HIDID, HID_Result_Type_t ResultType, Byte_t IdleRate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to respond to the request.                             */
      ret_val = HID_Get_Idle_Response(_BluetoothStackID, HIDID, ResultType, IdleRate);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_IDLE transaction.  This function   */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The rtData     */
   /* Result Type is Invalid for use with this function.  If the        */
   /* rtSuccessful through rtErrFatal Result Types are specified, then  */
   /* this function will respond to the SET_IDLE Transaction with a     */
   /* HANDSHAKE response that has a Result Code parameter that matches  */
   /* the specified Result Type.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Set_Idle_Response(unsigned int HIDID, HID_Result_Type_t ResultType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to respond to the request.                             */
      ret_val = HID_Set_Idle_Response(_BluetoothStackID, HIDID, ResultType);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending Reports over    */
   /* the Interrupt Channel.  This function accepts the Bluetooth Stack */
   /* ID of the Bluetooth Stack which is to send the Report Data and    */
   /* the HID ID for which the Connection has been established.  The    */
   /* third parameter is the type of report being sent.  The final two  */
   /* parameters are the Length of the Report Payload to send and a     */
   /* pointer to the Report Payload that will be sent.  This function   */
   /* returns a zero if successful, or a negative return error code if  */
   /* there was an error.                                               */
int _HDDM_Data_Write(unsigned int HIDID, Word_t ReportPayloadSize, Byte_t *ReportDataPayload)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to send the report.                                    */
      ret_val = HID_Data_Write(_BluetoothStackID, HIDID, rtInput, ReportPayloadSize, ReportDataPayload);

      if(ret_val)
      {
         //XXX
         if(ret_val == BTHID_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


