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
#include "HDDMMSG.h"             /* BTPM HID Manager Message Formats.         */
#include "HDDMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include <string.h>

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
   /* initialize the HDD Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HDD Manager  */
   /* Implementation.                                                   */
int _HDDM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HDD Manager (Imp)\n"));

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

   /* The following function is responsible for shutting down the HDD   */
   /* Manager Implementation.  After this function is called the HDD    */
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
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* from a remote HID Host.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HDD Connected   */
   /*          event will be dispatched to signify the actual result.   */
int _HDDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HDDM_Connection_Request_Response_Request_t  ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

      ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      ConnectionRequestResponseRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
      ConnectionRequestResponseRequest.MessageHeader.MessageLength   = HDDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConnectionRequestResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      ConnectionRequestResponseRequest.Accept                        = Accept;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
            ret_val = ((HDDM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Connect to a remote HID Host device.  The RemoteDeviceAddress is  */
   /* the Bluetooth Address of the remote HID Host.  The ConnectionFlags*/
   /* specifiy whay security, if any, is required for the connection.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HDDM_Connect_Remote_Host(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   HDDM_Connect_Remote_Host_Request_t  ConnectRemoteHostRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ConnectRemoteHostRequest, 0, sizeof(ConnectRemoteHostRequest));

      ConnectRemoteHostRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConnectRemoteHostRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConnectRemoteHostRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      ConnectRemoteHostRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CONNECT_REMOTE_HOST;
      ConnectRemoteHostRequest.MessageHeader.MessageLength   = HDDM_CONNECT_REMOTE_HOST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConnectRemoteHostRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      ConnectRemoteHostRequest.ConnectionFlags               = ConnectionFlags;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteHostRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_CONNECT_REMOTE_HOST_RESPONSE_SIZE)
            ret_val = ((HDDM_Connect_Remote_Host_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Disconnect from a remote HID Host.  The RemoteDeviceAddress       */
   /* is the Bluetooth Address of the remote HID Host.  The             */
   /* SendVirtualCableUnplug parameter indicates whether the device     */
   /* should be disconnected with a Virtual Cable Unplug (TRUE) or      */
   /* simply at the Bluetooth Link (FALSE).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HDDM_Disconnect(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug)
{
   int                        ret_val;
   BTPM_Message_t            *ResponseMessage;
   HDDM_Disconnect_Request_t  DisconnectRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&DisconnectRequest, 0, sizeof(DisconnectRequest));

      DisconnectRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      DisconnectRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      DisconnectRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      DisconnectRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_DISCONNECT;
      DisconnectRequest.MessageHeader.MessageLength   = HDDM_DISCONNECT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DisconnectRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      DisconnectRequest.SendVirtualCableUnplug        = SendVirtualCableUnplug;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_DISCONNECT_RESPONSE_SIZE)
            ret_val = ((HDDM_Disconnect_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Determine if there are currently any connected HID Hosts.  This   */
   /* function accepts a pointer to a buffer that will receive any      */
   /* currently connected HID Hosts.  The first parameter specifies the */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer).  The next parameter is      */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _HDDM_Query_Connected_Hosts(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HDDM_Query_Connected_Hosts_Request_t  QueryConnectedHosts;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || ((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)))
      {
         /* All that we really need to do is to build a Query Connected */
         /* HID Devices message and send it to the server.              */
         QueryConnectedHosts.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedHosts.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedHosts.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         QueryConnectedHosts.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HOSTS;
         QueryConnectedHosts.MessageHeader.MessageLength   = HDDM_QUERY_CONNECTED_HOSTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryConnectedHosts.MaximumNumberDevices          = MaximumRemoteDeviceListEntries;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedHosts, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_QUERY_CONNECTED_HOSTS_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_QUERY_CONNECTED_HOSTS_RESPONSE_SIZE(((HDDM_Query_Connected_Hosts_Response_t *)ResponseMessage)->NumberDevicesConnected)))
            {
               if(!(ret_val = ((HDDM_Query_Connected_Hosts_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  if(TotalNumberConnectedDevices)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Total Number is Supplied: %u\n", ((HDDM_Query_Connected_Hosts_Response_t *)ResponseMessage)->TotalNumberDevices));
                     *TotalNumberConnectedDevices = ((HDDM_Query_Connected_Hosts_Response_t *)ResponseMessage)->TotalNumberDevices;
                  }
                  else
                     DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Total Number not supplied\n"));


                  /* If the caller specified a buffer to receive the    */
                  /* list into, go ahead and copy it over.              */
                  if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
                  {
                     /* The PM server will only return the number of    */
                     /* requested devices.                              */
                     BTPS_MemCopy(RemoteDeviceAddressList, ((HDDM_Query_Connected_Hosts_Response_t *)ResponseMessage)->DeviceConnectedList, ((HDDM_Query_Connected_Hosts_Response_t *)ResponseMessage)->NumberDevicesConnected*sizeof(BD_ADDR_t));

                     /* Flag how many devices that we returned.         */
                     ret_val = (int)((HDDM_Query_Connected_Hosts_Response_t *)ResponseMessage)->NumberDevicesConnected;
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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int _HDDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HDDM_Change_Incoming_Connection_Flags_Request_t  ChangeIncomingConnectionFlagsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ChangeIncomingConnectionFlagsRequest, 0, sizeof(ChangeIncomingConnectionFlagsRequest));

      ChangeIncomingConnectionFlagsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageLength   = HDDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ChangeIncomingConnectionFlagsRequest.ConnectionFlags               = ConnectionFlags;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeIncomingConnectionFlagsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE)
            ret_val = ((HDDM_Change_Incoming_Connection_Flags_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Send the specified HID Report Data to a currently connected       */
   /* remote device.  This function accepts as input the HDD            */
   /* Manager Report Data Handler ID (registered via call to the        */
   /* HDDM_Register_Data_Event_Callback() function), followed by the    */
   /* remote device address of the remote HID Host to send the report   */
   /* data to, followed by the report data itself.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _HDDM_Send_Report_Data(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                              ret_val;
   HDDM_Send_Report_Data_Request_t *SendReportDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u, %p\n", ReportDataLength, ReportData));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((ReportDataLength) && (ReportData) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         if((SendReportDataRequest = (HDDM_Send_Report_Data_Request_t *)BTPS_AllocateMemory(HDDM_SEND_REPORT_DATA_REQUEST_SIZE(ReportDataLength))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendReportDataRequest, 0, HDDM_SEND_REPORT_DATA_REQUEST_SIZE(ReportDataLength));

            SendReportDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendReportDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendReportDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
            SendReportDataRequest->MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SEND_REPORT_DATA;
            SendReportDataRequest->MessageHeader.MessageLength   = HDDM_SEND_REPORT_DATA_REQUEST_SIZE(ReportDataLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendReportDataRequest->DataCallbackID                = HDDManagerDataCallbackID;
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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a GetReportRequest.  The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* ReportType indicates the type of report being sent as the         */
   /* response.  The ReportDataLength indicates the size of the report  */
   /* data.  ReportData is a pointer to the report data buffer.  This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Get_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   HDDM_Get_Report_Response_Request_t *GetReportResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((ReportDataLength) && (ReportData) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         if((GetReportResponseRequest = (HDDM_Get_Report_Response_Request_t *)BTPS_AllocateMemory(HDDM_GET_REPORT_RESPONSE_REQUEST_SIZE(ReportDataLength))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(GetReportResponseRequest, 0, HDDM_GET_REPORT_RESPONSE_REQUEST_SIZE(ReportDataLength));

            GetReportResponseRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            GetReportResponseRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            GetReportResponseRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
            GetReportResponseRequest->MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SEND_GET_REPORT_RESPONSE;
            GetReportResponseRequest->MessageHeader.MessageLength   = HDDM_GET_REPORT_RESPONSE_REQUEST_SIZE(ReportDataLength) - BTPM_MESSAGE_HEADER_SIZE;

            GetReportResponseRequest->DataCallbackID                = HDDManagerDataCallbackID;
            GetReportResponseRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            GetReportResponseRequest->Result                        = Result;
            GetReportResponseRequest->ReportType                    = ReportType;
            GetReportResponseRequest->ReportLength                  = ReportDataLength;

            BTPS_MemCopy(GetReportResponseRequest->ReportData, ReportData, ReportDataLength);

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)GetReportResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid.  If it is, go ahead and note the      */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_GET_REPORT_RESPONSE_RESPONSE_SIZE)
                  ret_val = ((HDDM_Get_Report_Response_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(GetReportResponseRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Responsd to a SetReportRequest. The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Set_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   HDDM_Set_Report_Response_Request_t  SetReportResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetReportResponseRequest, 0, sizeof(SetReportResponseRequest));

      SetReportResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetReportResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetReportResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      SetReportResponseRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SEND_SET_REPORT_RESPONSE;
      SetReportResponseRequest.MessageHeader.MessageLength   = HDDM_SET_REPORT_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetReportResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      SetReportResponseRequest.DataCallbackID                = HDDManagerDataCallbackID;
      SetReportResponseRequest.Result                        = Result;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetReportResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_SET_REPORT_RESPONSE_RESPONSE_SIZE)
            ret_val = ((HDDM_Set_Report_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a GetProtocolRequest.  The HDDManagerDataCallback      */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* Protocol indicates the current HID Protocol.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _HDDM_Get_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HDDM_Get_Protocol_Response_Request_t  GetProtocolResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&GetProtocolResponseRequest, 0, sizeof(GetProtocolResponseRequest));

      GetProtocolResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetProtocolResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetProtocolResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      GetProtocolResponseRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_RESPONSE;
      GetProtocolResponseRequest.MessageHeader.MessageLength   = HDDM_GET_PROTOCOL_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetProtocolResponseRequest.DataCallbackID                = HDDManagerDataCallbackID;
      GetProtocolResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      GetProtocolResponseRequest.Result                        = Result;
      GetProtocolResponseRequest.Protocol                      = Protocol;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetProtocolResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_GET_PROTOCOL_RESPONSE_RESPONSE_SIZE)
            ret_val = ((HDDM_Get_Protocol_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a SetProtocolResponse.  The HDDManagerDataCallback     */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Set_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HDDM_Set_Protocol_Response_Request_t  SetProtocolResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetProtocolResponseRequest, 0, sizeof(SetProtocolResponseRequest));

      SetProtocolResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetProtocolResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetProtocolResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      SetProtocolResponseRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_RESPONSE;
      SetProtocolResponseRequest.MessageHeader.MessageLength   = HDDM_SET_PROTOCOL_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetProtocolResponseRequest.DataCallbackID                = HDDManagerDataCallbackID;
      SetProtocolResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      SetProtocolResponseRequest.Result                        = Result;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetProtocolResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_SET_PROTOCOL_RESPONSE_RESPONSE_SIZE)
            ret_val = ((HDDM_Set_Protocol_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a GetIdleResponse.  The HDDManagerDataCallback         */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* IdleRate is the current Idle Rate.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
int _HDDM_Get_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   HDDM_Get_Idle_Response_Request_t  GetIdleResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&GetIdleResponseRequest, 0, sizeof(GetIdleResponseRequest));

      GetIdleResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetIdleResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetIdleResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      GetIdleResponseRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SEND_GET_IDLE_RESPONSE;
      GetIdleResponseRequest.MessageHeader.MessageLength   = HDDM_GET_IDLE_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetIdleResponseRequest.DataCallbackID                = HDDManagerDataCallbackID;
      GetIdleResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      GetIdleResponseRequest.Result                        = Result;
      GetIdleResponseRequest.IdleRate                      = IdleRate;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetIdleResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_GET_IDLE_RESPONSE_RESPONSE_SIZE)
            ret_val = ((HDDM_Get_Idle_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to a SetIdleRequest.  The HDDManagerDataCallback          */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Set_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   HDDM_Set_Idle_Response_Request_t  SetIdleResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetIdleResponseRequest, 0, sizeof(SetIdleResponseRequest));

      SetIdleResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetIdleResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetIdleResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      SetIdleResponseRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SEND_SET_IDLE_RESPONSE;
      SetIdleResponseRequest.MessageHeader.MessageLength   = HDDM_SET_IDLE_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetIdleResponseRequest.DataCallbackID                = HDDManagerDataCallbackID;
      SetIdleResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      SetIdleResponseRequest.Result                        = Result;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetIdleResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_SET_IDLE_RESPONSE_RESPONSE_SIZE)
            ret_val = ((HDDM_Set_Idle_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service.  This Callback will be dispatched by*/
   /* the HID Manager when various HID Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HID Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          HDDM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int _HDDM_Register_Event_Callback()
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   HDDM_Register_Events_Request_t  RegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&RegisterEventsRequest, 0, sizeof(RegisterEventsRequest));

      RegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      RegisterEventsRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_REGISTER_EVENTS;
      RegisterEventsRequest.MessageHeader.MessageLength   = HDDM_REGISTER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_REGISTER_EVENTS_RESPONSE_SIZE)
            ret_val = ((HDDM_Register_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_RegisterEventCallback() function).  This function accepts as */
   /* input the HID Manager Event Callback ID (return value from        */
   /* HDDM_RegisterEventCallback() function).                           */
int _HDDM_Un_Register_Event_Callback(unsigned int HDDManagerCallbackID)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   HDDM_Un_Register_Events_Request_t  UnRegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&UnRegisterEventsRequest, 0, sizeof(UnRegisterEventsRequest));

      UnRegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      UnRegisterEventsRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS;
      UnRegisterEventsRequest.MessageHeader.MessageLength   = HDDM_UN_REGISTER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterEventsRequest.CallbackID                    = HDDManagerCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_UN_REGISTER_EVENTS_RESPONSE_SIZE)
            ret_val = ((HDDM_Un_Register_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service to explicitly process HID report     */
   /* data.  This Callback will be dispatched by the HID Manager when   */
   /* various HID Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HID Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HDDM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int _HDDM_Register_Data_Event_Callback()
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   HDDM_Register_Data_Events_Request_t  RegisterDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&RegisterDataEventsRequest, 0, sizeof(RegisterDataEventsRequest));

      RegisterDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      RegisterDataEventsRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_REGISTER_DATA_EVENTS;
      RegisterDataEventsRequest.MessageHeader.MessageLength   = HDDM_REGISTER_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_REGISTER_DATA_EVENTS_RESPONSE_SIZE)
            ret_val = ((HDDM_Register_Data_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from HDDM_Register_Data_Event_Callback() function).         */
int _HDDM_Un_Register_Data_Event_Callback(unsigned int HDDManagerDataCallbackID)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HDDM_Un_Register_Data_Events_Request_t  UnRegisterDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&UnRegisterDataEventsRequest, 0, sizeof(UnRegisterDataEventsRequest));

      UnRegisterDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
      UnRegisterDataEventsRequest.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_UN_REGISTER_DATA_EVENTS;
      UnRegisterDataEventsRequest.MessageHeader.MessageLength   = HDDM_UN_REGISTER_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterDataEventsRequest.CallbackID = HDDManagerDataCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDDM_UN_REGISTER_DATA_EVENTS_RESPONSE_SIZE)
            ret_val = ((HDDM_Un_Register_Data_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
