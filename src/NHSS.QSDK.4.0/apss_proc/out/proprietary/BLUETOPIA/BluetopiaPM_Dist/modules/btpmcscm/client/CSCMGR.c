/*****< cscmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CSCMGR - CSC Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/07/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMCSCM.h"            /* BTPM CSC Manager Prototypes/Constants.    */
#include "CSCMMSG.h"             /* BTPM CSC Manager Message Formats.         */
#include "CSCMGR.h"              /* CSC Manager Impl. Prototypes/Constants.   */

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
   /* initialize the CSC Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager CSC Manager  */
   /* Implementation.                                                   */
int _CSCM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing CSC Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALREADY_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the CSC   */
   /* Manager Implementation.  After this function is called the CSC    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _CSCM_Initialize() function.  */
void _CSCM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Collector callback function with the CSC    */
   /* Manager Service.  This Callback will be dispatched by the CSC     */
   /* Manager when various CSC Manager Collector Events occur.  This    */
   /* function returns a positive (non-zero) value if successful, or a  */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _CSCM_Un_Register_Collector_Event_Callback() function to */
   /*          un-register the callback from this module.               */
int _CSCM_Register_Collector_Event_Callback()
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   CSCM_Register_Collector_Events_Request_t  RegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&RegisterCollectorEventsRequest, 0, CSCM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE);

      RegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      RegisterCollectorEventsRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS;
      RegisterCollectorEventsRequest.MessageHeader.MessageLength   = CSCM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Group: %08X\n", RegisterCollectorEventsRequest.MessageHeader.MessageGroup));

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((CSCM_Register_Collector_Events_Response_t *)ResponseMessage)->Status))
               ret_val = ((CSCM_Register_Collector_Events_Response_t *)ResponseMessage)->CallbackID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered CSC Manager Collector         */
   /* Event Callback (registered via a successful call to the           */
   /* _CSCM_Register_Collector_Event_Callback() function).  This        */
   /* function accepts as input the Collector Event Callback ID (return */
   /* value from _CSCM_Register_Collector_Event_Callback() function).   */
int _CSCM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   CSCM_Un_Register_Collector_Events_Request_t  UnRegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&UnRegisterCollectorEventsRequest, 0, CSCM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE);

      UnRegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      UnRegisterCollectorEventsRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS;
      UnRegisterCollectorEventsRequest.MessageHeader.MessageLength   = CSCM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterCollectorEventsRequest.CallbackID                    = CollectorCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
         {
            ret_val = ((CSCM_Un_Register_Collector_Events_Response_t *)ResponseMessage)->Status;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of entries that the buffer will      */
   /* support (i.e. can be copied into the buffer).  The next parameter */
   /* is optional and, if specified, will be populated with the total   */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _CSCM_Query_Connected_Sensors(unsigned int MaximumRemoteDeviceListEntries, CSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   CSCM_Query_Connected_Sensors_Request_t  QueryConnectedSensorsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&QueryConnectedSensorsRequest, 0, CSCM_QUERY_CONNECTED_SENSORS_REQUEST_SIZE);

      QueryConnectedSensorsRequest.MessageHeader.AddressID        = MSG_GetServerAddressID();
      QueryConnectedSensorsRequest.MessageHeader.MessageID        = MSG_GetNextMessageID();
      QueryConnectedSensorsRequest.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      QueryConnectedSensorsRequest.MessageHeader.MessageFunction  = CSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS;
      QueryConnectedSensorsRequest.MessageHeader.MessageLength    = CSCM_QUERY_CONNECTED_SENSORS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      QueryConnectedSensorsRequest.MaximumRemoteDeviceListEntries = MaximumRemoteDeviceListEntries;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedSensorsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(((CSCM_Query_Connected_Sensors_Response_t *)ResponseMessage)->NumberDevices))
         {
            /* If successful, copy any potention data.                  */
            if(!(((CSCM_Query_Connected_Sensors_Response_t *)ResponseMessage)->Status))
            {
               /* If the caller requested the total number of devices,  */
               /* set the value.                                        */
               if(TotalNumberConnectedDevices)
                  *TotalNumberConnectedDevices = ((CSCM_Query_Connected_Sensors_Response_t *)ResponseMessage)->TotalNumberConnectedDevices;

               /* Return the number of returned devices.                */
               ret_val = ((CSCM_Query_Connected_Sensors_Response_t *)ResponseMessage)->NumberDevices;

               /* If the caller supplied a buffer, copy any potential   */
               /* data.                                                 */
               if((ConnectedDeviceList) && (ret_val))
                  BTPS_MemCopy(ConnectedDeviceList, ((CSCM_Query_Connected_Sensors_Response_t *)ResponseMessage)->ConnectedSensors, CSCM_CONNECTED_SENSOR_SIZE * ret_val);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function will attempt to configure a remote device  */
   /* which supports the CSC sensor role. It will register measurement  */
   /* and control point notifications and set the device up for         */
   /* usage. The RemoteDeviceAddress is the address of the remote       */
   /* sensor. This function returns zero if successful and a negative   */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return from this call does not mean the     */
   /*          device has been configured. An                           */
   /*          etCSCConfigurationStatusChanged event will indicate the  */
   /*          status of the attempt to configure.                      */
int _CSCM_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   CSCM_Configure_Remote_Sensor_Request_t  ConfigureRemoteSensorRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&ConfigureRemoteSensorRequest, 0, CSCM_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE);

      ConfigureRemoteSensorRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConfigureRemoteSensorRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConfigureRemoteSensorRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      ConfigureRemoteSensorRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR;
      ConfigureRemoteSensorRequest.MessageHeader.MessageLength   = CSCM_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConfigureRemoteSensorRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      ConfigureRemoteSensorRequest.Flags                         = Flags;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConfigureRemoteSensorRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE)
         {
            ret_val = ((CSCM_Configure_Remote_Sensor_Response_t *)ResponseMessage)->Status;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function will un-configure a remote sensor which was*/
   /* previously configured. All notifications will be disabled. The    */
   /* RemoteDeviceAddress is the address of the remote sensor. This     */
   /* function returns zero if success and a negative return code if    */
   /* there was an error.                                               */
int _CSCM_Un_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   CSCM_Un_Configure_Remote_Sensor_Request_t  UnConfiugreRemoteSensorRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&UnConfiugreRemoteSensorRequest, 0, CSCM_UN_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE);

      UnConfiugreRemoteSensorRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnConfiugreRemoteSensorRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnConfiugreRemoteSensorRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      UnConfiugreRemoteSensorRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR;
      UnConfiugreRemoteSensorRequest.MessageHeader.MessageLength   = CSCM_UN_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnConfiugreRemoteSensorRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnConfiugreRemoteSensorRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_UN_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE)
         {
            ret_val = ((CSCM_Un_Configure_Remote_Sensor_Response_t *)ResponseMessage)->Status;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function queries information about a known remote   */
   /* sensor. The RemoteDeviceAddress parameter is the address of       */
   /* the remote sensor. The DeviceInfo parameter is a pointer to       */
   /* the Connected Sensor structure in which the data should be        */
   /* populated. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
int _CSCM_Get_Connected_Sensor_Info(BD_ADDR_t RemoteDeviceAddress, CSCM_Connected_Sensor_t *DeviceInfo)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   CSCM_Get_Connected_Sensor_Info_Request_t  GetConnectedSensorInfoRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&GetConnectedSensorInfoRequest, 0, CSCM_GET_CONNECTED_SENSOR_INFO_REQUEST_SIZE);

      GetConnectedSensorInfoRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetConnectedSensorInfoRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetConnectedSensorInfoRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      GetConnectedSensorInfoRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO;
      GetConnectedSensorInfoRequest.MessageHeader.MessageLength   = CSCM_GET_CONNECTED_SENSOR_INFO_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetConnectedSensorInfoRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetConnectedSensorInfoRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_GET_CONNECTED_SENSOR_INFO_RESPONSE_SIZE)
         {
            if(!(ret_val = ((CSCM_Get_Connected_Sensor_Info_Response_t *)ResponseMessage)->Status))
               *DeviceInfo = ((CSCM_Get_Connected_Sensor_Info_Response_t *)ResponseMessage)->ConnectedSensorInfo;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function queries the current sensor location from a */
   /* remote sensor. The RemoteDeviceAddress parameter is the address of*/
   /* the remote sensor. This function returns zero if successful or a  */
   /* negative error code if there was an error.                        */
   /* * NOTE * If this function is succesful, the status of the request */
   /*          will be returned in a cetCSCSensorLocationResponse event.*/
int _CSCM_Get_Sensor_Location(BD_ADDR_t RemoteDeviceAddress)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   CSCM_Get_Sensor_Location_Request_t  GetSensorLocationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&GetSensorLocationRequest, 0, CSCM_GET_SENSOR_LOCATION_REQUEST_SIZE);

      GetSensorLocationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetSensorLocationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetSensorLocationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      GetSensorLocationRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION;
      GetSensorLocationRequest.MessageHeader.MessageLength   = CSCM_GET_SENSOR_LOCATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetSensorLocationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetSensorLocationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_GET_SENSOR_LOCATION_RESPONSE_SIZE)
         {
            ret_val = ((CSCM_Get_Sensor_Location_Response_t *)ResponseMessage)->Status;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function sends a control point opcode to update     */
   /* the cumulative value on a remote sensor. The RemoteDeviceAddress  */
   /* parameter is the address of the remote sensor. The CumulativeValue*/
   /* parameter is the value to set on the remote sensor. If successful,*/
   /* this function returns a postive integer which represents the      */
   /* Procedure ID associated with this procedure. If there is an error,*/
   /* this function returns a negative error code.                      */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetCSCCumulativeValueUpdated event will notify all       */
   /*          registered callbacks that the value has changed.         */
int _CSCM_Update_Cumulative_Value(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   CSCM_Update_Cumulative_Value_Request_t  UpdateCumulativeValueRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&UpdateCumulativeValueRequest, 0, CSCM_UPDATE_CUMULATIVE_VALUE_REQUEST_SIZE);

      UpdateCumulativeValueRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UpdateCumulativeValueRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UpdateCumulativeValueRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      UpdateCumulativeValueRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE;
      UpdateCumulativeValueRequest.MessageHeader.MessageLength   = CSCM_UPDATE_CUMULATIVE_VALUE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UpdateCumulativeValueRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      UpdateCumulativeValueRequest.CumulativeValue               = CumulativeValue;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UpdateCumulativeValueRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_UPDATE_CUMULATIVE_VALUE_RESPONSE_SIZE)
         {
            ret_val = ((CSCM_Update_Cumulative_Value_Response_t *)ResponseMessage)->Status;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function sends a control point opcode to update     */
   /* the sensor location on a remote sensor. The RemoteDeviceAddress   */
   /* parameter is the address of the remote sensor. The SensorLocation */
   /* parameter is the new location to set.  If successful, this        */
   /* function returns a postive integer which represents the Procedure */
   /* ID associated with this procedure. If there is an error, this     */
   /* function returns a negative error code.                           */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * Only one procedure can be outstanding at a time. If this */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetCSCSensorLocationUpdated event will notify all        */
   /*          registered callbacks that the location has changed.      */
int _CSCM_Update_Sensor_Location(BD_ADDR_t RemoteDeviceAddress, CSCM_Sensor_Location_t SensorLocation)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   CSCM_Update_Sensor_Location_Request_t  UpdateSensorLocationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&UpdateSensorLocationRequest, 0, CSCM_UPDATE_SENSOR_LOCATION_REQUEST_SIZE);

      UpdateSensorLocationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UpdateSensorLocationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UpdateSensorLocationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER;
      UpdateSensorLocationRequest.MessageHeader.MessageFunction = CSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION;
      UpdateSensorLocationRequest.MessageHeader.MessageLength   = CSCM_UPDATE_SENSOR_LOCATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UpdateSensorLocationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      UpdateSensorLocationRequest.SensorLocation                = SensorLocation;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UpdateSensorLocationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= CSCM_UPDATE_SENSOR_LOCATION_RESPONSE_SIZE)
         {
            ret_val = ((CSCM_Update_Sensor_Location_Response_t *)ResponseMessage)->Status;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

