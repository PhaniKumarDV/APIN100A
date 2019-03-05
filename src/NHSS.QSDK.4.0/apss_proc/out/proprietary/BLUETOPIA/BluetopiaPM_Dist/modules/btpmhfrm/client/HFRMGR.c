/*****< hfrmgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HFRMGR - Hands Free Manager Implementation for Stonestreet One Bluetooth  */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHFRM.h"            /* BTPM HFRE Manager Prototypes/Constants.   */
#include "HFRMMSG.h"             /* BTPM HFRE Manager Message Formats.        */
#include "HFRMGR.h"              /* HFR Manager Impl. Prototypes/Constants.   */

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
   /* initialize the Hands Free Manager implementation.  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Hands Free Manager Implementation.                                */
int _HFRM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Hands Free Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the Hands */
   /* Free Manager implementation.  After this function is called the   */
   /* Hands Free Manager implementation will no longer operate until it */
   /* is initialized again via a call to the _HFRM_Initialize()         */
   /* function.                                                         */
void _HFRM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server. This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened. An  */
   /*          hfetOpenPortIndication event will notify of this status. */
int _HFRM_Connection_Request_Response(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   HFRM_Connection_Request_Response_Request_t   ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Type: 0x%08X\n", ConnectionType));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Disconnect Audio*/
         /* Stream message and send it to the server.                   */
         BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

         ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         ConnectionRequestResponseRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
         ConnectionRequestResponseRequest.MessageHeader.MessageLength   = HFRM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectionRequestResponseRequest.ConnectionType                = ConnectionType;
         ConnectionRequestResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectionRequestResponseRequest.Accept                        = AcceptConnection;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
               ret_val = ((HFRM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Hands Free/Audio Gateway device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.  This function accepts the      */
   /* connection type to make as the first parameter.  This parameter   */
   /* specifies the LOCAL connection type (i.e. if the caller would     */
   /* like to connect the local Hands Free service to a remote Audio    */
   /* Gateway device, the Hands Free connection type would be specified */
   /* for this parameter).  This function also accepts the connection   */
   /* information for the remote device (address and server port).      */
   /* This function accepts the connection flags to apply to control    */
   /* how the connection is made regarding encryption and/or            */
   /* authentication.                                                   */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Hands Free Manager Connection Status Event (if           */
   /*          specified).                                              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHFRConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HFRM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int _HFRM_Connect_Remote_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HFRM_Connect_Remote_Device_Request_t  ConnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort))
      {
         /* All that we really need to do is to build a Connect Audio   */
         /* Stream message and send it to the server.                   */
         BTPS_MemInitialize(&ConnectRemoteDeviceRequest, 0, sizeof(ConnectRemoteDeviceRequest));

         ConnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         ConnectRemoteDeviceRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE;
         ConnectRemoteDeviceRequest.MessageHeader.MessageLength   = HFRM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

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
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
               ret_val = ((HFRM_Connect_Remote_Device_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Hands Free or    */
   /* Audio Gateway connection that was previously opened by any of the */
   /* following mechanisms:                                             */
   /*   - Successful call to HFRM_Connect_Remote_Device() function.     */
   /*   - Incoming open request (Hands Free or Audio Gateway) which was */
   /*     accepted either automatically or by a call to                 */
   /*     HFRM_Connection_Request_Response().                           */
   /* This function accepts as input the type of the local connection   */
   /* which should close its active connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.  This function does NOT un-register any Hands Free or Audio*/
   /* Gateway services from the system, it ONLY disconnects any         */
   /* connection that is currently active on the specified service.     */
int _HFRM_Disconnect_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   HFRM_Disconnect_Device_Request_t  DisconnectDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Disconnect      */
         /* Device message and send it to the server.                   */
         BTPS_MemInitialize(&DisconnectDeviceRequest, 0, sizeof(DisconnectDeviceRequest));

         DisconnectDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisconnectDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisconnectDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         DisconnectDeviceRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DISCONNECT_DEVICE;
         DisconnectDeviceRequest.MessageHeader.MessageLength   = HFRM_DISCONNECT_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisconnectDeviceRequest.ConnectionType                = ConnectionType;
         DisconnectDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_DISCONNECT_DEVICE_RESPONSE_SIZE)
               ret_val = ((HFRM_Disconnect_Device_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Hands   */
   /* Free or Audio Gateway Devices (specified by the first parameter). */
   /* This function accepts a the local service type to query, followed */
   /* by buffer information to receive any currently connected device   */
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
int _HFRM_Query_Connected_Devices(HFRM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   HFRM_Query_Connected_Devices_Request_t  QueryConnectedDevices;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || ((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices))))
      {
         /* All that we really need to do is to build a Query Connected */
         /* Devices message and send it to the server.                  */
         QueryConnectedDevices.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedDevices.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedDevices.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QueryConnectedDevices.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES;
         QueryConnectedDevices.MessageHeader.MessageLength   = HFRM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryConnectedDevices.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedDevices, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(((HFRM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)))
            {
               if(!(ret_val = ((HFRM_Query_Connected_Devices_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = ((HFRM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                  /* If the caller specified a buffer to receive the    */
                  /* list into, go ahead and copy it over.              */
                  if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
                  {
                     /* If there are more entries in the returned list  */
                     /* than the buffer specified, we need to truncate  */
                     /* it.                                             */
                     if(MaximumRemoteDeviceListEntries >= ((HFRM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)
                        MaximumRemoteDeviceListEntries = ((HFRM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                     BTPS_MemCopy(RemoteDeviceAddressList, ((HFRM_Query_Connected_Devices_Response_t *)ResponseMessage)->DeviceConnectedList, MaximumRemoteDeviceListEntries*sizeof(BD_ADDR_t));

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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Hands Free or Audio*/
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * On input the TotalNumberAdditionalIndicators member of   */
   /*          the structure should be set to the total number of       */
   /*          additional indicator entry structures that the           */
   /*          AdditionalIndicatorList member points to.  On return from*/
   /*          this function this structure member holds the total      */
   /*          number of additional indicator entries that are supported*/
   /*          by the connection.  The NumberAdditionalIndicators       */
   /*          member will hold (on return) the number of indicator     */
   /*          entries that are actually present in the list.           */
   /* * NOTE * It is possible to not query the additional indicators    */
   /*          by passing zero for the TotalNumberAdditionalIndicators  */
   /*          member.  This member will still hold the total of        */
   /*          supported indicators on return of this function.         */
int _HFRM_Query_Current_Configuration(HFRM_Connection_Type_t ConnectionType, HFRM_Current_Configuration_t *CurrentConfiguration)
{
   int                                         ret_val;
   unsigned int                                NumberToCopy;
   BTPM_Message_t                             *ResponseMessage;
   HFRM_Query_Current_Configuration_Request_t  QueryCurrentConfigurationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (CurrentConfiguration))
      {
         /* All that we really need to do is to build a Query Connected */
         /* Devices message and send it to the server.                  */
         QueryCurrentConfigurationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryCurrentConfigurationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryCurrentConfigurationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QueryCurrentConfigurationRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION;
         QueryCurrentConfigurationRequest.MessageHeader.MessageLength   = HFRM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryCurrentConfigurationRequest.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryCurrentConfigurationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE(((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->TotalNumberAdditionalIndicators)))
            {
               if(!(ret_val = ((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->Status))
               {
                  /* Determine if the caller specified an array to      */
                  /* receive the additional indicators in.              */
                  if(CurrentConfiguration->TotalNumberAdditionalIndicators)
                     NumberToCopy = CurrentConfiguration->TotalNumberAdditionalIndicators;
                  else
                     NumberToCopy = 0;

                  /* Copy the fields in the response.                   */
                  CurrentConfiguration->IncomingConnectionFlags         = ((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->IncomingConnectionFlags;
                  CurrentConfiguration->SupportedFeaturesMask           = ((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->SupportedFeaturesMask;
                  CurrentConfiguration->CallHoldingSupportMask          = ((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->CallHoldingSupportMask;
                  CurrentConfiguration->NetworkType                     = ((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->NetworkType;
                  CurrentConfiguration->TotalNumberAdditionalIndicators = ((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->TotalNumberAdditionalIndicators;
                  CurrentConfiguration->NumberAdditionalIndicators      = 0;

                  /* Copy the Additional Indicators into structure      */
                  /* provided by the caller if a list was passed in.    */
                  if(NumberToCopy)
                  {
                     /* If we have more space specified than was        */
                     /* returned in the response than we will truncate  */
                     /* the number to copy.                             */
                     if(NumberToCopy > CurrentConfiguration->TotalNumberAdditionalIndicators)
                        NumberToCopy = CurrentConfiguration->TotalNumberAdditionalIndicators;

                     /* Save the number of entries in the Additional    */
                     /* Indicator List.                                 */
                     CurrentConfiguration->NumberAdditionalIndicators = NumberToCopy;

                     /* Copy the additional indicators back to the      */
                     /* caller.                                         */
                     BTPS_MemCopy(CurrentConfiguration->AdditionalIndicatorList, ((HFRM_Query_Current_Configuration_Response_t *)ResponseMessage)->AdditionalIndicatorList, (NumberToCopy*HFRM_CONFIGURATION_INDICATOR_ENTRY_SIZE));
                  }
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Hands Free and*/
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _HFRM_Change_Incoming_Connection_Flags(HFRM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HFRM_Change_Incoming_Connection_Flags_Request_t  ChangeIncomingConnectionFlagsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway))
      {
         /* All that we really need to do is to build a Start Audio     */
         /* Stream message and send it to the server.                   */
         ChangeIncomingConnectionFlagsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS;
         ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageLength   = HFRM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ChangeIncomingConnectionFlagsRequest.ConnectionType                = ConnectionType;
         ChangeIncomingConnectionFlagsRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeIncomingConnectionFlagsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE)
               ret_val = ((HFRM_Change_Incoming_Connection_Flags_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for disabling echo cancellation and  */
   /* noise reduction on the remote device.  This function may be       */
   /* performed by both the Hands Free and the Audio Gateway connections*/
   /* for which a valid service level connection exists but no audio    */
   /* connection exists.  This function accepts as its input parameter  */
   /* the connection type indicating which local service which will send*/
   /* this command.  This function returns zero if successful or a      */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * It is not possible to enable this feature once it has    */
   /*          been disbled because the specification provides no means */
   /*          to re-enable this feature.  This feature will remained   */
   /*          disabled until the current service level connection has  */
   /*          been dropped.                                            */
int _HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                                             ret_val;
   BTPM_Message_t                                 *ResponseMessage;
   HFRM_Disable_Echo_Noise_Cancellation_Request_t  DisableEchoNoiseCancellationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && ((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Start Audio     */
         /* Stream message and send it to the server.                   */
         DisableEchoNoiseCancellationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisableEchoNoiseCancellationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisableEchoNoiseCancellationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         DisableEchoNoiseCancellationRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DISABLE_ECHO_NOISE_CANCELLATION;
         DisableEchoNoiseCancellationRequest.MessageHeader.MessageLength   = HFRM_DISABLE_ECHO_NOISE_CANCELLATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisableEchoNoiseCancellationRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         DisableEchoNoiseCancellationRequest.ConnectionType                = ConnectionType;
         DisableEchoNoiseCancellationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisableEchoNoiseCancellationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_DISABLE_ECHO_NOISE_CANCELLATION_RESPONSE_SIZE)
               ret_val = ((HFRM_Disable_Echo_Noise_Cancellation_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* When called by a Hands Free device, this function is responsible  */
   /* for requesting activation or deactivation of the voice recognition*/
   /* which resides on the remote Audio Gateway.  When called by an     */
   /* Audio Gateway, this function is responsible for informing the     */
   /* remote Hands Free device of the current activation state of the   */
   /* local voice recognition function.  This function may only be      */
   /* called by local devices that were opened with support for voice   */
   /* recognition.  This function accepts as its input parameters the   */
   /* connectoin type indicating the local connection which will process*/
   /* the command and a BOOLEAN flag specifying the type of request or  */
   /* notification to send.  When active the voice recognition function */
   /* on the Audio Gateway is turned on, when inactive the voice        */
   /* recognition function on the Audio Gateway is turned off.  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Set_Remote_Voice_Recognition_Activation(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t VoiceRecognitionActive)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HFRM_Set_Voice_Recognition_Activation_Request_t  SetVoiceRecognitionActiveRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && ((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Voice       */
         /* Recognition Activation message and send it to the server.   */
         SetVoiceRecognitionActiveRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetVoiceRecognitionActiveRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetVoiceRecognitionActiveRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SetVoiceRecognitionActiveRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SET_VOICE_RECOGNITION_ACTIVATION;
         SetVoiceRecognitionActiveRequest.MessageHeader.MessageLength   = HFRM_SET_VOICE_RECOGNITION_ACTIVATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetVoiceRecognitionActiveRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SetVoiceRecognitionActiveRequest.ConnectionType                = ConnectionType;
         SetVoiceRecognitionActiveRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetVoiceRecognitionActiveRequest.VoiceRecognitionActive        = VoiceRecognitionActive;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetVoiceRecognitionActiveRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SET_VOICE_RECOGNITION_ACTIVATION_RESPONSE_SIZE)
               ret_val = ((HFRM_Set_Voice_Recognition_Activation_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  This function may    */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current speaker   */
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the speaker gain*/
   /* of the remote Hands Free device.  This function accepts as its    */
   /* input parameters the connection type indicating the local         */
   /* connection which will process the command and the speaker gain to */
   /* be sent to the remote device.  The speaker gain Parameter *MUST*  */
   /* be between the values:                                            */
   /*                                                                   */
   /*    HFRE_SPEAKER_GAIN_MINIMUM                                      */
   /*    HFRE_SPEAKER_GAIN_MAXIMUM                                      */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Set_Remote_Speaker_Gain(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain)
{
   int                               ret_val;
   BTPM_Message_t                  *ResponseMessage;
   HFRM_Set_Speaker_Gain_Request_t  SetSpeakerGainRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && ((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (SpeakerGain >= HFRE_SPEAKER_GAIN_MINIMUM) && (SpeakerGain <= HFRE_SPEAKER_GAIN_MAXIMUM))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetSpeakerGainRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetSpeakerGainRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetSpeakerGainRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SetSpeakerGainRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SET_SPEAKER_GAIN;
         SetSpeakerGainRequest.MessageHeader.MessageLength   = HFRM_SET_SPEAKER_GAIN_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetSpeakerGainRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SetSpeakerGainRequest.ConnectionType                = ConnectionType;
         SetSpeakerGainRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetSpeakerGainRequest.SpeakerGain                   = SpeakerGain;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetSpeakerGainRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SET_SPEAKER_GAIN_RESPONSE_SIZE)
               ret_val = ((HFRM_Set_Speaker_Gain_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  This function may */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current microphone*/
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the microphone  */
   /* gain of the remote Hands Free device.  This function accepts as   */
   /* its input parameters the connection type indicating the local     */
   /* connection which will process the command and the microphone gain */
   /* to be sent to the remote device.  The microphone gain Parameter   */
   /* *MUST* be between the values:                                     */
   /*                                                                   */
   /*    HFRE_MICROPHONE_GAIN_MINIMUM                                   */
   /*    HFRE_MICROPHONE_GAIN_MAXIMUM                                   */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Set_Remote_Microphone_Gain(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   HFRM_Set_Microphone_Gain_Request_t  SetMicrophoneGainRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && ((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (MicrophoneGain >= HFRE_MICROPHONE_GAIN_MINIMUM) && (MicrophoneGain <= HFRE_MICROPHONE_GAIN_MAXIMUM))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetMicrophoneGainRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetMicrophoneGainRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetMicrophoneGainRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SetMicrophoneGainRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SET_MICROPHONE_GAIN;
         SetMicrophoneGainRequest.MessageHeader.MessageLength   = HFRM_SET_MICROPHONE_GAIN_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetMicrophoneGainRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SetMicrophoneGainRequest.ConnectionType                = ConnectionType;
         SetMicrophoneGainRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetMicrophoneGainRequest.MicrophoneGain                = MicrophoneGain;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetMicrophoneGainRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SET_MICROPHONE_GAIN_RESPONSE_SIZE)
               ret_val = ((HFRM_Set_Microphone_Gain_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* Send a codec ID. The audio gateway uses this function to send the */
   /* preferred codec. The hands free device uses the function to       */ 
   /* confirm the audio gateway's choice. The EventCallback ID is       */
   /* returned from HFRM_Register_Event_Callback(). ConnectionType      */
   /* indicates whether the local role is audio gateway or hands free.  */
   /* RemoteDeviceAddress is the address of the remote device. CodecId  */
   /* identifies the selected codec. This function returns zero if      */
   /* successful or a negative code in case of error.                   */
int _HFRM_Send_Select_Codec(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned char CodecID)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   HFRM_Send_Select_Codec_Request_t  SendSelectCodecRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (CodecID))
      {
         /* Build a Send Select Codec Hands Free event message  and     */
         /* send it to the server.                                      */
         BTPS_MemInitialize(&SendSelectCodecRequest, 0, HFRM_SEND_SELECT_CODEC_REQUEST_SIZE);

         SendSelectCodecRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendSelectCodecRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendSelectCodecRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SendSelectCodecRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_SELECT_CODEC;
         SendSelectCodecRequest.MessageHeader.MessageLength   = HFRM_SEND_SELECT_CODEC_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendSelectCodecRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SendSelectCodecRequest.ConnectionType                = ConnectionType;
         SendSelectCodecRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendSelectCodecRequest.CodecID                       = CodecID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendSelectCodecRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_SELECT_CODEC_RESPONSE_SIZE)
               ret_val = ((HFRM_Send_Select_Codec_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for querying the remote     */
   /* control indicator status.  This function may only be performed by */
   /* a local Hands Free unit with a valid service level connection to a*/
   /* connected remote Audio Gateway.  The results to this query will be*/
   /* returned as part of the control indicator status confirmation     */
   /* event (hetHFRControlIndicatorStatus).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Query_Remote_Control_Indicator_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                            ret_val;
   BTPM_Message_t                                *ResponseMessage;
   HFRM_Query_Control_Indicator_Status_Request_t  QueryRemoteControlIndicatorStatusRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         QueryRemoteControlIndicatorStatusRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryRemoteControlIndicatorStatusRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryRemoteControlIndicatorStatusRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QueryRemoteControlIndicatorStatusRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CONTROL_INDICATOR_STATUS;
         QueryRemoteControlIndicatorStatusRequest.MessageHeader.MessageLength   = HFRM_QUERY_CONTROL_INDICATOR_STATUS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryRemoteControlIndicatorStatusRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         QueryRemoteControlIndicatorStatusRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryRemoteControlIndicatorStatusRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_CONTROL_INDICATOR_STATUS_RESPONSE_SIZE)
               ret_val = ((HFRM_Query_Control_Indicator_Status_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling the        */
   /* indicator event notification on a remote Audio Gateway.  This     */
   /* function may only be performed by Hands Free devices that have a  */
   /* valid service level connection to a connected remote Audio        */
   /* Gateway.  When enabled, the remote Audio Gateway device will send */
   /* unsolicited responses to update the local device of the current   */
   /* control indicator values.  This function accepts as its input     */
   /* parameter a BOOLEAN flag used to enable or disable event          */
   /* notification.  This function returns zero if successful or a      */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Enable_Remote_Indicator_Event_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableEventNotification)
{
   int                                                 ret_val;
   BTPM_Message_t                                     *ResponseMessage;
   HFRM_Enable_Indicator_Event_Notification_Request_t  EnableRemoteIndicatorEventNotificationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         EnableRemoteIndicatorEventNotificationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         EnableRemoteIndicatorEventNotificationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnableRemoteIndicatorEventNotificationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         EnableRemoteIndicatorEventNotificationRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ENABLE_INDICATOR_NOTIFICATION;
         EnableRemoteIndicatorEventNotificationRequest.MessageHeader.MessageLength   = HFRM_ENABLE_INDICATOR_EVENT_NOTIFICATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableRemoteIndicatorEventNotificationRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         EnableRemoteIndicatorEventNotificationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         EnableRemoteIndicatorEventNotificationRequest.EnableEventNotification       = EnableEventNotification;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableRemoteIndicatorEventNotificationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_ENABLE_INDICATOR_EVENT_NOTIFICATION_RESPONSE_SIZE)
               ret_val = ((HFRM_Enable_Indicator_Event_Notification_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for updating an indicator   */
   /* notification state.  This function may only be performed by Hands */
   /* Free Devices with a valid service level connection to a connected */
   /* remote Audio Gateway device.  This function accepts as its input  */
   /* parameters the number of indicators and list of indicator names   */
   /* and the indication state.  This function returns zero if          */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful call*/
   /*          to the HFRM_Register_Event_Callback() function           */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Update_Remote_Indicator_Notification_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberIndicators, HFRE_Notification_Update_t *UpdateIndicators)
{
   int                                                 ret_val;
   unsigned int                                        IndicatorDescriptionLength;
   unsigned int                                        Index;
   BTPM_Message_t                                     *ResponseMessage;
   HFRM_Notification_Update_Entry_t                   *IndicatorUpdateEntry;
   HFRM_Update_Indicator_Notification_State_Request_t *UpdateIndicatorNotificationStateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberIndicators) && (UpdateIndicators))
      {
         /* Allocate memory to hold the message with the requested      */
         /* number of indicators.                                       */
         UpdateIndicatorNotificationStateRequest = (HFRM_Update_Indicator_Notification_State_Request_t *)BTPS_AllocateMemory(HFRM_UPDATE_INDICATOR_NOTIFICATION_STATE_REQUEST_SIZE(NumberIndicators));
         if(UpdateIndicatorNotificationStateRequest)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(UpdateIndicatorNotificationStateRequest, 0, HFRM_UPDATE_INDICATOR_NOTIFICATION_STATE_REQUEST_SIZE(NumberIndicators));

            /* Initialize the return value to success so that we can    */
            /* determine if any of the Indicator Descriptors are of an  */
            /* invalid length.                                          */
            ret_val = 0;

            /* Loop through and format the passed in entries into the   */
            /* message buffer.                                          */
            Index                = 0;
            while((!ret_val) && (Index < NumberIndicators))
            {
               /* Get a pointer to the Indicator Update Information.    */
               IndicatorUpdateEntry = &UpdateIndicatorNotificationStateRequest->NotificationUpdateList[Index];
               
                  /* Verify that there is a description.                   */
               if((IndicatorUpdateEntry) && (UpdateIndicators[Index].IndicatorDescription))
               {
                  IndicatorDescriptionLength = BTPS_StringLength(UpdateIndicators[Index].IndicatorDescription);
                  if(IndicatorDescriptionLength <= HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                  {
                     IndicatorUpdateEntry->NotificationUpdate.IndicatorDescription = IndicatorUpdateEntry->IndicatorDescription;
                     IndicatorUpdateEntry->NotificationUpdate.NotificationEnabled  = UpdateIndicators[Index].NotificationEnabled;
                     IndicatorUpdateEntry->IndicatorDescriptionLength              = IndicatorDescriptionLength;
                     BTPS_MemCopy(IndicatorUpdateEntry->IndicatorDescription, UpdateIndicators[Index].IndicatorDescription, IndicatorDescriptionLength);
                  }
                  else
                    ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }

            /* Only continue if no error occurred while processing the  */
            /* input parameters.                                        */
            if(!ret_val)
            {
               UpdateIndicatorNotificationStateRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               UpdateIndicatorNotificationStateRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               UpdateIndicatorNotificationStateRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               UpdateIndicatorNotificationStateRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_NOTIFICATION_STATE;
               UpdateIndicatorNotificationStateRequest->MessageHeader.MessageLength   = HFRM_UPDATE_INDICATOR_NOTIFICATION_STATE_REQUEST_SIZE(NumberIndicators) - BTPM_MESSAGE_HEADER_SIZE;

               UpdateIndicatorNotificationStateRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               UpdateIndicatorNotificationStateRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               UpdateIndicatorNotificationStateRequest->NumberUpdateIndicators        = NumberIndicators;

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)UpdateIndicatorNotificationStateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_UPDATE_INDICATOR_NOTIFICATION_STATE_RESPONSE_SIZE)
                     ret_val = ((HFRM_Update_Indicator_Notification_State_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(UpdateIndicatorNotificationStateRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for querying the call holding and    */
   /* multi-party services which are supported by the remote Audio      */
   /* Gateway.  This function is used by Hands Free connections which   */
   /* support three way calling and call waiting to determine the       */
   /* features supported by the remote Audio Gateway.  This function can*/
   /* only be used if a valid service level connection to a connected   */
   /* remote Audio Gateway exists.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   HFRM_Query_Call_Holding_Multiparty_Request_t  QueryCallHoldingMultipartySupportRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         QueryCallHoldingMultipartySupportRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryCallHoldingMultipartySupportRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryCallHoldingMultipartySupportRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QueryCallHoldingMultipartySupportRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CALL_HOLD_MULTI_SUPPORT;
         QueryCallHoldingMultipartySupportRequest.MessageHeader.MessageLength   = HFRM_QUERY_CALL_HOLDING_MULTIPARTY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryCallHoldingMultipartySupportRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         QueryCallHoldingMultipartySupportRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryCallHoldingMultipartySupportRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_CALL_HOLDING_MULTIPARTY_RESPONSE_SIZE)
               ret_val = ((HFRM_Query_Call_Holding_Multiparty_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for allowing the control of multiple */
   /* concurrent calls and provides a means for holding calls, releasing*/
   /* calls, switching between two calls and adding a call to a         */
   /* multi-party conference.  This function may only be performed by   */
   /* Hands Free units that support call waiting and multi-party        */
   /* services as well as have a valid service level connection to a    */
   /* connected remote Audio Gateway.  The selection which is made      */
   /* should be one that is supported by the remote Audio Gateway       */
   /* (queried via a call to the                                        */
   /* HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support()       */
   /* function).  This function accepts as its input parameter the      */
   /* selection of how to handle the currently waiting call.  If the    */
   /* selected handling type requires an index it should be provided in */
   /* the last parameter.  Otherwise the final paramter is ignored.     */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Call_Holding_Multiparty_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index)
{
   int                                                    ret_val;
   BTPM_Message_t                                        *ResponseMessage;
   HFRM_Send_Call_Holding_Multiparty_Selection_Request_t  SendCallHoldingMultipartySelectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((CallHoldMultipartyHandling >= chReleaseAllHeldCalls) && (CallHoldMultipartyHandling <= chPrivateConsultationMode)))
      {
         /* All that we really need to do is to build a Send Call       */
         /* Holding Multiparty Selection Request message and send it to */
         /* the server.                                                 */
         SendCallHoldingMultipartySelectionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendCallHoldingMultipartySelectionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendCallHoldingMultipartySelectionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SendCallHoldingMultipartySelectionRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_CALL_HOLD_MULTI_SELECTION;
         SendCallHoldingMultipartySelectionRequest.MessageHeader.MessageLength   = HFRM_SEND_CALL_HOLDING_MULTIPARTY_SELECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendCallHoldingMultipartySelectionRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SendCallHoldingMultipartySelectionRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendCallHoldingMultipartySelectionRequest.CallHoldMultipartyHandling    = CallHoldMultipartyHandling;
         SendCallHoldingMultipartySelectionRequest.Index                         = Index;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendCallHoldingMultipartySelectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_CALL_HOLDING_MULTIPARTY_SELECTION_RESPONSE_SIZE)
               ret_val = ((HFRM_Send_Call_Holding_Multiparty_Selection_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling call       */
   /* waiting notification on a remote Audio Gateway.  By default the   */
   /* call waiting notification is enabled in the network but disabled  */
   /* for notification via the service level connection (between Hands  */
   /* Free and Audio Gateway).  This function may only be performed by a*/
   /* Hands Free unit for which a valid service level connection to a   */
   /* connected remote Audio Gateway exists.  This function may only be */
   /* used to enable call waiting notifications if the local Hands Free */
   /* service supports call waiting and multi-party services.  This     */
   /* function accepts as its input parameter a BOOLEAN flag specifying */
   /* if this is a call to enable or disable this functionality.  This  */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Enable_Remote_Call_Waiting_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableNotification)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HFRM_Enable_Call_Waiting_Notification_Request_t  EnableCallWaitingNotificationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         EnableCallWaitingNotificationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         EnableCallWaitingNotificationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnableCallWaitingNotificationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         EnableCallWaitingNotificationRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ENABLE_CALL_WAIT_NOTIFICATION;
         EnableCallWaitingNotificationRequest.MessageHeader.MessageLength   = HFRM_ENABLE_CALL_WAITING_NOTIFICATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableCallWaitingNotificationRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         EnableCallWaitingNotificationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         EnableCallWaitingNotificationRequest.EnableNotification            = EnableNotification;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableCallWaitingNotificationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_ENABLE_CALL_WAITING_NOTIFICATION_RESPONSE_SIZE)
               ret_val = ((HFRM_Enable_Call_Waiting_Notification_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling call line  */
   /* identification notification on a remote Audio Gateway.  By        */
   /* default, the call line identification notification via the service*/
   /* level connection is disabled.  This function may only be performed*/
   /* by Hands Free units for which a valid service level connection to */
   /* a connected remote Audio Gateway exists.  This function may only  */
   /* be used to enable call line notifications if the local Hands Free */
   /* unit supports call line identification.  This function accepts as */
   /* its input parameters a BOOLEAN flag specifying if this is a call  */
   /* to enable or disable this functionality.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Enable_Remote_Call_Line_Identification_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableNotification)
{
   int                                                          ret_val;
   BTPM_Message_t                                              *ResponseMessage;
   HFRM_Enable_Call_Line_Identification_Notification_Request_t  EnableCallLineIdentificationNotificationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         EnableCallLineIdentificationNotificationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         EnableCallLineIdentificationNotificationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnableCallLineIdentificationNotificationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         EnableCallLineIdentificationNotificationRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ENABLE_CALL_LINE_ID_NOTIFICATION;
         EnableCallLineIdentificationNotificationRequest.MessageHeader.MessageLength   = HFRM_ENABLE_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableCallLineIdentificationNotificationRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         EnableCallLineIdentificationNotificationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         EnableCallLineIdentificationNotificationRequest.EnableNotification            = EnableNotification;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableCallLineIdentificationNotificationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_ENABLE_CALL_LINE_IDENTIFICATION_NOTIFICATION_RESPONSE_SIZE)
               ret_val = ((HFRM_Enable_Call_Line_Identification_Notification_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for dialing a phone number on a      */
   /* remote Audio Gateway.  This function may only be performed by     */
   /* Hands Free units for which a valid service level connection to a  */
   /* remote Audio Gateway exists.  This function accepts as its input  */
   /* parameter the phone number to dial on the remote Audio Gateway.   */
   /* This parameter should be a pointer to a NULL terminated string and*/
   /* its length *MUST* be between the values of:                       */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Dial_Phone_Number(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                               ret_val;
   unsigned int                      PhoneNumberLength;
   BTPM_Message_t                   *ResponseMessage;
   HFRM_Dial_Phone_Number_Request_t *DialPhoneNumberRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber))
      {
         /* Get the string length of the phone number string.           */
         PhoneNumberLength = BTPS_StringLength(PhoneNumber);

         /* Verify that the length of the phone number to dial is valid.*/
         if((PhoneNumberLength >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (PhoneNumberLength <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
         {
            /* Allocate memory to hold the message.                     */
            if((DialPhoneNumberRequest = (HFRM_Dial_Phone_Number_Request_t *)BTPS_AllocateMemory(HFRM_DIAL_PHONE_NUMBER_REQUEST_SIZE(PhoneNumberLength + 1))) != NULL)
            {
               /* All that we really need to do is to build the message */
               /* and send it to the server.                            */
               BTPS_MemInitialize(DialPhoneNumberRequest, 0, HFRM_DIAL_PHONE_NUMBER_REQUEST_SIZE(PhoneNumberLength + 1));

               DialPhoneNumberRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               DialPhoneNumberRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               DialPhoneNumberRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               DialPhoneNumberRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER;
               DialPhoneNumberRequest->MessageHeader.MessageLength   = HFRM_DIAL_PHONE_NUMBER_REQUEST_SIZE(PhoneNumberLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

               DialPhoneNumberRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               DialPhoneNumberRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               DialPhoneNumberRequest->PhoneNumberLength             = (PhoneNumberLength + 1);

               BTPS_MemCopy(DialPhoneNumberRequest->PhoneNumber, PhoneNumber, PhoneNumberLength);

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)DialPhoneNumberRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_RESPONSE_SIZE)
                     ret_val = ((HFRM_Dial_Phone_Number_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }

               /* Free the memory that was allocated for the packet.    */
               BTPS_FreeMemory(DialPhoneNumberRequest);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for dialing a phone number from a    */
   /* memory location (index) found on the remote Audio Gateway.  This  */
   /* function may only be performed by Hands Free devices for which a  */
   /* valid service level connection to a connected remote Audio Gateway*/
   /* exists.  This function accepts as its input parameter the memory  */
   /* location (index) for which the phone number to dial already exists*/
   /* on the remote Audio Gateway.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Dial_Phone_Number_From_Memory(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MemoryLocation)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   HFRM_Dial_Phone_Number_From_Memory_Request_t  DialPhoneNumberFromMemoryRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         DialPhoneNumberFromMemoryRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DialPhoneNumberFromMemoryRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DialPhoneNumberFromMemoryRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         DialPhoneNumberFromMemoryRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_DIAL_PHONE_NUMBER_FROM_MEMORY;
         DialPhoneNumberFromMemoryRequest.MessageHeader.MessageLength   = HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DialPhoneNumberFromMemoryRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         DialPhoneNumberFromMemoryRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         DialPhoneNumberFromMemoryRequest.MemoryLocation                = MemoryLocation;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DialPhoneNumberFromMemoryRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_DIAL_PHONE_NUMBER_FROM_MEMORY_RESPONSE_SIZE)
               ret_val = ((HFRM_Dial_Phone_Number_From_Memory_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for re-dialing the last number dialed*/
   /* on a remote Audio Gateway.  This function may only be performed by*/
   /* Hands Free devices for which a valid service level connection to a*/
   /* connected remote Audio Gateway exists.  This function returns zero*/
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Redial_Last_Phone_Number(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   HFRM_Re_Dial_Last_Phone_Number_Request_t  ReDialLastPhoneNumberRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         ReDialLastPhoneNumberRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ReDialLastPhoneNumberRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ReDialLastPhoneNumberRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         ReDialLastPhoneNumberRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_RE_DIAL_LAST_PHONE_NUMBER;
         ReDialLastPhoneNumberRequest.MessageHeader.MessageLength   = HFRM_RE_DIAL_LAST_PHONE_NUMBER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ReDialLastPhoneNumberRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         ReDialLastPhoneNumberRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ReDialLastPhoneNumberRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_RE_DIAL_LAST_PHONE_NUMBER_RESPONSE_SIZE)
               ret_val = ((HFRM_Re_Dial_Last_Phone_Number_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending the command to a remote  */
   /* Audi Gateway to answer an incoming call.  This function may only  */
   /* be performed by Hands Free devices for which a valid service level*/
   /* connection to a connected remote Audio Gateway exists.  This      */
   /* function return zero if successful or a negative return error code*/
   /* if there was an error.                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Answer_Incoming_Call(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   HFRM_Answer_Incoming_Call_Request_t  AnswerIncomingCallRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         AnswerIncomingCallRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         AnswerIncomingCallRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         AnswerIncomingCallRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         AnswerIncomingCallRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ANSWER_INCOMING_CALL;
         AnswerIncomingCallRequest.MessageHeader.MessageLength   = HFRM_ANSWER_INCOMING_CALL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         AnswerIncomingCallRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         AnswerIncomingCallRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&AnswerIncomingCallRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_ANSWER_INCOMING_CALL_RESPONSE_SIZE)
               ret_val = ((HFRM_Answer_Incoming_Call_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for transmitting DTMF codes to a     */
   /* remote Audio Gateway to be sent as a DTMF code over an on-going   */
   /* call.  This function may only be performed by Hands Free devices  */
   /* for which a valid service level connection to a connected remote  */
   /* Audio Gateway exists and an on-going call exists.  This function  */
   /* accepts as input the DTMF code to be transmitted.  This Code must */
   /* be one of the characters:                                         */
   /*                                                                   */
   /*   0-9, *, #, or A-D.                                              */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Transmit_DTMF_Code(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char DTMFCode)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   HFRM_Transmit_DTMF_Code_Request_t  TransmitDTMFCodeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         TransmitDTMFCodeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         TransmitDTMFCodeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         TransmitDTMFCodeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         TransmitDTMFCodeRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_TRANSMIT_DTMF_CODE;
         TransmitDTMFCodeRequest.MessageHeader.MessageLength   = HFRM_TRANSMIT_DTMF_CODE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         TransmitDTMFCodeRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         TransmitDTMFCodeRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         TransmitDTMFCodeRequest.DTMFCode                      = DTMFCode;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&TransmitDTMFCodeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_TRANSMIT_DTMF_CODE_RESPONSE_SIZE)
               ret_val = ((HFRM_Transmit_DTMF_Code_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for retrieving a phone number to     */
   /* associate with a unique voice tag to be stored in memory by the   */
   /* local Hands Free device.  This function may only be performed by a*/
   /* Hands Free device for which a valid service level connection to a */
   /* connected remote Audio Gateway exists.  The Hands Free unit must  */
   /* also support voice recognition to be able to use this function.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * When this function is called no other function may be    */
   /*          called until a voice tag response is received from the   */
   /*          remote Audio Gateway.                                    */
int _HFRM_Voice_Tag_Request(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   HFRM_Voice_Tag_Request_Request_t  VoiceTagRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         VoiceTagRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         VoiceTagRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         VoiceTagRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         VoiceTagRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_VOICE_TAG_REQUEST;
         VoiceTagRequest.MessageHeader.MessageLength   = HFRM_VOICE_TAG_REQUEST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         VoiceTagRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         VoiceTagRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&VoiceTagRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_REQUEST_RESPONSE_SIZE)
               ret_val = ((HFRM_Voice_Tag_Request_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a hang-up command to a   */
   /* remote Audio Gateway.  This function may be used to reject an     */
   /* incoming call or to terminate an on-going call.  This function may*/
   /* only be performed by Hands Free devices for which a valid service */
   /* level connection exists.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Hang_Up_Call(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                          ret_val;
   BTPM_Message_t              *ResponseMessage;
   HFRM_Hang_Up_Call_Request_t  HangUpCallRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         HangUpCallRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         HangUpCallRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         HangUpCallRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         HangUpCallRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_HANG_UP_CALL;
         HangUpCallRequest.MessageHeader.MessageLength   = HFRM_HANG_UP_CALL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         HangUpCallRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         HangUpCallRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&HangUpCallRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_HANG_UP_CALL_RESPONSE_SIZE)
               ret_val = ((HFRM_Hang_Up_Call_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for querying the current    */
   /* call list of the remote Audio Gateway device.  This function may  */
   /* only be performed by a Hands Free device with a valid service     */
   /* level connection to a connected Audio Gateway.  This function     */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Query_Remote_Current_Calls_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HFRM_Query_Current_Calls_List_Request_t  QueryCurrentCallsListRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         QueryCurrentCallsListRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryCurrentCallsListRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryCurrentCallsListRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QueryCurrentCallsListRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_CURRENT_CALLS_LIST;
         QueryCurrentCallsListRequest.MessageHeader.MessageLength   = HFRM_QUERY_CURRENT_CALLS_LIST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryCurrentCallsListRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         QueryCurrentCallsListRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryCurrentCallsListRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_CURRENT_CALLS_LIST_RESPONSE_SIZE)
               ret_val = ((HFRM_Query_Current_Calls_List_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for setting the network     */
   /* operator format to long alphanumeric.  This function may only be  */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected Audio Gateway.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Set_Network_Operator_Selection_Format(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                                   ret_val;
   BTPM_Message_t                                       *ResponseMessage;
   HFRM_Set_Network_Operator_Selection_Format_Request_t  SetNetworkOperatorSelectionFormatRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetNetworkOperatorSelectionFormatRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetNetworkOperatorSelectionFormatRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetNetworkOperatorSelectionFormatRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SetNetworkOperatorSelectionFormatRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SET_NETWORK_OPERATOR_FORMAT;
         SetNetworkOperatorSelectionFormatRequest.MessageHeader.MessageLength   = HFRM_SET_NETWORK_OPERATOR_SELECTION_FORMAT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetNetworkOperatorSelectionFormatRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SetNetworkOperatorSelectionFormatRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetNetworkOperatorSelectionFormatRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SET_NETWORK_OPERATOR_SELECTION_FORMAT_RESPONSE_SIZE)
               ret_val = ((HFRM_Set_Network_Operator_Selection_Format_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for reading the network     */
   /* operator.  This function may only be performed by a Hands Free    */
   /* device with a valid service level connection.  This function      */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The network operator format must be set before querying  */
   /*          the current network operator.                            */
int _HFRM_Query_Remote_Network_Operator_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HFRM_Query_Network_Operator_Selection_Request_t  QueryNetworkOperatorSelectionFormatRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         QueryNetworkOperatorSelectionFormatRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryNetworkOperatorSelectionFormatRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryNetworkOperatorSelectionFormatRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QueryNetworkOperatorSelectionFormatRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_NETWORK_OPERATOR_SELECTION;
         QueryNetworkOperatorSelectionFormatRequest.MessageHeader.MessageLength   = HFRM_QUERY_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryNetworkOperatorSelectionFormatRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         QueryNetworkOperatorSelectionFormatRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryNetworkOperatorSelectionFormatRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_NETWORK_OPERATOR_SELECTION_RESPONSE_SIZE)
               ret_val = ((HFRM_Query_Network_Operator_Selection_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* extended error results reporting.  This function may only be      */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* accepts as its input parameter a BOOLEAN flag indicating whether  */
   /* the reporting should be enabled (TRUE) or disabled (FALSE).  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Enable_Remote_Extended_Error_Result(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableExtendedErrorResults)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   HFRM_Enable_Extended_Error_Result_Request_t  EnableExtendedErrorResultRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         EnableExtendedErrorResultRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         EnableExtendedErrorResultRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnableExtendedErrorResultRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         EnableExtendedErrorResultRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ENABLE_EXTENDED_ERROR_RESULT;
         EnableExtendedErrorResultRequest.MessageHeader.MessageLength   = HFRM_ENABLE_EXTENDED_ERROR_RESULT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableExtendedErrorResultRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         EnableExtendedErrorResultRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         EnableExtendedErrorResultRequest.EnableExtendedErrorResults    = EnableExtendedErrorResults;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableExtendedErrorResultRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_ENABLE_EXTENDED_ERROR_RESULT_RESPONSE_SIZE)
               ret_val = ((HFRM_Enable_Extended_Error_Result_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for retrieving the          */
   /* subscriber number information.  This function may only be         */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Query_Subscriber_Number_Information(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                                 ret_val;
   BTPM_Message_t                                     *ResponseMessage;
   HFRM_Query_Subscriber_Number_Information_Request_t  QuerySubscriberNumberInformationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         QuerySubscriberNumberInformationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QuerySubscriberNumberInformationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QuerySubscriberNumberInformationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QuerySubscriberNumberInformationRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_SUBSCRIBER_NUMBER_INFO;
         QuerySubscriberNumberInformationRequest.MessageHeader.MessageLength   = HFRM_QUERY_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QuerySubscriberNumberInformationRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         QuerySubscriberNumberInformationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QuerySubscriberNumberInformationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_SUBSCRIBER_NUMBER_INFORMATION_RESPONSE_SIZE)
               ret_val = ((HFRM_Query_Subscriber_Number_Information_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for retrieving the current  */
   /* response and hold status.  This function may only be performed by */
   /* a Hands Free device with a valid service level connection to a    */
   /* connected Audio Gateway.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Query_Response_Hold_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   HFRM_Query_Response_Hold_Status_Request_t  QueryResponseHoldStatusRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         QueryResponseHoldStatusRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryResponseHoldStatusRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryResponseHoldStatusRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QueryResponseHoldStatusRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_RESPONSE_HOLD_STATUS;
         QueryResponseHoldStatusRequest.MessageHeader.MessageLength   = HFRM_QUERY_RESPONSE_HOLD_STATUS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryResponseHoldStatusRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         QueryResponseHoldStatusRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryResponseHoldStatusRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_RESPONSE_HOLD_STATUS_RESPONSE_SIZE)
               ret_val = ((HFRM_Query_Response_Hold_Status_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for setting the state of an */
   /* incoming call.  This function may only be performed by a Hands    */
   /* Free unit with a valid service level connection to a remote Audio */
   /* Gateway.  This function accepts as its input parameter the call   */
   /* state to set as part of this message.  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Set_Incoming_Call_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   HFRM_Set_Incoming_Call_State_Request_t  SetIncomingCallStateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (CallState >= csHold) && (CallState <= csNone))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetIncomingCallStateRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetIncomingCallStateRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetIncomingCallStateRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SetIncomingCallStateRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SET_INCOMING_CALL_STATE;
         SetIncomingCallStateRequest.MessageHeader.MessageLength   = HFRM_SET_INCOMING_CALL_STATE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetIncomingCallStateRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SetIncomingCallStateRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetIncomingCallStateRequest.CallState                     = CallState;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetIncomingCallStateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SET_INCOMING_CALL_STATE_RESPONSE_SIZE)
               ret_val = ((HFRM_Set_Incoming_Call_State_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an arbitrary    */
   /* command to the remote Audio Gateway (i.e.  non Bluetooth Hands    */
   /* Free Profile command).  This function may only be performed by a  */
   /* Hands Free with a valid service level connection.  This function  */
   /* accepts as its input parameter a NULL terminated ASCII string that*/
   /* represents the arbitrary command to send.  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The Command string passed to this function *MUST* begin  */
   /*          with AT and *MUST* end with the a carriage return ('\r') */
   /*          if this is the first portion of an arbitrary command     */
   /*          that will span multiple writes.  Subsequent calls (until */
   /*          the actual status reponse is received) can begin with    */
   /*          any character, however, they must end with a carriage    */
   /*          return ('\r').                                           */
int _HFRM_Send_Arbitrary_Command(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryCommand)
{
   int                                    ret_val;
   unsigned int                           ArbitraryCommandLength;
   BTPM_Message_t                        *ResponseMessage;
   HFRM_Send_Arbitrary_Command_Request_t *SendArbitraryCommandRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ArbitraryCommand))
      {
         /* Get the string length of the arbitrary command string.      */
         ArbitraryCommandLength = BTPS_StringLength(ArbitraryCommand);

         /* Allocate memory to hold the message.                        */
         if((SendArbitraryCommandRequest = (HFRM_Send_Arbitrary_Command_Request_t *)BTPS_AllocateMemory(HFRM_SEND_ARBITRARY_COMMAND_REQUEST_SIZE(ArbitraryCommandLength + 1))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendArbitraryCommandRequest, 0, HFRM_SEND_ARBITRARY_COMMAND_REQUEST_SIZE(ArbitraryCommandLength + 1));

            SendArbitraryCommandRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendArbitraryCommandRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendArbitraryCommandRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            SendArbitraryCommandRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_COMMAND;
            SendArbitraryCommandRequest->MessageHeader.MessageLength   = HFRM_SEND_ARBITRARY_COMMAND_REQUEST_SIZE(ArbitraryCommandLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

            SendArbitraryCommandRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
            SendArbitraryCommandRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SendArbitraryCommandRequest->ArbitraryCommandLength        = ArbitraryCommandLength + 1;

            BTPS_MemCopy(SendArbitraryCommandRequest->ArbitraryCommand, ArbitraryCommand, ArbitraryCommandLength);

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendArbitraryCommandRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid.  If it is, go ahead and note the      */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_ARBITRARY_COMMAND_RESPONSE_SIZE)
                  ret_val = ((HFRM_Send_Arbitrary_Command_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendArbitraryCommandRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


   /* Send the list of supported codecs to a remote audio gateway.      */
   /* The EventCallback ID is returned from a successful call to        */
   /* HFRM_Register_Event_Callback(). RemoteDeviceAddress is the        */
   /* address of the remote audio gateway. NumberSupportedCodecs is the */
   /* number of codecs in the list. AvailableCodecList is the codec     */
   /* list. This function returns zero if successful or a negative code */
   /* in case of error.                                                 */
int _HFRM_Send_Available_Codec_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   HFRM_Send_Available_Codec_List_Request_t  SendAvailableCodecListRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberSupportedCodecs) && (NumberSupportedCodecs <= HFRE_MAX_SUPPORTED_CODECS) && (AvailableCodecList))
      {
         /* Build a Send Available Codec List Hands Free event message  */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&SendAvailableCodecListRequest, 0, HFRM_SEND_AVAILABLE_CODEC_LIST_REQUEST_SIZE);

         SendAvailableCodecListRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendAvailableCodecListRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendAvailableCodecListRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SendAvailableCodecListRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_AVAILABLE_CODEC_LIST;
         SendAvailableCodecListRequest.MessageHeader.MessageLength   = HFRM_SEND_AVAILABLE_CODEC_LIST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendAvailableCodecListRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SendAvailableCodecListRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendAvailableCodecListRequest.NumberSupportedCodecs         = NumberSupportedCodecs;

         BTPS_MemCopy(SendAvailableCodecListRequest.AvailableCodecList, AvailableCodecList, NumberSupportedCodecs);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendAvailableCodecListRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_AVAILABLE_CODEC_LIST_RESPONSE_SIZE)
               ret_val = ((HFRM_Send_Available_Codec_List_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway with a valid service level connection to a       */
   /* connected remote Hands Free device.  This function accepts as its */
   /* input parameters the number of indicators and list of name/value  */
   /* pairs for the indicators to be updated.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Update_Current_Control_Indicator_Status(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberUpdateIndicators, HFRE_Indicator_Update_t *UpdateIndicators)
{
   int                                             ret_val;
   unsigned int                                    IndicatorDescriptionLength;
   unsigned int                                    Index;
   BTPM_Message_t                                 *ResponseMessage;
   HFRE_Indicator_Update_t                        *CurrentIndicator;
   HFRM_Indicator_Update_List_Entry_t             *CurrentIndicatorUpdateEntry;
   HFRM_Update_Control_Indicator_Status_Request_t *UpdateControlIndicatorStatusRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberUpdateIndicators) && (UpdateIndicators))
      {
         /* Allocate memory to hold the message with the requested      */
         /* number of indicators.                                       */
         if((UpdateControlIndicatorStatusRequest = (HFRM_Update_Control_Indicator_Status_Request_t *)BTPS_AllocateMemory(HFRM_UPDATE_CONTROL_INDICATOR_STATUS_REQUEST_SIZE(NumberUpdateIndicators))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(UpdateControlIndicatorStatusRequest, 0, HFRM_UPDATE_CONTROL_INDICATOR_STATUS_REQUEST_SIZE(0));

            /* Initialize the return value to success so that we can    */
            /* determine if any of the Indicator Descriptors are of an  */
            /* invalid length.                                          */
            ret_val = 0;

            /* Loop through and format the passed in entries into the   */
            /* message buffer.                                          */
            CurrentIndicatorUpdateEntry = UpdateControlIndicatorStatusRequest->UpdateIndicatorsList;
            CurrentIndicator            = UpdateIndicators;
            for(Index=0;(Index<NumberUpdateIndicators)&&(!ret_val);Index++,CurrentIndicatorUpdateEntry++,CurrentIndicator++)
            {
               /* Verify that the description was specified.            */
               if(CurrentIndicator->IndicatorDescription)
               {
                  /* Get the length of the current indicator            */
                  /* description.                                       */
                  IndicatorDescriptionLength = BTPS_StringLength(CurrentIndicator->IndicatorDescription);
                  if(IndicatorDescriptionLength <= HFRE_CONTROL_INDICATOR_DESCRIPTION_LENGTH_MAXIMUM)
                  {
                     /* Initialize the current entry to all ZEROs.      */
                     BTPS_MemInitialize(CurrentIndicatorUpdateEntry, 0, sizeof(HFRM_Indicator_Update_List_Entry_t));

                     /* Set the Indicator entry to the current entry    */
                     /* that was passed in.                             */
                     CurrentIndicatorUpdateEntry->IndicatorUpdate                      = *CurrentIndicator;

                     /* Note that the Indicator Description in the      */
                     /* IndicatorUpdate member will be set to NULL.     */
                     CurrentIndicatorUpdateEntry->IndicatorUpdate.IndicatorDescription = NULL;

                     /* Copy in the Indicator Description and the       */
                     /* length.                                         */
                     CurrentIndicatorUpdateEntry->IndicatorDescriptionLength           = IndicatorDescriptionLength + 1;

                     BTPS_MemCopy(CurrentIndicatorUpdateEntry->IndicatorDescription, CurrentIndicator->IndicatorDescription, IndicatorDescriptionLength);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }

            /* Only continue if no error occurred while processing the  */
            /* input parameters.                                        */
            if(!ret_val)
            {
               UpdateControlIndicatorStatusRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               UpdateControlIndicatorStatusRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               UpdateControlIndicatorStatusRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               UpdateControlIndicatorStatusRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS;
               UpdateControlIndicatorStatusRequest->MessageHeader.MessageLength   = HFRM_UPDATE_CONTROL_INDICATOR_STATUS_REQUEST_SIZE(NumberUpdateIndicators) - BTPM_MESSAGE_HEADER_SIZE;

               UpdateControlIndicatorStatusRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               UpdateControlIndicatorStatusRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               UpdateControlIndicatorStatusRequest->NumberUpdateIndicators        = NumberUpdateIndicators;

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)UpdateControlIndicatorStatusRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_UPDATE_CONTROL_INDICATOR_STATUS_RESPONSE_SIZE)
                     ret_val = ((HFRM_Update_Control_Indicator_Status_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(UpdateControlIndicatorStatusRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway.  The function will initially set the specified  */
   /* indicator, then, if a valid service level connection exists and   */
   /* event reporting is activated (via the set remote event indicator  */
   /* event notification function by the remote device) an event        */
   /* notification will be sent to the remote device.  This function    */
   /* accepts as its input parameters the name of the indicator to be   */
   /* updated and the new indicator value.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Update_Current_Control_Indicator_Status_By_Name(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *IndicatorName, unsigned int IndicatorValue)
{
   int                                                     ret_val;
   unsigned int                                            IndicatorNameLength;
   BTPM_Message_t                                         *ResponseMessage;
   HFRM_Update_Control_Indicator_Status_By_Name_Request_t *UpdateIndicatorStatusByNameRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (IndicatorName))
      {
         /* Get the string length of the indicator name string.         */
         IndicatorNameLength = BTPS_StringLength(IndicatorName);

         /* Allocate memory to hold the message.                        */
         if((UpdateIndicatorStatusByNameRequest = (HFRM_Update_Control_Indicator_Status_By_Name_Request_t *)BTPS_AllocateMemory(HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_REQUEST_SIZE(IndicatorNameLength + 1))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(UpdateIndicatorStatusByNameRequest, 0, HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_REQUEST_SIZE(IndicatorNameLength + 1));

            UpdateIndicatorStatusByNameRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            UpdateIndicatorStatusByNameRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            UpdateIndicatorStatusByNameRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            UpdateIndicatorStatusByNameRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_UPDATE_INDICATOR_STATUS_BY_NAME;
            UpdateIndicatorStatusByNameRequest->MessageHeader.MessageLength   = HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_REQUEST_SIZE(IndicatorNameLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

            UpdateIndicatorStatusByNameRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
            UpdateIndicatorStatusByNameRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            UpdateIndicatorStatusByNameRequest->IndicatorNameLength           = IndicatorNameLength + 1;
            UpdateIndicatorStatusByNameRequest->IndicatorValue                = IndicatorValue;

            BTPS_MemCopy(UpdateIndicatorStatusByNameRequest->IndicatorName, IndicatorName, IndicatorNameLength);

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)UpdateIndicatorStatusByNameRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid.  If it is, go ahead and note the      */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_UPDATE_CONTROL_INDICATOR_STATUS_BY_NAME_RESPONSE_SIZE)
                  ret_val = ((HFRM_Update_Control_Indicator_Status_By_Name_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(UpdateIndicatorStatusByNameRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a call waiting           */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call waiting       */
   /* notification enabled and have a valid service level connection to */
   /* a connected remote Hands Free device.  This function accepts as   */
   /* its input parameter the phone number of the incoming call, if a   */
   /* number is available.  This parameter should be a pointer to a NULL*/
   /* string and its length *MUST* be between the values of:            */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * It is valid to either pass a NULL for the PhoneNumber    */
   /*          parameter or a blank string to specify that there is no  */
   /*          phone number present.                                    */
int _HFRM_Send_Call_Waiting_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                                            ret_val;
   unsigned int                                   PhoneNumberLength;
   BTPM_Message_t                                *ResponseMessage;
   HFRM_Send_Call_Waiting_Notification_Request_t *SendCallWaitingNotificationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber))
      {
         /* Get the string length of the phone number string.           */
         PhoneNumberLength = BTPS_StringLength(PhoneNumber);

         /* Verify that the length of the phone number to dial is valid.*/
         if((PhoneNumberLength >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (PhoneNumberLength <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
         {
            /* Allocate memory to hold the message.                     */
            if((SendCallWaitingNotificationRequest = (HFRM_Send_Call_Waiting_Notification_Request_t *)BTPS_AllocateMemory(HFRM_SEND_CALL_WAITING_NOTIFICATION_REQUEST_SIZE(PhoneNumberLength + 1))) != NULL)
            {
               /* All that we really need to do is to build the message */
               /* and send it to the server.                            */
               BTPS_MemInitialize(SendCallWaitingNotificationRequest, 0, HFRM_SEND_CALL_WAITING_NOTIFICATION_REQUEST_SIZE(PhoneNumberLength + 1));

               SendCallWaitingNotificationRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               SendCallWaitingNotificationRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SendCallWaitingNotificationRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               SendCallWaitingNotificationRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_CALL_WAITING_NOTIFICATION;
               SendCallWaitingNotificationRequest->MessageHeader.MessageLength   = HFRM_SEND_CALL_WAITING_NOTIFICATION_REQUEST_SIZE(PhoneNumberLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

               SendCallWaitingNotificationRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               SendCallWaitingNotificationRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               SendCallWaitingNotificationRequest->PhoneNumberLength             = PhoneNumberLength + 1;

               BTPS_MemCopy(SendCallWaitingNotificationRequest->PhoneNumber, PhoneNumber, PhoneNumberLength);

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendCallWaitingNotificationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_CALL_WAITING_NOTIFICATION_RESPONSE_SIZE)
                     ret_val = ((HFRM_Send_Call_Waiting_Notification_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }

               /* Free the memory that was allocated for the packet.    */
               BTPS_FreeMemory(SendCallWaitingNotificationRequest);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending call line identification */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call line          */
   /* identification notification enabled and have a valid service level*/
   /* connection to a connected remote Hands Free device.  This function*/
   /* accepts as its input parameters the phone number of the incoming  */
   /* call.  This parameter should be a pointer to a NULL terminated    */
   /* string and its length *MUST* be between the values of:            */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function return zero if successful or a negative return error*/
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Call_Line_Identification_Notification(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                                                        ret_val;
   unsigned int                                               PhoneNumberLength;
   BTPM_Message_t                                            *ResponseMessage;
   HFRM_Send_Call_Line_Identification_Notification_Request_t *SendCallLineIdentificationNotificationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber))
      {
         /* Get the string length of the phone number string.           */
         PhoneNumberLength = BTPS_StringLength(PhoneNumber);

         /* Verify that the length of the phone number to dial is valid.*/
         if((PhoneNumberLength >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (PhoneNumberLength <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
         {
            /* Allocate memory to hold the message.                     */
            if((SendCallLineIdentificationNotificationRequest = (HFRM_Send_Call_Line_Identification_Notification_Request_t *)BTPS_AllocateMemory(HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE(PhoneNumberLength + 1))) != NULL)
            {
               /* All that we really need to do is to build the message */
               /* and send it to the server.                            */
               BTPS_MemInitialize(SendCallLineIdentificationNotificationRequest, 0, HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE(PhoneNumberLength + 1));

               SendCallLineIdentificationNotificationRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               SendCallLineIdentificationNotificationRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SendCallLineIdentificationNotificationRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               SendCallLineIdentificationNotificationRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_CALL_LINE_ID_NOTIFICATION;
               SendCallLineIdentificationNotificationRequest->MessageHeader.MessageLength   = HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_REQUEST_SIZE(PhoneNumberLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

               SendCallLineIdentificationNotificationRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               SendCallLineIdentificationNotificationRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               SendCallLineIdentificationNotificationRequest->PhoneNumberLength             = PhoneNumberLength + 1;

               BTPS_MemCopy(SendCallLineIdentificationNotificationRequest->PhoneNumber, PhoneNumber, PhoneNumberLength);

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendCallLineIdentificationNotificationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_CALL_LINE_IDENTIFICATION_NOTIFICATION_RESPONSE_SIZE)
                     ret_val = ((HFRM_Send_Call_Line_Identification_Notification_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }

               /* Free the memory that was allocated for the packet.    */
               BTPS_FreeMemory(SendCallLineIdentificationNotificationRequest);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for sending a ring indication to a   */
   /* remote Hands Free unit.  This function may only be performed by   */
   /* Audio Gateways for which a valid service level connection to a    */
   /* connected remote Hands Free device exists.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Ring_Indication(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   HFRM_Ring_Indication_Request_t  RingIndicationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         RingIndicationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         RingIndicationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         RingIndicationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         RingIndicationRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_RING_INDICATION;
         RingIndicationRequest.MessageHeader.MessageLength   = HFRM_RING_INDICATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         RingIndicationRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         RingIndicationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RingIndicationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_RING_INDICATION_RESPONSE_SIZE)
               ret_val = ((HFRM_Ring_Indication_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for enabling or disabling in-band    */
   /* ring tone capabilities for a connected Hands Free device.  This   */
   /* function may only be performed by Audio Gateways for which a valid*/
   /* service kevel connection exists.  This function may only be used  */
   /* to enable in-band ring tone capabilities if the local Audio       */
   /* Gateway supports this feature.  This function accepts as its input*/
   /* parameter a BOOLEAN flag specifying if this is a call to Enable or*/
   /* Disable this functionality.  This function returns zero if        */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful call*/
   /*          to the HFRM_Register_Event_Callback() function           */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t EnableInBandRing)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HFRM_Enable_In_Band_Ring_Tone_Setting_Request_t  EnableInBandRingToneSettingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         EnableInBandRingToneSettingRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         EnableInBandRingToneSettingRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnableInBandRingToneSettingRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         EnableInBandRingToneSettingRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ENABLE_IN_BAND_RING_TONE_SETTING;
         EnableInBandRingToneSettingRequest.MessageHeader.MessageLength   = HFRM_ENABLE_IN_BAND_RING_TONE_SETTING_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableInBandRingToneSettingRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         EnableInBandRingToneSettingRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         EnableInBandRingToneSettingRequest.EnableInBandRing              = EnableInBandRing;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableInBandRingToneSettingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_ENABLE_IN_BAND_RING_TONE_SETTING_RESPONSE_SIZE)
               ret_val = ((HFRM_Enable_In_Band_Ring_Tone_Setting_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for responding to a request that was */
   /* received for a phone number to be associated with a unique voice  */
   /* tag by a remote Hands Free device.  This function may only be     */
   /* performed by Audio Gateways that have received a voice tag request*/
   /* Indication.  This function accepts as its input parameter the     */
   /* phone number to be associated with the voice tag.  If the request */
   /* is accepted, the phone number Parameter string length *MUST* be   */
   /* between the values:                                               */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* If the caller wishes to reject the request, the phone number      */
   /* parameter should be set to NULL to indicate this.  This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Voice_Tag_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *PhoneNumber)
{
   int                                ret_val;
   unsigned int                       PhoneNumberLength;
   BTPM_Message_t                    *ResponseMessage;
   HFRM_Voice_Tag_Response_Request_t *VoiceTagResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (PhoneNumber))
      {
         /* Get the string length of the phone number string.           */
         PhoneNumberLength = BTPS_StringLength(PhoneNumber);

         /* Verify that the length of the phone number to dial is valid.*/
         if((PhoneNumberLength >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (PhoneNumberLength <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
         {
            /* Allocate memory to hold the message.                     */
            if((VoiceTagResponseRequest = (HFRM_Voice_Tag_Response_Request_t *)BTPS_AllocateMemory(HFRM_VOICE_TAG_RESPONSE_REQUEST_SIZE(PhoneNumberLength + 1))) != NULL)
            {
               /* All that we really need to do is to build the message */
               /* and send it to the server.                            */
               BTPS_MemInitialize(VoiceTagResponseRequest, 0, HFRM_VOICE_TAG_RESPONSE_REQUEST_SIZE(PhoneNumberLength + 1));

               VoiceTagResponseRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               VoiceTagResponseRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               VoiceTagResponseRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               VoiceTagResponseRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_VOICE_TAG_RESPONSE;
               VoiceTagResponseRequest->MessageHeader.MessageLength   = HFRM_VOICE_TAG_RESPONSE_REQUEST_SIZE(PhoneNumberLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

               VoiceTagResponseRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               VoiceTagResponseRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               VoiceTagResponseRequest->PhoneNumberLength             = PhoneNumberLength + 1;

               BTPS_MemCopy(VoiceTagResponseRequest->PhoneNumber, PhoneNumber, PhoneNumberLength);

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)VoiceTagResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_VOICE_TAG_RESPONSE_RESPONSE_SIZE)
                     ret_val = ((HFRM_Voice_Tag_Response_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }

               /* Free the memory that was allocated for the packet.    */
               BTPS_FreeMemory(VoiceTagResponseRequest);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the current     */
   /* calls list entries to a remote Hands Free device.  This function  */
   /* may only be performed by Audio Gateways that have received a      */
   /* request to query the remote current calls list.  This function    */
   /* accepts as its input parameters the list of current call entries  */
   /* to be sent and length of the list.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Current_Calls_List(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRE_Current_Call_List_Entry_t *CurrentCallListEntry)
{
   int                                        ret_val;
   unsigned int                               Length;
   unsigned int                               Index;
   BTPM_Message_t                            *ResponseMessage;
   HFRM_Call_List_List_Entry_v2_t            *MessageCurrentCallListEntry;
   HFRE_Current_Call_List_Entry_t            *CurrentCall;
   HFRM_Send_Current_Calls_List_Request_v2_t *SendCurrentCallsListRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!NumberListEntries) || ((NumberListEntries) && (CurrentCallListEntry))))
      {
         /* Allocate memory to hold the message with the requested      */
         /* number of indicators.                                       */
         if((SendCurrentCallsListRequest = (HFRM_Send_Current_Calls_List_Request_v2_t *)BTPS_AllocateMemory(HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V2_SIZE(NumberListEntries))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendCurrentCallsListRequest, 0, HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V2_SIZE(0));

            /* Initialize the return value to success so that we can    */
            /* determine if any of the Indicator Descriptors are of an  */
            /* invalid length.                                          */
            ret_val = 0;

            /* Loop through and format the passed in entries into the   */
            /* message buffer.                                          */
            MessageCurrentCallListEntry = SendCurrentCallsListRequest->CallListEntryList;
            CurrentCall                 = CurrentCallListEntry;
            for(Index=0;(Index<NumberListEntries) && (!ret_val);Index++, MessageCurrentCallListEntry++, CurrentCall++)
            {
               /* Initialize the current entry to all ZEROs.            */
               BTPS_MemInitialize(MessageCurrentCallListEntry, 0, sizeof(HFRM_Call_List_List_Entry_v2_t));

               /* Set the Indicator entry to the current entry that was */
               /* passed in.                                            */
               MessageCurrentCallListEntry->Index         = CurrentCall->Index;
               MessageCurrentCallListEntry->CallDirection = CurrentCall->CallDirection;
               MessageCurrentCallListEntry->CallStatus    = CurrentCall->CallStatus;
               MessageCurrentCallListEntry->CallMode      = CurrentCall->CallMode;
               MessageCurrentCallListEntry->Multiparty    = CurrentCall->Multiparty;
               MessageCurrentCallListEntry->NumberFormat  = CurrentCall->NumberFormat;

               /* Copy the phone number, if it is available.            */
               if(CurrentCall->PhoneNumber)
               {
                  /* Get the length of the current phone number.        */
                  Length = BTPS_StringLength(CurrentCall->PhoneNumber);
                  if((Length >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (Length <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
                  {
                     /* Copy the phone number into the message.         */
                     MessageCurrentCallListEntry->Flags |= HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONE_NUMBER_VALID;

                     BTPS_MemCopy(MessageCurrentCallListEntry->PhoneNumber, CurrentCall->PhoneNumber, Length);

                     MessageCurrentCallListEntry->PhoneNumber[Length] = '\0';
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }

               /* Copy the phonebook name, if it is available.          */
               if((!ret_val) && (CurrentCall->PhonebookName))
               {
                  /* Get the length of the phonebook name.              */
                  Length = BTPS_StringLength(CurrentCall->PhonebookName);
                  if((Length >= HFRE_PHONEBOOK_NAME_LENGTH_MINIMUM) && (Length <= HFRE_PHONEBOOK_NAME_LENGTH_MAXIMUM))
                  {
                     /* Copy the phonebook name into the message.       */
                     MessageCurrentCallListEntry->Flags |= HFRM_CALL_LIST_LIST_ENTRY_V2_FLAG_PHONEBOOK_NAME_VALID;

                     BTPS_MemCopy(MessageCurrentCallListEntry->PhonebookName, CurrentCall->PhonebookName, Length);

                     MessageCurrentCallListEntry->PhonebookName[Length] = '\0';
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
            }

            /* Only continue if no error occurred while processing the  */
            /* input parameters.                                        */
            if(!ret_val)
            {
               SendCurrentCallsListRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               SendCurrentCallsListRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SendCurrentCallsListRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               SendCurrentCallsListRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_CURRENT_CALLS_LIST_V2;
               SendCurrentCallsListRequest->MessageHeader.MessageLength   = HFRM_SEND_CURRENT_CALLS_LIST_REQUEST_V2_SIZE(NumberListEntries) - BTPM_MESSAGE_HEADER_SIZE;

               SendCurrentCallsListRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               SendCurrentCallsListRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               SendCurrentCallsListRequest->NumberCallListEntries         = NumberListEntries;

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendCurrentCallsListRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_CURRENT_CALLS_LIST_RESPONSE_SIZE)
                     ret_val = ((HFRM_Send_Current_Calls_List_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendCurrentCallsListRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the network     */
   /* operator.  This function may only be performed by Audio Gateways  */
   /* that have received a request to query the remote network operator */
   /* selection.  This function accepts as input the current network    */
   /* mode and the current network operator.  The network operator      */
   /* should be expressed as a NULL terminated ASCII string (if         */
   /* specified) and must have a length less than:                      */
   /*                                                                   */
   /*    HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM                           */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * It is valid to either pass a NULL for the NetworkOperator*/
   /*          parameter or a blank string to specify that there is no  */
   /*          network operator present.                                */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Network_Operator_Selection(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NetworkMode, char *NetworkOperator)
{
   int                                             ret_val;
   unsigned int                                    NetworkOperatorLength;
   BTPM_Message_t                                 *ResponseMessage;
   HFRM_Send_Network_Operator_Selection_Request_t *SendNetworkOperatorSelectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NetworkOperator))
      {
         /* Get the string length of the network operator string.       */
         NetworkOperatorLength = BTPS_StringLength(NetworkOperator);

         /* Verify that the length of the network operator is valid.    */
         if((NetworkOperatorLength >= HFRE_NETWORK_OPERATOR_LENGTH_MINIMUM) && (NetworkOperatorLength <= HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM))
         {
            /* Allocate memory to hold the message.                     */
            if((SendNetworkOperatorSelectionRequest = (HFRM_Send_Network_Operator_Selection_Request_t *)BTPS_AllocateMemory(HFRM_SEND_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE(NetworkOperatorLength + 1))) != NULL)
            {
               /* All that we really need to do is to build the message */
               /* and send it to the server.                            */
               BTPS_MemInitialize(SendNetworkOperatorSelectionRequest, 0, HFRM_SEND_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE(NetworkOperatorLength + 1));

               SendNetworkOperatorSelectionRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               SendNetworkOperatorSelectionRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SendNetworkOperatorSelectionRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               SendNetworkOperatorSelectionRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_NETWORK_OPERATOR_SELECTION;
               SendNetworkOperatorSelectionRequest->MessageHeader.MessageLength   = HFRM_SEND_NETWORK_OPERATOR_SELECTION_REQUEST_SIZE(NetworkOperatorLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

               SendNetworkOperatorSelectionRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
               SendNetworkOperatorSelectionRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               SendNetworkOperatorSelectionRequest->NetworkOperatorLength         = NetworkOperatorLength + 1;
               SendNetworkOperatorSelectionRequest->NetworkMode                   = NetworkMode;

               BTPS_MemCopy(SendNetworkOperatorSelectionRequest->NetworkOperator, NetworkOperator, NetworkOperatorLength);

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendNetworkOperatorSelectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_NETWORK_OPERATOR_SELECTION_RESPONSE_SIZE)
                     ret_val = ((HFRM_Send_Network_Operator_Selection_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }

               /* Free the memory that was allocated for the packet.    */
               BTPS_FreeMemory(SendNetworkOperatorSelectionRequest);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending extended error  */
   /* results.  This function may only be performed by an Audio Gateway */
   /* with a valid service level connection.  This function accepts as  */
   /* its input parameter the result code to send as part of the error  */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Extended_Error_Result(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ResultCode)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   HFRM_Send_Extended_Error_Result_Request_t  SendExtendedErrorResultRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SendExtendedErrorResultRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendExtendedErrorResultRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendExtendedErrorResultRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SendExtendedErrorResultRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_EXTENDED_ERROR_RESULT;
         SendExtendedErrorResultRequest.MessageHeader.MessageLength   = HFRM_SEND_EXTENDED_ERROR_RESULT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendExtendedErrorResultRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SendExtendedErrorResultRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendExtendedErrorResultRequest.ResultCode                    = ResultCode;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendExtendedErrorResultRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_EXTENDED_ERROR_RESULT_RESPONSE_SIZE)
               ret_val = ((HFRM_Send_Extended_Error_Result_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending subscriber      */
   /* number information.  This function may only be performed by an    */
   /* Audio Gateway that has received a request to query the subscriber */
   /* number information.  This function accepts as its input parameters*/
   /* the number of subscribers followed by a list of subscriber        */
   /* numbers.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Subscriber_Number_Information(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int NumberListEntries, HFRM_Subscriber_Number_Information_t *SubscriberNumberList)
{
   int                                                ret_val;
   unsigned int                                       PhoneNumberLength;
   unsigned int                                       Index;
   BTPM_Message_t                                    *ResponseMessage;
   HFRM_Subscriber_Number_Information_t              *CurrentSubscriber;
   HFRM_Subscriber_Information_List_Entry_t          *MessageSubscriberInformationListEntry;
   HFRM_Send_Subscriber_Number_Information_Request_t *SendSubscriberNumberInformationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberListEntries) && (SubscriberNumberList))
      {
         /* Allocate memory to hold the message with the requested      */
         /* number of indicators.                                       */
         if((SendSubscriberNumberInformationRequest = (HFRM_Send_Subscriber_Number_Information_Request_t *)BTPS_AllocateMemory(HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE(NumberListEntries))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendSubscriberNumberInformationRequest, 0, HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE(0));

            /* Initialize the return value to success so that we can    */
            /* determine if any of the Indicator Descriptors are of an  */
            /* invalid length.                                          */
            ret_val = 0;

            /* Loop through and format the passed in entries into the   */
            /* message buffer.                                          */
            MessageSubscriberInformationListEntry = SendSubscriberNumberInformationRequest->SubscriberInformationList;
            CurrentSubscriber                     = SubscriberNumberList;
            for(Index=0;(Index<NumberListEntries)&&(!ret_val);Index++,MessageSubscriberInformationListEntry++,CurrentSubscriber++)
            {
               /* Verify that the description was specified.            */
               if(CurrentSubscriber->PhoneNumber)
               {
                  /* Get the length of the current phone number string. */
                  PhoneNumberLength = BTPS_StringLength(CurrentSubscriber->PhoneNumber);
                  if((PhoneNumberLength >= HFRE_PHONE_NUMBER_LENGTH_MINIMUM) && (PhoneNumberLength <= HFRE_PHONE_NUMBER_LENGTH_MAXIMUM))
                  {
                     /* Initialize the current entry to all ZEROs.      */
                     BTPS_MemInitialize(MessageSubscriberInformationListEntry, 0, sizeof(HFRM_Subscriber_Information_List_Entry_t));

                     /* Set the Indicator entry to the current entry    */
                     /* that was passed in.                             */
                     MessageSubscriberInformationListEntry->SubscriberNumberInformationEntry             = *CurrentSubscriber;

                     /* Note that the Indicator Description in the      */
                     /* IndicatorUpdate member will be set to NULL.     */
                     MessageSubscriberInformationListEntry->SubscriberNumberInformationEntry.PhoneNumber = NULL;

                     /* Copy in the Indicator Description and the       */
                     /* length.                                         */
                     MessageSubscriberInformationListEntry->PhoneNumberLength = PhoneNumberLength + 1;

                     BTPS_MemCopy(MessageSubscriberInformationListEntry->PhoneNumber, CurrentSubscriber->PhoneNumber, PhoneNumberLength);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }

            /* Only continue if no error occurred while processing the  */
            /* input parameters.                                        */
            if(!ret_val)
            {
               SendSubscriberNumberInformationRequest->MessageHeader.AddressID            = MSG_GetServerAddressID();
               SendSubscriberNumberInformationRequest->MessageHeader.MessageID            = MSG_GetNextMessageID();
               SendSubscriberNumberInformationRequest->MessageHeader.MessageGroup         = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
               SendSubscriberNumberInformationRequest->MessageHeader.MessageFunction      = HFRM_MESSAGE_FUNCTION_SEND_SUBSCRIBER_NUMBER_INFO;
               SendSubscriberNumberInformationRequest->MessageHeader.MessageLength        = HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_REQUEST_SIZE(NumberListEntries) - BTPM_MESSAGE_HEADER_SIZE;

               SendSubscriberNumberInformationRequest->ControlEventsHandlerID             = HandsFreeManagerEventCallbackID;
               SendSubscriberNumberInformationRequest->RemoteDeviceAddress                = RemoteDeviceAddress;
               SendSubscriberNumberInformationRequest->NumberSubscriberInformationEntries = NumberListEntries;

               /* Message has been formatted, go ahead and send it off. */
               if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendSubscriberNumberInformationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
               {
                  /* Response received, go ahead and see if the return  */
                  /* value is valid.  If it is, go ahead and note the   */
                  /* returned status.                                   */
                  if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_SUBSCRIBER_NUMBER_INFORMATION_RESPONSE_SIZE)
                     ret_val = ((HFRM_Send_Subscriber_Number_Information_Response_t *)ResponseMessage)->Status;
                  else
                     ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                  /* All finished with the message, go ahead and free   */
                  /* it.                                                */
                  MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
               }
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendSubscriberNumberInformationRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending information     */
   /* about the incoming call state.  This function may only be         */
   /* performed by an Audio Gateway that has a valid service level      */
   /* connection to a remote Hands Free device.  This function accepts  */
   /* as its input parameter the call state to set as part of this      */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Incoming_Call_State(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Call_State_t CallState)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HFRM_Send_Incoming_Call_State_Request_t  SendIncomingCallStateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (CallState >= csHold) && (CallState <= csNone))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SendIncomingCallStateRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendIncomingCallStateRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendIncomingCallStateRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SendIncomingCallStateRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_INCOMING_CALL_STATE;
         SendIncomingCallStateRequest.MessageHeader.MessageLength   = HFRM_SEND_INCOMING_CALL_STATE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendIncomingCallStateRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SendIncomingCallStateRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendIncomingCallStateRequest.CallState                     = CallState;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendIncomingCallStateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_INCOMING_CALL_STATE_RESPONSE_SIZE)
               ret_val = ((HFRM_Send_Incoming_Call_State_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a terminating   */
   /* response code from an Audio Gateway to a remote Hands Free device.*/
   /* This function may only be performed by an Audio Gateway that has a*/
   /* valid service level connection to a remote Hands Free device.     */
   /* This function can be called in any context where a normal Audio   */
   /* Gateway response function is called if the intention is to        */
   /* generate an error in response to the request.  It also must be    */
   /* called after certain requests that previously automatically       */
   /* generated an OK response.  In general, either this function or an */
   /* explicit response must be called after each request to the Audio  */
   /* Gateway.  This function accepts as its input parameters the type  */
   /* of result to return in the terminating response and, if the result*/
   /* type indicates an extended error code value, the error code.  This*/
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Send_Terminating_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, HFRE_Extended_Result_t ResultType, unsigned int ResultValue)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   HFRM_Send_Terminating_Response_Request_t  SendTerminatingResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ResultType >= erOK) && (ResultType <= erResultCode))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SendTerminatingResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendTerminatingResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendTerminatingResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SendTerminatingResponseRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_TERMINATING_RESPONSE;
         SendTerminatingResponseRequest.MessageHeader.MessageLength   = HFRM_SEND_TERMINATING_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendTerminatingResponseRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SendTerminatingResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SendTerminatingResponseRequest.ResultType                    = ResultType;
         SendTerminatingResponseRequest.ResultValue                   = ResultValue;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendTerminatingResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_TERMINATING_RESPONSE_RESPONSE_SIZE)
               ret_val = ((HFRM_Send_Terminating_Response_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for enabling the processing */
   /* of arbitrary commands from a remote Hands Free device.  Once this */
   /* function is called the hetHFRArbitraryCommandIndication event will*/
   /* be dispatched when an arbitrary command is received (i.e. a non   */
   /* Hands Free profile command).  If this function is not called, the */
   /* Audio Gateway will silently respond to any arbitrary commands with*/
   /* an error response ("ERROR").  If support is enabled, then the     */
   /* caller is responsible for responding TO ALL arbitrary command     */
   /* indications (hetHFRArbitraryCommandIndication).  If the arbitrary */
   /* command is not supported, then the caller should simply respond   */
   /* with:                                                             */
   /*                                                                   */
   /*   HFRM_Send_Terminating_Response()                                */
   /*                                                                   */
   /* specifying the erError response. This function returns zero if    */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * Once arbitrary command processing is enabled for an      */
   /*          Audio Gateway it cannot be disabled.                     */
   /* * NOTE * The default value is disabled (i.e. the                  */
   /*          hetHFRArbitraryCommandIndication will NEVER be dispatched*/
   /*          and the Audio Gateway will always respond with an error  */
   /*          response ("ERROR") when an arbitrary command is received.*/
   /* * NOTE * If support is enabled, the caller is guaranteed that a   */
   /*          hetHFRArbitraryCommandIndication will NOT be dispatched  */
   /*          before a service level indication is present. If an      */
   /*          arbitrary command is received, it will be responded with */
   /*          silently with an error response ("ERROR").               */
   /* * NOTE * This function is not applicable to Hands Free devices,   */
   /*          as Hands Free devices will always receive the            */
   /*          hetHFRArbitraryResponseIndication.  No action is required*/
   /*          and the event can simply be ignored.                     */
int _HFRM_Enable_Arbitrary_Command_Processing(unsigned int HandsFreeManagerEventCallbackID)
{
   int                                                 ret_val;
   BTPM_Message_t                                     *ResponseMessage;
   HFRM_Enable_Arbitrary_Command_Processing_Request_t  EnableArbitraryCommandProcessingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(HandsFreeManagerEventCallbackID)
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         EnableArbitraryCommandProcessingRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         EnableArbitraryCommandProcessingRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnableArbitraryCommandProcessingRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         EnableArbitraryCommandProcessingRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_ENABLE_ARBITRARY_CMD_PROCESSING;
         EnableArbitraryCommandProcessingRequest.MessageHeader.MessageLength   = HFRM_ENABLE_ARBITRARY_COMMAND_PROCESSING_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableArbitraryCommandProcessingRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableArbitraryCommandProcessingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_ENABLE_ARBITRARY_COMMAND_PROCESSING_RESPONSE_SIZE)
               ret_val = ((HFRM_Enable_Arbitrary_Command_Processing_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an arbitrary    */
   /* response to the remote Hands Free device (i.e. non Bluetooth      */
   /* Hands Free Profile response) - either solicited or non-solicited. */
   /* This function may only be performed by an Audio Gateway with a    */
   /* valid service level connection. This function accepts as its      */
   /* input parameter a NULL terminated ASCII string that represents    */
   /* the arbitrary response to send. This function returns zero if     */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
   /* * NOTE * The Response string passed to this function *MUST* begin */
   /*          with a carriage return/line feed ("\r\n").               */
int _HFRM_Send_Arbitrary_Response(unsigned int HandsFreeManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress, char *ArbitraryResponse)
{
   int                                     ret_val;
   unsigned int                            ArbitraryResponseLength;
   BTPM_Message_t                         *ResponseMessage;
   HFRM_Send_Arbitrary_Response_Request_t *SendArbitraryResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ArbitraryResponse))
      {
         /* Get the string length of the arbitrary response string.     */
         ArbitraryResponseLength = BTPS_StringLength(ArbitraryResponse);

         /* Allocate memory to hold the message.                        */
         if((SendArbitraryResponseRequest = (HFRM_Send_Arbitrary_Response_Request_t *)BTPS_AllocateMemory(HFRM_SEND_ARBITRARY_RESPONSE_REQUEST_SIZE(ArbitraryResponseLength + 1))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendArbitraryResponseRequest, 0, HFRM_SEND_ARBITRARY_RESPONSE_REQUEST_SIZE(0));

            SendArbitraryResponseRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendArbitraryResponseRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendArbitraryResponseRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            SendArbitraryResponseRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_ARBITRARY_RESPONSE;
            SendArbitraryResponseRequest->MessageHeader.MessageLength   = HFRM_SEND_ARBITRARY_RESPONSE_REQUEST_SIZE(ArbitraryResponseLength + 1) - BTPM_MESSAGE_HEADER_SIZE;

            SendArbitraryResponseRequest->ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
            SendArbitraryResponseRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SendArbitraryResponseRequest->ArbitraryResponseLength       = ArbitraryResponseLength + 1;

            BTPS_MemCopy(SendArbitraryResponseRequest->ArbitraryResponse, ArbitraryResponse, ArbitraryResponseLength);

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendArbitraryResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid.  If it is, go ahead and note the      */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SEND_ARBITRARY_RESPONSE_RESPONSE_SIZE)
                  ret_val = ((HFRM_Send_Arbitrary_Response_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendArbitraryResponseRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Hands Free device for which a valid  */
   /* service level connection Exists.  This function accepts as its    */
   /* input parameter the connection type indicating which connection   */
   /* will process the command.  This function returns zero if          */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Setup_Audio_Connection(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   HFRM_Setup_Audio_Connection_Request_t  SetupAudioConnectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && ((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         SetupAudioConnectionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetupAudioConnectionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetupAudioConnectionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         SetupAudioConnectionRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SETUP_AUDIO_CONNECTION;
         SetupAudioConnectionRequest.MessageHeader.MessageLength   = HFRM_SETUP_AUDIO_CONNECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetupAudioConnectionRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         SetupAudioConnectionRequest.ConnectionType                = ConnectionType;
         SetupAudioConnectionRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetupAudioConnectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_SETUP_AUDIO_CONNECTION_RESPONSE_SIZE)
               ret_val = ((HFRM_Setup_Audio_Connection_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HFRM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Hands   */
   /* Free device.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The HandsFreeManagerEventCallbackID parameter *MUST* be  */
   /*          the callback ID that was registered via a successful     */
   /*          call to the HFRM_Register_Event_Callback() function      */
   /*          (specifying it as a control callback for the specified   */
   /*          service type).  There can only be a single control       */
   /*          callback registered for each service type in the system. */
int _HFRM_Release_Audio_Connection(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HFRM_Release_Audio_Connection_Request_t  ReleaseAudioConnectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((HandsFreeManagerEventCallbackID) && ((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Speaker Gain*/
         /* message and send it to the server.                          */
         ReleaseAudioConnectionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ReleaseAudioConnectionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ReleaseAudioConnectionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         ReleaseAudioConnectionRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_RELEASE_AUDIO_CONNECTION;
         ReleaseAudioConnectionRequest.MessageHeader.MessageLength   = HFRM_RELEASE_AUDIO_CONNECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ReleaseAudioConnectionRequest.ControlEventsHandlerID        = HandsFreeManagerEventCallbackID;
         ReleaseAudioConnectionRequest.ConnectionType                = ConnectionType;
         ReleaseAudioConnectionRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ReleaseAudioConnectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_RELEASE_AUDIO_CONNECTION_RESPONSE_SIZE)
               ret_val = ((HFRM_Release_Audio_Connection_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Hands Free Manager Data Handler ID           */
   /* (registered via call to the HFRM_Register_Data_Event_Callback()   */
   /* function), followed by the the connection type indicating which   */
   /* connection will transmit the audio data, the length (in Bytes) of */
   /* the audio data to send, and a pointer to the audio data to send to*/
   /* the remote dntity.  This function returns zero if successful or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function is only applicable for Bluetooth devices   */
   /*          that are configured to support packetized SCO audio.     */
   /*          This function will have no effect on Bluetooth devices   */
   /*          that are configured to process SCO audio via hardare     */
   /*          codec.                                                   */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to process the audio data themselves (as */
   /*          opposed to having the hardware process the audio data    */
   /*          via a hardware codec.                                    */
   /* * NOTE * The data that is sent *MUST* be formatted in the correct */
   /*          SCO format that is expected by the device.               */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _HFRM_Send_Audio_Data(unsigned int HandsFreeManagerDataEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData)
{
   int                             ret_val;
   HFRM_Send_Audio_Data_Request_t *SendAudioDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerDataEventCallbackID) && ((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway)) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (AudioDataLength) && (AudioData))
      {
         /* Allocate memory to hold the message.                        */
         if((SendAudioDataRequest = (HFRM_Send_Audio_Data_Request_t *)BTPS_AllocateMemory(HFRM_SEND_AUDIO_DATA_REQUEST_SIZE(AudioDataLength))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendAudioDataRequest, 0, HFRM_SEND_AUDIO_DATA_REQUEST_SIZE(0));

            SendAudioDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendAudioDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendAudioDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
            SendAudioDataRequest->MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_SEND_AUDIO_DATA;
            SendAudioDataRequest->MessageHeader.MessageLength   = HFRM_SEND_AUDIO_DATA_REQUEST_SIZE(AudioDataLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendAudioDataRequest->DataEventsHandlerID           = HandsFreeManagerDataEventCallbackID;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the HFRE Manager     */
   /* Service.  This Callback will be dispatched by the HFRE Manager    */
   /* when various HFRE Manager events occur.  This function accepts as */
   /* its parameters the ConnectionType of the type of connection to    */
   /* register for events, and a boolean that specifies if this is the  */
   /* control callback (there can only be one control callback in the   */
   /* system).  This function returns a non-zero value if successful or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * The return value from this function specifies the HFRE   */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _HFRM_Un_Register_Events() function to un-Register the   */
   /*          event handler.                                           */
int _HFRM_Register_Events(HFRM_Connection_Type_t ConnectionType, Boolean_t ControlHandler)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   HFRM_Register_Hands_Free_Events_Request_t  RegisterHandsFreeEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway))
      {
         /* All that we really need to do is to build a Register HFRE   */
         /* Events message and send it to the server.                   */
         BTPS_MemInitialize(&RegisterHandsFreeEventsRequest, 0, sizeof(RegisterHandsFreeEventsRequest));

         RegisterHandsFreeEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         RegisterHandsFreeEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         RegisterHandsFreeEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         RegisterHandsFreeEventsRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_EVENTS;
         RegisterHandsFreeEventsRequest.MessageHeader.MessageLength   = HFRM_REGISTER_HANDS_FREE_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         RegisterHandsFreeEventsRequest.ConnectionType                = ConnectionType;
         RegisterHandsFreeEventsRequest.ControlHandler                = ControlHandler;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHandsFreeEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_REGISTER_HANDS_FREE_EVENTS_RESPONSE_SIZE)
            {
               if(!(ret_val = ((HFRM_Register_Hands_Free_Events_Response_t *)ResponseMessage)->Status))
                  ret_val = (int)(((HFRM_Register_Hands_Free_Events_Response_t *)ResponseMessage)->EventsHandlerID);
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Hands Free Manager event      */
   /* callback (registered via a successful call to the                 */
   /* _HFRM_Register_Events() function.  This function accepts as input */
   /* the Hands Free Manager event callback ID (return value from the   */
   /* _HFRM_Register_Events() function).                                */
int _HFRM_Un_Register_Events(unsigned int HandsFreeManagerEventCallbackID)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   HFRM_Un_Register_Hands_Free_Events_Request_t  UnRegisterHandsFreeEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if(HandsFreeManagerEventCallbackID)
      {
         /* All that we really need to do is to build an Un-Register    */
         /* HFRE Events message and send it to the server.              */
         BTPS_MemInitialize(&UnRegisterHandsFreeEventsRequest, 0, sizeof(UnRegisterHandsFreeEventsRequest));

         UnRegisterHandsFreeEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterHandsFreeEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterHandsFreeEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         UnRegisterHandsFreeEventsRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_EVENTS;
         UnRegisterHandsFreeEventsRequest.MessageHeader.MessageLength   = HFRM_UN_REGISTER_HANDS_FREE_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterHandsFreeEventsRequest.EventsHandlerID               = HandsFreeManagerEventCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHandsFreeEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_UN_REGISTER_HANDS_FREE_EVENTS_RESPONSE_SIZE)
               ret_val = ((HFRM_Un_Register_Hands_Free_Events_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Hands Free*/
   /* Profile Manager service to explicitly process SCO audio data.     */
   /* This callback will be dispatched by the Hands Free Manager when   */
   /* various Hands Free Manager events occur.  This function accepts   */
   /* the connection type which indicates the connection type the data  */
   /* registration callback to register for.  This function returns a   */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HFRM_Send_Audio_Data() function to send SCO audio data. */
   /* * NOTE * There can only be a single data event handler registered */
   /*          for each type of Hands Free Manager connection type.     */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HFRM_Un_Register_Data_Events() function to un-register  */
   /*          the callback from this module.                           */
int _HFRM_Register_Data_Events(HFRM_Connection_Type_t ConnectionType)
{
   int                                             ret_val;
   BTPM_Message_t                                 *ResponseMessage;
   HFRM_Register_Hands_Free_Data_Events_Request_t  RegisterHandsFreeDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((ConnectionType == hctHandsFree) || (ConnectionType == hctAudioGateway))
      {
         /* All that we really need to do is to build a Register HFRE   */
         /* Data Events message and send it to the server.              */
         BTPS_MemInitialize(&RegisterHandsFreeDataEventsRequest, 0, sizeof(RegisterHandsFreeDataEventsRequest));

         RegisterHandsFreeDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         RegisterHandsFreeDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         RegisterHandsFreeDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         RegisterHandsFreeDataEventsRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_REGISTER_HANDS_FREE_DATA;
         RegisterHandsFreeDataEventsRequest.MessageHeader.MessageLength   = HFRM_REGISTER_HANDS_FREE_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         RegisterHandsFreeDataEventsRequest.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHandsFreeDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_REGISTER_HANDS_FREE_DATA_EVENTS_RESPONSE_SIZE)
            {
               if(!(ret_val = ((HFRM_Register_Hands_Free_Data_Events_Response_t *)ResponseMessage)->Status))
                  ret_val = (int)(((HFRM_Register_Hands_Free_Data_Events_Response_t *)ResponseMessage)->DataEventsHandlerID);
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Hands Free Manager data event */
   /* callback (registered via a successful call to the                 */
   /* _HFRM_Register_Data_Events() function.  This function accepts as  */
   /* input the Hands Free Manager event callback ID (return value from */
   /* the _HFRM_Register_Data_Events() function).                       */
int _HFRM_Un_Register_Data_Events(unsigned int HandsFreeManagerDataCallbackID)
{
   int                                                ret_val;
   BTPM_Message_t                                    *ResponseMessage;
   HFRM_Un_Register_Hands_Free_Data_Events_Request_t  UnRegisterHandsFreeDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if(HandsFreeManagerDataCallbackID)
      {
         /* All that we really need to do is to build an Un-Register    */
         /* HFRE Events message and send it to the server.              */
         BTPS_MemInitialize(&UnRegisterHandsFreeDataEventsRequest, 0, sizeof(UnRegisterHandsFreeDataEventsRequest));

         UnRegisterHandsFreeDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterHandsFreeDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterHandsFreeDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         UnRegisterHandsFreeDataEventsRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_UN_REGISTER_HANDS_FREE_DATA;
         UnRegisterHandsFreeDataEventsRequest.MessageHeader.MessageLength   = HFRM_UN_REGISTER_HANDS_FREE_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterHandsFreeDataEventsRequest.DataEventsHandlerID           = HandsFreeManagerDataCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHandsFreeDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_UN_REGISTER_HANDS_FREE_DATA_EVENTS_RESPONSE_SIZE)
               ret_val = ((HFRM_Un_Register_Hands_Free_Data_Events_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Data Callback ID that is returned from     */
   /* a successful call to HFRM_Register_Data_Event_Callback().         */
   /* The second parameter is the local connection type of the SCO      */
   /* connection.  The third parameter is the address of the remote     */
   /* device of the SCO connection.  The fourth parameter is a pointer  */
   /* to the location to store the SCO Handle. This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Query_SCO_Connection_Handle(unsigned int HandsFreeManagerEventCallbackID, HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HFRM_Query_SCO_Connection_Handle_Request_t  QuerySCOConnectionHandleRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((HandsFreeManagerEventCallbackID) && (SCOHandle))
      {
         /* All that we really need to do is to build an Un-Register    */
         /* HDSET Events message and send it to the server.             */
         BTPS_MemInitialize(&QuerySCOConnectionHandleRequest, 0, sizeof(QuerySCOConnectionHandleRequest));

         QuerySCOConnectionHandleRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QuerySCOConnectionHandleRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QuerySCOConnectionHandleRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HANDS_FREE_MANAGER;
         QuerySCOConnectionHandleRequest.MessageHeader.MessageFunction = HFRM_MESSAGE_FUNCTION_QUERY_SCO_CONNECTION_HANDLE;
         QuerySCOConnectionHandleRequest.MessageHeader.MessageLength   = HFRM_QUERY_SCO_CONNECTION_HANDLE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QuerySCOConnectionHandleRequest.EventsHandlerID               = HandsFreeManagerEventCallbackID;
         QuerySCOConnectionHandleRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         QuerySCOConnectionHandleRequest.ConnectionType                = ConnectionType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QuerySCOConnectionHandleRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HFRM_QUERY_SCO_CONNECTION_HANDLE_RESPONSE_SIZE)
            {
               *SCOHandle = ((HFRM_Query_SCO_Connection_Handle_Response_t *)ResponseMessage)->SCOHandle;
               ret_val    = ((HFRM_Query_SCO_Connection_Handle_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_HANDS_FREE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
