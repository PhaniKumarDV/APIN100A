/*****< glpmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GLPMGR - GLP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMGLPM.h"            /* BTPM GLP Manager Prototypes/Constants.    */
#include "GLPMMSG.h"             /* BTPM GLP Manager Message Formats.         */
#include "GLPMGR.h"              /* GLP Manager Impl. Prototypes/Constants.   */

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
   /* initialize the GLP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager GLP Manager  */
   /* Implementation.                                                   */
int _GLPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing GLP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the GLP   */
   /* Manager Implementation.  After this function is called the GLP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _GLPM_Initialize() function.  */
void _GLPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the GLP Manager      */
   /* Service.  This Callback will be dispatched by the GLP Manager when*/
   /* various GLP Manager Events occur.  This function returns a        */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the GLP    */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _GLPM_Un_Register_Collector_Events() function to         */
   /*          Un-Register the Event Handler.                           */
int _GLPM_Register_Collector_Events(void)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   GLPM_Register_Collector_Events_Request_t  RegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register GLP Events*/
      /* message and send it to the server.                             */
      RegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
      RegisterCollectorEventsRequest.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS;
      RegisterCollectorEventsRequest.MessageHeader.MessageLength   = GLPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= GLPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((GLPM_Register_Collector_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((GLPM_Register_Collector_Events_Response_t *)ResponseMessage)->GLPEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered GLP Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _GLPM_Register_Collector_Events() function).  This function       */
   /* accepts input the GLP Event Handler ID (return value from         */
   /* _GLPM_Register_Collector_Events() function).                      */
int _GLPM_Un_Register_Collector_Events(unsigned int GLPMCollectorHandlerID)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   GLPM_Un_Register_Collector_Events_Request_t  UnRegisterCollectorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register GLP   */
      /* Events message and send it to the server.                      */
      UnRegisterCollectorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterCollectorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterCollectorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
      UnRegisterCollectorEventsRequest.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS;
      UnRegisterCollectorEventsRequest.MessageHeader.MessageLength   = GLPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterCollectorEventsRequest.GLPEventsHandlerID            = GLPMCollectorHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterCollectorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= GLPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE)
            ret_val = ((GLPM_Un_Register_Collector_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* starting a Glucose Procedure to a remote Glucose Device.  This    */
   /* function accepts as input the GLP Manager Collector Event Handler */
   /* ID (return value from _GLPM_Register_Collector_Events() function),*/
   /* the BD_ADDR of the remote Glucose Device and a pointer to a       */
   /* structure containing the procedure data.  This function returns   */
   /* the positive, non-zero, Procedure ID of the request on success or */
   /* a negative error code.                                            */
int _GLPM_Start_Procedure_Request(unsigned int GLPMCollectorHandlerID, BD_ADDR_t RemoteDeviceAddress, GLPM_Procedure_Data_t *ProcedureData)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   GLPM_Start_Procedure_Request_t  StartProcedureRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((GLPMCollectorHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ProcedureData))
      {
         /* All that we really need to do is to build a Get Report      */
         /* message and send it to the server.                          */
         StartProcedureRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         StartProcedureRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         StartProcedureRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
         StartProcedureRequest.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_START_PROCEDURE;
         StartProcedureRequest.MessageHeader.MessageLength   = GLPM_START_PROCEDURE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         StartProcedureRequest.GLPCollectorEventsHandlerID   = GLPMCollectorHandlerID;
         StartProcedureRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         StartProcedureRequest.ProcedureData                 = *ProcedureData;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&StartProcedureRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= GLPM_START_PROCEDURE_RESPONSE_SIZE)
            {
               if(!(ret_val = ((GLPM_Start_Procedure_Response_t *)ResponseMessage)->Status))
                  ret_val = (int)(((GLPM_Start_Procedure_Response_t *)ResponseMessage)->ProcedureID);
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* stopping a previouly started Glucose Procedure to a remote Glucose*/
   /* Device.  This function accepts as input the GLP Manager Collector */
   /* Event Handler ID (return value from                               */
   /* _GLPM_Register_Collector_Events() function), the BD_ADDR of the   */
   /* remote Glucose Device and the Procedure ID that was returned via a*/
   /* successfull call to GLPM_Start_Procedure_Request().  This function*/
   /* returns zero on success or a negative error code.                 */
int _GLPM_Stop_Procedure_Request(unsigned int GLPMCollectorHandlerID, BD_ADDR_t RemoteDeviceAddress, unsigned int ProcedureID)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   GLPM_Stop_Procedure_Request_t  StopProcedureRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((GLPMCollectorHandlerID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ProcedureID))
      {
         /* All that we really need to do is to build a Get Report      */
         /* message and send it to the server.                          */
         StopProcedureRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         StopProcedureRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         StopProcedureRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
         StopProcedureRequest.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_STOP_PROCEDURE;
         StopProcedureRequest.MessageHeader.MessageLength   = GLPM_STOP_PROCEDURE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         StopProcedureRequest.GLPCollectorEventsHandlerID   = GLPMCollectorHandlerID;
         StopProcedureRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         StopProcedureRequest.ProcedureID                   = ProcedureID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&StopProcedureRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= GLPM_STOP_PROCEDURE_RESPONSE_SIZE)
               ret_val = ((GLPM_Stop_Procedure_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

