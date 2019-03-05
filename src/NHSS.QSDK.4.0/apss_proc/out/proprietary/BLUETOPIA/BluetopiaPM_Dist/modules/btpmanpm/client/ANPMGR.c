/*****< anpmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPMGR - ANP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/06/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMANPM.h"            /* BTPM ANP Manager Prototypes/Constants.    */
#include "ANPMMSG.h"             /* BTPM ANP Manager Message Formats.         */
#include "ANPMGR.h"              /* ANP Manager Impl. Prototypes/Constants.   */

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
   /* initialize the ANP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager ANP Manager  */
   /* Implementation.                                                   */
int _ANPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALREADY_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the ANP   */
   /* Manager Implementation.  After this function is called the ANP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _ANPM_Initialize() function.  */
void _ANPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of New Alerts for a specific category and*/
   /* the text of the last alert for the specified category.  This      */
   /* function accepts as the Category ID of the specific category, the */
   /* number of new alerts for the specified category and a text string */
   /* that describes the last alert for the specified category (if any).*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
   /* * NOTE * If there is not a last alert text available, then specify*/
   /*          NULL.                                                    */
int _ANPM_Set_New_Alert(ANPM_Category_Identification_t CategoryID, unsigned int NewAlertCount, char *LastAlertText)
{
   int                           ret_val;
   unsigned int                  LastAlertTextLength;
   BTPM_Message_t               *ResponseMessage;
   ANPM_Set_New_Alert_Request_t *SetNewAlert;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Calculate the variable data length based on the string size.   */
      if(LastAlertText)
         LastAlertTextLength = (BTPS_StringLength(LastAlertText) * sizeof(char)) + 1;
      else
         LastAlertTextLength = 0;

      /* Allocate a buffer to hold the entire message.                  */
      if((SetNewAlert = BTPS_AllocateMemory(ANPM_SET_NEW_ALERT_REQUEST_SIZE(LastAlertTextLength))) != NULL)
      {
         /* All that we really need to do is to build a Set New Alert   */
         /* message and send it to the server.                          */
         SetNewAlert->MessageHeader.AddressID       = MSG_GetServerAddressID(); //xxx why is this server address?
         SetNewAlert->MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetNewAlert->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
         SetNewAlert->MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_SET_NEW_ALERT;
         SetNewAlert->MessageHeader.MessageLength   = ANPM_SET_NEW_ALERT_REQUEST_SIZE(LastAlertTextLength) - BTPM_MESSAGE_HEADER_SIZE;

         SetNewAlert->CategoryID                    = CategoryID;
         SetNewAlert->NewAlertCount                 = NewAlertCount;
         SetNewAlert->LastAlertTextLength           = LastAlertTextLength;

         /* Copy over the alert text into the message.                  */
         if(LastAlertTextLength)
            BTPS_StringCopy(SetNewAlert->LastAlertText, LastAlertText);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SetNewAlert, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= (ANPM_SET_NEW_ALERT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE))
               ret_val = ((ANPM_Set_New_Alert_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of Un-Read Alerts for a specific         */
   /* category.  This function accepts as the Category ID of the        */
   /* specific category, and the number of un-read alerts for the       */
   /* specified category.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
int _ANPM_Set_Un_Read_Alert(ANPM_Category_Identification_t CategoryID, unsigned int UnReadAlertCount)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   ANPM_Set_Un_Read_Alert_Request_t  SetUnReadAlert;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Set New Alert      */
      /* message and send it to the server.                             */
      SetUnReadAlert.MessageHeader.AddressID       = MSG_GetServerAddressID(); //xxx why is this server address?
      SetUnReadAlert.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetUnReadAlert.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      SetUnReadAlert.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_SET_UN_READ_ALERT;
      SetUnReadAlert.MessageHeader.MessageLength   = ANPM_SET_UN_READ_ALERT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetUnReadAlert.CategoryID                    = CategoryID;
      SetUnReadAlert.UnReadAlertCount              = UnReadAlertCount;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetUnReadAlert, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= (ANPM_SET_UN_READ_ALERT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE))
            ret_val = ((ANPM_Set_Un_Read_Alert_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Alert            */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Events     */
   /* occur.  This function accepts the Callback Function and Callback  */
   /* Parameter (respectively) to call when a ANP Manager Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANPM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int _ANPM_Register_ANP_Events(void)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   ANPM_Register_ANP_Events_Request_t  RegisterANPEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register ANP Events*/
      /* message and send it to the server.                             */
      RegisterANPEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterANPEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterANPEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      RegisterANPEventsRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_REGISTER_ANP_EVENTS;
      RegisterANPEventsRequest.MessageHeader.MessageLength   = ANPM_REGISTER_ANP_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterANPEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_REGISTER_ANP_EVENTS_RESPONSE_SIZE)
            ret_val = ((ANPM_Register_ANP_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* ANPM_Register_Event_Callback() function).  This function accepts  */
   /* as input the ANP Manager Event Callback ID (return value from     */
   /* ANPM_Register_Event_Callback() function).                         */
int _ANPM_Un_Register_ANP_Events(void)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   ANPM_Un_Register_ANP_Events_Request_t  UnRegisterANPEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register ANP   */
      /* Events message and send it to the server.                      */
      UnRegisterANPEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterANPEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterANPEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      UnRegisterANPEventsRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_EVENTS;
      UnRegisterANPEventsRequest.MessageHeader.MessageLength   = ANPM_UN_REGISTER_ANP_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterANPEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_UN_REGISTER_ANP_EVENTS_RESPONSE_SIZE)
            ret_val = ((ANPM_Un_Register_ANP_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* ANPM Client functions.                                            */

   /* This functions submits a request to a remote ANP Server to get    */
   /* the supported categories.  The ClientCallbackID parameter should  */
   /* be an ID return from ANPM_Register_Client_Event_Callback().  The  */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * An aetANPSupportedNewAlertCategoriesResult event will be */
   /*          dispatched when this request completes.                  */
int _ANPM_Get_Supported_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   ANPM_Get_Supported_Categories_Request_t  GetSupportedCategoriesRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&GetSupportedCategoriesRequest, 0, sizeof(GetSupportedCategoriesRequest));

      GetSupportedCategoriesRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      GetSupportedCategoriesRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      GetSupportedCategoriesRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      GetSupportedCategoriesRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_GET_SUPPORTED_CATEGORIES;
      GetSupportedCategoriesRequest.MessageHeader.MessageLength   = ANPM_GET_SUPPORTED_CATEGORIES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      GetSupportedCategoriesRequest.ClientCallbackID              = ClientCallbackID;
      GetSupportedCategoriesRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      GetSupportedCategoriesRequest.NotificationType              = Type;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetSupportedCategoriesRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_GET_SUPPORTED_CATEGORIES_RESPONSE_SIZE)
            ret_val = ((ANPM_Get_Supported_Categories_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions will enable or disable notifications from a        */
   /* remote ANP server.  The ClientCallbackID parameter should be      */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  If successful and a GATT write was triggered to   */
   /* enabled notifications on the remote ANP server, this function     */
   /* will return a positive value representing the Transaction ID of   */
   /* the submitted write. If this function successfully registers the  */
   /* callback, but a GATT write is not necessary, it will return 0. If */
   /* an error occurs, this function will return a negative error code. */
int _ANPM_Enable_Disable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type, Boolean_t Enable)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   ANPM_Enable_Disable_Notifications_Request_t  EnableDisableNotificationsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&EnableDisableNotificationsRequest, 0, sizeof(EnableDisableNotificationsRequest));

      EnableDisableNotificationsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      EnableDisableNotificationsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      EnableDisableNotificationsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      EnableDisableNotificationsRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_NOTIFICATIONS;
      EnableDisableNotificationsRequest.MessageHeader.MessageLength   = ANPM_ENABLE_DISABLE_NOTIFICATIONS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      EnableDisableNotificationsRequest.ClientCallbackID              = ClientCallbackID;
      EnableDisableNotificationsRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      EnableDisableNotificationsRequest.NotificationType              = Type;
      EnableDisableNotificationsRequest.Enable                        = Enable;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableDisableNotificationsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_ENABLE_DISABLE_NOTIFICATIONS_RESPONSE_SIZE)
            ret_val = ((ANPM_Enable_Disable_Notifications_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server to enable */
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  If successful and a GATT write was triggered to enabled  */
   /* notifications on the remote ANP server, this function will return */
   /* a positive value representing the Transaction ID of the submitted */
   /* write. If this function successfully registers the callback, but a*/
   /* GATT write is not necessary, it will return 0. If an error occurs,*/
   /* this function will return a negative error code.                  */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value if positive.                            */
int _ANPM_Enable_Disable_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type, Boolean_t Enable)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   ANPM_Enable_Disable_Category_Request_t  EnableDisableCategoryRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&EnableDisableCategoryRequest, 0, sizeof(EnableDisableCategoryRequest));

      EnableDisableCategoryRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      EnableDisableCategoryRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      EnableDisableCategoryRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      EnableDisableCategoryRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_CATEGORY;
      EnableDisableCategoryRequest.MessageHeader.MessageLength   = ANPM_ENABLE_DISABLE_CATEGORY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      EnableDisableCategoryRequest.ClientCallbackID              = ClientCallbackID;
      EnableDisableCategoryRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      EnableDisableCategoryRequest.CategoryID                    = CategoryID;
      EnableDisableCategoryRequest.NotificationType              = Type;
      EnableDisableCategoryRequest.Enable                        = Enable;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableDisableCategoryRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_ENABLE_DISABLE_CATEGORY_RESPONSE_SIZE)
            ret_val = ((ANPM_Enable_Disable_Category_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server to request*/
   /* an immediate Notification.  The ClientCallbackID parameter should */
   /* be an ID return from ANPM_Register_Client_Event_Callback().  The  */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  This function returns a positive value representing the  */
   /* Transaction ID if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value.                                        */
int _ANPM_Request_Notification(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   ANPM_Request_Notification_Request_t  RequestNotificationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&RequestNotificationRequest, 0, sizeof(RequestNotificationRequest));

      RequestNotificationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RequestNotificationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RequestNotificationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      RequestNotificationRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_REQUEST_NOTIFICATION;
      RequestNotificationRequest.MessageHeader.MessageLength   = ANPM_REQUEST_NOTIFICATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      RequestNotificationRequest.ClientCallbackID              = ClientCallbackID;
      RequestNotificationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      RequestNotificationRequest.CategoryID                    = CategoryID;
      RequestNotificationRequest.NotificationType              = Type;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RequestNotificationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_REQUEST_NOTIFICATION_RESPONSE_SIZE)
            ret_val = ((ANPM_Request_Notification_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Client     */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int _ANPM_Register_Client_Event_Callback(void)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   ANPM_Register_ANP_Client_Events_Request_t  RegisterANPClientEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&RegisterANPClientEventsRequest, 0, sizeof(RegisterANPClientEventsRequest));

      RegisterANPClientEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterANPClientEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterANPClientEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      RegisterANPClientEventsRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_REGISTER_ANP_CLIENT_EVENTS;
      RegisterANPClientEventsRequest.MessageHeader.MessageLength   = ANPM_REGISTER_ANP_CLIENT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterANPClientEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_REGISTER_ANP_CLIENT_EVENTS_RESPONSE_SIZE)
            ret_val = ((ANPM_Register_ANP_Client_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* ANPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the ANP Manager Event Callback ID (return value  */
   /* from ANPM_Register_Client_Event_Callback() function).             */
int _ANPM_Un_Register_Client_Event_Callback(unsigned int ANPManagerCallbackID)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   ANPM_Un_Register_ANP_Client_Events_Request_t  UnRegisterANPClientEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&UnRegisterANPClientEventsRequest, 0, sizeof(UnRegisterANPClientEventsRequest));

      UnRegisterANPClientEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterANPClientEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterANPClientEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      UnRegisterANPClientEventsRequest.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_CLIENT_EVENTS;
      UnRegisterANPClientEventsRequest.MessageHeader.MessageLength   = ANPM_UN_REGISTER_ANP_CLIENT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterANPClientEventsRequest.ClientCallbackID              = ANPManagerCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterANPClientEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANPM_UN_REGISTER_ANP_CLIENT_EVENTS_RESPONSE_SIZE)
            ret_val = ((ANPM_Un_Register_ANP_Client_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

