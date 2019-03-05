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
   /* initialize the PAN Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAN Manager  */
   /* Implementation.                                                   */
int _PANM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAN Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the PAN   */
   /* Manager Implementation.  After this function is called the PAN    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PANM_Initialize() function.  */
void _PANM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming PAN connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A PAN Connection  */
   /*          Indication event will be dispatched to signify the actual*/
   /*          result.                                                  */
int _PANM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   PANM_Connection_Request_Response_Request_t  ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!(COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Connection      */
         /* Request Response message and send it to the server.         */
         BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

         ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
         ConnectionRequestResponseRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
         ConnectionRequestResponseRequest.MessageHeader.MessageLength   = (PANM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         ConnectionRequestResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectionRequestResponseRequest.Accept                        = AcceptConnection;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
               ret_val = ((PANM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to open a remote PAN server port. This function returns   */
   /* zero if successful and a negative value if there was an error.    */
int _PANM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, unsigned long ConnectionFlags)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   PANM_Connect_Remote_Device_Request_t  ConnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!(COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build an Open message   */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&ConnectRemoteDeviceRequest, 0, sizeof(ConnectRemoteDeviceRequest));

         ConnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
         ConnectRemoteDeviceRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE;
         ConnectRemoteDeviceRequest.MessageHeader.MessageLength   = (PANM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         ConnectRemoteDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectRemoteDeviceRequest.LocalServiceType              = LocalServiceType;
         ConnectRemoteDeviceRequest.RemoteServiceType             = RemoteServiceType;
         ConnectRemoteDeviceRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
               ret_val = ((PANM_Connect_Remote_Device_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to close a previously opened PAN Connection. This function */
   /* returns zero if successful, or a negative value if there was an   */
   /* error                                                             */
int _PANM_Close_Connection(BD_ADDR_t RemoteDeviceAddress)
{
   int                              ret_val;
   BTPM_Message_t                  *ResponseMessage;
   PANM_Close_Connection_Request_t  CloseConnectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!(COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Close Connection*/
         /* Message and send it to the server.                          */
         BTPS_MemInitialize(&CloseConnectionRequest, 0, sizeof(CloseConnectionRequest));

         CloseConnectionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         CloseConnectionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         CloseConnectionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
         CloseConnectionRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_CLOSE_CONNECTION;
         CloseConnectionRequest.MessageHeader.MessageLength   = (PANM_CLOSE_CONNECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         CloseConnectionRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&CloseConnectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_CLOSE_CONNECTION_RESPONSE_SIZE)
               ret_val = ((PANM_Close_Connection_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Personal*/
   /* Area Networking devices.  This function accepts the buffer        */
   /* information to receive any currently connected devices.  The first*/
   /* parameter specifies the maximum number of BD_ADDR entries that the*/
   /* buffer will support (i.e. can be copied into the buffer).  The    */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of connected devices if the function is     */
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
int _PANM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   PANM_Query_Connected_Devices_Request_t QueryConnectedDevicesRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || (TotalNumberConnectedDevices))
      {
         /* All that we really need to do is to build a Query Connected */
         /* Devices Message and send it to the server.                  */
         BTPS_MemInitialize(&QueryConnectedDevicesRequest, 0, sizeof(QueryConnectedDevicesRequest));

         QueryConnectedDevicesRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedDevicesRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedDevicesRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
         QueryConnectedDevicesRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES;
         QueryConnectedDevicesRequest.MessageHeader.MessageLength   = (PANM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedDevicesRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(((PANM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)))
            {
               if(!(ret_val = ((PANM_Query_Connected_Devices_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = ((PANM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                  /* If the caller specified a buffer to receive the    */
                  /* list into, go ahead and copy it over.              */
                  if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
                  {
                     /* If there are more entries in the returned list  */
                     /* than the buffer specified, we need to truncate  */
                     /* it.                                             */
                     if(MaximumRemoteDeviceListEntries >= ((PANM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)
                        MaximumRemoteDeviceListEntries = ((PANM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                     BTPS_MemCopy(RemoteDeviceAddressList, ((PANM_Query_Connected_Devices_Response_t *)ResponseMessage)->DeviceConnectedList, MaximumRemoteDeviceListEntries*sizeof(BD_ADDR_t));

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
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for the Personal Area  */
   /* Networking (PAN) Manager.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
int _PANM_Query_Current_Configuration(PANM_Current_Configuration_t *CurrentConfiguration)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   PANM_Query_Current_Configuration_Request_t QueryCurrentConfigurationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(CurrentConfiguration)
      {
         /* All that we really need to do is to build a Query Current   */
         /* Configuration Message and send it to the server.            */
         BTPS_MemInitialize(&QueryCurrentConfigurationRequest, 0, sizeof(QueryCurrentConfigurationRequest));

         QueryCurrentConfigurationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryCurrentConfigurationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryCurrentConfigurationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
         QueryCurrentConfigurationRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION;
         QueryCurrentConfigurationRequest.MessageHeader.MessageLength   = (PANM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryCurrentConfigurationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE)
            {
               if(!(ret_val = ((PANM_Query_Current_Configuration_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  CurrentConfiguration->ServiceTypeFlags        = ((PANM_Query_Current_Configuration_Response_t *)ResponseMessage)->ServiceTypeFlags;
                  CurrentConfiguration->IncomingConnectionFlags = ((PANM_Query_Current_Configuration_Response_t *)ResponseMessage)->IncomingConnectionFlags;
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
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the flags that control how incoming connections */
   /* are handled. This function returns zero if successful and a       */
   /* negative value if there was an error.                             */
int _PANM_Change_Incoming_Connection_Flags(unsigned int ConnectionFlags)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   PANM_Change_Incoming_Connection_Flags_Request_t  ChangeIncomingConnectionFlagsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Change Incoming    */
      /* Connection Flags message and send it to the server.            */
      BTPS_MemInitialize(&ChangeIncomingConnectionFlagsRequest, 0, sizeof(ChangeIncomingConnectionFlagsRequest));

      ChangeIncomingConnectionFlagsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageLength   = (PANM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      ChangeIncomingConnectionFlagsRequest.ConnectionFlags               = ConnectionFlags;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeIncomingConnectionFlagsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE)
            ret_val = ((PANM_Change_Incoming_Connection_Flags_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to register with the PAN Manager Server to receive PAN    */
   /* Event Notifications. This function returns a positive non-zero    */
   /* value if successful, or a negative value if there was an error.   */
   /* * NOTE * The value this function returns (if successful) can be   */
   /*          passed to the _PANM_Un_Register_Events() function to     */
   /*          un-register for events.                                  */
int _PANM_Register_Events(void)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   PANM_Register_Events_Request_t  RegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register Events    */
      /* message and send it to the server.                             */
      BTPS_MemInitialize(&RegisterEventsRequest, 0, sizeof(RegisterEventsRequest));

      RegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
      RegisterEventsRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_REGISTER_EVENTS;
      RegisterEventsRequest.MessageHeader.MessageLength   = (PANM_REGISTER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_REGISTER_EVENTS_RESPONSE_SIZE)
         {
            if((ret_val = ((PANM_Register_Events_Response_t *)ResponseMessage)->Status) == 0)
               ret_val = ((PANM_Register_Events_Response_t *)ResponseMessage)->EventsHandlerID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to un-register from the server for receiving events.  This*/
   /* function accepts as input the EventCallbackID returned from       */
   /* _PANM_Register_Events() function.                                 */
int _PANM_Un_Register_Events(unsigned int PANEventsHandlerID)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   PANM_Un_Register_Events_Request_t  UnRegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Un-Register Events */
      /* message and send it to the server.                             */
      BTPS_MemInitialize(&UnRegisterEventsRequest, 0, sizeof(UnRegisterEventsRequest));

      UnRegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
      UnRegisterEventsRequest.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS;
      UnRegisterEventsRequest.MessageHeader.MessageLength   = (PANM_UN_REGISTER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      UnRegisterEventsRequest.EventsHandlerID               = PANEventsHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PANM_UN_REGISTER_EVENTS_RESPONSE_SIZE)
            ret_val = ((PANM_Un_Register_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

