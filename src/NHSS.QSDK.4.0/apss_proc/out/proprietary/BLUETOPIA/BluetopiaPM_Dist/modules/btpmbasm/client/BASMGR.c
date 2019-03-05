/*****< basmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BASMGR - BAS Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMBASM.h"            /* BTPM BAS Manager Prototypes/Constants.    */
#include "BASMMSG.h"             /* BTPM BAS Manager Message Formats.         */
#include "BASMGR.h"              /* BAS Manager Impl. Prototypes/Constants.   */

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

   /* Internal Function Prototypes.                                     */
static int FormatSendRequestResponse(BTPM_Message_t *RequestMessage);
static int FormatSendStandardRequest(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, unsigned int MessageFunction);

   /* The following function formats the AddressID, MessageID, and      */
   /* MessageGroup of a message, sends the message, and returns the     */
   /* status of the response message.                                   */
static int FormatSendRequestResponse(BTPM_Message_t *RequestMessage)
{
   int             ret_val;
   BTPM_Message_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the header.                                                */
   RequestMessage->MessageHeader.AddressID    = MSG_GetServerAddressID();
   RequestMessage->MessageHeader.MessageID    = MSG_GetNextMessageID();
   RequestMessage->MessageHeader.MessageGroup = BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER;

   /* Message has been formatted, go ahead and send it off.             */
   if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)RequestMessage, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
   {
      /* Response received, go ahead and see if the return value is     */
      /* valid.  If it is, go ahead and note the returned status.       */
      if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= (BASM_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE))
         ret_val = ((BASM_Response_Message_t *)ResponseMessage)->Status;
      else
         ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* All finished with the message, go ahead and free it.           */
      MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function formats and sends a standard request       */
   /* message given the Battery Instance's Bluetooth Address and        */
   /* Instance ID as well as the MessageFunction to request.  The       */
   /* response status is returned.                                      */
static int FormatSendStandardRequest(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, unsigned int MessageFunction)
{
   int                    ret_val;
   BASM_Request_Message_t RequestMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Module-specific fields first.                          */
   RequestMessage.CallbackID                               = CallbackID;
   RequestMessage.RemoteDeviceAddress                      = *RemoteServer;
   RequestMessage.InstanceID                               = InstanceID;
   RequestMessage.MessageHeader.MessageFunction            = MessageFunction;
   RequestMessage.MessageHeader.MessageLength              = BASM_REQUEST_MESSAGE_SIZE;

   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&RequestMessage);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the BAS Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager BAS Manager  */
   /* Implementation.                                                   */
int _BASM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the BAS   */
   /* Manager Implementation.  After this function is called the BAS    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _BASM_Initialize() function.  */
void _BASM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Battery Service  */
   /* (BAS) Manager.  This Callback will be dispatched by the BAS       */
   /* Manager when various BAS Manager Events occur.  This function     */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BASM_Un_Register_Client_Events() function to un-register */
   /*          the callback from this module.                           */
int _BASM_Register_Client_Event_Callback(void)
{
   int                                   ret_val;
   BASM_Register_Client_Events_Request_t RegisterClientEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* All that we really need to do is to build a Register BAS Events   */
   /* message and send it to the server.                                */
   RegisterClientEventsRequest.MessageHeader.MessageFunction = BASM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS;
   RegisterClientEventsRequest.MessageHeader.MessageLength   = BASM_REGISTER_CLIENT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

   /* Message has been formatted, go ahead and send it off.             */
   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&RegisterClientEventsRequest);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BAS Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BASM_Register_Client_Events() function).  This function accepts as*/
   /* input the BAS Manager Event Callback ID (return value from        */
   /* BASM_Register_Client_Events() function).                          */
int _BASM_Un_Register_Client_Event_Callback(unsigned int CallbackID)
{
   int                                       ret_val;
   BASM_Un_Register_Client_Events_Request_t  UnRegisterClientEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* All that we really need to do is to build an Un-Register BAS      */
   /* Events message and send it to the server.                         */
   UnRegisterClientEventsRequest.MessageHeader.MessageFunction = BASM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS;
   UnRegisterClientEventsRequest.MessageHeader.MessageLength   = BASM_UN_REGISTER_CLIENT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   UnRegisterClientEventsRequest.CallbackID                = CallbackID;

   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&UnRegisterClientEventsRequest);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to Enable */
   /* Notifications for a specific Battery Service instance on a remote */
   /* device.  This function accepts as input the Callback ID (obtained */
   /* from _BASM_Register_Client_Event_Callback() function) as the first*/
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device.  The third parameter is the Instance*/
   /* ID of the Battery server on the remote device.  This function     */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int _BASM_Enable_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   return(FormatSendStandardRequest(CallbackID, RemoteServer, InstanceID, BASM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS));
}

   /* The following function is provided to allow a mechanism to Disable*/
   /* Notifications for a specific Battery Service instance on a remote */
   /* device.  This function accepts as input the Callback ID (obtained */
   /* from _BASM_Register_Client_Event_Callback() function) as the first*/
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device.  The third parameter is the Instance*/
   /* ID of the Battery server on the remote device.  This function     */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int _BASM_Disable_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   return(FormatSendStandardRequest(CallbackID, RemoteServer, InstanceID, BASM_MESSAGE_FUNCTION_DISABLE_NOTIFICATIONS));
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Level Request to a remote server.  This function    */
   /* accepts as input the Callback ID (obtained from                   */
   /* _BASM_Register_Client_Event_Callback() function) as the first     */
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device to request the Battery Level from.   */
   /* The third parameter is the Instance ID of the Battery server on   */
   /* the remote device.  This function returns a positive Transaction  */
   /* ID on success; otherwise, a negative error value is returned.     */
int _BASM_Get_Battery_Level(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   return(FormatSendStandardRequest(CallbackID, RemoteServer, InstanceID, BASM_MESSAGE_FUNCTION_GET_BATTERY_LEVEL));
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Identification Request to a remote server.  This    */
   /* function accepts as input the Callback ID (obtained from          */
   /* _BASM_Register_Client_Event_Callback() function) as the first     */
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device to request the Battery Identification*/
   /* from.  The third parameter is the Instance ID of the Battery      */
   /* server on the remote device.  This function returns a positive    */
   /* Transaction ID on success; otherwise, a negative error value is   */
   /* returned.                                                         */
int _BASM_Get_Battery_Identification(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   return(FormatSendStandardRequest(CallbackID, RemoteServer, InstanceID, BASM_MESSAGE_FUNCTION_GET_BATTERY_IDENTIFICATION));
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (obtained from                   */
   /* _BASM_Register_Client_Event_Callback() function) as the first     */
   /* parameter.  The second parameter is the Transaction ID of the     */
   /* outstanding transaction.  This function returns zero on success;  */
   /* otherwise, a negative error value is returned.                    */
int _BASM_Cancel_Transaction(unsigned int CallbackID, unsigned int TransactionID)
{
   int                               ret_val;
   BASM_Cancel_Transaction_Request_t CancelTransactionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Module-specific fields first.                          */
   CancelTransactionRequest.MessageHeader.MessageFunction = BASM_MESSAGE_FUNCTION_CANCEL_TRANSACTION;
   CancelTransactionRequest.MessageHeader.MessageLength   = BASM_REQUEST_MESSAGE_SIZE;
   CancelTransactionRequest.TransactionID                 = TransactionID;

   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&CancelTransactionRequest);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
