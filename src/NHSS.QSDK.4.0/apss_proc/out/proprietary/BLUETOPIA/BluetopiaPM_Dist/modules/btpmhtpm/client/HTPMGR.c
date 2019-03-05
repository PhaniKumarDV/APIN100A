/*****< htpmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HTPMGR - HTP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHTPM.h"            /* BTPM HTP Manager Prototypes/Constants.    */
#include "HTPMMSG.h"             /* BTPM HTP Manager Message Formats.         */
#include "HTPMGR.h"              /* HTP Manager Impl. Prototypes/Constants.   */

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
   /* initialize the HTP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HTP Manager  */
   /* Implementation.                                                   */
int _HTPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the HTP   */
   /* Manager Implementation.  After this function is called the HTP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HTPM_Initialize() function.  */
void _HTPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Do nothing.                                                       */

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Heart Rate (HTP) */
   /* Manager Service.  This Callback will be dispatched by the HTP     */
   /* Manager when various HTP Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a HTP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HTPM_Un_Register_Collector_Events() function to          */
   /*          un-register the callback from this module.               */
int _HTPM_Register_Collector_Events(void)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   HTPM_Register_Collector_Events_Request_t  RegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* All that we really need to do is to build a Register HTP Events   */
   /* message and send it to the server.                                */
   RegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
   RegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
   RegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
   RegisterCollectorEventsRequest.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS;
   RegisterCollectorEventsRequest.MessageHeader.MessageLength   = HTPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

   /* Message has been formatted, go ahead and send it off.             */
   if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
   {
      /* Response received, go ahead and see if the return value is     */
      /* valid.  If it is, go ahead and note the returned status.       */
      if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HTPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
      {
         if((ret_val = ((HTPM_Register_Collector_Events_Response_t *)ResponseMessage)->Status) == 0)
            ret_val = ((HTPM_Register_Collector_Events_Response_t *)ResponseMessage)->HTPCollectorEventsHandlerID;
      }
      else
         ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* All finished with the message, go ahead and free it.           */
      MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HTP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HTPM_Register_Collector_Events() function).  This function accepts*/
   /* as input the HTP Manager Event Callback ID (return value from     */
   /* HTPM_Register_Collector_Events() function).                       */
int _HTPM_Un_Register_Collector_Events(unsigned int HTPCollectorEventHandlerID)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   HTPM_Un_Register_Collector_Events_Request_t  UnRegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(HTPCollectorEventHandlerID)
   {
      /* All that we really need to do is to build an Un-Register HTP   */
      /* Events message and send it to the server.                      */
      UnRegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      UnRegisterCollectorEventsRequest.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS;
      UnRegisterCollectorEventsRequest.MessageHeader.MessageLength   = HTPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterCollectorEventsRequest.HTPCollectorEventsHandlerID   = HTPCollectorEventHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HTPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
            ret_val = ((HTPM_Un_Register_Collector_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Temperature Type procedure to a remote HTP   */
   /* Sensor.  This function accepts as input the HTP Collector Event   */
   /* Handler ID (return value from _HTPM_Register_Collector_Events()   */
   /* function), and the BD_ADDR of the remote HTP Sensor.  This        */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
int _HTPM_Get_Temperature_Type_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   HTPM_Get_Temperature_Type_Request_t  GetTemperatureTypeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((HTPCollectorEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* All that we really need to do is to build a Get Temperature    */
      /* Type Request message and send it to the server.                */
      GetTemperatureTypeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetTemperatureTypeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetTemperatureTypeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      GetTemperatureTypeRequest.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE;
      GetTemperatureTypeRequest.MessageHeader.MessageLength   = HTPM_GET_TEMPERATURE_TYPE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetTemperatureTypeRequest.HTPCollectorEventsHandlerID   = HTPCollectorEventHandlerID;
      GetTemperatureTypeRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetTemperatureTypeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HTPM_GET_TEMPERATURE_TYPE_RESPONSE_SIZE)
         {
            if((ret_val = ((HTPM_Get_Temperature_Type_Response_t *)ResponseMessage)->Status) == 0)
               ret_val = ((HTPM_Get_Temperature_Type_Response_t *)ResponseMessage)->TransactionID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Event Handler ID (return value from                               */
   /* _HTPM_Register_Collector_Events() function), and the BD_ADDR of   */
   /* the remote HTP Sensor.  This function returns the positive,       */
   /* non-zero, Transaction ID of the request on success or a negative  */
   /* error code.                                                       */
int _HTPM_Get_Measurement_Interval_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HTPM_Get_Measurement_Interval_Request_t  GetMeasurementIntervalRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((HTPCollectorEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* All that we really need to do is to build a Get Temperature    */
      /* Type Request message and send it to the server.                */
      GetMeasurementIntervalRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetMeasurementIntervalRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetMeasurementIntervalRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      GetMeasurementIntervalRequest.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL;
      GetMeasurementIntervalRequest.MessageHeader.MessageLength   = HTPM_GET_MEASUREMENT_INTERVAL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetMeasurementIntervalRequest.HTPCollectorEventsHandlerID   = HTPCollectorEventHandlerID;
      GetMeasurementIntervalRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetMeasurementIntervalRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_SIZE)
         {
            if((ret_val = ((HTPM_Get_Measurement_Interval_Response_t *)ResponseMessage)->Status) == 0)
               ret_val = ((HTPM_Get_Measurement_Interval_Response_t *)ResponseMessage)->TransactionID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Set Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Event Handler ID (return value from                               */
   /* _HTPM_Register_Collector_Events() function), the BD_ADDR of the   */
   /* remote HTP Sensor, and the Measurement Interval to attempt to set.*/
   /* This function returns the positive, non-zero, Transaction ID of   */
   /* the request on success or a negative error code.                  */
int _HTPM_Set_Measurement_Interval_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress, unsigned int MeasurementInterval)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HTPM_Set_Measurement_Interval_Request_t  SetMeasurementIntervalRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((HTPCollectorEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* All that we really need to do is to build a Set Temperature    */
      /* Type Request message and send it to the server.                */
      SetMeasurementIntervalRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetMeasurementIntervalRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetMeasurementIntervalRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      SetMeasurementIntervalRequest.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL;
      SetMeasurementIntervalRequest.MessageHeader.MessageLength   = HTPM_SET_MEASUREMENT_INTERVAL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetMeasurementIntervalRequest.HTPCollectorEventsHandlerID   = HTPCollectorEventHandlerID;
      SetMeasurementIntervalRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      SetMeasurementIntervalRequest.MeasurementInterval           = MeasurementInterval;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetMeasurementIntervalRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_SIZE)
         {
            if((ret_val = ((HTPM_Set_Measurement_Interval_Response_t *)ResponseMessage)->Status) == 0)
               ret_val = ((HTPM_Set_Measurement_Interval_Response_t *)ResponseMessage)->TransactionID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval Valid Range procedure to*/
   /* a remote HTP Sensor.  This function accepts as input the HTP      */
   /* Collector Event Handler ID (return value from                     */
   /* _HTPM_Register_Collector_Events() function), and the BD_ADDR of   */
   /* the remote HTP Sensor.  This function returns the positive,       */
   /* non-zero, Transaction ID of the request on success or a negative  */
   /* error code.                                                       */
int _HTPM_Get_Measurement_Interval_Valid_Range_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                                  ret_val;
   BTPM_Message_t                                      *ResponseMessage;
   HTPM_Get_Measurement_Interval_Valid_Range_Request_t  GetMeasurementIntervaValidRangeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((HTPCollectorEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* All that we really need to do is to build a Get Temperature    */
      /* Type Request message and send it to the server.                */
      GetMeasurementIntervaValidRangeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetMeasurementIntervaValidRangeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetMeasurementIntervaValidRangeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      GetMeasurementIntervaValidRangeRequest.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE;
      GetMeasurementIntervaValidRangeRequest.MessageHeader.MessageLength   = HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetMeasurementIntervaValidRangeRequest.HTPCollectorEventsHandlerID   = HTPCollectorEventHandlerID;
      GetMeasurementIntervaValidRangeRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetMeasurementIntervaValidRangeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_SIZE)
         {
            if((ret_val = ((HTPM_Get_Measurement_Interval_Valid_Range_Response_t *)ResponseMessage)->Status) == 0)
               ret_val = ((HTPM_Get_Measurement_Interval_Valid_Range_Response_t *)ResponseMessage)->TransactionID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

