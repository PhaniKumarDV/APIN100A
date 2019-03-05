/*****< pasmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PASMGR - Phone Alert Status (PAS) Manager Implementation for Stonestreet  */
/*           One Bluetooth Protocol Stack Platform Manager.                   */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMPASM.h"            /* BTPM PAS Manager Prototypes/Constants.    */
#include "PASMMSG.h"             /* BTPM PAS Manager Message Formats.         */
#include "PASMGR.h"              /* PAS Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */

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
   /* initialize the PAS Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAS Manager  */
   /* Implementation.                                                   */
int _PASM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PXP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALREADY_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the PAS   */
   /* Manager Implementation.  After this function is called the PAS    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PASM_Initialize() function.  */
void _PASM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Phone Alert Status Server callback function */
   /* with the Phone Alert Status (PAS) Manager Service.  Events will be*/
   /* dispatched by the PAS Manager when various PAS Manager Server     */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _PASM_Un_Register_Server_Events() function to un-register*/
   /*          the event callback from this module.                     */
   /* * NOTE * Only 1 Server Event Callback can be registered in the    */
   /*          system at a time.                                        */
int _PASM_Register_Server_Events(void)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   PASM_Register_Server_Events_Request_t  RegisterServerEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register Server    */
      /* Events message and send it to the server.                      */
      RegisterServerEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterServerEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterServerEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
      RegisterServerEventsRequest.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS;
      RegisterServerEventsRequest.MessageHeader.MessageLength   = PASM_REGISTER_SERVER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterServerEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PASM_REGISTER_SERVER_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((PASM_Register_Server_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((PASM_Register_Server_Events_Response_t *)ResponseMessage)->ServerEventHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Phone Alert Status (PAS)      */
   /* Manager Server Event Callback (registered via a successful call to*/
   /* the PASM_Register_Server_Events() function).  This function       */
   /* accepts as input the PAS Manager Event Callback ID (return value  */
   /* from PASM_Register_Server_Events() function).  This function      */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PASM_Un_Register_Server_Events(unsigned int ServerCallbackID)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   PASM_Un_Register_Server_Events_Request_t  UnRegisterServerEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Un-register Server */
      /* Events message and send it to the server.                      */
      UnRegisterServerEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterServerEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterServerEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
      UnRegisterServerEventsRequest.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS;
      UnRegisterServerEventsRequest.MessageHeader.MessageLength   = PASM_UN_REGISTER_SERVER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterServerEventsRequest.ServerEventHandlerID          = ServerCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterServerEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PASM_UN_REGISTER_SERVER_EVENTS_RESPONSE_SIZE)
            ret_val = ((PASM_Un_Register_Server_Events_Response_t *)ResponseMessage)->Status;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS).  This function is  */
   /* responsible for updating the Alert Status internally, as well as  */
   /* dispatching any Alert Notifications that have been registered by  */
   /* Phone Alert Status (PAS) clients.  This function accepts as it's  */
   /* parameter the Server callback ID that was returned from a         */
   /* successful call to PASM_Register_Server_Event_Callback() followed */
   /* by the Alert Status value to set.  This function returns zero if  */
   /* successful, or a negative return error code if there was an error.*/
int _PASM_Set_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus)
{
   int                              ret_val;
   BTPM_Message_t                  *ResponseMessage;
   PASM_Set_Alert_Status_Request_t  SetAlertStatusRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the input parameters appear to be     */
      /* semi-valid.                                                    */
      if(AlertStatus)
      {
         /* All that we really need to do is to build a Set Alert Status*/
         /* message and send it to the server.                          */
         SetAlertStatusRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetAlertStatusRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetAlertStatusRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
         SetAlertStatusRequest.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_SET_ALERT_STATUS;
         SetAlertStatusRequest.MessageHeader.MessageLength   = PASM_SET_ALERT_STATUS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetAlertStatusRequest.ServerEventHandlerID          = ServerCallbackID;
         SetAlertStatusRequest.AlertStatus                   = *AlertStatus;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetAlertStatusRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PASM_SET_ALERT_STATUS_RESPONSE_SIZE)
               ret_val = ((PASM_Set_Alert_Status_Response_t *)ResponseMessage)->Status;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) alert       */
   /* status.  This function accepts as it's parameter the Server       */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured alert status upon successful   */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the AlertStatus    */
   /*          buffer will contain the currently configured alert       */
   /*          status.  If this function returns an error then the      */
   /*          contents of the AlertStatus buffer will be undefined.    */
int _PASM_Query_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   PASM_Query_Alert_Status_Request_t  QueryAlertStatusRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the input parameters appear to be     */
      /* semi-valid.                                                    */
      if(AlertStatus)
      {
         /* All that we really need to do is to build a Query Alert     */
         /* Status message and send it to the server.                   */
         QueryAlertStatusRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryAlertStatusRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryAlertStatusRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
         QueryAlertStatusRequest.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_QUERY_ALERT_STATUS;
         QueryAlertStatusRequest.MessageHeader.MessageLength   = PASM_QUERY_ALERT_STATUS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryAlertStatusRequest.ServerEventHandlerID          = ServerCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryAlertStatusRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PASM_QUERY_ALERT_STATUS_RESPONSE_SIZE)
            {
               if(!(ret_val = ((PASM_Query_Alert_Status_Response_t *)ResponseMessage)->Status))
                  *AlertStatus = ((PASM_Query_Alert_Status_Response_t *)ResponseMessage)->AlertStatus;
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
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS) ringer setting as   */
   /* well as dispatching any Alert Notifications that have been        */
   /* registered by PAS clients.  This function accepts as it's         */
   /* parameter the PAS Server callback ID that was returned from a     */
   /* successful call to PASM_Register_Server_Event_Callback() and the  */
   /* ringer value to configure.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _PASM_Set_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t RingerSetting)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   PASM_Set_Ringer_Setting_Request_t  SetRingerSettingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Set Ringer Setting */
      /* message and send it to the server.                             */
      SetRingerSettingRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetRingerSettingRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetRingerSettingRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
      SetRingerSettingRequest.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_SET_RINGER_SETTING;
      SetRingerSettingRequest.MessageHeader.MessageLength   = PASM_SET_RINGER_SETTING_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetRingerSettingRequest.ServerEventHandlerID          = ServerCallbackID;
      SetRingerSettingRequest.RingerSetting                 = RingerSetting;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetRingerSettingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PASM_SET_RINGER_SETTING_RESPONSE_SIZE)
            ret_val = ((PASM_Set_Ringer_Setting_Response_t *)ResponseMessage)->Status;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) ringer      */
   /* setting.  This function accepts as it's parameter the Server      */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured ringer setting upon successful */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the RingerSetting  */
   /*          buffer will contain the currently configured ringer      */
   /*          setting.  If this function returns an error then the     */
   /*          contents of the RingerSetting buffer will be undefined.  */
int _PASM_Query_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t *RingerSetting)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   PASM_Query_Ringer_Setting_Request_t  QueryRingerSettingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the input parameters appear to be     */
      /* semi-valid.                                                    */
      if(RingerSetting)
      {
         /* All that we really need to do is to build a Query Ringer    */
         /* Setting message and send it to the server.                  */
         QueryRingerSettingRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryRingerSettingRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryRingerSettingRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
         QueryRingerSettingRequest.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_QUERY_RINGER_SETTING;
         QueryRingerSettingRequest.MessageHeader.MessageLength   = PASM_QUERY_RINGER_SETTING_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryRingerSettingRequest.ServerEventHandlerID          = ServerCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryRingerSettingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PASM_QUERY_RINGER_SETTING_RESPONSE_SIZE)
            {
               if(!(ret_val = ((PASM_Query_Ringer_Setting_Response_t *)ResponseMessage)->Status))
                  *RingerSetting = ((PASM_Query_Ringer_Setting_Response_t *)ResponseMessage)->RingerSetting;
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
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

