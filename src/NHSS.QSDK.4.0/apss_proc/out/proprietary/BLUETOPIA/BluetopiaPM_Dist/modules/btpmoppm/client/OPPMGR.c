/*****< oppmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OPPMGR - Object Push Profile Manager Implementation for Stonestreet One   */
/*          Bluetooth Protocol Stack Platform Manager.                        */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/11/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMOPPM.h"            /* BTPM OPP Manager Prototypes/Constants.    */
#include "OPPMMSG.h"             /* BTPM OPP Manager Message Formats.         */
#include "OPPMGR.h"              /* OPP Manager Impl. Prototypes/Constants.   */

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
int _OPPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the PM client module has been initialized.               */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Message Access Manager (Imp)\n"));

      /* Flag that the PM client module is initialized.                 */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* This function sets the PM client Initialized global flag to false.*/
   /* The other methods will fail until _OPPM_Initialize has been       */
   /* called.                                                           */
void _OPPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the PM client module has been initialized.               */
   if(Initialized)
   {
      /* Flag that the PM client module is not initialized.             */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming OPP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A OPP Connected   */
   /*          event will be dispatched to signify the actual result.   */
int _OPPM_Connection_Request_Response(unsigned int ServerID, Boolean_t Accept)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   OPPM_Connection_Request_Response_Request_t  ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(ServerID)
      {
         /* All that we really need to do is to build a Connection      */
         /* Request message and send it to the server.                  */
         BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

         ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         ConnectionRequestResponseRequest.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
         ConnectionRequestResponseRequest.MessageHeader.MessageLength   = OPPM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectionRequestResponseRequest.ServerID                      = ServerID;
         ConnectionRequestResponseRequest.Accept                        = Accept;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
               ret_val = ((OPPM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allows a mechanism for local*/
   /* modules to register an Object Push Server. This first parameter   */
   /* is the RFCOMM server port. If this parameter is zero, the server  */
   /* will be opened on an available port. The SupportedObjectTypes     */
   /* parameter is a bitmask representing the types of objects supported*/
   /* by this server. The IncomingConnectionFlags parameter is a        */
   /* bitmask which indicates whether incoming connections should       */
   /* be authorized, autenticated, or encrypted. The ServiceName        */
   /* parameter is a null-terminate string represting the name of the   */
   /* service to be placed in the Service Record. The EventCallback     */
   /* is the function which will receive events related to this         */
   /* server. The CallbackParameter will be included in each call to    */
   /* the CallbackFunction. This function returns a positive integer    */
   /* representing the ServerID of the created server if successful and */
   /* a negative error code if there was an error.                      */
int _OPPM_Register_Server(unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName)
{
   int                              ret_val;
   unsigned int                     ServiceNameLength;
   BTPM_Message_t                  *ResponseMessage;
   OPPM_Register_Server_Request_t  *RegisterServerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %08lX\n", SupportedObjectTypes));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(SupportedObjectTypes)
      {
         /* Calculate the length needed for the Service Name.           */
         if(ServiceName)
            ServiceNameLength = BTPS_StringLength(ServiceName) + 1;
         else
            ServiceNameLength = 0;

         /* Allocate the space to hold the message.                     */
         if((RegisterServerRequest = (OPPM_Register_Server_Request_t *)BTPS_AllocateMemory(OPPM_REGISTER_SERVER_REQUEST_SIZE(ServiceNameLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(RegisterServerRequest, 0, OPPM_REGISTER_SERVER_REQUEST_SIZE(ServiceNameLength));

            RegisterServerRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            RegisterServerRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            RegisterServerRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
            RegisterServerRequest->MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_REGISTER_SERVER;
            RegisterServerRequest->MessageHeader.MessageLength   = OPPM_REGISTER_SERVER_REQUEST_SIZE(ServiceNameLength) - BTPM_MESSAGE_HEADER_SIZE;

            RegisterServerRequest->ServerPort                    = ServerPort;
            RegisterServerRequest->SupportedObjectTypes          = SupportedObjectTypes;
            RegisterServerRequest->IncomingConnectionFlags       = IncomingConnectionFlags;
            RegisterServerRequest->ServiceNameLength             = ServiceNameLength;

            if(ServiceNameLength)
               BTPS_MemCopy(RegisterServerRequest->ServiceName, ServiceName, ServiceNameLength);

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)RegisterServerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_REGISTER_SERVER_RESPONSE_SIZE)
                  ret_val = ((OPPM_Register_Server_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(RegisterServerRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allows a mechanism for      */
   /* local modules to register an Object Push Server registered by a   */
   /* successful call to OPPM_Register_Server(). This function accepts  */
   /* as a parameter the ServerID returned from a successful call to    */
   /* OPPM_Register_Server(). This function returns zero if successful  */
   /* and a negative error code if there was an error.                  */
int _OPPM_Un_Register_Server(unsigned int ServerID)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   OPPM_Un_Register_Server_Request_t  UnRegisterServerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(ServerID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&UnRegisterServerRequest, 0, sizeof(UnRegisterServerRequest));

         UnRegisterServerRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterServerRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterServerRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         UnRegisterServerRequest.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER;
         UnRegisterServerRequest.MessageHeader.MessageLength   = OPPM_UN_REGISTER_SERVER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterServerRequest.ServerID                      = ServerID;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterServerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_UN_REGISTER_SERVER_RESPONSE_SIZE)
               ret_val = ((OPPM_Un_Register_Server_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Object Push Server device.  The    */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The ConnectionFlags*/
   /* parameter specifies whether authentication or encryption should   */
   /* be used to create this connection.  The CallbackFunction is the   */
   /* function that will be registered to receive events for this       */
   /* connection.  The CallbackParameter is a parameter which will be   */
   /* included in the status callback.  This function returns zero if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Message Access Manager Event Callback supplied.          */
int _OPPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   OPPM_Connect_Remote_Device_Request_t  ConnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&ConnectRemoteDeviceRequest, 0, sizeof(ConnectRemoteDeviceRequest));

         ConnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         ConnectRemoteDeviceRequest.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE;
         ConnectRemoteDeviceRequest.MessageHeader.MessageLength   = OPPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectRemoteDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectRemoteDeviceRequest.RemoteServerPort              = RemoteServerPort;
         ConnectRemoteDeviceRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
               ret_val = ((OPPM_Connect_Remote_Device_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Object Push      */
   /* connection that was previously opened by a successful call to     */
   /* OPPM_Connect_Remote_Device() function or by a oetConnected        */
   /* event. This function accpets the either the ClientID or ServerID  */
   /* of the connection as a parameter. This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
int _OPPM_Disconnect(unsigned int PortID)
{
   int                        ret_val;
   BTPM_Message_t            *ResponseMessage;
   OPPM_Disconnect_Request_t  DisconnectRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(PortID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&DisconnectRequest, 0, sizeof(DisconnectRequest));

         DisconnectRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisconnectRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisconnectRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         DisconnectRequest.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_DISCONNECT;
         DisconnectRequest.MessageHeader.MessageLength   = OPPM_DISCONNECT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisconnectRequest.PortID                        = PortID;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_DISCONNECT_RESPONSE_SIZE)
               ret_val = ((OPPM_Disconnect_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding OPPM profile client request.  This function accepts as*/
   /* input the ClientID of the device specifying which connection is to*/
   /* have the Abort issued.  This function returns zero if successful, */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int _OPPM_Abort(unsigned int ClientID)
{
   int                   ret_val;
   BTPM_Message_t       *ResponseMessage;
   OPPM_Abort_Request_t  AbortRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(ClientID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&AbortRequest, 0, sizeof(AbortRequest));

         AbortRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         AbortRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         AbortRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         AbortRequest.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_ABORT;
         AbortRequest.MessageHeader.MessageLength   = OPPM_ABORT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         AbortRequest.ClientID                      = ClientID;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&AbortRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_ABORT_RESPONSE_SIZE)
               ret_val = ((OPPM_Abort_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter    */
   /* is the ClientID of the remote device connection. The ObjectType   */
   /* parameter specifies the type of object being pushed. The Object   */
   /* Name parameter is a UNICODE encoded string representing the name  */
   /* of the object to push. The DataLength and DataBuffer specify the  */
   /* length and contents of the object. This function returns zero if  */
   /* successful and a negative error code if there is an error.        */
   /* * NOTE * The Object Name is a pointer to a NULL Terminated        */
   /*          UNICODE String.                                          */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _OPPM_Push_Object_Request(unsigned int ClientID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                                  ret_val;
   unsigned int                         ObjectNameLength;
   BTPM_Message_t                      *ResponseMessage;
   OPPM_Push_Object_Request_Request_t  *PushObjectRequestRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((ClientID) && ((!DataLength) || ((DataLength) && (DataBuffer))))
      {
         /* Calculate the length needed for the Service Name.           */
         if(ObjectName)
            ObjectNameLength = BTPS_StringLength(ObjectName) + 1;
         else
            ObjectNameLength = 0;

         /* Allocate the space to hold the message.                     */
         if((PushObjectRequestRequest = (OPPM_Push_Object_Request_Request_t *)BTPS_AllocateMemory(OPPM_PUSH_OBJECT_REQUEST_REQUEST_SIZE(ObjectNameLength, DataLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(PushObjectRequestRequest, 0, OPPM_PUSH_OBJECT_REQUEST_REQUEST_SIZE(ObjectNameLength, DataLength));

            PushObjectRequestRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            PushObjectRequestRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            PushObjectRequestRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
            PushObjectRequestRequest->MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_REQUEST;
            PushObjectRequestRequest->MessageHeader.MessageLength   = OPPM_PUSH_OBJECT_REQUEST_REQUEST_SIZE(ObjectNameLength, DataLength) - BTPM_MESSAGE_HEADER_SIZE;

            PushObjectRequestRequest->ClientID                      = ClientID;
            PushObjectRequestRequest->ObjectType                    = ObjectType;
            PushObjectRequestRequest->ObjectTotalLength             = ObjectTotalLength;
            PushObjectRequestRequest->Final                         = Final;
            PushObjectRequestRequest->ObjectNameLength              = ObjectNameLength;
            PushObjectRequestRequest->DataLength                    = DataLength;

            if(ObjectNameLength)
               BTPS_MemCopy(PushObjectRequestRequest->VariableData, ObjectName, ObjectNameLength);

            if(DataLength)
               BTPS_MemCopy(&(PushObjectRequestRequest->VariableData[ObjectNameLength]), DataBuffer, DataLength);

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)PushObjectRequestRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_PUSH_OBJECT_REQUEST_RESPONSE_SIZE)
                  ret_val = ((OPPM_Push_Object_Request_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(PushObjectRequestRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an Object       */
   /* Push Response to the remote Client.  The first parameter is       */
   /* the ServerID of the local Object Push server. The ResponseCode    */
   /* parameter is the OPPM response status code associated with this   */
   /* response. The function returns zero if successful and a negative  */
   /* error code if there is an error.                                  */
int _OPPM_Push_Object_Response(unsigned int ServerID, unsigned int ResponseCode)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   OPPM_Push_Object_Response_Request_t  PushObjectResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(ServerID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&PushObjectResponseRequest, 0, sizeof(PushObjectResponseRequest));

         PushObjectResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         PushObjectResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         PushObjectResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         PushObjectResponseRequest.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_RESPONSE;
         PushObjectResponseRequest.MessageHeader.MessageLength   = OPPM_PUSH_OBJECT_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         PushObjectResponseRequest.ServerID                      = ServerID;
         PushObjectResponseRequest.ResponseCode                  = ResponseCode;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&PushObjectResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_PUSH_OBJECT_RESPONSE_RESPONSE_SIZE)
               ret_val = ((OPPM_Push_Object_Response_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The Client        */
   /* parameter is the ClientID of the remote Object Push server        */
   /* connection. This function returns zero if successful and a        */
   /* negative error code if there was an error.                        */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int _OPPM_Pull_Business_Card_Request(unsigned int ClientID)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   OPPM_Pull_Business_Card_Request_Request_t  PullBusinessCardRequestRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if(ClientID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&PullBusinessCardRequestRequest, 0, sizeof(PullBusinessCardRequestRequest));

         PullBusinessCardRequestRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         PullBusinessCardRequestRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         PullBusinessCardRequestRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         PullBusinessCardRequestRequest.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_REQUEST;
         PullBusinessCardRequestRequest.MessageHeader.MessageLength   = OPPM_PULL_BUSINESS_CARD_REQUEST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         PullBusinessCardRequestRequest.ClientID                      = ClientID;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&PullBusinessCardRequestRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_PULL_BUSINESS_CARD_REQUEST_RESPONSE_SIZE)
               ret_val = ((OPPM_Pull_Business_Card_Request_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter is the   */
   /* ServerID of the remote Object Push client. The ResponseCode       */
   /* parameter is the OPPM response status code associated with the    */
   /* response. The DataLength and DataBuffer parameters contain the    */
   /* business card data to be sent. This function returns zero if      */
   /* successful and a negative return error code if there is an error. */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _OPPM_Pull_Business_Card_Response(unsigned int ServerID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   OPPM_Pull_Business_Card_Response_Request_t  *PullBusinessCardResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((ServerID) && ((!DataLength) || ((DataLength) && (DataBuffer))))
      {
         /* Allocate the space to hold the message.                     */
         if((PullBusinessCardResponseRequest = (OPPM_Pull_Business_Card_Response_Request_t *)BTPS_AllocateMemory(OPPM_PULL_BUSINESS_CARD_RESPONSE_REQUEST_SIZE(DataLength))) != NULL)
         {
            /* Memory allocated, go ahead and format the message.       */
            BTPS_MemInitialize(PullBusinessCardResponseRequest, 0, OPPM_PULL_BUSINESS_CARD_RESPONSE_REQUEST_SIZE(DataLength));

            PullBusinessCardResponseRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            PullBusinessCardResponseRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            PullBusinessCardResponseRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
            PullBusinessCardResponseRequest->MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_RESPONSE;
            PullBusinessCardResponseRequest->MessageHeader.MessageLength   = OPPM_PULL_BUSINESS_CARD_RESPONSE_REQUEST_SIZE(DataLength) - BTPM_MESSAGE_HEADER_SIZE;

            PullBusinessCardResponseRequest->ServerID                      = ServerID;
            PullBusinessCardResponseRequest->ResponseCode                  = ResponseCode;
            PullBusinessCardResponseRequest->ObjectTotalLength             = ObjectTotalLength;
            PullBusinessCardResponseRequest->Final                         = Final;
            PullBusinessCardResponseRequest->DataLength                    = DataLength;

            if(DataLength)
               BTPS_MemCopy(PullBusinessCardResponseRequest->DataBuffer, DataBuffer, DataLength);

            /* Message has been formatted, go ahead and send it off.    */
            if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)PullBusinessCardResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= OPPM_PULL_BUSINESS_CARD_RESPONSE_RESPONSE_SIZE)
                  ret_val = ((OPPM_Pull_Business_Card_Response_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;
            }

            /* Free the memory that was allocated.                      */
            BTPS_FreeMemory(PullBusinessCardResponseRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

