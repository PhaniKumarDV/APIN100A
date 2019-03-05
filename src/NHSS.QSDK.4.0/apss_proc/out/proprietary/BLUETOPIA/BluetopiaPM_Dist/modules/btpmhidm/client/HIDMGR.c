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
#include "HIDMMSG.h"             /* BTPM HID Manager Message Formats.         */
#include "HIDMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following constant defines the maximum timeout (in ms) that is*/
   /* allowed for a response to be received from a general message sent */
   /* to the Device Manager through the Message system.                 */
#define MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT               (BTPM_CONFIGURATION_DEVICE_MANAGER_GENERAL_MESSAGE_RESPONSE_TIME_MS)

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

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
      Initialized = FALSE;
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
   int                                         ret_val;
   BD_ADDR_t                                   NULL_BD_ADDR;
   BTPM_Message_t                             *ResponseMessage;
   HIDM_Connection_Request_Response_Request_t  ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!(COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)))
      {
         /* All that we really need to do is to build a Connection      */
         /* Request Response message and send it to the server.         */
         BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

         ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         ConnectionRequestResponseRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
         ConnectionRequestResponseRequest.MessageHeader.MessageLength   = HIDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectionRequestResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectionRequestResponseRequest.Accept                        = Accept;
         ConnectionRequestResponseRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
               ret_val = ((HIDM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote HID device.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int _HIDM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags)
{
   int                                ret_val;
   BD_ADDR_t                          NULL_BD_ADDR;
   BTPM_Message_t                    *ResponseMessage;
   HIDM_Connect_HID_Device_Request_t  ConnectHIDDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!(COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)))
      {
         /* All that we really need to do is to build a Connect HID     */
         /* Device message and send it to the server.                   */
         BTPS_MemInitialize(&ConnectHIDDeviceRequest, 0, sizeof(ConnectHIDDeviceRequest));

         ConnectHIDDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectHIDDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectHIDDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         ConnectHIDDeviceRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_CONNECT_HID_DEVICE;
         ConnectHIDDeviceRequest.MessageHeader.MessageLength   = HIDM_CONNECT_HID_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectHIDDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectHIDDeviceRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectHIDDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_CONNECT_HID_DEVICE_RESPONSE_SIZE)
               ret_val = ((HIDM_Connect_HID_Device_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

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
   int                                   ret_val;
   BD_ADDR_t                             NULL_BD_ADDR;
   BTPM_Message_t                       *ResponseMessage;
   HIDM_Disconnect_HID_Device_Request_t  DisconnectHIDDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!(COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)))
      {
         /* All that we really need to do is to build a Disconnect HID  */
         /* Device message and send it to the server.                   */
         BTPS_MemInitialize(&DisconnectHIDDeviceRequest, 0, sizeof(DisconnectHIDDeviceRequest));

         DisconnectHIDDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisconnectHIDDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisconnectHIDDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         DisconnectHIDDeviceRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_DISCONNECT_HID_DEVICE;
         DisconnectHIDDeviceRequest.MessageHeader.MessageLength   = HIDM_DISCONNECT_HID_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisconnectHIDDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         DisconnectHIDDeviceRequest.SendVirtualCableDisconnect    = SendVirtualCableDisconnect;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectHIDDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_DISCONNECT_HID_DEVICE_RESPONSE_SIZE)
               ret_val = ((HIDM_Disconnect_HID_Device_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected HID     */
   /* Devices.  This function accepts a pointer to a buffer that will   */
   /* receive any currently connected HID devices.  The first parameter */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated with   */
   /* the total number of connected devices if the function is          */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns a  */
   /* non-negative value if successful which represents the number of   */
   /* connected devices that were copied into the specified input       */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _HIDM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HIDM_Query_Connected_HID_Devices_Request_t  QueryConnectedHIDDevices;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || ((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)))
      {
         /* All that we really need to do is to build a Query Connected */
         /* HID Devices message and send it to the server.              */
         QueryConnectedHIDDevices.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedHIDDevices.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedHIDDevices.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         QueryConnectedHIDDevices.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HID_DEVICES;
         QueryConnectedHIDDevices.MessageHeader.MessageLength   = HIDM_QUERY_CONNECTED_HID_DEVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedHIDDevices, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_QUERY_CONNECTED_HID_DEVICES_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_QUERY_CONNECTED_HID_DEVICES_RESPONSE_SIZE(((HIDM_Query_Connected_HID_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)))
            {
               if(!(ret_val = ((HIDM_Query_Connected_HID_Devices_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = ((HIDM_Query_Connected_HID_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                  /* If the caller specified a buffer to receive the    */
                  /* list into, go ahead and copy it over.              */
                  if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
                  {
                     /* If there are more entries in the returned list  */
                     /* than the buffer specified, we need to truncate  */
                     /* it.                                             */
                     if(MaximumRemoteDeviceListEntries >= ((HIDM_Query_Connected_HID_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)
                        MaximumRemoteDeviceListEntries = ((HIDM_Query_Connected_HID_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                     BTPS_MemCopy(RemoteDeviceAddressList, ((HIDM_Query_Connected_HID_Devices_Response_t *)ResponseMessage)->DeviceConnectedList, MaximumRemoteDeviceListEntries*sizeof(BD_ADDR_t));

                     /* Flag how many devices that we returned.         */
                     ret_val = (int)MaximumRemoteDeviceListEntries;
                  }
                  else
                     ret_val = 0;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int _HIDM_Change_Incoming_Connection_Flags(unsigned int ConnectionFlags)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HIDM_Change_Incoming_Connection_Flags_Request_t  ChangeIncomingConnectionFlagsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Change Incoming    */
      /* Connection Flags message and send it to the server.            */
      ChangeIncomingConnectionFlagsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageLength   = HIDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ChangeIncomingConnectionFlagsRequest.ConnectionFlags               = ConnectionFlags;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeIncomingConnectionFlagsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE)
            ret_val = ((HIDM_Change_Incoming_Connection_Flags_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

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
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HIDM_Set_Keyboard_Repeat_Rate_Request_t  SetKeyboardRepeatRateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Set Keyboard Repeat*/
      /* Rate message and send it to the server.                        */
      SetKeyboardRepeatRateRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetKeyboardRepeatRateRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetKeyboardRepeatRateRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      SetKeyboardRepeatRateRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SET_KEYBOARD_REPEAT_RATE;
      SetKeyboardRepeatRateRequest.MessageHeader.MessageLength   = HIDM_SET_KEYBOARD_REPEAT_RATE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetKeyboardRepeatRateRequest.RepeatDelay                   = RepeatDelay;
      SetKeyboardRepeatRateRequest.RepeatRate                    = RepeatRate;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetKeyboardRepeatRateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_SET_KEYBOARD_REPEAT_RATE_RESPONSE_SIZE)
            ret_val = ((HIDM_Set_Keyboard_Repeat_Rate_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* HID Report Data to a currently connected remote device.  This     */
   /* function accepts as input the HID Manager Data Handler ID         */
   /* (registered via call to the HIDM_Register_Data_Event_Callback()   */
   /* function), followed by the remote device address of the remote HID*/
   /* device to send the report data to, followed by the report data    */
   /* itself.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _HIDM_Send_Report_Data(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                              ret_val;
   BD_ADDR_t                        NULL_BD_ADDR;
   HIDM_Send_Report_Data_Request_t *SendReportDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((ReportDataLength) && (ReportData) && (!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)))
      {
         if((SendReportDataRequest = (HIDM_Send_Report_Data_Request_t *)BTPS_AllocateMemory(HIDM_SEND_REPORT_DATA_REQUEST_SIZE(ReportDataLength))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendReportDataRequest, 0, HIDM_SEND_REPORT_DATA_REQUEST_SIZE(0));

            SendReportDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendReportDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendReportDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
            SendReportDataRequest->MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SEND_REPORT_DATA;
            SendReportDataRequest->MessageHeader.MessageLength   = HIDM_SEND_REPORT_DATA_REQUEST_SIZE(ReportDataLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendReportDataRequest->HIDDataEventsHandlerID        = HIDManagerDataCallbackID;
            SendReportDataRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SendReportDataRequest->ReportLength                  = ReportDataLength;

            BTPS_MemCopy(SendReportDataRequest->ReportData, ReportData, ReportDataLength);

            /* Message has been formatted, go ahead and send it off.    */
            /* * NOTE * There is NO Response for this message, so we    */
            /*          are not going to wait for one.                  */
            ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendReportDataRequest, 0, NULL);

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendReportDataRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as input */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report requested.  The    */
   /* fourth parameter is the Report ID determined by the Device's SDP  */
   /* record.  Passing HIDM_INVALID_REPORT_ID as the value for this     */
   /* parameter will indicate that this parameter is not used and will  */
   /* exclude the appropriate byte from the transaction payload.  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Get_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID)
{
   int                                     ret_val;
   BD_ADDR_t                               NULL_BD_ADDR;
   BTPM_Message_t                         *ResponseMessage;
   HIDM_Send_Get_Report_Request_Request_t  SendGetReportRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* All that we really need to do is to build a Send Get Report */
         /* Request message and send it to the server.                  */
         SendGetReportRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendGetReportRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendGetReportRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         SendGetReportRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SEND_GET_REPORT_REQUEST;
         SendGetReportRequest.MessageHeader.MessageLength   = HIDM_SEND_GET_REPORT_REQUEST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendGetReportRequest.HIDDataEventsHandlerID        = HIDManagerDataCallbackID;
         SendGetReportRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendGetReportRequest.ReportType                    = ReportType;
         SendGetReportRequest.ReportID                      = ReportID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendGetReportRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_SEND_GET_REPORT_REQUEST_RESPONSE_SIZE)
               ret_val = ((HIDM_Send_Get_Report_Request_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input     */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report being sent.  The   */
   /* final two parameters to this function are the Length of the Report*/
   /* Data to send and a pointer to the Report Data that will be sent.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Set_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData)
{
   int                                     ret_val;
   BD_ADDR_t                               NULL_BD_ADDR;
   BTPM_Message_t                         *ResponseMessage;
   HIDM_Send_Set_Report_Request_Request_t *SendSetReportRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((ReportDataLength) && (ReportData) && (!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR)))
      {
         if((SendSetReportRequest = (HIDM_Send_Set_Report_Request_Request_t *)BTPS_AllocateMemory(HIDM_SEND_SET_REPORT_REQUEST_REQUEST_SIZE(ReportDataLength))) != NULL)
         {
            /* Initialize the data fields of the message.               */
            BTPS_MemInitialize(SendSetReportRequest, 0, HIDM_SEND_SET_REPORT_REQUEST_REQUEST_SIZE(0));

            /* All that we really need to do is to build a Send Set     */
            /* Report Request message and send it to the server.        */
            SendSetReportRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendSetReportRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendSetReportRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
            SendSetReportRequest->MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SEND_SET_REPORT_REQUEST;
            SendSetReportRequest->MessageHeader.MessageLength   = HIDM_SEND_SET_REPORT_REQUEST_REQUEST_SIZE(ReportDataLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendSetReportRequest->HIDDataEventsHandlerID        = HIDManagerDataCallbackID;
            SendSetReportRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SendSetReportRequest->ReportType                    = ReportType;
            SendSetReportRequest->ReportDataLength              = ReportDataLength;

            BTPS_MemCopy(SendSetReportRequest->ReportData, ReportData, ReportDataLength);

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendSetReportRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid.  If it is, go ahead and note the      */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_SEND_SET_REPORT_REQUEST_RESPONSE_SIZE)
                  ret_val = ((HIDM_Send_Set_Report_Request_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendSetReportRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Get_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                       ret_val;
   BD_ADDR_t                                 NULL_BD_ADDR;
   BTPM_Message_t                           *ResponseMessage;
   HIDM_Send_Get_Protocol_Request_Request_t  SendGetProtocolRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* All that we really need to do is to build a Send Get        */
         /* Protocol Request message and send it to the server.         */
         SendGetProtocolRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendGetProtocolRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendGetProtocolRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         SendGetProtocolRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_REQUEST;
         SendGetProtocolRequest.MessageHeader.MessageLength   = HIDM_SEND_GET_PROTOCOL_REQUEST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendGetProtocolRequest.HIDDataEventsHandlerID        = HIDManagerDataCallbackID;
         SendGetProtocolRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendGetProtocolRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_SEND_GET_PROTOCOL_REQUEST_RESPONSE_SIZE)
               ret_val = ((HIDM_Send_Get_Protocol_Request_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the protocol to be set.  This     */
   /* function returns a zero if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Set_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol)
{
   int                                       ret_val;
   BD_ADDR_t                                 NULL_BD_ADDR;
   BTPM_Message_t                           *ResponseMessage;
   HIDM_Send_Set_Protocol_Request_Request_t  SendSetProtocolRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* All that we really need to do is to build a Send Set        */
         /* Protocol Request message and send it to the server.         */
         SendSetProtocolRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendSetProtocolRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendSetProtocolRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         SendSetProtocolRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_REQUEST;
         SendSetProtocolRequest.MessageHeader.MessageLength   = HIDM_SEND_SET_PROTOCOL_REQUEST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendSetProtocolRequest.HIDDataEventsHandlerID        = HIDManagerDataCallbackID;
         SendSetProtocolRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendSetProtocolRequest.Protocol                      = Protocol;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendSetProtocolRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_SEND_SET_PROTOCOL_REQUEST_RESPONSE_SIZE)
               ret_val = ((HIDM_Send_Set_Protocol_Request_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Get_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                   ret_val;
   BD_ADDR_t                             NULL_BD_ADDR;
   BTPM_Message_t                       *ResponseMessage;
   HIDM_Send_Get_Idle_Request_Request_t  SendGetIdleRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* All that we really need to do is to build a Send Get Idle   */
         /* Request message and send it to the server.                  */
         SendGetIdleRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendGetIdleRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendGetIdleRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         SendGetIdleRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SEND_GET_IDLE_REQUEST;
         SendGetIdleRequest.MessageHeader.MessageLength   = HIDM_SEND_GET_IDLE_REQUEST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendGetIdleRequest.HIDDataEventsHandlerID        = HIDManagerDataCallbackID;
         SendGetIdleRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendGetIdleRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_SEND_GET_IDLE_REQUEST_RESPONSE_SIZE)
               ret_val = ((HIDM_Send_Get_Idle_Request_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the Idle Rate to be set.  The Idle*/
   /* Rate LSB is weighted to 4ms (i.e. the Idle Rate resolution is 4ms */
   /* with a range from 4ms to 1.020s).  This function returns a zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Set_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate)
{
   int                                   ret_val;
   BD_ADDR_t                             NULL_BD_ADDR;
   BTPM_Message_t                       *ResponseMessage;
   HIDM_Send_Set_Idle_Request_Request_t  SendSetIdleRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format an invalid BD_ADDR to test against.                        */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* All that we really need to do is to build a Send Set Idle   */
         /* Request message and send it to the server.                  */
         SendSetIdleRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendSetIdleRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendSetIdleRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         SendSetIdleRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SEND_SET_IDLE_REQUEST;
         SendSetIdleRequest.MessageHeader.MessageLength   = HIDM_SEND_SET_IDLE_REQUEST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendSetIdleRequest.HIDDataEventsHandlerID        = HIDManagerDataCallbackID;
         SendSetIdleRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendSetIdleRequest.IdleRate                      = IdleRate;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendSetIdleRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_SEND_SET_IDLE_REQUEST_RESPONSE_SIZE)
               ret_val = ((HIDM_Send_Set_Idle_Request_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the HID Manager      */
   /* Service.  This Callback will be dispatched by the HID Manager when*/
   /* various HID Manager Events occur.  This function returns a        */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the HID    */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _HIDM_Un_Register_HID_Events() function to Un-Register   */
   /*          the Event Handler.                                       */
int _HIDM_Register_HID_Events(void)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   HIDM_Register_HID_Events_Request_t  RegisterHIDEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register HID Events*/
      /* message and send it to the server.                             */
      RegisterHIDEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterHIDEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterHIDEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      RegisterHIDEventsRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS;
      RegisterHIDEventsRequest.MessageHeader.MessageLength   = HIDM_REGISTER_HID_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHIDEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_REGISTER_HID_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((HIDM_Register_HID_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((HIDM_Register_HID_Events_Response_t *)ResponseMessage)->HIDEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _HIDM_Register_HID_Events() function).  This function accepts     */
   /* input the HID Event Handler ID (return value from                 */
   /* _HIDM_Register_HID_Events() function).                            */
int _HIDM_Un_Register_HID_Events(unsigned int HIDEventsHandlerID)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   HIDM_Un_Register_HID_Events_Request_t  UnRegisterHIDEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register HID   */
      /* Events message and send it to the server.                      */
      UnRegisterHIDEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterHIDEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterHIDEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      UnRegisterHIDEventsRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS;
      UnRegisterHIDEventsRequest.MessageHeader.MessageLength   = HIDM_UN_REGISTER_HID_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterHIDEventsRequest.HIDEventsHandlerID            = HIDEventsHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHIDEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_UN_REGISTER_HID_EVENTS_RESPONSE_SIZE)
            ret_val = ((HIDM_Un_Register_HID_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the HID       */
   /* Manager Service to explicitly process HID Data.  This Callback    */
   /* will be dispatched by the HID Manager when various HID Manager    */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HIDM_Send_Report_Data() function to send data).         */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          in the system.                                           */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HIDM_Un_Register_HID_Data_Events() function to          */
   /*          un-register the callback from this module.               */
int _HIDM_Register_HID_Data_Events(void)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HIDM_Register_HID_Data_Events_Request_t  RegisterHIDDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register HID Data  */
      /* Events message and send it to the server.                      */
      RegisterHIDDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterHIDDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterHIDDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      RegisterHIDDataEventsRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_REGISTER_HID_DATA;
      RegisterHIDDataEventsRequest.MessageHeader.MessageLength   = HIDM_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHIDDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((HIDM_Register_HID_Data_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((HIDM_Register_HID_Data_Events_Response_t *)ResponseMessage)->HIDDataEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Data Event        */
   /* Callback (registered via a successful call to the                 */
   /* _HIDM_Register_HID_Data_Events() function).  This function accepts*/
   /* as input the HID Manager Event Callback ID (return value from     */
   /* _HIDM_Register_HID_Data_Events() function).                       */
int _HIDM_Un_Register_HID_Data_Events(unsigned int HIDManagerDataCallbackID)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HIDM_Un_Register_HID_Data_Events_Request_t  UnRegisterHIDDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register HID   */
      /* Data Events message and send it to the server.                 */
      UnRegisterHIDDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA;
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageLength   = HIDM_UN_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterHIDDataEventsRequest.HIDDataEventsHandlerID        = HIDManagerDataCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHIDDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HIDM_UN_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE)
            ret_val = ((HIDM_Un_Register_HID_Data_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

