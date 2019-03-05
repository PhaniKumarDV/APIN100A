/*****< blpmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLPMGR - BLP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMBLPM.h"            /* BTPM BLP Manager Prototypes/Constants.    */
#include "BLPMMSG.h"             /* BTPM BLP Manager Message Formats.         */
#include "BLPMGR.h"              /* BLP Manager Impl. Prototypes/Constants.   */

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
static int FormatSendDeviceRequest(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int MessageFunction);

   /* The following function formats the AddressID, MessageID, and      */
   /* MessageGroup of a message, sends the message, and returns the     */
   /* status of the response message.                                   */
static int FormatSendRequestResponse(BTPM_Message_t *RequestMessage)
{
   int             ret_val;
   BTPM_Message_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the header.                                                */
   RequestMessage->MessageHeader.AddressID    = MSG_GetServerAddressID();
   RequestMessage->MessageHeader.MessageID    = MSG_GetNextMessageID();
   RequestMessage->MessageHeader.MessageGroup = BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER;

   /* Message has been formatted, go ahead and send it off.             */
   if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)RequestMessage, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
   {
      /* Response received, go ahead and see if the return value is     */
      /* valid.  If it is, go ahead and note the returned status.       */
      if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= (BLPM_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE))
         ret_val = ((BLPM_Response_Message_t *)ResponseMessage)->Status;
      else
         ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* All finished with the message, go ahead and free it.           */
      MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function formats and sends a device request message */
   /* given the Blood Pressure Sensor's Bluetooth Address as well as the*/
   /* MessageFunction to request.  The response status is returned.     */
static int FormatSendDeviceRequest(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, unsigned int MessageFunction)
{
   int                           ret_val;
   BLPM_Device_Request_Message_t RequestMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Module-specific fields first.                          */
   RequestMessage.CallbackID                    = CallbackID;
   RequestMessage.RemoteDeviceAddress           = *RemoteSensor;
   RequestMessage.MessageHeader.MessageFunction = MessageFunction;
   RequestMessage.MessageHeader.MessageLength   = BLPM_DEVICE_REQUEST_MESSAGE_SIZE;

   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&RequestMessage);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the BLP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager BLP Manager  */
   /* Implementation.                                                   */
int _BLPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the BLP   */
   /* Manager Implementation.  After this function is called the BLP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _BLPM_Initialize() function.  */
void _BLPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Blood Pressure   */
   /* (BLP) Manager Service.  This Callback will be dispatched by the   */
   /* BLP Manager when various BLP Manager Events occur.  This function */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a BLP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BLPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int _BLPM_Register_Collector_Event_Callback(BLPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                                      ret_val;
   BLPM_Register_Collector_Events_Request_t RegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* All that we really need to do is to build a Register BLP Events   */
   /* message and send it to the server.                                */
   RegisterEventsRequest.MessageHeader.MessageFunction = BLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS;
   RegisterEventsRequest.MessageHeader.MessageLength   = BLPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

   /* Message has been formatted, go ahead and send it off.             */
   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&RegisterEventsRequest);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BLP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BLPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the BLP Manager Event Callback ID (return value  */
   /* from BLPM_Register_Collector_Event_Callback() function).          */
int _BLPM_Un_Register_Collector_Event_Callback(unsigned int CallbackID)
{
   int                                         ret_val;
   BLPM_Un_Register_Collector_Events_Request_t UnRegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* All that we really need to do is to build an Un-Register BLP      */
   /* Events message and send it to the server.                         */
   UnRegisterEventsRequest.MessageHeader.MessageFunction = BLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS;
   UnRegisterEventsRequest.MessageHeader.MessageLength   = BLPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   UnRegisterEventsRequest.CallbackID                    = CallbackID;

   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&UnRegisterEventsRequest);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to enable */
   /* notifications for Blood Pressure Measurements on a specified Blood*/
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable notifications on.*/
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Enable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   return(FormatSendDeviceRequest(CallbackID, RemoteSensor, BLPM_MESSAGE_FUNCTION_ENABLE_BPM_INDICATIONS));
}

   /* The following function is provided to allow a mechanism to disable*/
   /* notifications for Blood Pressure Measurements on a specified Blood*/
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable notifications on.*/
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Disable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   return(FormatSendDeviceRequest(CallbackID, RemoteSensor, BLPM_MESSAGE_FUNCTION_DISABLE_BPM_INDICATIONS));
}

   /* The following function is provided to allow a mechanism to enable */
   /* indications for Intermediate Cuff Pressure on a specified Blood   */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable indications on.  */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Enable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   return(FormatSendDeviceRequest(CallbackID, RemoteSensor, BLPM_MESSAGE_FUNCTION_ENABLE_ICP_NOTIFICATIONS));
}

   /* The following function is provided to allow a mechanism to disable*/
   /* indications for Intermediate Cuff Pressure on a specified Blood   */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable indications on.  */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Disable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   return(FormatSendDeviceRequest(CallbackID, RemoteSensor, BLPM_MESSAGE_FUNCTION_DISABLE_ICP_NOTIFICATIONS));
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Blood Pressure Feature Request to a remote sensor.  This    */
   /* function accepts as input the Callback ID (return value from      */
   /* BLPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Blood Pressure Feature from.  This   */
   /* function returns a positive Transaction ID on success; otherwise, */
   /* a negative error value is returned.                               */
int _BLPM_Get_Blood_Pressure_Feature(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   return(FormatSendDeviceRequest(CallbackID, RemoteSensor, BLPM_MESSAGE_FUNCTION_GET_BLOOD_PRESSURE_FEATURE));
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (obtained from                   */
   /* _BLPM_Register_Client_Event_Callback() function) as the first     */
   /* parameter.  The second parameter is the Transaction ID of the     */
   /* outstanding transaction.  This function returns zero on success;  */
   /* otherwise, a negative error value is returned.                    */
int _BLPM_Cancel_Transaction(unsigned int CallbackID, unsigned int TransactionID)
{
   int                               ret_val;
   BLPM_Cancel_Transaction_Request_t RequestMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Module-specific fields first.                          */
   RequestMessage.CallbackID                    = CallbackID;
   RequestMessage.TransactionID                 = TransactionID;
   RequestMessage.MessageHeader.MessageFunction = BLPM_MESSAGE_FUNCTION_CANCEL_TRANSACTION;
   RequestMessage.MessageHeader.MessageLength   = BLPM_CANCEL_TRANSACTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

   ret_val = FormatSendRequestResponse((BTPM_Message_t *)&RequestMessage);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
