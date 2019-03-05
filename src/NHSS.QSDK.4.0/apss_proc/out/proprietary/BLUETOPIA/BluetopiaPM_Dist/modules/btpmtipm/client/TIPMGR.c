/*****< tipmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMGR - TIP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
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

#include "BTPMTIPM.h"            /* BTPM TIP Manager Prototypes/Constants.    */
#include "TIPMMSG.h"             /* BTPM TIP Manager Message Formats.         */
#include "TIPMGR.h"              /* TIP Manager Impl. Prototypes/Constants.   */

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
   /* initialize the TIP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager TIP Manager  */
   /* Implementation.                                                   */
int _TIPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing TIP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALREADY_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the TIP   */
   /* Manager Implementation.  After this function is called the TIP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _TIPM_Initialize() function.  */
void _TIPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Server Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Server Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _TIPM_Un_Register_Server_Events() function to un-register*/
   /*          the callback from this module.                           */
int _TIPM_Register_Server_Events(void)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   TIPM_Register_Server_Events_Request_t  RegisterServerEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register TIP Events*/
      /* message and send it to the server.                             */
      RegisterServerEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterServerEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterServerEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      RegisterServerEventsRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS;
      RegisterServerEventsRequest.MessageHeader.MessageLength   = TIPM_REGISTER_SERVER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterServerEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_REGISTER_SERVER_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((TIPM_Register_Server_Events_Response_t *)ResponseMessage)->Status))
               ret_val = ((TIPM_Register_Server_Events_Response_t *)ResponseMessage)->ServerEventHandlerID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* _TIPM_Register_Server_Events() function).  This function accepts  */
   /* the Server Event Handler ID that was returned via a successful    */
   /* call to _TIPM_Register_Server_Events().  This function returns a  */
   /* zero on success or a negative return error code if there was an   */
   /* error.                                                            */
int _TIPM_Un_Register_Server_Events(unsigned int ServerEventHandlerID)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   TIPM_Un_Register_Server_Events_Request_t  UnRegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register TIP   */
      /* Events message and send it to the server.                      */
      UnRegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      UnRegisterEventsRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS;
      UnRegisterEventsRequest.MessageHeader.MessageLength   = TIPM_UN_REGISTER_SERVER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterEventsRequest.ServerEventHandlerID          = ServerEventHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_UN_REGISTER_SERVER_EVENTS_RESPONSE_SIZE)
            ret_val = ((TIPM_Un_Register_Server_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to set  */
   /* the current Local Time Information.  This function accepts the    */
   /* Server Event Handler ID (return value from                        */
   /* _TIPM_Register_Server_Events() function) and a pointer to the     */
   /* Local Time Information to set.  This function returns ZERO if     */
   /* successful, or a negative return error code if there was an error.*/
int _TIPM_Set_Local_Time_Information(unsigned int ServerEventHandlerID, TIPM_Local_Time_Information_Data_t *LocalTimeInformation)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   TIPM_Set_Local_Time_Information_Request_t  SetLocalTimeInformationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if(LocalTimeInformation)
      {
         /* All that we really need to do is to build a Set Local Time  */
         /* Information message and send it to the server.              */
         SetLocalTimeInformationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetLocalTimeInformationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetLocalTimeInformationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
         SetLocalTimeInformationRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_SET_LOCAL_TIME_INFORMATION;
         SetLocalTimeInformationRequest.MessageHeader.MessageLength   = TIPM_SET_LOCAL_TIME_INFORMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetLocalTimeInformationRequest.ServerEventHandlerID          = ServerEventHandlerID;
         SetLocalTimeInformationRequest.LocalTimeInformation          = *LocalTimeInformation;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetLocalTimeInformationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_SET_LOCAL_TIME_INFORMATION_RESPONSE_SIZE)
               ret_val = ((TIPM_Set_Local_Time_Information_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to force*/
   /* an update of the Current Time.  This function accepts the Server  */
   /* Event Handler ID (return value from _TIPM_Register_Server_Events()*/
   /* function) and a bit mask that contains the reason for the Current */
   /* Time Update.  This function returns ZERO if successful, or a      */
   /* negative return error code if there was an error.                 */
int _TIPM_Update_Current_Time(unsigned int ServerEventHandlerID, unsigned long AdjustReasonMask)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   TIPM_Update_Current_Time_Request_t  UpdateCurrentTimeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Set Local Time     */
      /* Information message and send it to the server.                 */
      UpdateCurrentTimeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UpdateCurrentTimeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UpdateCurrentTimeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      UpdateCurrentTimeRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_UPDATE_CURRENT_TIME;
      UpdateCurrentTimeRequest.MessageHeader.MessageLength   = TIPM_UPDATE_CURRENT_TIME_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UpdateCurrentTimeRequest.ServerEventHandlerID          = ServerEventHandlerID;
      UpdateCurrentTimeRequest.AdjustReasonFlags             = AdjustReasonMask;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UpdateCurrentTimeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_UPDATE_CURRENT_TIME_RESPONSE_SIZE)
            ret_val = ((TIPM_Update_Current_Time_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* respond to a request for the Reference Time Information.  This    */
   /* function accepts the Server Event Handler ID (return value from   */
   /* _TIPM_Register_Server_Events() function) and a pointer to the     */
   /* Reference Time Information to respond to the request with.  This  */
   /* function returns ZERO if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _TIPM_Reference_Time_Response(unsigned int ServerEventHandlerID, TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   TIPM_Reference_Time_Response_Request_t  ReferenceTimeResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if(ReferenceTimeInformation)
      {
         /* All that we really need to do is to build a Set Local Time  */
         /* Information message and send it to the server.              */
         ReferenceTimeResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ReferenceTimeResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ReferenceTimeResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
         ReferenceTimeResponseRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_RESPONSE;
         ReferenceTimeResponseRequest.MessageHeader.MessageLength   = TIPM_REFERENCE_TIME_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ReferenceTimeResponseRequest.ServerEventHandlerID          = ServerEventHandlerID;
         ReferenceTimeResponseRequest.ReferenceTimeInformation      = *ReferenceTimeInformation;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ReferenceTimeResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_REFERENCE_TIME_RESPONSE_RESPONSE_SIZE)
               ret_val = ((TIPM_Reference_Time_Response_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Client callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Client Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Client Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int _TIPM_Register_Client_Events(void)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   TIPM_Register_Client_Events_Request_t  RegisterClientEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register TIP Events*/
      /* message and send it to the server.                             */
      RegisterClientEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterClientEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterClientEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      RegisterClientEventsRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS;
      RegisterClientEventsRequest.MessageHeader.MessageLength   = TIPM_REGISTER_CLIENT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterClientEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_REGISTER_CLIENT_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((TIPM_Register_Client_Events_Response_t *)ResponseMessage)->Status))
               ret_val = ((TIPM_Register_Client_Events_Response_t *)ResponseMessage)->ClientEventHandlerID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the Client Event Callback ID (return value from  */
   /* TIPM_Register_Client_Event_Callback() function).                  */
int _TIPM_Un_Register_Client_Events(unsigned int ClientEventHandlerID)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   TIPM_Un_Register_Client_Events_Request_t  UnRegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register TIP   */
      /* Events message and send it to the server.                      */
      UnRegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      UnRegisterEventsRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS;
      UnRegisterEventsRequest.MessageHeader.MessageLength   = TIPM_UN_REGISTER_CLIENT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterEventsRequest.ClientEventHandlerID          = ClientEventHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_UN_REGISTER_CLIENT_EVENTS_RESPONSE_SIZE)
            ret_val = ((TIPM_Un_Register_Client_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int _TIPM_Get_Current_Time(unsigned int ClientEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                              ret_val;
   BTPM_Message_t                  *ResponseMessage;
   TIPM_Get_Current_Time_Request_t  GetCurrentTimeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a get current time   */
      /* message and send it to the server.                             */
      GetCurrentTimeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetCurrentTimeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetCurrentTimeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      GetCurrentTimeRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME;
      GetCurrentTimeRequest.MessageHeader.MessageLength   = TIPM_GET_CURRENT_TIME_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetCurrentTimeRequest.ClientEventHandlerID          = ClientEventHandlerID;
      GetCurrentTimeRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetCurrentTimeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_GET_CURRENT_TIME_RESPONSE_SIZE)
            ret_val = ((TIPM_Get_Current_Time_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int _TIPM_Enable_Time_Notifications(unsigned int ClientEventHandlerID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Enable)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   TIPM_Enable_Time_Notifications_Request_t  EnableTimeNotificationsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a enable time        */
      /* notifications message and send it to the server.               */
      EnableTimeNotificationsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      EnableTimeNotificationsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      EnableTimeNotificationsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      EnableTimeNotificationsRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_ENABLE_TIME_NOTIFICATIONS;
      EnableTimeNotificationsRequest.MessageHeader.MessageLength   = TIPM_ENABLE_TIME_NOTIFICATIONS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      EnableTimeNotificationsRequest.ClientEventHandlerID          = ClientEventHandlerID;
      EnableTimeNotificationsRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      EnableTimeNotificationsRequest.Enable                        = Enable;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableTimeNotificationsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_ENABLE_TIME_NOTIFICATIONS_RESPONSE_SIZE)
            ret_val = ((TIPM_Enable_Time_Notifications_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int _TIPM_Get_Local_Time_Information(unsigned int ClientEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   TIPM_Get_Local_Time_Information_Request_t  GetLocalTimeInformationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a get local time     */
      /* information message and send it to the server.                 */
      GetLocalTimeInformationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetLocalTimeInformationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetLocalTimeInformationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      GetLocalTimeInformationRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_GET_LOCAL_TIME_INFORMATION;
      GetLocalTimeInformationRequest.MessageHeader.MessageLength   = TIPM_GET_LOCAL_TIME_INFORMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetLocalTimeInformationRequest.ClientEventHandlerID          = ClientEventHandlerID;
      GetLocalTimeInformationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetLocalTimeInformationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_GET_LOCAL_TIME_INFORMATION_RESPONSE_SIZE)
            ret_val = ((TIPM_Get_Local_Time_Information_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int _TIPM_Get_Time_Accuracy(unsigned int ClientEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   TIPM_Get_Time_Accuracy_Request_t  GetTimeAccuracyRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a get time accuracy  */
      /* message and send it to the server.                             */
      GetTimeAccuracyRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetTimeAccuracyRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetTimeAccuracyRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      GetTimeAccuracyRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_GET_TIME_ACCURACY;
      GetTimeAccuracyRequest.MessageHeader.MessageLength   = TIPM_GET_TIME_ACCURACY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetTimeAccuracyRequest.ClientEventHandlerID          = ClientEventHandlerID;
      GetTimeAccuracyRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetTimeAccuracyRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_GET_TIME_ACCURACY_RESPONSE_SIZE)
            ret_val = ((TIPM_Get_Time_Accuracy_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int _TIPM_Get_Next_DST_Change_Information(unsigned int ClientEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                             ret_val;
   BTPM_Message_t                                 *ResponseMessage;
   TIPM_Get_Next_DST_Change_Information_Request_t  GetNextDSTChangeInformationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a get next DST change*/
      /* information message and send it to the server.                 */
      GetNextDSTChangeInformationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetNextDSTChangeInformationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetNextDSTChangeInformationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      GetNextDSTChangeInformationRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_GET_NEXT_DST_CHANGE_INFORMATION;
      GetNextDSTChangeInformationRequest.MessageHeader.MessageLength   = TIPM_GET_NEXT_DST_CHANGE_INFORMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetNextDSTChangeInformationRequest.ClientEventHandlerID          = ClientEventHandlerID;
      GetNextDSTChangeInformationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetNextDSTChangeInformationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_GET_NEXT_DST_CHANGE_INFORMATION_RESPONSE_SIZE)
            ret_val = ((TIPM_Get_Next_DST_Change_Information_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int _TIPM_Get_Reference_Time_Update_State(unsigned int ClientEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   TIPM_Get_Reference_Time_Update_State_Request_t  GetReferenceTimeUpdateStateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a get reference time */
      /* update state message and send it to the server.                */
      GetReferenceTimeUpdateStateRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetReferenceTimeUpdateStateRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetReferenceTimeUpdateStateRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      GetReferenceTimeUpdateStateRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_GET_REFERENCE_TIME_UPDATE_STATE;
      GetReferenceTimeUpdateStateRequest.MessageHeader.MessageLength   = TIPM_GET_REFERENCE_TIME_UPDATE_STATE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetReferenceTimeUpdateStateRequest.ClientEventHandlerID          = ClientEventHandlerID;
      GetReferenceTimeUpdateStateRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetReferenceTimeUpdateStateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_GET_REFERENCE_TIME_UPDATE_STATE_RESPONSE_SIZE)
            ret_val = ((TIPM_Get_Reference_Time_Update_State_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int _TIPM_Request_Reference_Time_Update(unsigned int ClientEventHandlerID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   TIPM_Request_Reference_Time_Update_Request_t  RequestReferenceTimeUpdateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request reference  */
      /* time update message and send it to the server.                 */
      RequestReferenceTimeUpdateRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RequestReferenceTimeUpdateRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RequestReferenceTimeUpdateRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      RequestReferenceTimeUpdateRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_REQUEST_REFERENCE_TIME_UPDATE;
      RequestReferenceTimeUpdateRequest.MessageHeader.MessageLength   = TIPM_REQUEST_REFERENCE_TIME_UPDATE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      RequestReferenceTimeUpdateRequest.ClientEventHandlerID          = ClientEventHandlerID;
      RequestReferenceTimeUpdateRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RequestReferenceTimeUpdateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_REQUEST_REFERENCE_TIME_UPDATE_RESPONSE_SIZE)
            ret_val = ((TIPM_Request_Reference_Time_Update_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated        */
   /* with the total number of connected devices if the function is     */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns    */
   /* a non-negative value if successful which represents the number    */
   /* of connected devices that were copied into the specified input    */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _TIPM_Query_Connected_Devices(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, TIPM_Remote_Device_t *RemoteDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int                                     ret_val;
   unsigned int                            DevicesToCopy;
   BTPM_Message_t                         *ResponseMessage;
   TIPM_Query_Connected_Devices_Request_t  QueryConnectedDevicesRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceList)) || (TotalNumberConnectedDevices))
      {
         /* All that we really need to do is to build a request         */
         /* reference time update message and send it to the server.    */
         QueryConnectedDevicesRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedDevicesRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedDevicesRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
         QueryConnectedDevicesRequest.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES;
         QueryConnectedDevicesRequest.MessageHeader.MessageLength   = TIPM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryConnectedDevicesRequest.ConnectionType                = ConnectionType; 

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedDevicesRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TIPM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(((TIPM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevices))
            {
               /* Message is valid. Check if we were successful.        */
               if(!(ret_val = ((TIPM_Query_Connected_Devices_Response_t *)ResponseMessage)->Status))
               {
                  /* We were successful, so go ahead and copy the data. */
                  if(MaximumRemoteDeviceListEntries <= ((TIPM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevices)
                     DevicesToCopy = MaximumRemoteDeviceListEntries;
                  else
                     DevicesToCopy = ((TIPM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevices;

                  /* If they supplied the TotalDevice parameter, set it.*/
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = ((TIPM_Query_Connected_Devices_Response_t *)ResponseMessage)->NumberDevices;

                  /* If they supplied a buffer, copy devices into it.   */
                  if(RemoteDeviceList)
                  {
                     /* Note the number of devices copied.              */
                     ret_val = DevicesToCopy;

                     /* Copy in the memory.                             */
                     BTPS_MemCopy(RemoteDeviceList, ((TIPM_Query_Connected_Devices_Response_t *)ResponseMessage)->RemoteDevices, sizeof(TIPM_Remote_Device_t) * DevicesToCopy);
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
      }
      else ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

