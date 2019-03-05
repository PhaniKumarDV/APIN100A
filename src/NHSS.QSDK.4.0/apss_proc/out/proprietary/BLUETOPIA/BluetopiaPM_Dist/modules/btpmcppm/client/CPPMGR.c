/*****< CPPMGR.c >*************************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMGR - Cycling Power Platform Manager Client                            */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "CPPMType.h"            /* CPP Manager Type Definitions              */
#include "CPPMAPI.h"             /* CPP Interface Header                      */
#include "CPPMMSG.h"             /* BTPM CPP Manager Message Formats          */

static int SendRequestCPPM(BTPM_Message_t *Request);

   /* _CPPM_Register_Collector_Event_Callback sends the request to      */
   /* register cycling power event messages. If successful the response */
   /* status value will be the callback ID used in the API functions.   */
int _CPPM_Register_Collector_Event_Callback(void)
{
   int                                      Result;
   CPPM_Register_Collector_Events_Request_t Request;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Request.MessageHeader.MessageFunction = CPPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS;
   Request.MessageHeader.MessageLength   = CPPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

   /* Send the request and wait for the response.                       */
   Result = SendRequestCPPM((BTPM_Message_t *)&Request);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* _CPPM_Unregister_Collector_Event_Callback sends the request to    */
   /* unregister the client for cycling power callbacks.                */ 
int _CPPM_Unregister_Collector_Event_Callback(unsigned int CallbackID)
{
   int                                         Result;
   CPPM_Unregister_Collector_Events_Request_t  Request;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Request.MessageHeader.MessageFunction = CPPM_MESSAGE_FUNCTION_UNREGISTER_COLLECTOR_EVENTS;
   Request.MessageHeader.MessageLength   = CPPM_UNREGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   Request.CallbackID                    = CallbackID;

   /* Send the request and wait for the response.                       */
   Result = SendRequestCPPM((BTPM_Message_t *)&Request);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* _CPPM_Register_Updates sends the request to register for          */
   /* unsolicited updates (notifications or indications) from the       */
   /* specified sensor. A request can be made for Measurement and       */
   /* Vector notifications and Control Point indications. If            */
   /* EnabledUpdates is true then the remote client configuration       */
   /* descriptor will be written to enable the updates. The             */
   /* MessageFunction parameter identifies the request.                 */
int _CPPM_Register_Updates(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates, unsigned int MessageFunction)
{
   int                                         Result;
   CPPM_Register_Unsolicited_Updates_Request_t Request;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Module-specific fields.                                */
   Request.MessageHeader.MessageFunction  = MessageFunction;
   Request.MessageHeader.MessageLength    = CPPM_REGISTER_UNSOLICITED_UPDATES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   Request.CallbackID                     = CallbackID;
   Request.RemoteDeviceAddress            = *RemoteSensor;
   Request.InstanceID                     = InstanceID;
   Request.EnableUnsolicitedUpdates       = EnableUpdates;

   Result = SendRequestCPPM((BTPM_Message_t *)&Request);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* _CPPM_Request is the generic request function used when no data   */
   /* other than the address and instance ID needs to be sent with the  */
   /* request. The supplied message function parameter identifies the   */
   /* request.                                                          */    
int _CPPM_Request(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, unsigned int InstanceID, unsigned int MessageFunction)
{
   int            Result;
   CPPM_Request_t Request;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Module-specific fields.                                */
   Request.MessageHeader.MessageFunction  = MessageFunction;
   Request.MessageHeader.MessageLength    = CPPM_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   Request.CallbackID                     = CallbackID;
   Request.RemoteDeviceAddress            = *RemoteSensor;
   Request.InstanceID                     = InstanceID;

   Result = SendRequestCPPM((BTPM_Message_t *)&Request);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* _CPPM_Write_Sensor_Control_Point sends an opcode and, depending   */
   /* on which opcode, additional procedure data to the PM server which */
   /* will be written to the control point characteristic of the        */
   /* specified sensor to initiate a procedure. See CPPMType.h for the  */
   /* type definition of the CPPM_Procedure_Data_t structure.           */
int _CPPM_Write_Sensor_Control_Point(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, unsigned int InstanceID, CPPM_Procedure_Data_t ProcedureData)
{
   int                                       Result;
   CPPM_Write_Sensor_Control_Point_Request_t Request;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Module-specific fields.                                */
   Request.MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_WRITE_SENSOR_CONTROL_POINT;
   Request.MessageHeader.MessageLength    = CPPM_WRITE_SENSOR_CONTROL_POINT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   Request.CallbackID                     = CallbackID;
   Request.RemoteDeviceAddress            = *RemoteSensor;
   Request.InstanceID                     = InstanceID;
   Request.ProcedureData                  = ProcedureData;

   Result = SendRequestCPPM((BTPM_Message_t *)&Request);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* _CPPM_Query_Sensors sends the sensors query and, if necessary,    */
   /* copies the sensor addresses from the response into the provided   */
   /* buffer. The NumberOfSensors field in the response is set to the   */
   /* number of addresses in the Sensors field. It will never be        */
   /* greater than the value pointed to by the NumberOfSensors          */
   /* parameter. See CPPMMSG.h for the defintion of                     */
   /* CPPM_Query_Sensors_Response_t.                                    */
int  _CPPM_Query_Sensors(unsigned int CallbackID, unsigned int *NumberOfSensors, BD_ADDR_t *RemoteSensors)
{
   int                           Result;
   CPPM_Query_Sensors_Request_t  Request;
   BTPM_Message_t               *Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the header.                                                */
   Request.MessageHeader.AddressID        = MSG_GetServerAddressID();
   Request.MessageHeader.MessageID        = MSG_GetNextMessageID();
   Request.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;

   /* Format the Module-specific fields.                                */
   Request.MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_QUERY_SENSORS;
   Request.MessageHeader.MessageLength    = CPPM_QUERY_SENSORS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   Request.CallbackID                     = CallbackID;

   if(NumberOfSensors)
      Request.NumberOfSensors             = *NumberOfSensors;
   else
      Request.NumberOfSensors             = 0;

    if((!(Result = MSG_SendMessageResponse((BTPM_Message_t *)&Request, BTPM_CONFIGURATION_DEVICE_MANAGER_GENERAL_MESSAGE_RESPONSE_TIME_MS, &Response))) && (Response))
   {
      /* Return the status of the response message.                     */
      if((BTPM_MESSAGE_SIZE(Response->MessageHeader.MessageLength) >= (CPPM_QUERY_SENSORS_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE)) && (BTPM_MESSAGE_SIZE(Response->MessageHeader.MessageLength) >= (CPPM_QUERY_SENSORS_RESPONSE_SIZE(((CPPM_Query_Sensors_Response_t *)Response)->NumberOfSensors) - BTPM_MESSAGE_HEADER_SIZE)))
      {
         if((Result = ((CPPM_Query_Sensors_Response_t *)Response)->Status) > 0)
         {
            if((NumberOfSensors) && ((*NumberOfSensors = ((CPPM_Query_Sensors_Response_t *)Response)->NumberOfSensors) > 0))
            {
               if(RemoteSensors)
                  BTPS_MemCopy(RemoteSensors, ((CPPM_Query_Sensors_Response_t *)Response)->Sensors, *NumberOfSensors * sizeof(BD_ADDR_t));
            }
         }
      }
      else
         Result = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* Free the message.                                              */
      MSG_FreeReceivedMessageGroupHandlerMessage(Response);
   }
   else
      Result = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* _CPPM_Query_Sensor_Instances sends the instances query and, if    */
   /* necessary, copies the instance records from the response into the */
   /* provided Instances buffer. The NumberOfInstances field in the     */
   /* response is set to the number of records in the Instances field.  */
   /* It will never be greater than the value pointed to by the         */
   /* NumberOfInstances parameter. See CPPMType.h for the type          */
   /* defintion of Instance_Record_t and CPPMMSG.h for the defintion of */
   /* CPPM_Query_Sensor_Instances_Response_t.                           */
int  _CPPM_Query_Sensor_Instances(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int *NumberOfInstances, Instance_Record_t *Instances)
{
   int                                    Result;
   CPPM_Query_Sensor_Instances_Request_t  Request;
   BTPM_Message_t                        *Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the header.                                                */
   Request.MessageHeader.AddressID        = MSG_GetServerAddressID();
   Request.MessageHeader.MessageID        = MSG_GetNextMessageID();
   Request.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;

   /* Format the Module-specific fields.                                */
   Request.MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_QUERY_SENSOR_INSTANCES;
   Request.MessageHeader.MessageLength    = CPPM_QUERY_SENSOR_INSTANCES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;
   Request.CallbackID                     = CallbackID;

   if(Sensor)
      Request.Sensor                      = *Sensor;

   if((NumberOfInstances) && (Instances))
      Request.NumberOfInstances           = *NumberOfInstances;
   else
      Request.NumberOfInstances           = 0;

   if((!(Result = MSG_SendMessageResponse((BTPM_Message_t *)&Request, BTPM_CONFIGURATION_DEVICE_MANAGER_GENERAL_MESSAGE_RESPONSE_TIME_MS, &Response))) && (Response))
   {
      /* Return the status of the response message.                     */
      if((BTPM_MESSAGE_SIZE(Response->MessageHeader.MessageLength) >= (CPPM_QUERY_SENSOR_INSTANCES_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE)) && (BTPM_MESSAGE_SIZE(Response->MessageHeader.MessageLength) >= (CPPM_QUERY_SENSOR_INSTANCES_RESPONSE_SIZE(((CPPM_Query_Sensor_Instances_Response_t *)Response)->NumberOfInstances) - BTPM_MESSAGE_HEADER_SIZE)))
      {
         if((Result = ((CPPM_Query_Sensor_Instances_Response_t *)Response)->Status) > 0)
         {
            if((NumberOfInstances) && ((*NumberOfInstances = ((CPPM_Query_Sensor_Instances_Response_t *)Response)->NumberOfInstances) > 0))
            {
               if(Instances)
                  BTPS_MemCopy(Instances, ((CPPM_Query_Sensor_Instances_Response_t *)Response)->Instances, *NumberOfInstances * sizeof(Instance_Record_t));
            }
         }
      }
      else
         Result = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* Free the message.                                              */
      MSG_FreeReceivedMessageGroupHandlerMessage(Response);
   }
   else
      Result = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* SendRequestCPPM sets the message header values, sends the message */
   /* and returns the response status. It is used by request functions  */
   /* that don't require special data processing in the response.       */
static int SendRequestCPPM(BTPM_Message_t *Request)
{
   int             Result;
   BTPM_Message_t *Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the header.                                                */
   Request->MessageHeader.AddressID    = MSG_GetServerAddressID();
   Request->MessageHeader.MessageID    = MSG_GetNextMessageID();
   Request->MessageHeader.MessageGroup = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;

   if((!(Result = MSG_SendMessageResponse((BTPM_Message_t *)Request, BTPM_CONFIGURATION_DEVICE_MANAGER_GENERAL_MESSAGE_RESPONSE_TIME_MS, &Response))) && (Response))
   {
      /* Return the status of the response message.                     */
      if(BTPM_MESSAGE_SIZE(Response->MessageHeader.MessageLength) >= (CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE))
         Result = ((CPPM_Response_t *)Response)->Status;
      else
         Result = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

      /* Free the message.                                              */
      MSG_FreeReceivedMessageGroupHandlerMessage(Response);
   }
   else
      Result = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}
