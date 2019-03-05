/*****< fmpmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FMPMGR - FMP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/27/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMFMPM.h"            /* BTPM FMP Manager Prototypes/Constants.    */
#include "FMPMMSG.h"             /* BTPM FMP Manager Message Formats.         */
#include "FMPMGR.h"              /* FMP Manager Impl. Prototypes/Constants.   */

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
   /* initialize the FMP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager FMP Manager  */
   /* Implementation.                                                   */
int _FMPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing FMP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALREADY_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the FMP   */
   /* Manager Implementation.  After this function is called the FMP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _FMPM_Initialize() function.  */
void _FMPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Alert            */
   /* Notification (FMP) Manager Service.  This Callback will be        */
   /* dispatched by the FMP Manager when various FMP Manager Events     */
   /* occur.  This function accepts the Callback Function and Callback  */
   /* Parameter (respectively) to call when a FMP Manager Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _FMPM_Un_Register_Target_Events() function to un-register*/
   /*          the callback from this module.                           */
int _FMPM_Register_Target_Events(void)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   FMPM_Register_Target_Events_Request_t  RegisterFMPTargetEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register FMP Events*/
      /* message and send it to the server.                             */
      RegisterFMPTargetEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterFMPTargetEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterFMPTargetEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_FIND_ME_MANAGER;
      RegisterFMPTargetEventsRequest.MessageHeader.MessageFunction = FMPM_MESSAGE_FUNCTION_REGISTER_TARGET_EVENTS;
      RegisterFMPTargetEventsRequest.MessageHeader.MessageLength   = FMPM_REGISTER_TARGET_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterFMPTargetEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= FMPM_REGISTER_TARGET_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((FMPM_Register_Target_Events_Response_t *)ResponseMessage)->Status))
               ret_val = ((FMPM_Register_Target_Events_Response_t *)ResponseMessage)->FMPTargetEventHandlerID;
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered FMP Target Manager Event      */
   /* Callback (registered via a successful call to the                 */
   /* _FMPM_Register_Target_Events() function).  This function accepts  */
   /* as input the FMP Manager Event Callback ID (return value from     */
   /* _FMPM_Register_Target_Events() function).                         */
int _FMPM_Un_Register_Target_Events(unsigned int FMPTargetEventHandlerID)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   FMPM_Un_Register_Target_Events_Request_t  UnRegisterFMPTargetEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register FMP   */
      /* Events message and send it to the server.                      */
      UnRegisterFMPTargetEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterFMPTargetEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterFMPTargetEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_FIND_ME_MANAGER;
      UnRegisterFMPTargetEventsRequest.MessageHeader.MessageFunction = FMPM_MESSAGE_FUNCTION_UN_REGISTER_TARGET_EVENTS;
      UnRegisterFMPTargetEventsRequest.MessageHeader.MessageLength   = FMPM_UN_REGISTER_TARGET_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterFMPTargetEventsRequest.FMPTargetEventHandlerID       = FMPTargetEventHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterFMPTargetEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= FMPM_UN_REGISTER_TARGET_EVENTS_RESPONSE_SIZE)
            ret_val = ((FMPM_Un_Register_Target_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
