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
static Boolean_t _HDPM_Initialized;

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HDP Manager implementation. This function returns  */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HDP Manager  */
   /* Implementation.                                                   */
int _HDPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!_HDPM_Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HDP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      _HDPM_Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* HDP Manager implementation. After this function is called the     */
   /* HDP Manager implementation will no longer operate until it is     */
   /* initialized again via a call to the _HDPM_Initialize() function.  */
void _HDPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      _HDPM_Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to register an Endpoint on the Local HDP Server. The first*/
   /* parameter defines the Data Type that will be supported by this    */
   /* endpoint. The second parameter specifies whether the Endpoint     */
   /* will be a data source or sink. The third parameter is optional    */
   /* and can be used to specify a short, human-readable description of */
   /* the Endpoint. The final parameters specify the Event Callback and */
   /* Callback parameter (to receive events related to the registered   */
   /* endpoint). This function returns a positive, non-zero, value if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value represents the Endpoint ID that*/
   /*          can be used with various functions in this module to     */
   /*          refer to this endpoint.                                  */
int _HDPM_Register_Endpoint(Word_t DataType, HDP_Device_Role_t LocalRole, char *Description)
{
   int                               ret_val;
   unsigned int                      DescriptionLength;
   BTPM_Message_t                   *ResponseMessage;
   HDPM_Register_Endpoint_Request_t *RegisterEndpointRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%04x, %d, %s\n", DataType, LocalRole, (Description ? Description : "")));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      DescriptionLength = ((Description) ? (BTPS_StringLength(Description) + 1) : 0);

      /* Allocate space for the message.                                */
      if((RegisterEndpointRequest = (HDPM_Register_Endpoint_Request_t *)BTPS_AllocateMemory(HDPM_REGISTER_ENDPOINT_REQUEST_SIZE(DescriptionLength))) != NULL)
      {
         /* All that we really need to do is to populate the message and*/
         /* send it to the server.                                      */
         BTPS_MemInitialize(RegisterEndpointRequest, 0, HDPM_REGISTER_ENDPOINT_REQUEST_SIZE(DescriptionLength));

         RegisterEndpointRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
         RegisterEndpointRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
         RegisterEndpointRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
         RegisterEndpointRequest->MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_REGISTER_ENDPOINT;
         RegisterEndpointRequest->MessageHeader.MessageLength   = HDPM_REGISTER_ENDPOINT_REQUEST_SIZE(DescriptionLength) - BTPM_MESSAGE_HEADER_SIZE;

         RegisterEndpointRequest->DataType                      = DataType;
         RegisterEndpointRequest->LocalRole                     = LocalRole;
         RegisterEndpointRequest->DescriptionLength             = DescriptionLength;

         if(DescriptionLength)
         {
            BTPS_MemCopy(RegisterEndpointRequest->Description, Description, (DescriptionLength - 1));
            RegisterEndpointRequest->Description[DescriptionLength - 1] = '\0';
         }

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)RegisterEndpointRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_REGISTER_ENDPOINT_RESPONSE_SIZE)
               ret_val = ((HDPM_Register_Endpoint_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }

         BTPS_FreeMemory(RegisterEndpointRequest);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Endpoint. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _HDPM_Un_Register_Endpoint(unsigned int EndpointID)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   HDPM_Un_Register_Endpoint_Request_t  UnRegisterEndpointRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", EndpointID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&UnRegisterEndpointRequest, 0, HDPM_UN_REGISTER_ENDPOINT_REQUEST_SIZE); 

      UnRegisterEndpointRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterEndpointRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterEndpointRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      UnRegisterEndpointRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_UN_REGISTER_ENDPOINT;
      UnRegisterEndpointRequest.MessageHeader.MessageLength   = HDPM_UN_REGISTER_ENDPOINT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterEndpointRequest.EndpointID                    = EndpointID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterEndpointRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_UN_REGISTER_ENDPOINT_RESPONSE_SIZE)
            ret_val = ((HDPM_Un_Register_Endpoint_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to establish a data connection to a*/
   /* local endpoint. The first parameter is the DataLinkID associated  */
   /* with the connection request. The second parameter is one of       */
   /* the MCAP_RESPONSE_CODE_* constants which indicates either that    */
   /* the request should be accepted (MCAP_RESPONSE_CODE_SUCCESS) or    */
   /* provides a reason for rejecting the request. If the request is to */
   /* be accepted, and the request is for a local Data Source, the final*/
   /* parameter indicates whether the connection shall use the Reliable */
   /* or Streaming communication mode. This function returns zero if    */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A Data Connected  */
   /*          event will be dispatched to signify the actual result.   */
   /* * NOTE * If the connection is accepted, and the connection request*/
   /*          is for a local Data Sink, then ChannelMode must be set to*/
   /*          the Mode indicated in the request.  If the connection is */
   /*          accepted for a local Data Source, ChannelMode must be set*/
   /*          to either cmReliable or cmStreaming. If the connection   */
   /*          request is rejected, ChannelMode is ignored.             */
int _HDPM_Data_Connection_Request_Response(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   HDPM_Data_Connection_Request_Response_Request_t  DataConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d, 0x%02x, %d\n", DataLinkID, ResponseCode, ChannelMode));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&DataConnectionRequestResponseRequest, 0, HDPM_DATA_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE);

      DataConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      DataConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      DataConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      DataConnectionRequestResponseRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_REQUEST_RESPONSE;
      DataConnectionRequestResponseRequest.MessageHeader.MessageLength   = HDPM_DATA_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DataConnectionRequestResponseRequest.DataLinkID                    = DataLinkID;
      DataConnectionRequestResponseRequest.ResponseCode                  = ResponseCode;
      DataConnectionRequestResponseRequest.ChannelMode                   = ChannelMode;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DataConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_DATA_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
            ret_val = ((HDPM_Data_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to query the available HDP Instances on a remote    */
   /* device. The first parameter specifies the Address of the Remote   */
   /* Device to query. The second parameter specifies the maximum       */
   /* number of Instances that the buffer will support (i.e. can be     */
   /* copied into the buffer). The next parameter is optional and,      */
   /* if specified, will be populated with up to the total number of    */
   /* Instances advertised by the remote device, if the function is     */
   /* successful. The final parameter is optional and can be used to    */
   /* retrieve the total number of available Instances (regardless of   */
   /* the size of the list specified by the first two parameters).      */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Instances that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Instance  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int _HDPM_Query_Remote_Device_Instances(BD_ADDR_t RemoteDeviceAddress, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   HDPM_Query_Remote_Device_Instances_Request_t  QueryRemoteDeviceInstancesRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02x:%02x:%02x:%02x:%02x:%02x\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&QueryRemoteDeviceInstancesRequest, 0, HDPM_QUERY_REMOTE_DEVICE_INSTANCES_REQUEST_SIZE);

      QueryRemoteDeviceInstancesRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      QueryRemoteDeviceInstancesRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      QueryRemoteDeviceInstancesRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      QueryRemoteDeviceInstancesRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_INSTANCES;
      QueryRemoteDeviceInstancesRequest.MessageHeader.MessageLength   = HDPM_QUERY_REMOTE_DEVICE_INSTANCES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      QueryRemoteDeviceInstancesRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryRemoteDeviceInstancesRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->NumberInstances)))
         {
            ret_val = ((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->Status;

            /* If the query was successful, populate the provided       */
            /* Instance List.                                           */
            if(!ret_val)
            {
               if(TotalNumberInstances)
                  *TotalNumberInstances = ((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->NumberInstances;

               if((MaximumInstanceListEntries) && (InstanceList))
               {
                  if(((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->NumberInstances > MaximumInstanceListEntries)
                     ((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->NumberInstances = MaximumInstanceListEntries;

                  BTPS_MemCopy(InstanceList, ((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->InstanceList, (sizeof(DWord_t) * ((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->NumberInstances));

                  ret_val = ((HDPM_Query_Remote_Device_Instances_Response_t *)ResponseMessage)->NumberInstances;
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the available Endpoints published for a specific */
   /* HDP Instances on a remote device. The first parameter specifies   */
   /* the Address of the Remote Device to query. The second parameter   */
   /* specifies Instance on the Remote Device. The third parameter      */
   /* specifies the maximum number of Endpoints that the buffer will    */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with up to the   */
   /* total number of Endpoints published by the remote device, if the  */
   /* function is successful. The final parameter is optional and can   */
   /* be used to retrieve the total number of Endpoints (regardless     */
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Endpoints that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Endpoint  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int _HDPM_Query_Remote_Device_Instance_Endpoints(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   HDPM_Query_Remote_Device_Endpoints_Request_t  QueryRemoteDeviceEndpointsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02x:%02x:%02x:%02x:%02x:%02x, 0x%08X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&QueryRemoteDeviceEndpointsRequest, 0, HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_REQUEST_SIZE);

      QueryRemoteDeviceEndpointsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      QueryRemoteDeviceEndpointsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      QueryRemoteDeviceEndpointsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      QueryRemoteDeviceEndpointsRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ENDPOINTS;
      QueryRemoteDeviceEndpointsRequest.MessageHeader.MessageLength   = HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      QueryRemoteDeviceEndpointsRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      QueryRemoteDeviceEndpointsRequest.Instance                      = Instance;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryRemoteDeviceEndpointsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->NumberEndpoints)))
         {
            ret_val = ((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->Status;

            /* If the query was successful, populate the provided       */
            /* endpoint list.                                           */
            if(!ret_val)
            {
               if(TotalNumberEndpoints)
                  *TotalNumberEndpoints = ((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->NumberEndpoints;

               if((MaximumEndpointListEntries) && (EndpointInfoList))
               {
                  if(((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->NumberEndpoints > MaximumEndpointListEntries)
                     ((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->NumberEndpoints = MaximumEndpointListEntries;

                  BTPS_MemCopy(EndpointInfoList, ((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->EndpointList, (sizeof(HDPM_Endpoint_Info_t) * ((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->NumberEndpoints));

                  ret_val = ((HDPM_Query_Remote_Device_Endpoints_Response_t *)ResponseMessage)->NumberEndpoints;
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the description of a known Endpoint published in */
   /* a specific HDP Instance by a remote device. The first parameter   */
   /* specifies the Address of the Remote Device to query. The second   */
   /* parameter specifies Instance on the Remote Device. The third      */
   /* parameter identifies the Endpoint to query. The fourth and fifth  */
   /* parameters specific the size of the buffer and the buffer to hold */
   /* the description string, respectively. The final parameter is      */
   /* optional and, if specified, will be set to the total size of the  */
   /* description string for the given Endpoint, if the function is     */
   /* successful (regardless of the size of the list specified by the   */
   /* first two parameters). This function returns a non-negative value */
   /* if successful which represents the number of bytes copied into the*/
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum           */
   /*          Description Length, in which case the final parameter    */
   /*          *MUST* be specified.                                     */
int _HDPM_Query_Remote_Device_Endpoint_Description(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Endpoint_Info_t *EndpointInfo, unsigned int MaximumDescriptionLength, char *DescriptionBuffer, unsigned int *TotalDescriptionLength)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   HDPM_Query_Endpoint_Description_Request_t  QueryEndpointDescriptionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02x:%02x:%02x:%02x:%02x:%02x, 0x%08X, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance, (EndpointInfo ? EndpointInfo->EndpointID : -1)));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((_HDPM_Initialized) && (EndpointInfo))
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&QueryEndpointDescriptionRequest, 0, HDPM_QUERY_ENDPOINT_DESCRIPTION_REQUEST_SIZE);

      QueryEndpointDescriptionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      QueryEndpointDescriptionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      QueryEndpointDescriptionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      QueryEndpointDescriptionRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_QUERY_ENDPOINT_DESCRIPTION;
      QueryEndpointDescriptionRequest.MessageHeader.MessageLength   = HDPM_QUERY_ENDPOINT_DESCRIPTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      QueryEndpointDescriptionRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      QueryEndpointDescriptionRequest.Instance                      = Instance;
      QueryEndpointDescriptionRequest.EndpointInfo                  = *EndpointInfo;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryEndpointDescriptionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionLength)))
         {
            ret_val = ((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->Status;

            /* If the query was successful, populate the provided       */
            /* endpoint description buffer.                             */
            if(!ret_val)
            {
               if(TotalDescriptionLength)
                  *TotalDescriptionLength = ((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionLength;

               if((MaximumDescriptionLength) && (DescriptionBuffer))
               {
                  if(((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionLength >= MaximumDescriptionLength)
                     ((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionLength = (MaximumDescriptionLength - 1);

                  BTPS_MemCopy(DescriptionBuffer, ((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionBuffer, ((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionLength);

                  DescriptionBuffer[((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionLength] = '\0';

                  ret_val = ((HDPM_Query_Endpoint_Description_Response_t *)ResponseMessage)->DescriptionLength;
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a connection to a specific HDP Instance on   */
   /* a Remote Device. The first parameter specifies the Remote Device  */
   /* to connect to. The second parameter specifies the HDP Instance on */
   /* the remote device. This function returns zero if successful, or a */
   /* negative return value if there was an error.                      */
int _HDPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HDPM_Connect_Remote_Device_Request_t  ConnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02x:%02x:%02x:%02x:%02x:%02x, 0x%08X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ConnectRemoteDeviceRequest, 0, HDPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE);

      ConnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      ConnectRemoteDeviceRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE;
      ConnectRemoteDeviceRequest.MessageHeader.MessageLength   = HDPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConnectRemoteDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      ConnectRemoteDeviceRequest.Instance                      = Instance;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
            ret_val = ((HDPM_Connect_Remote_Device_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an existing connection to a specific HDP Instance*/
   /* on a Remote Device. The first parameter specifies the Remote      */
   /* Device. The second parameter specifies the HDP Instance on the    */
   /* remote device from which to disconnect. This function returns zero*/
   /* if successful, or a negative return value if there was an error.  */
int _HDPM_Disconnect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HDPM_Disconnect_Remote_Device_Request_t  DisconnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02x:%02x:%02x:%02x:%02x:%02x, 0x%08X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&DisconnectRemoteDeviceRequest, 0, HDPM_DISCONNECT_REMOTE_DEVICE_REQUEST_SIZE);

      DisconnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      DisconnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      DisconnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      DisconnectRemoteDeviceRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE;
      DisconnectRemoteDeviceRequest.MessageHeader.MessageLength   = HDPM_DISCONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DisconnectRemoteDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      DisconnectRemoteDeviceRequest.Instance                      = Instance;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectRemoteDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_DISCONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
            ret_val = ((HDPM_Disconnect_Remote_Device_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to establish an HDP connection to an Endpoint of    */
   /* a specific HDP Instance on a Remote Device. The first parameter   */
   /* specifies the Remote Device to connect to. The second parameter   */
   /* specifies the HDP Instance on the remote device. The third        */
   /* parameter specifies the Endpoint of that Instance to which the    */
   /* connection will be attempted. The final parameter specifies the   */
   /* type of connection that will be established. This function returns*/
   /* a positive value if successful, or a negative return value if     */
   /* there was an error.                                               */
   /* * NOTE * A successful return value represents the Data Link ID    */
   /*          shall be used with various functions and by various      */
   /*          events in this module to reference this data connection. */
int _HDPM_Connect_Remote_Device_Endpoint(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, Byte_t EndpointID, HDP_Channel_Mode_t ChannelMode)
{
   int                                            ret_val;
   BTPM_Message_t                                *ResponseMessage;
   HDPM_Connect_Remote_Device_Endpoint_Request_t  ConnectRemoteDeviceEndpointRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02x:%02x:%02x:%02x:%02x:%02x, 0x%08X, %u, %d\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance, EndpointID, ChannelMode));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ConnectRemoteDeviceEndpointRequest, 0, HDPM_CONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE);

      ConnectRemoteDeviceEndpointRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConnectRemoteDeviceEndpointRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConnectRemoteDeviceEndpointRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      ConnectRemoteDeviceEndpointRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE_ENDPOINT;
      ConnectRemoteDeviceEndpointRequest.MessageHeader.MessageLength   = HDPM_CONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConnectRemoteDeviceEndpointRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      ConnectRemoteDeviceEndpointRequest.Instance                      = Instance;
      ConnectRemoteDeviceEndpointRequest.EndpointID                    = EndpointID;
      ConnectRemoteDeviceEndpointRequest.ChannelMode                   = ChannelMode;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteDeviceEndpointRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_CONNECT_REMOTE_DEVICE_ENDPOINT_RESPONSE_SIZE)
            ret_val = ((HDPM_Connect_Remote_Device_Endpoint_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect an established HDP data connection.   */
   /* This function accepts the Data Link ID of the data connection     */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
int _HDPM_Disconnect_Remote_Device_Endpoint(unsigned int DataLinkID)
{
   int                                               ret_val;
   BTPM_Message_t                                   *ResponseMessage;
   HDPM_Disconnect_Remote_Device_Endpoint_Request_t  DisconnectRemoteDeviceEndpointRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", DataLinkID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* All that we really need to do is to populate the message and   */
      /* send it to the server.                                         */
      BTPS_MemInitialize(&DisconnectRemoteDeviceEndpointRequest, 0, HDPM_DISCONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE);

      DisconnectRemoteDeviceEndpointRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      DisconnectRemoteDeviceEndpointRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      DisconnectRemoteDeviceEndpointRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      DisconnectRemoteDeviceEndpointRequest.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE_ENDPOINT;
      DisconnectRemoteDeviceEndpointRequest.MessageHeader.MessageLength   = HDPM_DISCONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DisconnectRemoteDeviceEndpointRequest.DataLinkID                    = DataLinkID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectRemoteDeviceEndpointRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid. If it is, go ahead and note the returned status.     */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_DISCONNECT_REMOTE_DEVICE_ENDPOINT_RESPONSE_SIZE)
            ret_val = ((HDPM_Disconnect_Remote_Device_Endpoint_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to send data over an established HDP data connection. The */
   /* first parameter is the Data Link ID which represents the data     */
   /* connection to use. The final parameters specify the data (and     */
   /* amount) to be sent. This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function will either send all of the data or none of*/
   /*          the data.                                                */
int _HDPM_Write_Data(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer)
{
   int                        ret_val;
   BTPM_Message_t            *ResponseMessage;
   HDPM_Write_Data_Request_t *WriteDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", DataLinkID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(_HDPM_Initialized)
   {
      /* Verify that the input parameters appear semi-valid.            */
      if((DataLength) && (DataBuffer))
      {
         /* Allocate space for the request message.                     */
         if((WriteDataRequest = (HDPM_Write_Data_Request_t *)BTPS_AllocateMemory(HDPM_WRITE_DATA_REQUEST_SIZE(DataLength))) != NULL)
         {
            /* Populate the message and send it to the server.          */
            BTPS_MemInitialize(WriteDataRequest, 0, HDPM_WRITE_DATA_REQUEST_SIZE(DataLength));

            WriteDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            WriteDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            WriteDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
            WriteDataRequest->MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_WRITE_DATA;
            WriteDataRequest->MessageHeader.MessageLength   = HDPM_WRITE_DATA_REQUEST_SIZE(DataLength) - BTPM_MESSAGE_HEADER_SIZE;

            WriteDataRequest->DataLinkID                    = DataLinkID;
            WriteDataRequest->DataLength                    = DataLength;

            BTPS_MemCopy(WriteDataRequest->DataBuffer, DataBuffer, DataLength);

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)WriteDataRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HDPM_WRITE_DATA_RESPONSE_SIZE)
                  ret_val = ((HDPM_Write_Data_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            BTPS_FreeMemory(WriteDataRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
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

