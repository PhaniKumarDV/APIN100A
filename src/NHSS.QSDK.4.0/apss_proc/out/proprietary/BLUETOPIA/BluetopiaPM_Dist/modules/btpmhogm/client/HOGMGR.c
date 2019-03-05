/*****< hidmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HOGMGR - HOG Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHOGM.h"            /* BTPM HID Manager Prototypes/Constants.    */
#include "HOGMMSG.h"             /* BTPM HID Manager Message Formats.         */
#include "HOGMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

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
   /* initialize the HID Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HID Manager  */
   /* Implementation.                                                   */
int _HOGM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the HID   */
   /* Manager Implementation.  After this function is called the HID    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HOGM_Initialize() function.  */
void _HOGM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the HID Manager      */
   /* Service.  This Callback will be dispatched by the HID Manager when*/
   /* various HID Manager Events occur.  This function returns a        */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the HID    */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _HOGM_Un_Register_HID_Events() function to Un-Register   */
   /*          the Event Handler.                                       */
int _HOGM_Register_HID_Events(void)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   HOGM_Register_HID_Events_Request_t  RegisterHIDEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register HID Events*/
      /* message and send it to the server.                             */
      RegisterHIDEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterHIDEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterHIDEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
      RegisterHIDEventsRequest.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS;
      RegisterHIDEventsRequest.MessageHeader.MessageLength   = HOGM_REGISTER_HID_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHIDEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_REGISTER_HID_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((HOGM_Register_HID_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((HOGM_Register_HID_Events_Response_t *)ResponseMessage)->HOGEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _HOGM_Register_HID_Events() function).  This function accepts     */
   /* input the HID Event Handler ID (return value from                 */
   /* _HOGM_Register_HID_Events() function).                            */
int _HOGM_Un_Register_HID_Events(unsigned int HOGEventsHandlerID)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   HOGM_Un_Register_HID_Events_Request_t  UnRegisterHIDEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register HID   */
      /* Events message and send it to the server.                      */
      UnRegisterHIDEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterHIDEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterHIDEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
      UnRegisterHIDEventsRequest.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS;
      UnRegisterHIDEventsRequest.MessageHeader.MessageLength   = HOGM_UN_REGISTER_HID_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterHIDEventsRequest.HOGEventsHandlerID            = HOGEventsHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHIDEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_UN_REGISTER_HID_EVENTS_RESPONSE_SIZE)
            ret_val = ((HOGM_Un_Register_HID_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the HID       */
   /* Manager Service to explicitly process HID Data.  This Callback    */
   /* will be dispatched by the HID Manager when various HID Manager    */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HOGM_Send_Report_Data() function to send data).         */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          in the system.                                           */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HOGM_Un_Register_HID_Data_Events() function to          */
   /*          un-register the callback from this module.               */
int _HOGM_Register_HID_Data_Events(void)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   HOGM_Register_HID_Data_Events_Request_t  RegisterHIDDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register HID Data  */
      /* Events message and send it to the server.                      */
      RegisterHIDDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterHIDDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterHIDDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
      RegisterHIDDataEventsRequest.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_REGISTER_HID_DATA;
      RegisterHIDDataEventsRequest.MessageHeader.MessageLength   = HOGM_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterHIDDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((HOGM_Register_HID_Data_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((HOGM_Register_HID_Data_Events_Response_t *)ResponseMessage)->HOGDataEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Data Event        */
   /* Callback (registered via a successful call to the                 */
   /* _HOGM_Register_HID_Data_Events() function).  This function accepts*/
   /* as input the HID Manager Event Callback ID (return value from     */
   /* _HOGM_Register_HID_Data_Events() function).                       */
int _HOGM_Un_Register_HID_Data_Events(unsigned int HOGDataEventHandlerID)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   HOGM_Un_Register_HID_Data_Events_Request_t  UnRegisterHIDDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register HID   */
      /* Data Events message and send it to the server.                 */
      UnRegisterHIDDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA;
      UnRegisterHIDDataEventsRequest.MessageHeader.MessageLength   = HOGM_UN_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterHIDDataEventsRequest.HOGDataEventsHandlerID        = HOGDataEventHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterHIDDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_UN_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE)
            ret_val = ((HOGM_Un_Register_HID_Data_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of setting*/
   /* the HID Protocol Mode on a remote HID Device.  This function      */
   /* accepts as input the HOG Manager Data Event Handler ID (return    */
   /* value from _HOGM_Register_HID_Data_Events() function), the BD_ADDR*/
   /* of the remote HID Device and the Protocol Mode to set.  This      */
   /* function returns zero on success or a negative error code.        */
int _HOGM_Set_Protocol_Mode(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Protocol_Mode_t ProtocolMode)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   HOGM_HID_Set_Protocol_Mode_Request_t  SetProtocolModeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((HOGDataEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((ProtocolMode == hpmBoot) || (ProtocolMode == hpmReport)))
      {
         /* All that we really need to do is to build a Set Protocol    */
         /* Mode message and send it to the server.                     */
         SetProtocolModeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetProtocolModeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetProtocolModeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
         SetProtocolModeRequest.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_SET_PROTOCOL_MODE;
         SetProtocolModeRequest.MessageHeader.MessageLength   = HOGM_HID_SET_PROTOCOL_MODE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetProtocolModeRequest.HOGManagerDataHandlerID       = HOGDataEventHandlerID;
         SetProtocolModeRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetProtocolModeRequest.ProtocolMode                  = ProtocolMode;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetProtocolModeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_HID_SET_PROTOCOL_MODE_RESPONSE_SIZE)
               ret_val = ((HOGM_HID_Set_Protocol_Mode_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* informing the specified remote HID Device that the local HID Host */
   /* is entering/exiting the Suspend State.  This function accepts as  */
   /* input the HOG Manager Data Event Callback ID (return value from   */
   /* _HOGM_Register_HID_Data_Events() function), the BD_ADDR of the    */
   /* remote HID Device and the a Boolean that indicates if the Host is */
   /* entering suspend state (TRUE) or exiting suspend state (FALSE).   */
   /* This function returns zero on success or a negative error code.   */
int _HOGM_Set_Suspend_Mode(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Suspend)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   HOGM_HID_Set_Suspend_Mode_Request_t  SetSuspendModeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((HOGDataEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* All that we really need to do is to build a Set Suspend Mode*/
         /* message and send it to the server.                          */
         SetSuspendModeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetSuspendModeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetSuspendModeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
         SetSuspendModeRequest.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_SET_SUSPEND_MODE;
         SetSuspendModeRequest.MessageHeader.MessageLength   = HOGM_HID_SET_SUSPEND_MODE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetSuspendModeRequest.HOGManagerDataHandlerID       = HOGDataEventHandlerID;
         SetSuspendModeRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         SetSuspendModeRequest.Suspend                       = Suspend;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetSuspendModeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_HID_SET_SUSPEND_MODE_RESPONSE_SIZE)
               ret_val = ((HOGM_HID_Set_Suspend_Mode_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Get Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from _HOGM_Register_HID_Data_Events() function), */
   /* the BD_ADDR of the remote HID Device and a pointer to a structure */
   /* containing information on the Report to set.  This function       */
   /* returns the positive, non-zero, Transaction ID of the request on  */
   /* success or a negative error code.                                 */
int _HOGM_Get_Report_Request(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   HOGM_HID_Get_Report_Request_t  GetReportRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((HOGDataEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ReportInformation))
      {
         /* All that we really need to do is to build a Get Report      */
         /* message and send it to the server.                          */
         GetReportRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         GetReportRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         GetReportRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
         GetReportRequest.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_GET_REPORT;
         GetReportRequest.MessageHeader.MessageLength   = HOGM_HID_GET_REPORT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         GetReportRequest.HOGManagerDataHandlerID       = HOGDataEventHandlerID;
         GetReportRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         GetReportRequest.ReportInformation             = *ReportInformation;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetReportRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_HID_GET_REPORT_RESPONSE_SIZE)
            {
               if(!(ret_val = ((HOGM_HID_Get_Report_Response_t *)ResponseMessage)->Status))
                  ret_val = (int)(((HOGM_HID_Get_Report_Response_t *)ResponseMessage)->TransactionID);
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
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Set Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from _HOGM_Register_HID_Data_Events() function), */
   /* the BD_ADDR of the remote HID Device, a pointer to a structure    */
   /* containing information on the Report to set, a Boolean that       */
   /* indicates if a response is expected, and the Report Data to set.  */
   /* This function returns the positive, non-zero, Transaction ID of   */
   /* the request (if a Response is expected, ZERO if no response is    */
   /* expected) on success or a negative error code.                    */
int _HOGM_Set_Report_Request(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation, Boolean_t ResponseExpected, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   HOGM_HID_Set_Report_Request_t *SetReportRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((HOGDataEventHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ReportInformation) && (ReportDataLength) && (ReportData))
      {
         /* Allocate a buffer to hold this message.                     */
         if((SetReportRequest = BTPS_AllocateMemory(HOGM_HID_SET_REPORT_REQUEST_SIZE(ReportDataLength))) != NULL)
         {
            /* All that we really need to do is to build a Set Report   */
            /* message and send it to the server.                       */
            SetReportRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SetReportRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SetReportRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
            SetReportRequest->MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_SET_REPORT;
            SetReportRequest->MessageHeader.MessageLength   = HOGM_HID_SET_REPORT_REQUEST_SIZE(ReportDataLength) - BTPM_MESSAGE_HEADER_SIZE;

            SetReportRequest->HOGManagerDataHandlerID       = HOGDataEventHandlerID;
            SetReportRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SetReportRequest->ReportInformation             = *ReportInformation;
            SetReportRequest->ResponseExpected              = ResponseExpected;
            SetReportRequest->ReportDataLength              = ReportDataLength;

            BTPS_MemCopy(SetReportRequest->ReportData, ReportData, ReportDataLength);

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SetReportRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid.  If it is, go ahead and note the      */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= HOGM_HID_SET_REPORT_RESPONSE_SIZE)
               {
                  if(!(ret_val = ((HOGM_HID_Set_Report_Response_t *)ResponseMessage)->Status))
                     ret_val = (int)(((HOGM_HID_Set_Report_Response_t *)ResponseMessage)->TransactionID);
               }
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }

            /* Free the previously allocated memory.                    */
            BTPS_FreeMemory(SetReportRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

