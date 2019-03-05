/*****< hrpmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HRPMGR - HRP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHRPM.h"            /* BTPM HRP Manager Prototypes/Constants.    */
#include "HRPMMSG.h"             /* BTPM HRP Manager Message Formats.         */
#include "HRPMGR.h"              /* HRP Manager Impl. Prototypes/Constants.   */

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

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HRP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HRP Manager  */
   /* Implementation.                                                   */
int _HRPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the HRP   */
   /* Manager Implementation.  After this function is called the HRP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HRPM_Initialize() function.  */
void _HRPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Heart Rate (HRP) */
   /* Manager Service.  This Callback will be dispatched by the HRP     */
   /* Manager when various HRP Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a HRP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HRPM_Un_Register_Collector_Events() function to          */
   /*          un-register the callback from this module.               */
int _HRPM_Register_Collector_Events(void)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   HRPM_Register_Collector_Events_Request_t  RegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* All that we really need to do is to build a Register HRP Events   */
   /* message and send it to the server.                                */
   RegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
   RegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
   RegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
   RegisterCollectorEventsRequest.MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS;
   RegisterCollectorEventsRequest.MessageHeader.MessageLength   = HRPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

   /* Message has been formatted, go ahead and send it off.             */
   if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
   {
      /* Response received, go ahead and see if the return value is     */
      /* valid.  If it is, go ahead and note the returned status.       */
      if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HRPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
         ret_val = ((HRPM_Register_Collector_Events_Response_t *)ResponseMessage)->Status;
      else
         ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* All finished with the message, go ahead and free it.           */
      MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HRP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HRPM_Register_Collector_Events() function).  This function accepts*/
   /* as input the HRP Manager Event Callback ID (return value from     */
   /* HRPM_Register_Collector_Events() function).                       */
int _HRPM_Un_Register_Collector_Events(void)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   HRPM_Un_Register_Collector_Events_Request_t  UnRegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* All that we really need to do is to build an Un-Register HRP      */
   /* Events message and send it to the server.                         */
   UnRegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
   UnRegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
   UnRegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
   UnRegisterCollectorEventsRequest.MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS;
   UnRegisterCollectorEventsRequest.MessageHeader.MessageLength   = HRPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

   /* Message has been formatted, go ahead and send it off.             */
   if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
   {
      /* Response received, go ahead and see if the return value is     */
      /* valid.  If it is, go ahead and note the returned status.       */
      if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HRPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
         ret_val = ((HRPM_Un_Register_Collector_Events_Response_t *)ResponseMessage)->Status;
      else
         ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* All finished with the message, go ahead and free it.           */
      MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Body Sensor Location Request to a remote sensor.  This      */
   /* function accepts as input the Bluetooth Address of the remote     */
   /* device to request the Body Sensor Location from.  This function   */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int _HRPM_Get_Body_Sensor_Location(BD_ADDR_t *RemoteSensor)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HRPM_Get_Body_Sensor_Location_Request_t *GetBodySensorLocation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Allocate a buffer to hold the entire message.                     */
   if((GetBodySensorLocation = BTPS_AllocateMemory(HRPM_GET_BODY_SENSOR_LOCATION_REQUEST_SIZE)) != NULL)
   {
      /* All that we really need to do is to build a Set New Alert      */
      /* message and send it to the server.                             */
      GetBodySensorLocation->MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetBodySensorLocation->MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetBodySensorLocation->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
      GetBodySensorLocation->MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION;
      GetBodySensorLocation->MessageHeader.MessageLength   = HRPM_GET_BODY_SENSOR_LOCATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetBodySensorLocation->RemoteDeviceAddress           = *RemoteSensor;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)GetBodySensorLocation, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= (HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE))
            ret_val = ((HRPM_Get_Body_Sensor_Location_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Reset Energy Expended Request to a remote sensor.  This function*/
   /* accepts as input the Callback ID (return value from               */
   /* HRPM_Register_Collector_Events() function) as the first parameter.*/
   /* The second parameter is the Bluetooth Address of the remote device*/
   /* to request the execution of the Reset Energy Expended command.    */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _HRPM_Reset_Energy_Expended(BD_ADDR_t *RemoteSensor)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HRPM_Reset_Energy_Expended_Request_t *ResetEnergyExpended;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Allocate a buffer to hold the entire message.                     */
   if((ResetEnergyExpended = BTPS_AllocateMemory(HRPM_RESET_ENERGY_EXPENDED_REQUEST_SIZE)) != NULL)
   {
      /* All that we really need to do is to build a Set New Alert      */
      /* message and send it to the server.                             */
      ResetEnergyExpended->MessageHeader.AddressID       = MSG_GetServerAddressID();
      ResetEnergyExpended->MessageHeader.MessageID       = MSG_GetNextMessageID();
      ResetEnergyExpended->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
      ResetEnergyExpended->MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED;
      ResetEnergyExpended->MessageHeader.MessageLength   = HRPM_RESET_ENERGY_EXPENDED_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResetEnergyExpended->RemoteDeviceAddress           = *RemoteSensor;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)ResetEnergyExpended, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= (HRPM_RESET_ENERGY_EXPENDED_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE))
            ret_val = ((HRPM_Reset_Energy_Expended_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

