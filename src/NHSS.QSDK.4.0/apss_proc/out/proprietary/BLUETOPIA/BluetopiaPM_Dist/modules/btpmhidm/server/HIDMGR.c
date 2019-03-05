/*****< hidmgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDMGR - HID Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/18/11  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHIDM.h"            /* BTPM HID Manager Prototypes/Constants.    */
#include "HIDMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

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
   /* _HIDM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI HID_Host_Event_Callback(unsigned int BluetoothStackID, HID_Host_Event_Data_t *HID_Host_Event_Data, unsigned long CallbackParameter);

   /* The following function the function that is installed to process  */
   /* HID Host Events from the stack.                                   */
static void BTPSAPI HID_Host_Event_Callback(unsigned int BluetoothStackID, HID_Host_Event_Data_t *HID_Host_Event_Data, unsigned long CallbackParameter)
{
   unsigned int        BufferLength;
   HIDM_Update_Data_t *HIDMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((HID_Host_Event_Data) && (HID_Host_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      HIDMUpdateData = NULL;

      switch(HID_Host_Event_Data->Event_Data_Type)
      {
         case etHID_Host_Open_Request_Indication:
         case etHID_Host_Open_Indication:
         case etHID_Host_Open_Confirmation:
         case etHID_Host_Close_Indication:
         case etHID_Host_Boot_Keyboard_Data_Indication:
         case etHID_Host_Boot_Keyboard_Repeat_Indication:
         case etHID_Host_Boot_Mouse_Data_Indication:
         case etHID_Host_Set_Report_Confirmation:
         case etHID_Host_Get_Protocol_Confirmation:
         case etHID_Host_Set_Protocol_Confirmation:
         case etHID_Host_Get_Idle_Confirmation:
         case etHID_Host_Set_Idle_Confirmation:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((HID_Host_Event_Data->Event_Data.HID_Host_Open_Request_Indication_Data) && (HIDMUpdateData = (HIDM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HIDM_Update_Data_t))) != NULL)
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               HIDMUpdateData->UpdateType                         = utHIDHEvent;
               HIDMUpdateData->UpdateData.HIDHEventData.EventType = HID_Host_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(HIDMUpdateData->UpdateData.HIDHEventData.EventData), HID_Host_Event_Data->Event_Data.HID_Host_Open_Request_Indication_Data, HID_Host_Event_Data->Event_Data_Size);
            }
            break;
         case etHID_Host_Data_Indication:
         case etHID_Host_Get_Report_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(HID_Host_Event_Data->Event_Data.HID_Host_Data_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               switch(HID_Host_Event_Data->Event_Data_Type)
               {
                  case etHID_Host_Data_Indication:
                     BufferLength = HID_Host_Event_Data->Event_Data.HID_Host_Data_Indication_Data->ReportLength;
                     break;
                  case etHID_Host_Get_Report_Confirmation:
                     BufferLength = HID_Host_Event_Data->Event_Data.HID_Host_Get_Report_Confirmation_Data->ReportLength;
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
               if((HIDMUpdateData = (HIDM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HIDM_Update_Data_t) + BufferLength)) != NULL)
               {
                  /* Note the Event type and copy the event data into   */
                  /* the Notification structure.                        */
                  HIDMUpdateData->UpdateType                         = utHIDHEvent;
                  HIDMUpdateData->UpdateData.HIDHEventData.EventType = HID_Host_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(HIDMUpdateData->UpdateData.HIDHEventData.EventData), HID_Host_Event_Data->Event_Data.HID_Host_Data_Indication_Data, HID_Host_Event_Data->Event_Data_Size);

                  /* Fix up the Pointer.                                */
                  switch(HID_Host_Event_Data->Event_Data_Type)
                  {
                     case etHID_Host_Data_Indication:
                        if(BufferLength)
                        {
                           HIDMUpdateData->UpdateData.HIDHEventData.EventData.HID_Host_Data_Indication_Data.ReportDataPayload = ((Byte_t *)(HIDMUpdateData)) + sizeof(HIDM_Update_Data_t);

                           BTPS_MemCopy(HIDMUpdateData->UpdateData.HIDHEventData.EventData.HID_Host_Data_Indication_Data.ReportDataPayload, HID_Host_Event_Data->Event_Data.HID_Host_Data_Indication_Data->ReportDataPayload, BufferLength);
                        }
                        else
                           HIDMUpdateData->UpdateData.HIDHEventData.EventData.HID_Host_Data_Indication_Data.ReportDataPayload = NULL;

                        break;
                     case etHID_Host_Get_Report_Confirmation:
                        if(BufferLength)
                        {
                           HIDMUpdateData->UpdateData.HIDHEventData.EventData.HID_Host_Get_Report_Confirmation_Data.ReportDataPayload = ((Byte_t *)(HIDMUpdateData)) + sizeof(HIDM_Update_Data_t);

                           BTPS_MemCopy(HIDMUpdateData->UpdateData.HIDHEventData.EventData.HID_Host_Get_Report_Confirmation_Data.ReportDataPayload, HID_Host_Event_Data->Event_Data.HID_Host_Get_Report_Confirmation_Data->ReportDataPayload, BufferLength);
                        }
                        else
                           HIDMUpdateData->UpdateData.HIDHEventData.EventData.HID_Host_Get_Report_Confirmation_Data.ReportDataPayload = NULL;

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
      if(HIDMUpdateData)
      {
         if(!HIDM_NotifyUpdate(HIDMUpdateData))
            BTPS_FreeMemory((void *)HIDMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HID Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HID Manager  */
   /* Implementation.                                                   */
int _HIDM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the HID   */
   /* Manager Implementation.  After this function is called the HID    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HIDM_Initialize() function.  */
void _HIDM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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
void _HIDM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int                 Result;
   HID_Configuration_t HIDConfiguration;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
         /* initialize the HID Module.                                  */
         Result = HID_Host_Initialize(BluetoothStackID, &HIDConfiguration, HID_Host_Event_Callback, 0);

         if(!Result )
         {
            /* HID Framework initialized successfully.                  */
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Framework Initialized\n"));

            /* Go ahead and set the incoming connection mode to Manual  */
            /* Accept.                                                  */
            HID_Host_Set_Server_Connection_Mode(BluetoothStackID, hsmManualAccept);

            _BluetoothStackID = BluetoothStackID;
         }
         else
         {
            /* Error initializing HID Framework.                        */
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Framework NOT Initialized: %d\n", Result));
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         HID_Host_Un_Initialize(_BluetoothStackID);

         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming HID connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HID Connected   */
   /*          event will be dispatched to signify the actual result.   */
int _HIDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept, unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Responding to Request: %d, 0x%08lX\n", Accept, ConnectionFlags));

      /* Attempt to respond to the request.                             */
      ret_val = HID_Host_Open_Request_Response(_BluetoothStackID, RemoteDeviceAddress, Accept, ConnectionFlags);

      if(ret_val)
      {
         if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_HID_DEVICE;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote HID device.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHIDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HIDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int _HIDM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Opening Device: 0x%08lX\n", ConnectionFlags));

      /* Attempt to open the remote device.                             */
      ret_val = HID_Host_Connect_Remote_Device(_BluetoothStackID, RemoteDeviceAddress, ConnectionFlags);

      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local device to disconnect a currently connected remote device    */
   /* (connected to the local device's HID Host).  This function accepts*/
   /* as input the Remote Device Address of the Remote HID Device to    */
   /* disconnect from the local HID Host.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
int _HIDM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableDisconnect)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Device\n"));

      /* Attempt to disconnect the remote device.                       */
      ret_val = HID_Host_Close_Connection(_BluetoothStackID, RemoteDeviceAddress, SendVirtualCableDisconnect);

      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is used to set the HID Host Keyboard Key   */
   /* Repeat behavior.  Some Host operating systems nativity support Key*/
   /* Repeat automatically, but for those Host operating systems that do*/
   /* not - this function will instruct the HID Manager to simulate Key */
   /* Repeat behavior (with the specified parameters).  This function   */
   /* accepts the initial amount to delay (in milliseconds) before      */
   /* starting the repeat functionality.  The final parameter specifies */
   /* the rate of repeat (in milliseconds).  This function returns zero */
   /* if successful or a negative value if there was an error.          */
   /* * NOTE * Specifying zero for the Repeat Delay (first parameter)   */
   /*          will disable HID Manager Key Repeat processing.  This    */
   /*          means that only Key Up/Down events will be dispatched    */
   /*          and No Key Repeat events will be dispatched.             */
   /* * NOTE * The Key Repeat parameters can only be changed when there */
   /*          are no actively connected HID devices.                   */
int _HIDM_Set_Keyboard_Repeat_Rate(unsigned int RepeatDelay, unsigned int RepeatRate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Setting Keyboard Key Repeat Rate: %u %u\n", RepeatDelay, RepeatRate));

      /* Attempt to set the Keyboard Repeat Rate.                       */
      ret_val = HID_Host_Set_Keyboard_Repeat(_BluetoothStackID, RepeatDelay, RepeatRate);

      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* HID Report Data to a currently connected remote device.  This     */
   /* function accepts as input the remote device address of the remote */
   /* HID device to send the report data to, followed by the report data*/
   /* itself.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _HIDM_Send_Report_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ReportDataLength) && (ReportData))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Data: %d\n", ReportDataLength));

         /* Attempt to write the data to the remote device.             */
         ret_val = HID_Host_Data_Write(_BluetoothStackID, RemoteDeviceAddress, (Word_t)ReportDataLength, ReportData);

         if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as       */
   /* input the remote device address of the remote HID device to       */
   /* send the report data to, the type of report requested, and        */
   /* the Report ID determined by the Device's SDP record.  Passing     */
   /* HIDM_INVALID_REPORT_ID as the value for this parameter will       */
   /* indicate that this parameter is not used and will exclude the     */
   /* appropriate byte from the transaction payload.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _HIDM_Send_Get_Report_Request(BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending 'Get Report' Request: %u %u\n", ReportType, ReportID));

      /* Attempt to send the 'Get Report' request to the remote device. */
      /* Platform Manager will always use the grSizeOfReport response   */
      /* size because it will allocate the incoming buffer on-demand and*/
      /* does not need to limit the response size.                      */
      ret_val = HID_Host_Get_Report_Request(_BluetoothStackID, RemoteDeviceAddress, grSizeOfReport, ReportType, ReportID, 0);

      /* XXX Convert more error types.                                  */
      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input the */
   /* remote device address of the remote HID device to send the report */
   /* data to, the type of report being sent, the Length of the Report  */
   /* Data to send, and a pointer to the Report Data that will be sent. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _HIDM_Send_Set_Report_Request(BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ReportDataLength) && (ReportData))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending 'Set Report' Request: %u %u\n", ReportType, ReportDataLength));

         /* Attempt to send the 'Set Report' request to the remote      */
         /* device.                                                     */
         ret_val = HID_Host_Set_Report_Request(_BluetoothStackID, RemoteDeviceAddress, ReportType, ReportDataLength, ReportData);

         /* XXX Convert more error types.                               */
         if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the remote device address of the remote HID device to send  */
   /* the report data to.  This function returns a zero if successful,  */
   /* or a negative return error code if there was an error.            */
int _HIDM_Send_Get_Protocol_Request(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending 'Get Protocol' Request\n"));

      /* Attempt to send the 'Get Protocol' request to the remote       */
      /* device.                                                        */
      ret_val = HID_Host_Get_Protocol_Request(_BluetoothStackID, RemoteDeviceAddress);

      /* XXX Convert more error types.                                  */
      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the remote device address of the remote HID device to send  */
   /* the report data to and the protocol to be set.  This function     */
   /* returns a zero if successful, or a negative return error code if  */
   /* there was an error.                                               */
int _HIDM_Send_Set_Protocol_Request(BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending 'Set Protocol' Request: %u\n", Protocol));

      /* Attempt to send the 'Set Protocol' request to the remote       */
      /* device.                                                        */
      ret_val = HID_Host_Set_Protocol_Request(_BluetoothStackID, RemoteDeviceAddress, Protocol);

      /* XXX Convert more error types.                                  */
      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the the remote device address of the remote HID device      */
   /* to send the report data to.  This function returns a zero if      */
   /* successful, or a negative return error code if there was an error.*/
int _HIDM_Send_Get_Idle_Request(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending 'Get Idle' Request\n"));

      /* Attempt to send the 'Get Idle' request to the remote device.   */
      ret_val = HID_Host_Get_Idle_Request(_BluetoothStackID, RemoteDeviceAddress);

      /* XXX Convert more error types.                                  */
      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the remote device address of the remote HID device to send  */
   /* the report data to and the Idle Rate to be set.  The Idle Rate    */
   /* LSB is weighted to 4ms (i.e. the Idle Rate resolution is 4ms with */
   /* a range from 4ms to 1.020s).  This function returns a zero if     */
   /* successful, or a negative return error code if there was an error.*/
int _HIDM_Send_Set_Idle_Request(BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending 'Set Idle' Request: %u\n", IdleRate));

      /* Attempt to send the 'Set Idle' request to the remote device.   */
      ret_val = HID_Host_Set_Idle_Request(_BluetoothStackID, RemoteDeviceAddress, IdleRate);

      /* XXX Convert more error types.                                  */
      if(ret_val == BTHID_HOST_ERROR_INVALID_PARAMETER)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

