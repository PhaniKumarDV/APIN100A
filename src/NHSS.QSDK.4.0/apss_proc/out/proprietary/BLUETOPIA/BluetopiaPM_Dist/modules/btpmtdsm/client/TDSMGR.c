/*****< tdsmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSMGR - 3D Sync Manager Implementation for Stonestreet One Bluetooth     */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMTDSM.h"            /* BTPM 3D Sync Manager Prototypes/Constants.*/
#include "TDSMMSG.h"             /* BTPM 3D Sync Manager Message Formats.     */
#include "TDSMGR.h"              /* 3D Sync Manager Imp. Prototypes/Constants.*/

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
   /* initialize the 3D Sync Manager implementation.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* 3D Sync Manager Implementation.                                   */
int _TDSM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing 3D Sync Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* 3D Sync Manager implementation.  After this function is called the*/
   /* 3D Sync Manager implementation will no longer operate until it is */
   /* initialized again via a call to the _TDSM_Initialize() function.  */
void _TDSM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will configure the Synchronization Train   */
   /* parameters on the Bluetooth controller.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The SyncTrainParams parameter is a pointer to the       */
   /* Synchronization Train parameters to be set.  The IntervalResult   */
   /* parameter is a pointer to the variable that will be populated with*/
   /* the Synchronization Interval chosen by the Bluetooth controller.  */
   /* This function will return zero on success; otherwise, a negative  */
   /* error value will be returned.                                     */
   /* * NOTE * The Timout value in the                                  */
   /*          TDS_Synchronization_Train_Parameters_t structure must be */
   /*          a value of at least 120 seconds, or the function call    */
   /*          will fail.                                               */
int _TDSM_Write_Synchronization_Train_Parameters(unsigned int TDSMControlCallbackID, TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult)
{
   int                                                    ret_val;
   BTPM_Message_t                                        *ResponseMessage;
   TDSM_Write_Synchronization_Train_Parameters_Request_t  WriteSynchronizationTrainParametersRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((TDSMControlCallbackID) && (SyncTrainParams) && (IntervalResult))
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&WriteSynchronizationTrainParametersRequest, 0, sizeof(WriteSynchronizationTrainParametersRequest));

         WriteSynchronizationTrainParametersRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         WriteSynchronizationTrainParametersRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         WriteSynchronizationTrainParametersRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
         WriteSynchronizationTrainParametersRequest.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS;
         WriteSynchronizationTrainParametersRequest.MessageHeader.MessageLength   = TDSM_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         WriteSynchronizationTrainParametersRequest.TDSMControlCallbackID         = TDSMControlCallbackID;
         WriteSynchronizationTrainParametersRequest.SyncTrainParams               = *SyncTrainParams;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&WriteSynchronizationTrainParametersRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS_RESPONSE_SIZE)
            {
               ret_val = ((TDSM_Write_Synchronization_Train_Parameters_Response_t *)ResponseMessage)->Status;
               *IntervalResult = ((TDSM_Write_Synchronization_Train_Parameters_Response_t *)ResponseMessage)->IntervalResult;
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
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function will enable the Synchronization Train. The */
   /* TDSMControlCallbackID parameter is an ID returned from a succesfull*/
   /* call to TDSM_Register_Event_Callback() with the Control parameter */
   /* set to TRUE.  This function will return zero on success;          */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The TDSM_Write_Synchronization_Train_Parameters function */
   /*          should be called at least once after initializing the    */
   /*          stack and before calling this function.                  */
   /* * NOTE * The tetTDSM_Display_Synchronization_Train_Complete event */
   /*          will be triggered when the Synchronization Train         */
   /*          completes.  This function can be called again at this    */
   /*          time to restart the Synchronization Train.               */
int _TDSM_Start_Synchronization_Train(unsigned int TDSMControlCallbackID)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   TDSM_Start_Synchronization_Train_Request_t   StartSynchronizationTrainRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(TDSMControlCallbackID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&StartSynchronizationTrainRequest, 0, sizeof(StartSynchronizationTrainRequest));

         StartSynchronizationTrainRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         StartSynchronizationTrainRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         StartSynchronizationTrainRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
         StartSynchronizationTrainRequest.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_START_SYNCHRONIZATION_TRAIN;
         StartSynchronizationTrainRequest.MessageHeader.MessageLength   = TDSM_START_SYNCHRONIZATION_TRAIN_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         StartSynchronizationTrainRequest.TDSMControlCallbackID         = TDSMControlCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&StartSynchronizationTrainRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_START_SYNCHRONIZATION_TRAIN_RESPONSE_SIZE)
               ret_val = ((TDSM_Start_Synchronization_Train_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function will configure and enable                  */
   /* the Connectionless Slave Broadcast channel on the                 */
   /* Bluetooth controller.  The TDSMControlCallbackID                   */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The ConnectionlessSlaveBroadcastParams parameter is a   */
   /* pointer to the Connectionless Slave Broadcast parameters to be    */
   /* set.  The IntervalResult parameter is a pointer to the variable   */
   /* that will be populated with the Broadcast Interval chosen by the  */
   /* Bluetooth controller.  This function will return zero on success; */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The MinInterval value should be greater than or equal to */
   /*          50 milliseconds, and the MaxInterval value should be less*/
   /*          than or equal to 100 milliseconds; otherwise, the        */
   /*          function will fail.                                      */
int _TDSM_Enable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID, TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult)
{
   int                                                   ret_val;
   BTPM_Message_t                                       *ResponseMessage;
   TDSM_Enable_Connectionless_Slave_Broadcast_Request_t  EnableConnectionlessSlaveBroadcastRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((TDSMControlCallbackID) && (ConnectionlessSlaveBroadcastParams) && (IntervalResult))
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&EnableConnectionlessSlaveBroadcastRequest, 0, sizeof(EnableConnectionlessSlaveBroadcastRequest));

         EnableConnectionlessSlaveBroadcastRequest.MessageHeader.AddressID            = MSG_GetServerAddressID();
         EnableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageID            = MSG_GetNextMessageID();
         EnableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageGroup         = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
         EnableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageFunction      = TDSM_MESSAGE_FUNCTION_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST;
         EnableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageLength        = TDSM_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         EnableConnectionlessSlaveBroadcastRequest.TDSMControlCallbackID              = TDSMControlCallbackID;
         EnableConnectionlessSlaveBroadcastRequest.ConnectionlessSlaveBroadcastParams = *ConnectionlessSlaveBroadcastParams;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableConnectionlessSlaveBroadcastRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST_RESPONSE_SIZE)
            {
               ret_val         = ((TDSM_Enable_Connectionless_Slave_Broadcast_Response_t *)ResponseMessage)->Status;
               *IntervalResult = ((TDSM_Enable_Connectionless_Slave_Broadcast_Response_t *)ResponseMessage)->IntervalResult;
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
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.                                                             */
   /* * NOTE * Calling this function will terminate the Synchronization */
   /*          Train (if it is currently enabled).                      */
int _TDSM_Disable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID)
{
   int                                                    ret_val;
   BTPM_Message_t                                        *ResponseMessage;
   TDSM_Disable_Connectionless_Slave_Broadcast_Request_t  DisableConnectionlessSlaveBroadcastRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(TDSMControlCallbackID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&DisableConnectionlessSlaveBroadcastRequest, 0, sizeof(DisableConnectionlessSlaveBroadcastRequest));

         DisableConnectionlessSlaveBroadcastRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
         DisableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST;
         DisableConnectionlessSlaveBroadcastRequest.MessageHeader.MessageLength   = TDSM_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisableConnectionlessSlaveBroadcastRequest.TDSMControlCallbackID         = TDSMControlCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisableConnectionlessSlaveBroadcastRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST_RESPONSE_SIZE)
               ret_val = ((TDSM_Disable_Connectionless_Slave_Broadcast_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is used to get the current information     */
   /* being used int the synchronization broadcasts.  The               */
   /* CurrentBroadcastInformation parameter to a structure in which the */
   /* current information will be placed.  This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
int _TDSM_Get_Current_Broadcast_Information(TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation)
{
   int                                               ret_val;
   BTPM_Message_t                                   *ResponseMessage;
   TDSM_Get_Current_Broadcast_Information_Request_t  GetCurrentBroadcastInformationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(CurrentBroadcastInformation)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&GetCurrentBroadcastInformationRequest, 0, sizeof(GetCurrentBroadcastInformationRequest));

         GetCurrentBroadcastInformationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         GetCurrentBroadcastInformationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         GetCurrentBroadcastInformationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
         GetCurrentBroadcastInformationRequest.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_GET_CURRENT_BROADCAST_INFORMATION;
         GetCurrentBroadcastInformationRequest.MessageHeader.MessageLength   = TDSM_GET_CURRENT_BROADCAST_INFORMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&GetCurrentBroadcastInformationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_GET_CURRENT_BROADCAST_INFORMATION_RESPONSE_SIZE)
            {
               ret_val                      = ((TDSM_Get_Current_Broadcast_Information_Response_t *)ResponseMessage)->Status;
               *CurrentBroadcastInformation = ((TDSM_Get_Current_Broadcast_Information_Response_t *)ResponseMessage)->CurrentBroadcastInformation;
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
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is used to update the information being    */
   /* sent in the synchronization broadcasts.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.  The BroadcastInformationUpdate parameter is a pointer to a */
   /* structure which contains the information to update.  This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TDSM_Update_Broadcast_Information(unsigned int TDSMControlCallbackID, TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   TDSM_Update_Broadcast_Information_Request_t  UpdateBroadcastInformationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((TDSMControlCallbackID) && (BroadcastInformationUpdate))
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&UpdateBroadcastInformationRequest, 0, sizeof(UpdateBroadcastInformationRequest));

         UpdateBroadcastInformationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UpdateBroadcastInformationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UpdateBroadcastInformationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
         UpdateBroadcastInformationRequest.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_UPDATE_BROADCAST_INFORMATION;
         UpdateBroadcastInformationRequest.MessageHeader.MessageLength   = TDSM_UPDATE_BROADCAST_INFORMATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UpdateBroadcastInformationRequest.TDSMControlCallbackID         = TDSMControlCallbackID;
         UpdateBroadcastInformationRequest.BroadcastInformationUpdate    = *BroadcastInformationUpdate;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UpdateBroadcastInformationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_UPDATE_BROADCAST_INFORMATION_RESPONSE_SIZE)
               ret_val = ((TDSM_Update_Broadcast_Information_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the 3D Sync Profile  */
   /* Manager Service.  This Callback will be dispatched by the 3D Sync */
   /* Manager when various 3D Sync Manager events occur.  This function */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TDSM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
   /* * NOTE * Any registered TDSM Callback will get all TDSM events,   */
   /*          but only a callback registered with the Control parameter*/
   /*          set to TRUE can manage the Sync Train and Broadcast      */
   /*          information. There can only be one of these Control      */
   /*          callbacks registered.                                    */
int _TDSM_Register_Event_Callback(Boolean_t Control)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   TDSM_Register_Events_Request_t  RegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* All that we really need to do is to build a message and send it*/
      /* to the server.                                                 */
      BTPS_MemInitialize(&RegisterEventsRequest, 0, sizeof(RegisterEventsRequest));

      RegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
      RegisterEventsRequest.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_REGISTER_EVENTS;
      RegisterEventsRequest.MessageHeader.MessageLength   = TDSM_REGISTER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      RegisterEventsRequest.Control                       = Control;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_REGISTER_EVENTS_RESPONSE_SIZE)
            ret_val = ((TDSM_Register_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered 3D Sync Manager event Callback*/
   /* (registered via a successful call to the                          */
   /* TDSM_Register_Event_Callback() function.  This function accepts as*/
   /* input the 3D Sync Manager event callback ID (return value from the*/
   /* TDSM_Register_Event_Callback() function).                         */
int _TDSM_Un_Register_Event_Callback(unsigned int TDSManagerEventCallbackID)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   TDSM_Un_Register_Events_Request_t  UnRegisterEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(TDSManagerEventCallbackID)
      {
         /* All that we really need to do is to build a message and send*/
         /* it to the server.                                           */
         BTPS_MemInitialize(&UnRegisterEventsRequest, 0, sizeof(UnRegisterEventsRequest));

         UnRegisterEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER;
         UnRegisterEventsRequest.MessageHeader.MessageFunction = TDSM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS;
         UnRegisterEventsRequest.MessageHeader.MessageLength   = TDSM_UN_REGISTER_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterEventsRequest.TDSManagerEventCallbackID     = TDSManagerEventCallbackID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= TDSM_UN_REGISTER_EVENTS_RESPONSE_SIZE)
               ret_val = ((TDSM_Un_Register_Events_Response_t *)ResponseMessage)->Status;
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
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}
