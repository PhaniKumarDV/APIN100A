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
   /* initialize the Headset Manager implementation.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Headset Manager Implementation.                                   */
int _HDSM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Headset Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
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
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened.  An */
   /*          hetHDSConnected event will notify of this status.        */
int _HDSM_Connection_Request_Response(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   HDSM_Connection_Request_Response_Request_t   ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Type: 0x%08X\n", ConnectionType));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Disconnect Audio*/
         /* Stream message and send it to the server.                   */
         BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

         ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         ConnectionRequestResponseRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
         ConnectionRequestResponseRequest.MessageHeader.MessageLength   = HDSM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectionRequestResponseRequest.ConnectionType                = ConnectionType;
         ConnectionRequestResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectionRequestResponseRequest.Accept                        = AcceptConnection;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
               ret_val = ((HDSM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Headset/Audio Gateway device.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function accepts the connection */
   /* type to make as the first parameter.  This parameter specifies the*/
   /* LOCAL connection type (i.e.  if the caller would like to connect  */
   /* the local Headset service to a remote Audio Gateway device, the   */
   /* Headset connection type would be specified for this parameter).   */
   /* This function also accepts the connection information for the     */
   /* remote device (address and server port).  This function accepts   */
   /* the connection flags to apply to control how the connection is    */
   /* made regarding encryption and/or authentication.                  */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Headset Manager Connection Status Event (if specified).  */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHDSConnectionStatus event will be dispatched to denote*/
   /*          the status of the connection.  This is the ONLY way to   */
   /*          receive this event, as an event callack registered with  */
   /*          the HDSM_Register_Event_Callback() will NOT receive      */
   /*          connection status events.                                */
int _HDSM_Connect_Remote_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HDSM_Connect_Remote_Device_Request_t  ConnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort))
      {
         /* All that we really need to do is to build a Connect Audio   */
         /* Stream message and send it to the server.                   */
         BTPS_MemInitialize(&ConnectRemoteDeviceRequest, 0, sizeof(ConnectRemoteDeviceRequest));

         ConnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         ConnectRemoteDeviceRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE;
         ConnectRemoteDeviceRequest.MessageHeader.MessageLength   = HDSM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectRemoteDeviceRequest.ConnectionType                = ConnectionType;
         ConnectRemoteDeviceRequest.ConnectionFlags               = ConnectionFlags;
         ConnectRemoteDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectRemoteDeviceRequest.RemoteServerPort              = RemoteServerPort;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
               ret_val = ((HDSM_Connect_Remote_Device_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
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
int _HDSM_Disconnect_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   HDSM_Disconnect_Device_Request_t  DisconnectDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Disconnect      */
         /* Device message and send it to the server.                   */
         BTPS_MemInitialize(&DisconnectDeviceRequest, 0, sizeof(DisconnectDeviceRequest));

         DisconnectDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisconnectDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisconnectDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         DisconnectDeviceRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_DISCONNECT_DEVICE;
         DisconnectDeviceRequest.MessageHeader.MessageLength   = HDSM_DISCONNECT_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisconnectDeviceRequest.ConnectionType                = ConnectionType;
         DisconnectDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_DISCONNECT_DEVICE_RESPONSE_SIZE)
               ret_val = ((HDSM_Disconnect_Device_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Headset */
   /* or Audio Gateway Devices (specified by the first parameter).  This*/
   /* function accepts a the local service type to query, followed by   */
   /* buffer information to receive any currently connected device      */
   /* addresses of the specified connection type.  The first parameter  */
   /* specifies the local service type to query the connection          */
   /* information for.  The second parameter specifies the maximum      */
   /* number of BD_ADDR entries that the buffer will support (i.e.  can */
   /* be copied into the buffer).  The next parameter is optional and,  */
   /* if specified, will be populated with the total number of connected*/
   /* devices if the function is successful.  The final parameter can be*/
   /* used to retrieve the total number of connected devices (regardless*/
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of connected devices that were copied into  */
   /* the specified input buffer.  This function returns a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _HDSM_Query_Connected_Devices(HDSM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   HDSM_Query_Connected_Devices_Request_t  QueryConnectedDevices;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || ((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices))))
      {
         /* All that we really need to do is to build a Query Connected */
         /* Devices message and send it to the server.                  */
         QueryConnectedDevices.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedDevices.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedDevices.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         QueryConnectedDevices.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES;
         QueryConnectedDevices.MessageHeader.MessageLength   = HDSM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryConnectedDevices.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedDevices, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(((HDSM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)))
            {
               if(!(ret_val = ((HDSM_Query_Connected_Devices_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = ((HDSM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                  /* If the caller specified a buffer to receive the    */
                  /* list into, go ahead and copy it over.              */
                  if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
                  {
                     /* If there are more entries in the returned list  */
                     /* than the buffer specified, we need to truncate  */
                     /* it.                                             */
                     if(MaximumRemoteDeviceListEntries >= ((HDSM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)
                        MaximumRemoteDeviceListEntries = ((HDSM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                     BTPS_MemCopy(RemoteDeviceAddressList, ((HDSM_Query_Connected_Devices_Response_t *)ResponseMessage)->DeviceConnectedList, MaximumRemoteDeviceListEntries*sizeof(BD_ADDR_t));

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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Headset or Audio   */
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * On input the TotalNumberAdditionalIndicators member of   */
   /*          the structure should be set to the total number of       */
   /*          additional indicator entry structures that the           */
   /*          AdditionalIndicatorList member points to.  On return from*/
   /*          this function this structure member holds the total      */
   /*          number of additional indicator entries that are supported*/
   /*          by the connection.  The NumberAdditionalIndicators member*/
   /*          will hold (on return) the number of indicator entries    */
   /*          that are actually present in the list.                   */
   /* * NOTE * It is possible to not query the additional indicators by */
   /*          passing zero for the TotalNumberAdditionalIndicators     */
   /*          member.  This member will still hold the total of        */
   /*          supported indicators on return of this function.         */
int _HDSM_Query_Current_Configuration(HDSM_Connection_Type_t ConnectionType, HDSM_Current_Configuration_t *CurrentConfiguration)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HDSM_Query_Current_Configuration_Request_t  QueryCurrentConfigurationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (CurrentConfiguration))
      {
         /* All that we really need to do is to build a Query Connected */
         /* Devices message and send it to the server.                  */
         QueryCurrentConfigurationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryCurrentConfigurationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryCurrentConfigurationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         QueryCurrentConfigurationRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION;
         QueryCurrentConfigurationRequest.MessageHeader.MessageLength   = HDSM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryCurrentConfigurationRequest.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryCurrentConfigurationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE)
            {
               if(!(ret_val = ((HDSM_Query_Current_Configuration_Response_t *)ResponseMessage)->Status))
               {
                  /* Copy the fields in the response.                   */
                  CurrentConfiguration->IncomingConnectionFlags = ((HDSM_Query_Current_Configuration_Response_t *)ResponseMessage)->IncomingConnectionFlags;
                  CurrentConfiguration->SupportedFeaturesMask   = ((HDSM_Query_Current_Configuration_Response_t *)ResponseMessage)->SupportedFeaturesMask;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Headset and   */
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _HDSM_Change_Incoming_Connection_Flags(HDSM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HDSM_Change_Incoming_Connection_Flags_Request_t  ChangeIncomingConnectionFlagsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway))
      {
         /* All that we really need to do is to build a Start Audio     */
         /* Stream message and send it to the server.                   */
         ChangeIncomingConnectionFlagsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS;
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageLength   = HDSM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ChangeIncomingConnectionFlagsRequest.ConnectionType                = ConnectionType;
         ChangeIncomingConnectionFlagsRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeIncomingConnectionFlagsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE)
               ret_val = ((HDSM_Change_Incoming_Connection_Flags_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  When called by a     */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current speaker gain value.  When     */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the speaker gain of the remote Headset   */
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the speaker gain to be sent to the remote device. */
   /* The speaker gain Parameter *MUST* be between the values:          */
   /*                                                                   */
   /*    HDSET_SPEAKER_GAIN_MINIMUM                                     */
   /*    HDSET_SPEAKER_GAIN_MAXIMUM                                     */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Set_Remote_Speaker_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain)
{
   int                               ret_val;
   BTPM_Message_t                  *ResponseMessage;
   HDSM_Set_Speaker_Gain_Request_t  SetSpeakerGainRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HeadsetManagerEventCallbackID) && ((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (SpeakerGain >= HDSET_SPEAKER_GAIN_MINIMUM) && (SpeakerGain <= HDSET_SPEAKER_GAIN_MAXIMUM))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetSpeakerGainRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetSpeakerGainRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetSpeakerGainRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         SetSpeakerGainRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN;
         SetSpeakerGainRequest.MessageHeader.MessageLength   = HDSM_SET_SPEAKER_GAIN_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetSpeakerGainRequest.ControlEventsHandlerID        = HeadsetManagerEventCallbackID;
         SetSpeakerGainRequest.ConnectionType                = ConnectionType;
         SetSpeakerGainRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetSpeakerGainRequest.SpeakerGain                   = SpeakerGain;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetSpeakerGainRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_SET_SPEAKER_GAIN_RESPONSE_SIZE)
               ret_val = ((HDSM_Set_Speaker_Gain_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  When called by a  */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current microphone gain value.  When  */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the microphone gain of the remote Headset*/
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the microphone gain to be sent to the remote      */
   /* device.  The microphone gain Parameter *MUST* be between the      */
   /* values:                                                           */
   /*                                                                   */
   /*    HDSET_MICROPHONE_GAIN_MINIMUM                                  */
   /*    HDSET_MICROPHONE_GAIN_MAXIMUM                                  */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Set_Remote_Microphone_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   HDSM_Set_Microphone_Gain_Request_t  SetMicrophoneGainRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HeadsetManagerEventCallbackID) && ((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (MicrophoneGain >= HDSET_MICROPHONE_GAIN_MINIMUM) && (MicrophoneGain <= HDSET_MICROPHONE_GAIN_MAXIMUM))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetMicrophoneGainRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetMicrophoneGainRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetMicrophoneGainRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         SetMicrophoneGainRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN;
         SetMicrophoneGainRequest.MessageHeader.MessageLength   = HDSM_SET_MICROPHONE_GAIN_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetMicrophoneGainRequest.ControlEventsHandlerID        = HeadsetManagerEventCallbackID;
         SetMicrophoneGainRequest.ConnectionType                = ConnectionType;
         SetMicrophoneGainRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetMicrophoneGainRequest.MicrophoneGain                = MicrophoneGain;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetMicrophoneGainRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_SET_MICROPHONE_GAIN_RESPONSE_SIZE)
               ret_val = ((HDSM_Set_Microphone_Gain_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a button press to a      */
   /* remote Audio Gateway to.  This function return zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Send_Button_Press(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   HDSM_Send_Button_Press_Request_t  SendButtonPressRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SendButtonPressRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendButtonPressRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendButtonPressRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         SendButtonPressRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_SEND_BUTTON_PRESS;
         SendButtonPressRequest.MessageHeader.MessageLength   = HDSM_SEND_BUTTON_PRESS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendButtonPressRequest.ControlEventsHandlerID        = HeadsetManagerEventCallbackID;
         SendButtonPressRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendButtonPressRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_SEND_BUTTON_PRESS_RESPONSE_SIZE)
               ret_val = ((HDSM_Send_Button_Press_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a ring indication to a   */
   /* remote Headset unit.  This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Ring_Indication(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   HDSM_Ring_Indication_Request_t  RingIndicationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HeadsetManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         RingIndicationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         RingIndicationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         RingIndicationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         RingIndicationRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_RING_INDICATION;
         RingIndicationRequest.MessageHeader.MessageLength   = HDSM_RING_INDICATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         RingIndicationRequest.ControlEventsHandlerID        = HeadsetManagerEventCallbackID;
         RingIndicationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RingIndicationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_RING_INDICATION_RESPONSE_SIZE)
               ret_val = ((HDSM_Ring_Indication_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Headset device.  This function       */
   /* accepts as its input parameter the connection type indicating     */
   /* which connection will process the command.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
   /* * NOTE * The InBandRinging parameter specifies if the purpose of  */
   /*          setting up the Audio Connection is for In-Band Ring      */
   /*          (TRUE) or not (FALSE).                                   */
int _HDSM_Setup_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t InBandRinging)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   HDSM_Setup_Audio_Connection_Request_t  SetupAudioConnectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HeadsetManagerEventCallbackID) && ((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetupAudioConnectionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetupAudioConnectionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetupAudioConnectionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         SetupAudioConnectionRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION;
         SetupAudioConnectionRequest.MessageHeader.MessageLength   = HDSM_SETUP_AUDIO_CONNECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetupAudioConnectionRequest.ControlEventsHandlerID        = HeadsetManagerEventCallbackID;
         SetupAudioConnectionRequest.ConnectionType                = ConnectionType;
         SetupAudioConnectionRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetupAudioConnectionRequest.InBandRinging                 = InBandRinging;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetupAudioConnectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_SETUP_AUDIO_CONNECTION_RESPONSE_SIZE)
               ret_val = ((HDSM_Setup_Audio_Connection_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HDSM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Headset */
   /* device.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Release_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HDSM_Release_Audio_Connection_Request_t  ReleaseAudioConnectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HeadsetManagerEventCallbackID) && ((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         ReleaseAudioConnectionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ReleaseAudioConnectionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ReleaseAudioConnectionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         ReleaseAudioConnectionRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION;
         ReleaseAudioConnectionRequest.MessageHeader.MessageLength   = HDSM_RELEASE_AUDIO_CONNECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ReleaseAudioConnectionRequest.ControlEventsHandlerID        = HeadsetManagerEventCallbackID;
         ReleaseAudioConnectionRequest.ConnectionType                = ConnectionType;
         ReleaseAudioConnectionRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ReleaseAudioConnectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_RELEASE_AUDIO_CONNECTION_RESPONSE_SIZE)
               ret_val = ((HDSM_Release_Audio_Connection_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Headset Manager Data Handler ID (registered  */
   /* via call to the HDSM_Register_Data_Event_Callback() function),    */
   /* followed by the the connection type indicating which connection   */
   /* will transmit the audio data, the length (in Bytes) of the audio  */
   /* data to send, and a pointer to the audio data to send to the      */
   /* remote dntity.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
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
int _HDSM_Send_Audio_Data(unsigned int HeadsetManagerDataEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData)
{
   int                             ret_val;
   HDSM_Send_Audio_Data_Request_t *SendAudioDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HeadsetManagerDataEventCallbackID) && ((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (AudioDataLength) && (AudioData))
      {
         /* Allocate memory to hold the message.                        */
         if((SendAudioDataRequest = (HDSM_Send_Audio_Data_Request_t *)BTPS_AllocateMemory(HDSM_SEND_AUDIO_DATA_REQUEST_SIZE(AudioDataLength))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendAudioDataRequest, 0, HDSM_SEND_AUDIO_DATA_REQUEST_SIZE(0));

            SendAudioDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendAudioDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendAudioDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
            SendAudioDataRequest->MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_SEND_AUDIO_DATA;
            SendAudioDataRequest->MessageHeader.MessageLength   = HDSM_SEND_AUDIO_DATA_REQUEST_SIZE(AudioDataLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendAudioDataRequest->DataEventsHandlerID           = HeadsetManagerDataEventCallbackID;
            SendAudioDataRequest->ConnectionType                = ConnectionType;
            SendAudioDataRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SendAudioDataRequest->AudioDataLength               = AudioDataLength;

            BTPS_MemCopy(SendAudioDataRequest->AudioData, AudioData, AudioDataLength);

            /* Message has been formatted, go ahead and send it.        */
            ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendAudioDataRequest, 0, NULL);

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendAudioDataRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the HDSET Manager    */
   /* Service.  This Callback will be dispatched by the HDSET Manager   */
   /* when various HDSET Manager events occur.  This function accepts as*/
   /* its parameters the ConnectionType of the type of connection to    */
   /* register for events, and a boolean that specifies if this is the  */
   /* control callback (there can only be one control callback in the   */
   /* system).  This function returns a non-zero value if successful or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * The return value from this function specifies the HDSET  */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _HDSM_Un_Register_Events() function to un-Register the   */
   /*          event handler.                                           */
int _HDSM_Register_Events(HDSM_Connection_Type_t ConnectionType, Boolean_t ControlHandler)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   HDSM_Register_Headset_Events_Request_t     RegisterHeadsetEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway))
      {
         /* All that we really need to do is to build a Register HDSET  */
         /* Events message and send it to the server.                   */
         BTPS_MemInitialize(&RegisterHeadsetEventsRequest, 0, sizeof(RegisterHeadsetEventsRequest));

         RegisterHeadsetEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         RegisterHeadsetEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         RegisterHeadsetEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         RegisterHeadsetEventsRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_EVENTS;
         RegisterHeadsetEventsRequest.MessageHeader.MessageLength   = HDSM_REGISTER_HEADSET_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         RegisterHeadsetEventsRequest.ConnectionType                = ConnectionType;
         RegisterHeadsetEventsRequest.ControlHandler                = ControlHandler;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHeadsetEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_REGISTER_HEADSET_EVENTS_RESPONSE_SIZE)
            {
               if(!(ret_val = ((HDSM_Register_Headset_Events_Response_t *)ResponseMessage)->Status))
                  ret_val = (int)(((HDSM_Register_Headset_Events_Response_t *)ResponseMessage)->EventsHandlerID);
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager event callback*/
   /* (registered via a successful call to the _HDSM_Register_Events()  */
   /* function.  This function accepts as input the Headset Manager     */
   /* event callback ID (return value from the _HDSM_Register_Events()  */
   /* function).                                                        */
int _HDSM_Un_Register_Events(unsigned int HeadsetManagerEventCallbackID)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   HDSM_Un_Register_Headset_Events_Request_t  UnRegisterHeadsetEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if(HeadsetManagerEventCallbackID)
      {
         /* All that we really need to do is to build an Un-Register    */
         /* HDSET Events message and send it to the server.             */
         BTPS_MemInitialize(&UnRegisterHeadsetEventsRequest, 0, sizeof(UnRegisterHeadsetEventsRequest));

         UnRegisterHeadsetEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterHeadsetEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterHeadsetEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         UnRegisterHeadsetEventsRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_EVENTS;
         UnRegisterHeadsetEventsRequest.MessageHeader.MessageLength   = HDSM_UN_REGISTER_HEADSET_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterHeadsetEventsRequest.EventsHandlerID               = HeadsetManagerEventCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHeadsetEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_UN_REGISTER_HEADSET_EVENTS_RESPONSE_SIZE)
               ret_val = ((HDSM_Un_Register_Headset_Events_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Headset   */
   /* Profile Manager service to explicitly process SCO audio data.     */
   /* This callback will be dispatched by the Headset Manager when      */
   /* various Headset Manager events occur.  This function accepts the  */
   /* connection type which indicates the connection type the data      */
   /* registration callback to register for.  This function returns a   */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HDSM_Send_Audio_Data() function to send SCO audio data. */
   /* * NOTE * There can only be a single data event handler registered */
   /*          for each type of Headset Manager connection type.        */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HDSM_Un_Register_Data_Events() function to un-register  */
   /*          the callback from this module.                           */
int _HDSM_Register_Data_Events(HDSM_Connection_Type_t ConnectionType)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   HDSM_Register_Headset_Data_Events_Request_t  RegisterHeadsetDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((ConnectionType == sctHeadset) || (ConnectionType == sctAudioGateway))
      {
         /* All that we really need to do is to build a Register HDSET  */
         /* Data Events message and send it to the server.              */
         BTPS_MemInitialize(&RegisterHeadsetDataEventsRequest, 0, sizeof(RegisterHeadsetDataEventsRequest));

         RegisterHeadsetDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         RegisterHeadsetDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         RegisterHeadsetDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         RegisterHeadsetDataEventsRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_REGISTER_HEADSET_DATA;
         RegisterHeadsetDataEventsRequest.MessageHeader.MessageLength   = HDSM_REGISTER_HEADSET_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         RegisterHeadsetDataEventsRequest.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHeadsetDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_REGISTER_HEADSET_DATA_EVENTS_RESPONSE_SIZE)
            {
               if(!(ret_val = ((HDSM_Register_Headset_Data_Events_Response_t *)ResponseMessage)->Status))
                  ret_val = (int)(((HDSM_Register_Headset_Data_Events_Response_t *)ResponseMessage)->DataEventsHandlerID);
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager data event    */
   /* callback (registered via a successful call to the                 */
   /* _HDSM_Register_Data_Events() function.  This function accepts as  */
   /* input the Headset Manager event callback ID (return value from the*/
   /* _HDSM_Register_Data_Events() function).                           */
int _HDSM_Un_Register_Data_Events(unsigned int HeadsetManagerDataCallbackID)
{
   int                                             ret_val;
   BTPM_Message_t                                 *ResponseMessage;
   HDSM_Un_Register_Headset_Data_Events_Request_t  UnRegisterHeadsetDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if(HeadsetManagerDataCallbackID)
      {
         /* All that we really need to do is to build an Un-Register    */
         /* HDSET Events message and send it to the server.             */
         BTPS_MemInitialize(&UnRegisterHeadsetDataEventsRequest, 0, sizeof(UnRegisterHeadsetDataEventsRequest));

         UnRegisterHeadsetDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterHeadsetDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterHeadsetDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         UnRegisterHeadsetDataEventsRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_UN_REGISTER_HEADSET_DATA;
         UnRegisterHeadsetDataEventsRequest.MessageHeader.MessageLength   = HDSM_UN_REGISTER_HEADSET_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterHeadsetDataEventsRequest.DataEventsHandlerID           = HeadsetManagerDataCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHeadsetDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_UN_REGISTER_HEADSET_DATA_EVENTS_RESPONSE_SIZE)
               ret_val = ((HDSM_Un_Register_Headset_Data_Events_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Callback ID that is returned from a        */
   /* successful call to HDSM_Register_Event_Callback().  The second    */
   /* parameter is the local connection type of the SCO connection.  The*/
   /* third parameter is the address of the remote device of the SCO    */
   /* connection.  The fourth parameter is a pointer to the location to */
   /* store the SCO Handle. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int _HDSM_Query_SCO_Connection_Handle(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HDSM_Query_SCO_Connection_Handle_Request_t  QuerySCOConnectionHandleRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HeadsetManagerEventCallbackID) && (SCOHandle))
      {
         /* All that we really need to do is to build an Un-Register    */
         /* HDSET Events message and send it to the server.             */
         BTPS_MemInitialize(&QuerySCOConnectionHandleRequest, 0, sizeof(QuerySCOConnectionHandleRequest));

         QuerySCOConnectionHandleRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QuerySCOConnectionHandleRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QuerySCOConnectionHandleRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEADSET_MANAGER;
         QuerySCOConnectionHandleRequest.MessageHeader.MessageFunction = HDSM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE;
         QuerySCOConnectionHandleRequest.MessageHeader.MessageLength   = HDSM_QUERY_SCO_CONNECTION_HANDLE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QuerySCOConnectionHandleRequest.EventsHandlerID               = HeadsetManagerEventCallbackID;
         QuerySCOConnectionHandleRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         QuerySCOConnectionHandleRequest.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QuerySCOConnectionHandleRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDSM_QUERY_SCO_CONNECTION_HANDLE_RESPONSE_SIZE)
            {
               *SCOHandle = ((HDSM_Query_SCO_Connection_Handle_Response_t *)ResponseMessage)->SCOHandle;
               ret_val    = ((HDSM_Query_SCO_Connection_Handle_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HEADSET | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

