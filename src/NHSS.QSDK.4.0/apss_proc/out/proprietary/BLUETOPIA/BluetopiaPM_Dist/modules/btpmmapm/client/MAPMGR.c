/*****< mapmgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAPMGR - Message Access Manager Implementation for Stonestreet One        */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/24/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMMAPM.h"            /* BTPM MAP Manager Prototypes/Constants.    */
#include "MAPMMSG.h"             /* BTPM MAP Manager Message Formats.         */
#include "MAPMGR.h"              /* MAP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT is the time in ms that   */
   /* the PM client functions wait for reponses from the PM server.     */
#define MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT               (BTPM_CONFIGURATION_DEVICE_MANAGER_GENERAL_MESSAGE_RESPONSE_TIME_MS)

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Initialized is a global flag indicating whether the initialization*/
   /* routine for the platform manager client was successfully called   */
static Boolean_t Initialized;

   /* Message Access Module Installation/Support Functions.             */

   /* Initializes the platform manager client Message Access Manager    */
   /* implementation.  This function returns zero if successful, or a   */
   /* negative integer if there was an error.                           */
int _MAPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the PM client module has been initialized.               */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Message Access Manager (Imp)\n"));

      /* Flag that the PM client module is initialized.                 */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* This function sets the PM client Initialized global flag to false.*/
   /* The other methods will fail until _MAPM_Initialize has been       */
   /* called.                                                           */
void _MAPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the PM client module has been initialized.               */
   if(Initialized)
   {
      /* Flag that the PM client module is not initialized.             */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Message Access Profile (MAP) Manager (MAPM) Common Functions.     */

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming MAP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _MAPM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Accept)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   MAPM_Connection_Request_Response_Request_t  ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Connection      */
         /* Request message and send it to the server.                  */
         BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

         ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         ConnectionRequestResponseRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
         ConnectionRequestResponseRequest.MessageHeader.MessageLength   = MAPM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectionRequestResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectionRequestResponseRequest.InstanceID                    = InstanceID;
         ConnectionRequestResponseRequest.Accept                        = Accept;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Responding to Connection Request: %02X%02X%02X%02X%02X%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, Accept));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
               ret_val = ((MAPM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* to register a MAP Server (MSE) on a specified RFCOMM Server Port. */
   /* This function accepts as it's parameter's the RFCOMM Server Port  */
   /* to register the server on, followed by the incoming connection    */
   /* flags to apply to incoming connections.  The third and fourth     */
   /* parameters specify the required MAP Information (MAP Server       */
   /* Instance ID - must be unique on the device, followed by the       */
   /* supported MAP Message Types).  The final two parameters specify   */
   /* the MAP Manager Event Callback function and Callback parameter    */
   /* which will be used when MAP Manager events need to be dispatched  */
   /* for the specified MAP Server.  This function returns zero if      */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * This function is only applicable to MAP Servers.  It will*/
   /*          not allow the ability to register a MAP Notification     */
   /*          Server (Notification Servers are handled internal to the */
   /*          MAP Manager module).                                     */
int _MAPM_Register_Server(unsigned int ServerPort, unsigned long ServerFlags, unsigned int InstanceID, unsigned long SupportedMessageTypes)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   MAPM_Register_Server_Request_t  RegisterServerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(((ServerPort >= MAP_PORT_NUMBER_MINIMUM) && (ServerPort <= MAP_PORT_NUMBER_MAXIMUM)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Register Server */
         /* message and send it to the server.                          */
         BTPS_MemInitialize(&RegisterServerRequest, 0, sizeof(RegisterServerRequest));

         RegisterServerRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         RegisterServerRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         RegisterServerRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         RegisterServerRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_REGISTER_SERVER;
         RegisterServerRequest.MessageHeader.MessageLength   = MAPM_REGISTER_SERVER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         RegisterServerRequest.ServerPort                    = ServerPort;
         RegisterServerRequest.ServerFlags                   = ServerFlags;
         RegisterServerRequest.InstanceID                    = InstanceID;
         RegisterServerRequest.SupportedMessageTypes         = SupportedMessageTypes;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Registering Server: %u, 0x%08lX, %u, 0x%08lX\n", ServerPort, ServerFlags, InstanceID, SupportedMessageTypes));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterServerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_REGISTER_SERVER_RESPONSE_SIZE)
               ret_val = ((MAPM_Register_Server_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered MAP server (registered via a  */
   /* successful call the the MAP_Register_Server() function).  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _MAPM_Un_Register_Server(unsigned int InstanceID)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   MAPM_Un_Register_Server_Request_t  UnRegisterServerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE))
      {
         /* All that we really need to do is to build a UnRegister      */
         /* Server message and send it to the server.                   */
         BTPS_MemInitialize(&UnRegisterServerRequest, 0, sizeof(UnRegisterServerRequest));

         UnRegisterServerRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterServerRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterServerRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         UnRegisterServerRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER;
         UnRegisterServerRequest.MessageHeader.MessageLength   = MAPM_UN_REGISTER_SERVER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterServerRequest.InstanceID                    = InstanceID;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("UnRegistering Server Instance: %d\n", InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterServerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_UN_REGISTER_SERVER_RESPONSE_SIZE)
               ret_val = ((MAPM_Un_Register_Server_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to Register an SDP Service Record for a previously        */
   /* registered MAP Server.  This function returns a positive,         */
   /* non-zero, value if successful, or a negative return error code if */
   /* there was an error.  If this function is successful, the value    */
   /* that is returned represents the SDP Service Record Handle of the  */
   /* Service Record that was added to the SDP Database.  The           */
   /* ServiceName parameter is a pointer to a NULL terminated UTF-8     */
   /* encoded string.                                                   */
long _MAPM_Register_Service_Record(unsigned int InstanceID, char *ServiceName)
{
   int                                     ret_val;
   unsigned int                            ServiceNameLength;
   BTPM_Message_t                         *ResponseMessage;
   MAPM_Register_Service_Record_Request_t *RegisterServiceRecordRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE))
      {
         /* All that we really need to do is to build a UnRegister      */
         /* Server message and send it to the server.                   */
         if(ServiceName)
            ServiceNameLength = BTPS_StringLength(ServiceName) + 1;
         else
            ServiceNameLength = 0;

         /* Now that the total size has been calculated, attempt to     */
         /* allocate space to hold the message.                         */
         if((RegisterServiceRecordRequest = (MAPM_Register_Service_Record_Request_t *)BTPS_AllocateMemory(MAPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(ServiceNameLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(RegisterServiceRecordRequest, 0, MAPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(ServiceNameLength));

            RegisterServiceRecordRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            RegisterServiceRecordRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            RegisterServiceRecordRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            RegisterServiceRecordRequest->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD;
            RegisterServiceRecordRequest->MessageHeader.MessageLength   = MAPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(ServiceNameLength) - BTPM_MESSAGE_HEADER_SIZE;

            RegisterServiceRecordRequest->InstanceID                    = InstanceID;
            RegisterServiceRecordRequest->ServiceNameLength             = ServiceNameLength;

            if(ServiceNameLength)
               BTPS_MemCopy(&(((unsigned char *)RegisterServiceRecordRequest)[MAPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(0)]), ServiceName, ServiceNameLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Registering Service Record for Instance: %d\n", InstanceID));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)RegisterServiceRecordRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_REGISTER_SERVICE_RECORD_RESPONSE_SIZE)
               {
                  if(!(ret_val = ((MAPM_Register_Service_Record_Response_t *)ResponseMessage)->Status))
                     ret_val = ((MAPM_Register_Service_Record_Response_t *)ResponseMessage)->ServiceRecordHandle;
               }
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(RegisterServiceRecordRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered SDP Service Record.*/
   /* This function accepts the Instance ID of the MAP Server that is to*/
   /* have the Service Record Removed.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _MAPM_Un_Register_Service_Record(unsigned int InstanceID)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   MAPM_Un_Register_Service_Record_Request_t  UnRegisterServiceRecordRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE))
      {
         /* All that we really need to do is to build a UnRegister      */
         /* Server message and send it to the server.                   */
         BTPS_MemInitialize(&UnRegisterServiceRecordRequest, 0, sizeof(UnRegisterServiceRecordRequest));

         UnRegisterServiceRecordRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterServiceRecordRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterServiceRecordRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         UnRegisterServiceRecordRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_UN_REGISTER_SERVICE_RECORD;
         UnRegisterServiceRecordRequest.MessageHeader.MessageLength   = MAPM_UN_REGISTER_SERVICE_RECORD_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterServiceRecordRequest.InstanceID                    = InstanceID;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("UnRegistering Service Record for Instance: %d\n", InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterServiceRecordRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_UN_REGISTER_SERVICE_RECORD_RESPONSE_SIZE)
               ret_val = ((MAPM_Un_Register_Service_Record_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the details of MAP services offered by a remote  */
   /* Message Access Server device. This function accepts the remote    */
   /* device address of the device whose SDP records will be parsed     */
   /* and the buffer which will hold the parsed service details. This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
   /* * NOTE * When this function is successful, the provided buffer    */
   /*          will be populated with the parsed service details. This  */
   /*          buffer MUST be passed to                                 */
   /*          MAPM_Free_Parsed_Message_Access_Service_Info() in order  */
   /*          to release any resources that were allocated during the  */
   /*          query process.                                           */
int _MAPM_Parse_Remote_Message_Access_Services(BD_ADDR_t RemoteDeviceAddress, MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo)
{
   int                                                  ret_val;
   char                                                *ReservedBuffer;
   char                                                *ReservedBufferPosition;
   unsigned int                                         Index;
   unsigned int                                         ServiceDetailsLength;
   BTPM_Message_t                                      *ResponseMessage;
   MAPM_MAS_Service_Details_t                          *ServiceDetailsBuffer;
   MAPM_Parse_Remote_Message_Access_Services_Request_t  ParseRemoteMessageAccessServicesRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ServiceInfo))
      {
         /* All that we really need to do is to build a Parse Remote    */
         /* Message Access Services message and send it to the server.  */
         BTPS_MemInitialize(&ParseRemoteMessageAccessServicesRequest, 0, sizeof(ParseRemoteMessageAccessServicesRequest));

         ParseRemoteMessageAccessServicesRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ParseRemoteMessageAccessServicesRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ParseRemoteMessageAccessServicesRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         ParseRemoteMessageAccessServicesRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES;
         ParseRemoteMessageAccessServicesRequest.MessageHeader.MessageLength   = MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ParseRemoteMessageAccessServicesRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Parsing Remote Message Access Services for Device: %02X%02X%02X%02X%02X%02X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ParseRemoteMessageAccessServicesRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_RESPONSE_SIZE(0, 0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_PARSE_REMOTE_MESSAGE_ACCESS_SERVICES_RESPONSE_SIZE(((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->NumberServices, ((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->ReservedBufferLength)))
            {
               ret_val = ((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->Status;

               if((!ret_val) && (((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->NumberServices))
               {
                  ServiceDetailsLength = ((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->NumberServices * sizeof(MAPM_MAS_Service_Details_t);

                  if((ServiceDetailsBuffer = (MAPM_MAS_Service_Details_t *)BTPS_AllocateMemory(ServiceDetailsLength)) != NULL)
                  {
                     if(((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->ReservedBufferLength)
                     {
                        if((ReservedBuffer = (char *)BTPS_AllocateMemory(((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->ReservedBufferLength)) != NULL)
                        {
                           BTPS_MemCopy(ReservedBuffer, &(((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->VariableData[ServiceDetailsLength]), ((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->ReservedBufferLength);
                           ReservedBufferPosition = ReservedBuffer;
                        }
                        else
                        {
                           ret_val                = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                           ReservedBufferPosition = NULL;
                        }
                     }
                     else
                     {
                        ReservedBuffer         = NULL;
                        ReservedBufferPosition = NULL;
                     }

                     for(Index = 0; Index < ((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->NumberServices; Index++)
                     {
                        ServiceDetailsBuffer[Index].ServerPort            = ((MAPM_Parse_Response_Service_Details_t *)((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->VariableData)[Index].ServerPort;
                        ServiceDetailsBuffer[Index].InstanceID            = ((MAPM_Parse_Response_Service_Details_t *)((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->VariableData)[Index].InstanceID;
                        ServiceDetailsBuffer[Index].SupportedMessageTypes = ((MAPM_Parse_Response_Service_Details_t *)((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->VariableData)[Index].SupportedMessageTypes;

                        if((ReservedBufferPosition) && (((MAPM_Parse_Response_Service_Details_t *)((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->VariableData)[Index].ServiceNameBytes))
                        {
                           ServiceDetailsBuffer[Index].ServiceName        = ReservedBufferPosition;
                           ReservedBufferPosition                        += ((MAPM_Parse_Response_Service_Details_t *)((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->VariableData)[Index].ServiceNameBytes;
                           *(ReservedBufferPosition - 1)                  = '\0';
                        }
                     }
                  }
                  else
                  {
                     ret_val        = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                     ReservedBuffer = NULL;
                  }

                  if(!ret_val)
                  {
                     ServiceInfo->NumberServices = ((MAPM_Parse_Remote_Message_Access_Services_Response_t *)ResponseMessage)->NumberServices;
                     ServiceInfo->ServiceDetails = ServiceDetailsBuffer;
                     ServiceInfo->RESERVED       = ReservedBuffer;
                  }
                  else
                  {
                     if(ServiceDetailsBuffer)
                        BTPS_FreeMemory(ServiceDetailsBuffer);

                     if(ReservedBuffer)
                        BTPS_FreeMemory(ReservedBuffer);

                     BTPS_MemInitialize(ServiceInfo, 0, sizeof(MAPM_Parsed_Message_Access_Service_Info_t));
                  }
               }
               else
                  BTPS_MemInitialize(ServiceInfo, 0, sizeof(MAPM_Parsed_Message_Access_Service_Info_t));

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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Message Access Server device.  The */
   /* first parameter to this function specifies the connection type to */
   /* make (either Notification or Message Access).  The                */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The InstancedID    */
   /* member *MUST* specify the Remote Instance ID of the remote MAP    */
   /* server that is to be connected with.  The ConnectionFlags         */
   /* parameter specifies whether authentication or encryption should be*/
   /* used to create this connection.  This function returns zero if    */
   /* successful, or a negative return error code if there was an error.*/
int _MAPM_Connect_Remote_Device(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned int InstanceID, unsigned long ConnectionFlags)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   MAPM_Connect_Remote_Device_Request_t  ConnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (((RemoteServerPort >= MAP_PORT_NUMBER_MINIMUM) && (RemoteServerPort <= MAP_PORT_NUMBER_MAXIMUM))) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Connection      */
         /* Request message and send it to the server.                  */
         BTPS_MemInitialize(&ConnectRemoteDeviceRequest, 0, sizeof(ConnectRemoteDeviceRequest));

         ConnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         ConnectRemoteDeviceRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE;
         ConnectRemoteDeviceRequest.MessageHeader.MessageLength   = MAPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectRemoteDeviceRequest.ConnectionType                = ConnectionType;
         ConnectRemoteDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectRemoteDeviceRequest.ServerPort                    = RemoteServerPort;
         ConnectRemoteDeviceRequest.InstanceID                    = InstanceID;
         ConnectRemoteDeviceRequest.ConnectionFlags               = ConnectionFlags;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connecting to Remote Device: %02X%02X%02X%02X%02X%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, RemoteServerPort, InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
               ret_val = ((MAPM_Connect_Remote_Device_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Message Access   */
   /* connection that was previously opened by a successful call to     */
   /* MAPM_Connect_Server() function or by a metConnectServer.          */
   /* This function accepts the RemoteDeviceAddress. The                */
   /* InstanceID parameter specifies which server instance to use.      */
   /* The ConnectionType parameter indicates what type of connection    */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
int _MAPM_Disconnect(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                        ret_val;
   BTPM_Message_t            *ResponseMessage;
   MAPM_Disconnect_Request_t  DisconnectRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Disconnect      */
         /* Request message and send it to the server.                  */
         BTPS_MemInitialize(&DisconnectRequest, 0, sizeof(DisconnectRequest));

         DisconnectRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisconnectRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisconnectRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         DisconnectRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_DISCONNECT;
         DisconnectRequest.MessageHeader.MessageLength   = MAPM_DISCONNECT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisconnectRequest.ConnectionType                = ConnectionType;
         DisconnectRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         DisconnectRequest.InstanceID                    = InstanceID;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting: %d, %02X%02X%02X%02X%02X%02X, %d\n", ConnectionType, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_DISCONNECT_RESPONSE_SIZE)
               ret_val = ((MAPM_Disconnect_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding MAPM profile client or notification client request.   */
   /* This function accepts as input the connection type of the remote  */
   /* connection, followed by the remote device address of the device to*/
   /* abort the current operation, followed by the InstanceID parameter.*/
   /* Together these parameters specify which connection is to have the */
   /* Abort issued.  This function returns zero if successful, or a     */
   /* negative return error code if there was an error.                 */
int _MAPM_Abort(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                   ret_val;
   BTPM_Message_t       *ResponseMessage;
   MAPM_Abort_Request_t  AbortRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build an Abort message  */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&AbortRequest, 0, sizeof(AbortRequest));

         AbortRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         AbortRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         AbortRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         AbortRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_ABORT;
         AbortRequest.MessageHeader.MessageLength   = MAPM_ABORT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         AbortRequest.ConnectionType                = ConnectionType;
         AbortRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         AbortRequest.InstanceID                    = InstanceID;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Aborting Current Command: %d, %02X%02X%02X%02X%02X%02X, %d\n", ConnectionType, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&AbortRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_ABORT_RESPONSE_SIZE)
               ret_val = ((MAPM_Abort_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current folder.  The first parameter is the  */
   /* Bluetooth address of the device whose connection we are querying. */
   /* The InstanceID parameter specifies which server instance on the   */
   /* remote device to use.  The second parameter is the size of the    */
   /* Buffer that is available to store the current path.  The final    */
   /* parameter is the buffer to copy the path in to.  This function    */
   /* returns a positive (or zero) value representing the total length  */
   /* of the path string (excluding the NULL character) if successful   */
   /* and a negative return error code if there was an error.           */
   /* * NOTE * If the current path is at root, then the Buffer will     */
   /*          contain an empty string and the length will be zero.     */
   /* * NOTE * If the supplied buffer was not large enough to hold the  */
   /*          returned size, it will still be NULL-terminated but will */
   /*          not contain the complete path.                           */
int _MAPM_Query_Current_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int BufferSize, char *Buffer)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   MAPM_Query_Current_Folder_Request_t  QueryCurrentFolderRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (((BufferSize) && (Buffer)) || (!BufferSize)))
      {
         /* All that we really need to do is to build a Query Current   */
         /* Folder message and send it to the server.                   */
         BTPS_MemInitialize(&QueryCurrentFolderRequest, 0, sizeof(QueryCurrentFolderRequest));

         QueryCurrentFolderRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryCurrentFolderRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryCurrentFolderRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         QueryCurrentFolderRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_QUERY_CURRENT_FOLDER;
         QueryCurrentFolderRequest.MessageHeader.MessageLength   = MAPM_QUERY_CURRENT_FOLDER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryCurrentFolderRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         QueryCurrentFolderRequest.InstanceID                    = InstanceID;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Querying Current Folder: %02X%02X%02X%02X%02X%02X, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryCurrentFolderRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_QUERY_CURRENT_FOLDER_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_QUERY_CURRENT_FOLDER_RESPONSE_SIZE(((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->FolderNameLength)))
            {
               if(!((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->Status)
               {
                  /* Note the string length of the current path. The    */
                  /* length from the message should include the NULL    */
                  /* character, so should always be at least one.       */
                  if(((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->FolderNameLength > 0)
                     ret_val = (int)((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->FolderNameLength - 1;
                  else
                     ret_val = 0;

                  /* Make sure we only copy as much of the path as will */
                  /* fit in the supplied buffer.                        */
                  if(BufferSize > ((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->FolderNameLength)
                     BufferSize = ((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->FolderNameLength;

                  if(BufferSize)
                  {
                     BTPS_MemCopy(Buffer, ((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->FolderName, (BufferSize - 1));
                     Buffer[BufferSize-1] = '\0';
                  }
               }
               else
                  ret_val = ((MAPM_Query_Current_Folder_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to create and enable a Notification server for a specified*/
   /* connection.  The RemoteDeviceAddress parameter specifies what     */
   /* connected device this server should be associated with.  The      */
   /* InstanceID parameter specifies which server instance on the remote*/
   /* device to use.  The ServerPort parameter is the local RFCOMM port */
   /* on which to open the server.  The Callback Function and Parameter */
   /* will be called for all events related to this notification server.*/
int _MAPM_Enable_Notifications(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Enable)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   MAPM_Enable_Notifications_Request_t  EnableNotificationsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build an Enable         */
         /* Notifications message and send it to the server.            */
         BTPS_MemInitialize(&EnableNotificationsRequest, 0, sizeof(EnableNotificationsRequest));

         EnableNotificationsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         EnableNotificationsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnableNotificationsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         EnableNotificationsRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS;
         EnableNotificationsRequest.MessageHeader.MessageLength   = MAPM_ENABLE_NOTIFICATIONS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableNotificationsRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         EnableNotificationsRequest.InstanceID                    = InstanceID;
         EnableNotificationsRequest.Enable                        = Enable;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enabling Notifications for Instance: %02X%02X%02X%02X%02X%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, Enable));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableNotificationsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_ENABLE_NOTIFICATIONS_RESPONSE_SIZE)
               ret_val = ((MAPM_Enable_Notifications_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Set Folder Request to the  */
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  The PathOption*/
   /* parameter contains an enumerated value that indicates the type of */
   /* path change to request.  The FolderName parameter contains the    */
   /* folder name to include with this Set Folder request.  This value  */
   /* can be NULL if no name is required for the selected PathOption.   */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Set_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Set_Folder_Option_t PathOption, char *FolderName)
{
   int                        ret_val;
   unsigned int               FolderNameLength;
   BTPM_Message_t            *ResponseMessage;
   MAPM_Set_Folder_Request_t *SetFolderRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (((PathOption == sfDown) && (FolderName)) || (PathOption != sfDown)))
      {
         /* All that we really need to do is to build a Set Folder      */
         /* message and send it to the server.                          */
         if(FolderName)
            FolderNameLength = BTPS_StringLength(FolderName) + 1;
         else
            FolderNameLength = 0;

         /* Now that the total size has been calculated, attempt to     */
         /* allocate space to hold the message.                         */
         if((SetFolderRequest = (MAPM_Set_Folder_Request_t *)BTPS_AllocateMemory(MAPM_SET_FOLDER_REQUEST_SIZE(FolderNameLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(SetFolderRequest, 0, MAPM_SET_FOLDER_REQUEST_SIZE(FolderNameLength));

            SetFolderRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SetFolderRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SetFolderRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            SetFolderRequest->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_SET_FOLDER;
            SetFolderRequest->MessageHeader.MessageLength   = MAPM_SET_FOLDER_REQUEST_SIZE(FolderNameLength) - BTPM_MESSAGE_HEADER_SIZE;

            SetFolderRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SetFolderRequest->InstanceID                    = InstanceID;
            SetFolderRequest->PathOption                    = PathOption;
            SetFolderRequest->FolderNameLength              = FolderNameLength;

            if(FolderNameLength)
               BTPS_MemCopy(&(((unsigned char *)SetFolderRequest)[MAPM_SET_FOLDER_REQUEST_SIZE(0)]), FolderName, FolderNameLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Setting Folder: %02X%02X%02X%02X%02X%02X, %u, %s, \"%s\"\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ((PathOption == sfRoot) ? "Root" : ((PathOption) == sfDown ? "Down" : ((PathOption == sfUp) ? "Up" : "Invalid"))), (FolderName ? FolderName : "")));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SetFolderRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_RESPONSE_SIZE)
                  ret_val = ((MAPM_Set_Folder_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(SetFolderRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules set the folder to an absolute path.  This function        */
   /* generates a sequence of MAP Set Folder Requests, navigating to the*/
   /* supplied path.  The RemoteDeviceAddress is the address of the     */
   /* remote server.  The InstanceID parameter specifies which server   */
   /* instance on the remote device to use.  The FolderName parameter is*/
   /* a string containing containg a path from the root to the desired  */
   /* folder (i.e. telecom/msg/inbox).  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Set_Folder_Absolute(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName)
{
   int                                 ret_val;
   unsigned int                        FolderNameLength;
   BTPM_Message_t                     *ResponseMessage;
   MAPM_Set_Folder_Absolute_Request_t *SetFolderAbsoluteRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Set Folder      */
         /* message and send it to the server.                          */
         if(FolderName)
            FolderNameLength = BTPS_StringLength(FolderName) + 1;
         else
            FolderNameLength = 0;

         /* Now that the total size has been calculated, attempt to     */
         /* allocate space to hold the message.                         */
         if((SetFolderAbsoluteRequest = (MAPM_Set_Folder_Absolute_Request_t *)BTPS_AllocateMemory(MAPM_SET_FOLDER_ABSOLUTE_REQUEST_SIZE(FolderNameLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(SetFolderAbsoluteRequest, 0, MAPM_SET_FOLDER_ABSOLUTE_REQUEST_SIZE(FolderNameLength));

            SetFolderAbsoluteRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SetFolderAbsoluteRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SetFolderAbsoluteRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            SetFolderAbsoluteRequest->MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_SET_FOLDER_ABSOLUTE;
            SetFolderAbsoluteRequest->MessageHeader.MessageLength   = MAPM_SET_FOLDER_ABSOLUTE_REQUEST_SIZE(FolderNameLength) - BTPM_MESSAGE_HEADER_SIZE;

            SetFolderAbsoluteRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SetFolderAbsoluteRequest->InstanceID                    = InstanceID;
            SetFolderAbsoluteRequest->FolderNameLength              = FolderNameLength;

            if(FolderNameLength)
               BTPS_MemCopy(&(((unsigned char *)SetFolderAbsoluteRequest)[MAPM_SET_FOLDER_ABSOLUTE_REQUEST_SIZE(0)]), FolderName, FolderNameLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Setting Folder: %02X%02X%02X%02X%02X%02X, %u, \"%s\"\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, (FolderName ? FolderName : "")));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SetFolderAbsoluteRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_ABSOLUTE_RESPONSE_SIZE)
                  ret_val = ((MAPM_Set_Folder_Absolute_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(SetFolderAbsoluteRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Get Folder Listing Request */
   /* to the specified remote MAP Server. The RemoteDeviceAddress       */
   /* is the address of the remote server. The InstanceID parameter     */
   /* specifies which server instance on the remote device to use.      */
   /* The MaxListCount is positive, non-zero integer representing the   */
   /* maximum amount of folder entries to return. The ListStartOffset   */
   /* signifies an offset to request. This function returns zero if     */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Get_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   MAPM_Get_Folder_Listing_Request_t  GetFolderListingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Get Folder      */
         /* Listing message and send it to the server.                  */
         BTPS_MemInitialize(&GetFolderListingRequest, 0, MAPM_GET_FOLDER_LISTING_REQUEST_SIZE);

         GetFolderListingRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         GetFolderListingRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         GetFolderListingRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         GetFolderListingRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING;
         GetFolderListingRequest.MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         GetFolderListingRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         GetFolderListingRequest.InstanceID                    = InstanceID;
         GetFolderListingRequest.MaxListCount                  = MaxListCount;
         GetFolderListingRequest.ListStartOffset               = ListStartOffset;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Getting Folder Listing: %02X%02X%02X%02X%02X%02X, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, MaxListCount, ListStartOffset));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetFolderListingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_RESPONSE_SIZE)
               ret_val = ((MAPM_Get_Folder_Listing_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to simply request the size of folder listing. It accepts  */
   /* as a parameter the address of the remote server. The InstanceID   */
   /* parameter specifies which server instance on the remote device    */
   /* to use. This function returns zero if successful and a negative   */
   /* return error code if there was an error.                          */
int _MAPM_Get_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   MAPM_Get_Folder_Listing_Size_Request_t  GetFolderListingSizeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Get Folder      */
         /* Listing Size message and send it to the server.             */
         BTPS_MemInitialize(&GetFolderListingSizeRequest, 0, MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_SIZE);

         GetFolderListingSizeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         GetFolderListingSizeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         GetFolderListingSizeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         GetFolderListingSizeRequest.MessageHeader.MessageFunction = MAPM_MESSAGE_FUNCTION_GET_FOLDER_LISTING_SIZE;
         GetFolderListingSizeRequest.MessageHeader.MessageLength   = MAPM_GET_FOLDER_LISTING_SIZE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         GetFolderListingSizeRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         GetFolderListingSizeRequest.InstanceID                    = InstanceID;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Getting Folder Listing Size: %02X%02X%02X%02X%02X%02X, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetFolderListingSizeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_GET_FOLDER_LISTING_SIZE_RESPONSE_SIZE)
               ret_val = ((MAPM_Get_Folder_Listing_Size_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Get Message Listing Request*/
   /* to the specified remote MAP Server.  The RemoteDeviceAddress is   */
   /* the address of the remote server.  The InstanceID parameter       */
   /* specifies which server instance on the remote device to use.  The */
   /* FolderName parameter specifies the direct sub-directory to pull   */
   /* the listing from.  If this is NULL, the listing will be from the  */
   /* current directory.  The MaxListCount is a positive, non-zero      */
   /* integer representing the maximum amount of folder entries to      */
   /* return.  The ListStartOffset signifies an offset to request.  The */
   /* ListingInfo parameter is an optional parameter which, if          */
   /* specified, points to a structure which contains a set of filters  */
   /* to use when pulling the listing.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Get_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo)
{
   int                                 ret_val;
   unsigned int                        FolderNameLength;
   unsigned int                        FilterRecipientLength;
   unsigned int                        FilterOriginatorLength;
   BTPM_Message_t                     *ResponseMessage;
   MAPM_Get_Message_Listing_Request_t *GetMessageListingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Get Message     */
         /* Listing message and send it to the server.                  */
         if((ListingInfo) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_RECIPIENT_PRESENT) && (ListingInfo->FilterRecipient))
            FilterRecipientLength = BTPS_StringLength(ListingInfo->FilterRecipient) + 1;
         else
            FilterRecipientLength = 0;

         if((ListingInfo) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_ORIGINATOR_PRESENT) && (ListingInfo->FilterOriginator))
            FilterOriginatorLength = BTPS_StringLength(ListingInfo->FilterOriginator) + 1;
         else
            FilterOriginatorLength = 0;

         if(FolderName)
            FolderNameLength = BTPS_StringLength(FolderName) + 1;
         else
            FolderNameLength = 0;

         /* Now that the total size has been calculated, attempt to     */
         /* allocate space to hold the message.                         */
         if((GetMessageListingRequest = (MAPM_Get_Message_Listing_Request_t *)BTPS_AllocateMemory(MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(FilterRecipientLength, FilterOriginatorLength, FolderNameLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(GetMessageListingRequest, 0, MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(FilterRecipientLength, FilterOriginatorLength, FolderNameLength));

            GetMessageListingRequest->MessageHeader.AddressID         = MSG_GetServerAddressID();
            GetMessageListingRequest->MessageHeader.MessageID         = MSG_GetNextMessageID();
            GetMessageListingRequest->MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            GetMessageListingRequest->MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING;
            GetMessageListingRequest->MessageHeader.MessageLength     = MAPM_GET_MESSAGE_LISTING_REQUEST_SIZE(FilterRecipientLength, FilterOriginatorLength, FolderNameLength) - BTPM_MESSAGE_HEADER_SIZE;

            GetMessageListingRequest->RemoteDeviceAddress             = RemoteDeviceAddress;
            GetMessageListingRequest->InstanceID                      = InstanceID;
            GetMessageListingRequest->MaxListCount                    = MaxListCount;
            GetMessageListingRequest->ListStartOffset                 = ListStartOffset;

            if(ListingInfo)
            {
               GetMessageListingRequest->ListingInfoPresent           = TRUE;
               GetMessageListingRequest->ListingInfo                  = *ListingInfo;

               /* Force these pointers to NULL because they are         */
               /* meaningless over IPC. They will be corrected on the   */
               /* receiving side, as appropriate.                       */
               GetMessageListingRequest->ListingInfo.FilterRecipient  = NULL;
               GetMessageListingRequest->ListingInfo.FilterOriginator = NULL;
            }
            else
               GetMessageListingRequest->ListingInfoPresent           = FALSE;

            GetMessageListingRequest->FilterRecipientLength           = FilterRecipientLength;
            GetMessageListingRequest->FilterOriginatorLength          = FilterOriginatorLength;
            GetMessageListingRequest->FolderNameLength                = FolderNameLength;

            if(FilterRecipientLength)
               BTPS_MemCopy(&(GetMessageListingRequest->VariableData[0]), ListingInfo->FilterRecipient, FilterRecipientLength);

            if(FilterOriginatorLength)
               BTPS_MemCopy(&(GetMessageListingRequest->VariableData[FilterRecipientLength]), ListingInfo->FilterOriginator, FilterOriginatorLength);

            if(FolderNameLength)
               BTPS_MemCopy(&(GetMessageListingRequest->VariableData[FilterRecipientLength + FilterOriginatorLength]), FolderName, FolderNameLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Getting Message Listing: %02X%02X%02X%02X%02X%02X, %u, \"%s\", %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, (FolderName ? FolderName : ""), MaxListCount, ListStartOffset));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)GetMessageListingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_RESPONSE_SIZE)
                  ret_val = ((MAPM_Get_Message_Listing_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(GetMessageListingRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to simply request the size of a message listing.  It      */
   /* accepts as a parameter the address of the remote server and the   */
   /* folder name from which to pull the listing.  A value of NULL      */
   /* indicates the current folder should be used.  The InstanceID      */
   /* parameter specifies which server instance on the remote device to */
   /* use.  The ListingInfo parameter is an optional parameter which, if*/
   /* specified, points to a structure which contains a set of filters  */
   /* to use when pulling the listing.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* return error code if there was an error.                          */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Get_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, MAP_Message_Listing_Info_t *ListingInfo)
{
   int                                      ret_val;
   unsigned int                             FolderNameLength;
   unsigned int                             FilterRecipientLength;
   unsigned int                             FilterOriginatorLength;
   BTPM_Message_t                          *ResponseMessage;
   MAPM_Get_Message_Listing_Size_Request_t *GetMessageListingSizeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Get Message     */
         /* Listing Size message and send it to the server.             */
         if((ListingInfo) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_RECIPIENT_PRESENT) && (ListingInfo->FilterRecipient))
            FilterRecipientLength = BTPS_StringLength(ListingInfo->FilterRecipient) + 1;
         else
            FilterRecipientLength = 0;

         if((ListingInfo) && (ListingInfo->OptionMask & MAP_MESSAGE_LISTING_INFO_OPTION_MASK_FILTER_ORIGINATOR_PRESENT) && (ListingInfo->FilterOriginator))
            FilterOriginatorLength = BTPS_StringLength(ListingInfo->FilterOriginator) + 1;
         else
            FilterOriginatorLength = 0;

         if(FolderName)
            FolderNameLength = BTPS_StringLength(FolderName) + 1;
         else
            FolderNameLength = 0;

         /* Now that the total size has been calculated, attempt to     */
         /* allocate space to hold the message.                         */
         if((GetMessageListingSizeRequest = (MAPM_Get_Message_Listing_Size_Request_t *)BTPS_AllocateMemory(MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(FilterRecipientLength, FilterOriginatorLength, FolderNameLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(GetMessageListingSizeRequest, 0, MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(FilterRecipientLength, FilterOriginatorLength, FolderNameLength));

            GetMessageListingSizeRequest->MessageHeader.AddressID         = MSG_GetServerAddressID();
            GetMessageListingSizeRequest->MessageHeader.MessageID         = MSG_GetNextMessageID();
            GetMessageListingSizeRequest->MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            GetMessageListingSizeRequest->MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_GET_MESSAGE_LISTING_SIZE;
            GetMessageListingSizeRequest->MessageHeader.MessageLength     = MAPM_GET_MESSAGE_LISTING_SIZE_REQUEST_SIZE(FilterRecipientLength, FilterOriginatorLength, FolderNameLength) - BTPM_MESSAGE_HEADER_SIZE;

            GetMessageListingSizeRequest->RemoteDeviceAddress             = RemoteDeviceAddress;
            GetMessageListingSizeRequest->InstanceID                      = InstanceID;

            if(ListingInfo)
            {
               GetMessageListingSizeRequest->ListingInfoPresent           = TRUE;
               GetMessageListingSizeRequest->ListingInfo                  = *ListingInfo;

               /* Force these pointers to NULL because they are         */
               /* meaningless over IPC. They will be corrected on the   */
               /* receiving side, as appropriate.                       */
               GetMessageListingSizeRequest->ListingInfo.FilterRecipient  = NULL;
               GetMessageListingSizeRequest->ListingInfo.FilterOriginator = NULL;
            }
            else
               GetMessageListingSizeRequest->ListingInfoPresent           = FALSE;

            GetMessageListingSizeRequest->FilterRecipientLength           = FilterRecipientLength;
            GetMessageListingSizeRequest->FilterOriginatorLength          = FilterOriginatorLength;
            GetMessageListingSizeRequest->FolderNameLength                = FolderNameLength;

            if(FilterRecipientLength)
               BTPS_MemCopy(&(GetMessageListingSizeRequest->VariableData[0]), ListingInfo->FilterRecipient, FilterRecipientLength);

            if(FilterOriginatorLength)
               BTPS_MemCopy(&(GetMessageListingSizeRequest->VariableData[FilterRecipientLength]), ListingInfo->FilterOriginator, FilterOriginatorLength);

            if(FolderNameLength)
               BTPS_MemCopy(&(GetMessageListingSizeRequest->VariableData[FilterRecipientLength + FilterOriginatorLength]), FolderName, FolderNameLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Getting Message Listing Size: %02X%02X%02X%02X%02X%02X, %u, \"%s\"\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, (FolderName ? FolderName : "")));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)GetMessageListingSizeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_LISTING_SIZE_RESPONSE_SIZE)
                  ret_val = ((MAPM_Get_Message_Listing_Size_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(GetMessageListingSizeRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Get Message Request to     */
   /* the specified remote MAP Server. The RemoteDeviceAddress is       */
   /* the address of the remote server. The InstanceID parameter        */
   /* specifies which server instance on the remote device to use. The  */
   /* MessageHandle is a 16 byte NULL terminated string containing      */
   /* Unicode hexadecimal characters which identifies the message.      */
   /* The Attachment parameter indicates whether any attachments to     */
   /* the message should be included in the response. The CharSet and   */
   /* FractionalType parameters specify the format of the response. This*/
   /* function returns zero if successful and a negative return error   */
   /* code if there was an error.                                       */
int _MAPM_Get_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType)
{
   int                         ret_val;
   BTPM_Message_t             *ResponseMessage;
   MAPM_Get_Message_Request_t  GetMessageRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (MessageHandle) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
      {
         /* All that we really need to do is to build a Get Message     */
         /* message and send it to the server.                          */
         BTPS_MemInitialize(&GetMessageRequest, 0, MAPM_GET_MESSAGE_REQUEST_SIZE);

         GetMessageRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         GetMessageRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         GetMessageRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         GetMessageRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_GET_MESSAGE;
         GetMessageRequest.MessageHeader.MessageLength     = MAPM_GET_MESSAGE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         GetMessageRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         GetMessageRequest.InstanceID                      = InstanceID;
         GetMessageRequest.Attachment                      = Attachment;
         GetMessageRequest.CharSet                         = CharSet;
         GetMessageRequest.FractionalType                  = FractionalType;

         BTPS_StringCopy(GetMessageRequest.MessageHandle, MessageHandle);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Getting Message: %02X%02X%02X%02X%02X%02X, %u, \"%s\", %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, MessageHandle, Attachment, CharSet, FractionalType));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetMessageRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_GET_MESSAGE_RESPONSE_SIZE)
               ret_val = ((MAPM_Get_Message_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Set Message Status Request */
   /* to the specified remote MAP Server.  The RemoteDeviceAddress is   */
   /* the address of the remote server.  The InstanceID parameter       */
   /* specifies which server instance on the remote device to use.  The */
   /* MessageHandle is a 16 byte NULL terminated string containing      */
   /* Unicode hexadecimal characters which identifies the message.  The */
   /* StatusIndicator signifies which indicator to be set.  The         */
   /* StatusValue is the value to set.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Set_Message_Status(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   MAPM_Set_Message_Status_Request_t  SetMessageStatusRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (MessageHandle) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
      {
         /* All that we really need to do is to build a Set Message     */
         /* Status message and send it to the server.                   */
         BTPS_MemInitialize(&SetMessageStatusRequest, 0, MAPM_SET_MESSAGE_STATUS_REQUEST_SIZE);

         SetMessageStatusRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         SetMessageStatusRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         SetMessageStatusRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         SetMessageStatusRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SET_MESSAGE_STATUS;
         SetMessageStatusRequest.MessageHeader.MessageLength     = MAPM_SET_MESSAGE_STATUS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetMessageStatusRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         SetMessageStatusRequest.InstanceID                      = InstanceID;
         SetMessageStatusRequest.StatusIndicator                 = StatusIndicator;
         SetMessageStatusRequest.StatusValue                     = StatusValue;

         BTPS_StringCopy(SetMessageStatusRequest.MessageHandle, MessageHandle);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Setting Message Status: %02X%02X%02X%02X%02X%02X, %u, \"%s\", %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, MessageHandle, StatusIndicator, StatusValue));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetMessageStatusRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SET_MESSAGE_STATUS_RESPONSE_SIZE)
               ret_val = ((MAPM_Set_Message_Status_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Push Message Request to the*/
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  The FolderName*/
   /* parameter specifies the direct sub-directory to pull the listing  */
   /* from.  If this is NULL, the listing will be from the current      */
   /* directory.  The Transparent parameter indicates whether a copy    */
   /* should be placed in the sent folder.  The Retry parameter         */
   /* indicates if any retries should be attempted if sending fails.    */
   /* The CharSet specifies the format of the message.  The DataLength  */
   /* parameter indicates the length of the supplied data.  The         */
   /* DataBuffer parameter is a pointer to the data to send.  The Final */
   /* parameter indicates if the buffer supplied is all of the data to  */
   /* be sent.  This function returns zero if successful and a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Push_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                          ret_val;
   unsigned int                 FolderNameLength;
   BTPM_Message_t              *ResponseMessage;
   MAPM_Push_Message_Request_t *PushMessageRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (((DataLength) && (DataBuffer)) || ((!DataLength) && (Final))))
      {
         /* All that we really need to do is to build a Push Message    */
         /* message and send it to the server.                          */
         if(FolderName)
            FolderNameLength = BTPS_StringLength(FolderName) + 1;
         else
            FolderNameLength = 0;

         /* Now that the total size has been calculated, attempt to     */
         /* allocate space to hold the message.                         */
         if((PushMessageRequest = (MAPM_Push_Message_Request_t *)BTPS_AllocateMemory(MAPM_PUSH_MESSAGE_REQUEST_SIZE(FolderNameLength, DataLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(PushMessageRequest, 0, MAPM_PUSH_MESSAGE_REQUEST_SIZE(FolderNameLength, DataLength));

            PushMessageRequest->MessageHeader.AddressID         = MSG_GetServerAddressID();
            PushMessageRequest->MessageHeader.MessageID         = MSG_GetNextMessageID();
            PushMessageRequest->MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            PushMessageRequest->MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE;
            PushMessageRequest->MessageHeader.MessageLength     = MAPM_PUSH_MESSAGE_REQUEST_SIZE(FolderNameLength, DataLength) - BTPM_MESSAGE_HEADER_SIZE;

            PushMessageRequest->RemoteDeviceAddress             = RemoteDeviceAddress;
            PushMessageRequest->InstanceID                      = InstanceID;
            PushMessageRequest->Transparent                     = Transparent;
            PushMessageRequest->Retry                           = Retry;
            PushMessageRequest->CharSet                         = CharSet;
            PushMessageRequest->FolderNameLength                = FolderNameLength;
            PushMessageRequest->MessageLength                   = DataLength;
            PushMessageRequest->Final                           = Final;

            if(FolderNameLength)
               BTPS_MemCopy(&(PushMessageRequest->VariableData[0]), FolderName, FolderNameLength);

            if(DataLength)
               BTPS_MemCopy(&(PushMessageRequest->VariableData[FolderNameLength]), DataBuffer, DataLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pushing Message: %02X%02X%02X%02X%02X%02X, %u, \"%s\", %u, %u, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, (FolderName ? FolderName : ""), Transparent, Retry, CharSet, DataLength, Final));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)PushMessageRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_RESPONSE_SIZE)
                  ret_val = ((MAPM_Push_Message_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(PushMessageRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Update Inbox Request to the*/
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _MAPM_Update_Inbox(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID)
{
   int                          ret_val;
   BTPM_Message_t              *ResponseMessage;
   MAPM_Update_Inbox_Request_t  UpdateInboxRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build an Update Inbox   */
         /* message and send it to the server.                          */
         BTPS_MemInitialize(&UpdateInboxRequest, 0, MAPM_UPDATE_INBOX_REQUEST_SIZE);

         UpdateInboxRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         UpdateInboxRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         UpdateInboxRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         UpdateInboxRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_UPDATE_INBOX;
         UpdateInboxRequest.MessageHeader.MessageLength     = MAPM_UPDATE_INBOX_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UpdateInboxRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         UpdateInboxRequest.InstanceID                      = InstanceID;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Updating Inbox: %02X%02X%02X%02X%02X%02X, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UpdateInboxRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_UPDATE_INBOX_RESPONSE_SIZE)
               ret_val = ((MAPM_Update_Inbox_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Message Access Client (MCE) Notification Functions.               */

   /* The following function generates a MAP Send Event Request to the  */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The DataLength */
   /* Parameter specifies the length of the data.  The Buffer contains  */
   /* the data to be sent.  This function returns zero if successful and*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Send_Notification(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int DataLength, Byte_t *EventData, Boolean_t Final)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   MAPM_Send_Notification_Request_t *SendNotificationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (((DataLength) && (EventData)) || ((!DataLength) && (Final))))
      {
         /* All that we really need to do is to build a Push Message    */
         /* message and send it to the server.                          */
         if((SendNotificationRequest = (MAPM_Send_Notification_Request_t *)BTPS_AllocateMemory(MAPM_SEND_NOTIFICATION_REQUEST_SIZE(DataLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(SendNotificationRequest, 0, MAPM_SEND_NOTIFICATION_REQUEST_SIZE(DataLength));

            SendNotificationRequest->MessageHeader.AddressID         = MSG_GetServerAddressID();
            SendNotificationRequest->MessageHeader.MessageID         = MSG_GetNextMessageID();
            SendNotificationRequest->MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            SendNotificationRequest->MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SEND_NOTIFICATION;
            SendNotificationRequest->MessageHeader.MessageLength     = MAPM_SEND_NOTIFICATION_REQUEST_SIZE(DataLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendNotificationRequest->RemoteDeviceAddress             = RemoteDeviceAddress;
            SendNotificationRequest->InstanceID                      = InstanceID;
            SendNotificationRequest->Final                           = Final;
            SendNotificationRequest->EventDataLength                 = DataLength;

            if(DataLength)
               BTPS_MemCopy(SendNotificationRequest->EventData, EventData, DataLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Notification: %02X%02X%02X%02X%02X%02X, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, DataLength, Final));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendNotificationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SEND_NOTIFICATION_RESPONSE_SIZE)
                  ret_val = ((MAPM_Send_Notification_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(SendNotificationRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Message Access Server (MSE) Functions.                            */

   /* The following function generates a MAP Set Notification           */
   /* Registration Response to the specified remote MAP Client.  The    */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode Parameter is the OBEX Response   */
   /* Code to send with the response.  This function returns zero if    */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Enable_Notifications_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                                               ret_val;
   BTPM_Message_t                                   *ResponseMessage;
   MAPM_Enable_Notifications_Confirmation_Request_t  EnableNotificationsConfirmationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build an Enable         */
         /* Notifications Confirmation message and send it to the       */
         /* server.                                                     */
         BTPS_MemInitialize(&EnableNotificationsConfirmationRequest, 0, MAPM_ENABLE_NOTIFICATIONS_CONFIRMATION_REQUEST_SIZE);

         EnableNotificationsConfirmationRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         EnableNotificationsConfirmationRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         EnableNotificationsConfirmationRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         EnableNotificationsConfirmationRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS_CONFIRMATION;
         EnableNotificationsConfirmationRequest.MessageHeader.MessageLength     = MAPM_ENABLE_NOTIFICATIONS_CONFIRMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableNotificationsConfirmationRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         EnableNotificationsConfirmationRequest.InstanceID                      = InstanceID;
         EnableNotificationsConfirmationRequest.ResponseStatusCode              = ResponseStatusCode;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Confirming Enable Notifications Request: %02X%02X%02X%02X%02X%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableNotificationsConfirmationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_ENABLE_NOTIFICATIONS_CONFIRMATION_RESPONSE_SIZE)
               ret_val = ((MAPM_Enable_Notifications_Confirmation_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Set Folder Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
int _MAPM_Set_Folder_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   MAPM_Set_Folder_Confirmation_Request_t  SetFolderConfirmationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Set Folder      */
         /* Confirmation message and send it to the server.             */
         BTPS_MemInitialize(&SetFolderConfirmationRequest, 0, MAPM_SET_FOLDER_CONFIRMATION_REQUEST_SIZE);

         SetFolderConfirmationRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         SetFolderConfirmationRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         SetFolderConfirmationRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         SetFolderConfirmationRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SET_FOLDER_CONFIRMATION;
         SetFolderConfirmationRequest.MessageHeader.MessageLength     = MAPM_SET_FOLDER_CONFIRMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetFolderConfirmationRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         SetFolderConfirmationRequest.InstanceID                      = InstanceID;
         SetFolderConfirmationRequest.ResponseStatusCode              = ResponseStatusCode;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Confirming Enable Notifications Request: %02X%02X%02X%02X%02X%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetFolderConfirmationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SET_FOLDER_CONFIRMATION_RESPONSE_SIZE)
               ret_val = ((MAPM_Set_Folder_Confirmation_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Folder Listing Response to */
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The DataLength parameter specifies the length of the   */
   /* data.  The Buffer contains the data to be sent.  This function    */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Send_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, unsigned int FolderListingLength, Byte_t *FolderListing, Boolean_t Final)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   MAPM_Send_Folder_Listing_Request_t *SendFolderListingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (((FolderListingLength) && (FolderListing)) || ((!FolderListingLength) && (Final))))
      {
         /* All that we really need to do is to build a Send Folder     */
         /* Listing message and send it to the server.                  */
         if((SendFolderListingRequest = (MAPM_Send_Folder_Listing_Request_t *)BTPS_AllocateMemory(MAPM_SEND_FOLDER_LISTING_REQUEST_SIZE(FolderListingLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(SendFolderListingRequest, 0, MAPM_SEND_FOLDER_LISTING_REQUEST_SIZE(FolderListingLength));

            SendFolderListingRequest->MessageHeader.AddressID         = MSG_GetServerAddressID();
            SendFolderListingRequest->MessageHeader.MessageID         = MSG_GetNextMessageID();
            SendFolderListingRequest->MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            SendFolderListingRequest->MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING;
            SendFolderListingRequest->MessageHeader.MessageLength     = MAPM_SEND_FOLDER_LISTING_REQUEST_SIZE(FolderListingLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendFolderListingRequest->RemoteDeviceAddress             = RemoteDeviceAddress;
            SendFolderListingRequest->InstanceID                      = InstanceID;
            SendFolderListingRequest->ResponseStatusCode              = ResponseStatusCode;
            SendFolderListingRequest->Final                           = Final;
            SendFolderListingRequest->FolderListingLength             = FolderListingLength;

            if(FolderListingLength)
               BTPS_MemCopy(SendFolderListingRequest->FolderListing, FolderListing, FolderListingLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Folder Listing: %02X%02X%02X%02X%02X%02X, %u, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode, FolderListingLength, Final));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendFolderListingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SEND_FOLDER_LISTING_RESPONSE_SIZE)
                  ret_val = ((MAPM_Send_Folder_Listing_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(SendFolderListingRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a Folder Listing Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The Size of the size of the listing to return.  This   */
   /* function returns zero if successful and a negative return error   */
   /* code if there was an error.                                       */
int _MAPM_Send_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t FolderCount)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   MAPM_Send_Folder_Listing_Size_Request_t  SendFolderListingSizeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Send Folder     */
         /* Listing Size message and send it to the server.             */
         BTPS_MemInitialize(&SendFolderListingSizeRequest, 0, MAPM_SEND_FOLDER_LISTING_SIZE_REQUEST_SIZE);

         SendFolderListingSizeRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         SendFolderListingSizeRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         SendFolderListingSizeRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         SendFolderListingSizeRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SEND_FOLDER_LISTING_SIZE;
         SendFolderListingSizeRequest.MessageHeader.MessageLength     = MAPM_SEND_FOLDER_LISTING_SIZE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendFolderListingSizeRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         SendFolderListingSizeRequest.InstanceID                      = InstanceID;
         SendFolderListingSizeRequest.ResponseStatusCode              = ResponseStatusCode;
         SendFolderListingSizeRequest.FolderCount                     = FolderCount;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Folder Listing Size: %02X%02X%02X%02X%02X%02X, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode, FolderCount));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendFolderListingSizeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SEND_FOLDER_LISTING_SIZE_RESPONSE_SIZE)
               ret_val = ((MAPM_Send_Folder_Listing_Size_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Message Listing Response to*/
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The MessageCount supplies the number of messages in the*/
   /* listing.  The NewMessage parameter indicates if there are new     */
   /* messages since the last pull.  CurrentTime indicates the time of  */
   /* the response.  The DataLength parameter specifies the length of   */
   /* the data.  The Buffer contains the data to be sent.  This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Send_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime, unsigned int MessageListingLength, Byte_t *MessageListing, Boolean_t Final)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   MAPM_Send_Message_Listing_Request_t *SendMessageListingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (((MessageListingLength) && (MessageListing)) || ((!MessageListingLength) && (Final))))
      {
         /* All that we really need to do is to build a Send Message    */
         /* Listing message and send it to the server.                  */
         if((SendMessageListingRequest = (MAPM_Send_Message_Listing_Request_t *)BTPS_AllocateMemory(MAPM_SEND_MESSAGE_LISTING_REQUEST_SIZE(MessageListingLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(SendMessageListingRequest, 0, MAPM_SEND_MESSAGE_LISTING_REQUEST_SIZE(MessageListingLength));

            SendMessageListingRequest->MessageHeader.AddressID         = MSG_GetServerAddressID();
            SendMessageListingRequest->MessageHeader.MessageID         = MSG_GetNextMessageID();
            SendMessageListingRequest->MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            SendMessageListingRequest->MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING;
            SendMessageListingRequest->MessageHeader.MessageLength     = MAPM_SEND_MESSAGE_LISTING_REQUEST_SIZE(MessageListingLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendMessageListingRequest->RemoteDeviceAddress             = RemoteDeviceAddress;
            SendMessageListingRequest->InstanceID                      = InstanceID;
            SendMessageListingRequest->ResponseStatusCode              = ResponseStatusCode;
            SendMessageListingRequest->MessageCount                    = MessageCount;
            SendMessageListingRequest->NewMessage                      = NewMessage;

            if(CurrentTime)
               SendMessageListingRequest->CurrentTime                  = *CurrentTime;

            SendMessageListingRequest->Final                           = Final;
            SendMessageListingRequest->MessageListingLength            = MessageListingLength;

            if(MessageListingLength)
               BTPS_MemCopy(SendMessageListingRequest->MessageListing, MessageListing, MessageListingLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Message Listing: %02X%02X%02X%02X%02X%02X, %u, %u, %u, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode, MessageCount, NewMessage, MessageListingLength, Final));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendMessageListingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_LISTING_RESPONSE_SIZE)
                  ret_val = ((MAPM_Send_Message_Listing_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(SendMessageListingRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Message Listing Size       */
   /* Response to the specified remote MAP Client.  The                 */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode parameter is the OBEX Response   */
   /* Code to send with the response.  The Size parameter is the size of*/
   /* the message listing to return.  This function returns zero if     */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Send_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   MAPM_Send_Message_Listing_Size_Request_t  SendMessageListingSizeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Send Message    */
         /* Listing Size message and send it to the server.             */
         BTPS_MemInitialize(&SendMessageListingSizeRequest, 0, MAPM_SEND_MESSAGE_LISTING_SIZE_REQUEST_SIZE);

         SendMessageListingSizeRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         SendMessageListingSizeRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         SendMessageListingSizeRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         SendMessageListingSizeRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SEND_MESSAGE_LISTING_SIZE;
         SendMessageListingSizeRequest.MessageHeader.MessageLength     = MAPM_SEND_MESSAGE_LISTING_SIZE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendMessageListingSizeRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         SendMessageListingSizeRequest.InstanceID                      = InstanceID;
         SendMessageListingSizeRequest.ResponseStatusCode              = ResponseStatusCode;
         SendMessageListingSizeRequest.MessageCount                    = MessageCount;
         SendMessageListingSizeRequest.NewMessage                      = NewMessage;

         if(CurrentTime)
            SendMessageListingSizeRequest.CurrentTime                  = *CurrentTime;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Message Listing Size: %02X%02X%02X%02X%02X%02X, %u, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode, MessageCount, NewMessage));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendMessageListingSizeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_LISTING_SIZE_RESPONSE_SIZE)
               ret_val = ((MAPM_Send_Message_Listing_Size_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Get Message Response       */
   /* Response to the specified remote MAP Client.  The                 */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode parameter is the OBEX Response   */
   /* Code to send with the response.  FractionalType indicates what    */
   /* sort of framented response this is.  The DataLength parameter     */
   /* specifies the length of the data.  The Buffer contains the data to*/
   /* be sent.  This function returns zero if successful and a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * Specifying the FractionalType as ftUnfragmented causes no*/
   /*          FractionalType Header to be added to the OBEX Header     */
   /*          List.  This is the value that should be specified for a a*/
   /*          message that is non-fragmented.  Note that if the Get    */
   /*          Message Indication specified a non-fragmented            */
   /*          FractionalType then you must respond with the correct    */
   /*          non-fragmented FractionalType (i.e. ftMore or ftLast).   */
int _MAPM_Send_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, MAP_Fractional_Type_t FractionalType, unsigned int MessageLength, Byte_t *Message, Boolean_t Final)
{
   int                          ret_val;
   BTPM_Message_t              *ResponseMessage;
   MAPM_Send_Message_Request_t *SendMessageRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (((MessageLength) && (Message)) || ((!MessageLength) && (Final))))
      {
         /* All that we really need to do is to build a Send Message    */
         /* message and send it to the server.                          */
         if((SendMessageRequest = (MAPM_Send_Message_Request_t *)BTPS_AllocateMemory(MAPM_SEND_MESSAGE_REQUEST_SIZE(MessageLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(SendMessageRequest, 0, MAPM_SEND_MESSAGE_REQUEST_SIZE(MessageLength));

            SendMessageRequest->MessageHeader.AddressID         = MSG_GetServerAddressID();
            SendMessageRequest->MessageHeader.MessageID         = MSG_GetNextMessageID();
            SendMessageRequest->MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
            SendMessageRequest->MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_SEND_MESSAGE;
            SendMessageRequest->MessageHeader.MessageLength     = MAPM_SEND_MESSAGE_REQUEST_SIZE(MessageLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendMessageRequest->RemoteDeviceAddress             = RemoteDeviceAddress;
            SendMessageRequest->InstanceID                      = InstanceID;
            SendMessageRequest->ResponseStatusCode              = ResponseStatusCode;
            SendMessageRequest->FractionalType                  = FractionalType;
            SendMessageRequest->Final                           = Final;
            SendMessageRequest->MessageDataLength               = MessageLength;

            if(MessageLength)
               BTPS_MemCopy(SendMessageRequest->MessageData, Message, MessageLength);

            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Message: %02X%02X%02X%02X%02X%02X, %u, %u, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode, FractionalType, MessageLength, Final));

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendMessageRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_SEND_MESSAGE_RESPONSE_SIZE)
                  ret_val = ((MAPM_Send_Message_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(SendMessageRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Set Message Status Response*/
   /* to the specified remote MAP Client.  The RemoteDeviceAddress is   */
   /* the address of the remote client.  The InstanceID parameter       */
   /* specifies which server instance on the local device to use.  The  */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
int _MAPM_Set_Message_Status_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                                             ret_val;
   BTPM_Message_t                                 *ResponseMessage;
   MAPM_Message_Status_Confirmation_Request_t  SetMessageStatusConfirmationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build a Set Message     */
         /* Status Confirmation message and send it to the server.      */
         BTPS_MemInitialize(&SetMessageStatusConfirmationRequest, 0, MAPM_MESSAGE_STATUS_CONFIRMATION_REQUEST_SIZE);

         SetMessageStatusConfirmationRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         SetMessageStatusConfirmationRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         SetMessageStatusConfirmationRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         SetMessageStatusConfirmationRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_MESSAGE_STATUS_CONFIRMATION;
         SetMessageStatusConfirmationRequest.MessageHeader.MessageLength     = MAPM_MESSAGE_STATUS_CONFIRMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetMessageStatusConfirmationRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         SetMessageStatusConfirmationRequest.InstanceID                      = InstanceID;
         SetMessageStatusConfirmationRequest.ResponseStatusCode              = ResponseStatusCode;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Set Message Status Confirmation: %02X%02X%02X%02X%02X%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetMessageStatusConfirmationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_MESSAGE_STATUS_CONFIRMATION_RESPONSE_SIZE)
               ret_val = ((MAPM_Message_Status_Confirmation_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Push Message Response to   */
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The message handle is the handle for the client to     */
   /* refer to the message.  This function returns zero if successful   */
   /* and a negative return error code if there was an error.           */
int _MAPM_Push_Message_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, char *MessageHandle)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   MAPM_Push_Message_Confirmation_Request_t  PushMessageConfirmationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)) && (MessageHandle) && (BTPS_StringLength(MessageHandle) <= MAP_MESSAGE_HANDLE_LENGTH))
      {
         /* All that we really need to do is to build a Push Message    */
         /* Confirmation message and send it to the server.             */
         BTPS_MemInitialize(&PushMessageConfirmationRequest, 0, MAPM_PUSH_MESSAGE_CONFIRMATION_REQUEST_SIZE);

         PushMessageConfirmationRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         PushMessageConfirmationRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         PushMessageConfirmationRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         PushMessageConfirmationRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_PUSH_MESSAGE_CONFIRMATION;
         PushMessageConfirmationRequest.MessageHeader.MessageLength     = MAPM_PUSH_MESSAGE_CONFIRMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         PushMessageConfirmationRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         PushMessageConfirmationRequest.InstanceID                      = InstanceID;
         PushMessageConfirmationRequest.ResponseStatusCode              = ResponseStatusCode;

         BTPS_StringCopy(PushMessageConfirmationRequest.MessageHandle, MessageHandle);

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Push Message Confirmation: %02X%02X%02X%02X%02X%02X, %u, %u, \"%s\"\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode, MessageHandle));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&PushMessageConfirmationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_PUSH_MESSAGE_CONFIRMATION_RESPONSE_SIZE)
               ret_val = ((MAPM_Push_Message_Confirmation_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a MAP Update Inbox Response to   */
   /* the specified remote MAP Client. The RemoteDeviceAddress is the   */
   /* address of the remote client. The InstanceID parameter specifies  */
   /* which server instance on the local device to use. The ResponseCode*/
   /* parameter is the OBEX Response Code to send with the response.    */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
int _MAPM_Update_Inbox_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   MAPM_Update_Inbox_Confirmation_Request_t  UpdateInboxConfirmationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((InstanceID >= MAPM_INSTANCE_ID_MINIMUM_VALUE) && (InstanceID <= MAPM_INSTANCE_ID_MAXIMUM_VALUE)))
      {
         /* All that we really need to do is to build an Update Inbox   */
         /* Confirmation message and send it to the server.             */
         BTPS_MemInitialize(&UpdateInboxConfirmationRequest, 0, MAPM_UPDATE_INBOX_CONFIRMATION_REQUEST_SIZE);

         UpdateInboxConfirmationRequest.MessageHeader.AddressID         = MSG_GetServerAddressID();
         UpdateInboxConfirmationRequest.MessageHeader.MessageID         = MSG_GetNextMessageID();
         UpdateInboxConfirmationRequest.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_MESSAGE_ACCESS_MANAGER;
         UpdateInboxConfirmationRequest.MessageHeader.MessageFunction   = MAPM_MESSAGE_FUNCTION_UPDATE_INBOX_CONFIRMATION;
         UpdateInboxConfirmationRequest.MessageHeader.MessageLength     = MAPM_UPDATE_INBOX_CONFIRMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UpdateInboxConfirmationRequest.RemoteDeviceAddress             = RemoteDeviceAddress;
         UpdateInboxConfirmationRequest.InstanceID                      = InstanceID;
         UpdateInboxConfirmationRequest.ResponseStatusCode              = ResponseStatusCode;

         DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Sending Update Inbox Confirmation: %02X%02X%02X%02X%02X%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, InstanceID, ResponseStatusCode));

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UpdateInboxConfirmationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= MAPM_UPDATE_INBOX_CONFIRMATION_RESPONSE_SIZE)
               ret_val = ((MAPM_Update_Inbox_Confirmation_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
