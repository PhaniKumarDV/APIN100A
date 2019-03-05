/*****< pxpmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PXPMGR - PXP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMPXPM.h"            /* BTPM PXP Manager Prototypes/Constants.    */
#include "PXPMMSG.h"             /* BTPM PXP Manager Message Formats.         */
#include "PXPMGR.h"              /* PXP Manager Impl. Prototypes/Constants.   */

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
   /* initialize the PXP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PXP Manager  */
   /* Implementation.                                                   */
int _PXPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PXP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALREADY_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the PXP   */
   /* Manager Implementation.  After this function is called the PXP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PXPM_Initialize() function.  */
void _PXPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the PXP Manager      */
   /* Service.  This Callback will be dispatched by the PXP Manager when*/
   /* various PXP Manager Monitor Events occur.  This function returns a*/
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the PXP    */
   /*          Monitor Event Handler ID.  This value can be passed to   */
   /*          the _PXPM_Un_Register_Monitor_Events() function to       */
   /*          Un-Register the Monitor Event Handler.                   */
int _PXPM_Register_Monitor_Events(void)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   PXPM_Register_Monitor_Events_Request_t  RegisterMonitorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register Monitor   */
      /* Events message and send it to the server.                      */
      RegisterMonitorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterMonitorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterMonitorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      RegisterMonitorEventsRequest.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_REGISTER_MONITOR_EVENTS;
      RegisterMonitorEventsRequest.MessageHeader.MessageLength   = PXPM_REGISTER_MONITOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterMonitorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PXPM_REGISTER_MONITOR_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((PXPM_Register_Monitor_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((PXPM_Register_Monitor_Events_Response_t *)ResponseMessage)->MonitorEventHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PXP Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _PXPM_Register_Monitor_Events() function).  This function accepts */
   /* input the PXP Event Handler ID (return value from                 */
   /* _PXPM_Register_Monitor_Events() function).                        */
int _PXPM_Un_Register_Monitor_Events(unsigned int MonitorEventHandlerID)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   PXPM_Un_Register_Monitor_Events_Request_t  UnRegisterMonitorEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register       */
      /* Monitor Events message and send it to the server.              */
      UnRegisterMonitorEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterMonitorEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterMonitorEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      UnRegisterMonitorEventsRequest.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_UN_REGISTER_MONITOR_EVENTS;
      UnRegisterMonitorEventsRequest.MessageHeader.MessageLength   = PXPM_UN_REGISTER_MONITOR_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterMonitorEventsRequest.MonitorEventHandlerID         = MonitorEventHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterMonitorEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PXPM_UN_REGISTER_MONITOR_EVENTS_RESPONSE_SIZE)
            ret_val = ((PXPM_Un_Register_Monitor_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the refresh time for checking the Path Loss (the time    */
   /* between checking the path loss for a given link).  This function  */
   /* accepts as it's parameter the MonitorEventHandlerID that was      */
   /* returned from a successful call to _PXPM_Register_Monitor_Events()*/
   /* and the Refresh Time (in milliseconds).  This function returns    */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int _PXPM_Set_Path_Loss_Refresh_Time(unsigned int MonitorEventHandlerID, unsigned int RefreshTime)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   PXPM_Set_Path_Loss_Refresh_Time_Request_t  SetPathLossRefreshTimeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register       */
      /* Monitor Events message and send it to the server.              */
      SetPathLossRefreshTimeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetPathLossRefreshTimeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetPathLossRefreshTimeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      SetPathLossRefreshTimeRequest.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_REFRESH_TIME;
      SetPathLossRefreshTimeRequest.MessageHeader.MessageLength   = PXPM_SET_PATH_LOSS_REFRESH_TIME_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetPathLossRefreshTimeRequest.MonitorEventHandlerID         = MonitorEventHandlerID;
      SetPathLossRefreshTimeRequest.RefreshTime                   = RefreshTime;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetPathLossRefreshTimeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PXPM_SET_PATH_LOSS_REFRESH_TIME_RESPONSE_SIZE)
            ret_val = ((PXPM_Set_Path_Loss_Refresh_Time_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Threshold for a specified PXP Client       */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert.  This   */
   /* function accepts as it's parameter the MonitorEventHandlerID that */
   /* was returned from a successful call to                            */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Client        */
   /* Connection to set the path loss for, and the Path Loss Threshold  */
   /* to set.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The Path Loss Threshold should be specified in units of  */
   /*          dBm.                                                     */
int _PXPM_Set_Path_Loss_Threshold(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, int PathLossThreshold)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   PXPM_Set_Path_Loss_Threshold_Request_t  SetPathLossThresholdRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register       */
      /* Monitor Events message and send it to the server.              */
      SetPathLossThresholdRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetPathLossThresholdRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetPathLossThresholdRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      SetPathLossThresholdRequest.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_THRESHOLD;
      SetPathLossThresholdRequest.MessageHeader.MessageLength   = PXPM_SET_PATH_LOSS_THRESHOLD_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetPathLossThresholdRequest.MonitorEventHandlerID         = MonitorEventHandlerID;
      SetPathLossThresholdRequest.RemoteDevice                  = BD_ADDR;
      SetPathLossThresholdRequest.PathLossThreshold             = PathLossThreshold;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetPathLossThresholdRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PXPM_SET_PATH_LOSS_THRESHOLD_RESPONSE_SIZE)
            ret_val = ((PXPM_Set_Path_Loss_Threshold_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the current Path Loss for a specified PXP Monitor        */
   /* Connection.  This function accepts as it's parameter the          */
   /* MonitorEventHandlerID that was returned from a successful call to */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Monitor       */
   /* Connection to set the path loss for, and a pointer to a buffer to */
   /* return the current Path Loss in (if successfull).  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * The Path Loss Threshold will be specified in units of    */
   /*          dBm.                                                     */
int _PXPM_Query_Current_Path_Loss(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, int *PathLossThreshold)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   PXPM_Query_Current_Path_Loss_Request_t  QueryPathLossRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear semi-valid.            */
      if(PathLossThreshold)
      {
         /* All that we really need to do is to build an Un-Register    */
         /* Monitor Events message and send it to the server.           */
         QueryPathLossRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryPathLossRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryPathLossRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
         QueryPathLossRequest.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_QUERY_CURRENT_PATH_LOSS;
         QueryPathLossRequest.MessageHeader.MessageLength   = PXPM_QUERY_CURRENT_PATH_LOSS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryPathLossRequest.MonitorEventHandlerID         = MonitorEventHandlerID;
         QueryPathLossRequest.RemoteDevice                  = BD_ADDR;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryPathLossRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PXPM_QUERY_CURRENT_PATH_LOSS_RESPONSE_SIZE)
            {
               /* Determine if the request was succesfull.              */
               if(!(ret_val = ((PXPM_Query_Current_Path_Loss_Response_t *)ResponseMessage)->Status))
                  *PathLossThreshold = ((PXPM_Query_Current_Path_Loss_Response_t *)ResponseMessage)->CurrentPathLoss;
               else
                  *PathLossThreshold = 0;
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
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Alert Level for a specified PXP Client     */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert at the   */
   /* specified level.  This function accepts as it's parameter the     */
   /* MonitorEventHandlerID that was returned from a successful call to */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Client        */
   /* Connection to set the path loss alert level for, and the Path Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int _PXPM_Set_Path_Loss_Alert_Level(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   PXPM_Set_Path_Loss_Alert_Level_Request_t  SetPathLossAlertLevelRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register       */
      /* Monitor Events message and send it to the server.              */
      SetPathLossAlertLevelRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetPathLossAlertLevelRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetPathLossAlertLevelRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      SetPathLossAlertLevelRequest.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_ALERT_LEVEL;
      SetPathLossAlertLevelRequest.MessageHeader.MessageLength   = PXPM_SET_PATH_LOSS_ALERT_LEVEL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetPathLossAlertLevelRequest.MonitorEventHandlerID         = MonitorEventHandlerID;
      SetPathLossAlertLevelRequest.RemoteDevice                  = BD_ADDR;
      SetPathLossAlertLevelRequest.AlertLevel                    = AlertLevel;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetPathLossAlertLevelRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PXPM_SET_PATH_LOSS_ALERT_LEVEL_RESPONSE_SIZE)
            ret_val = ((PXPM_Set_Path_Loss_Alert_Level_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Link Loss Alert Level for a specified PXP Client     */
   /* Connection.  If the Link Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert at the   */
   /* specified level.  This function accepts as it's parameter the     */
   /* MonitorEventHandlerID that was returned from a successful call to */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Client        */
   /* Connection to set the path loss alert level for, and the Link Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int _PXPM_Set_Link_Loss_Alert_Level(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   PXPM_Set_Link_Loss_Alert_Level_Request_t  SetLinkLossAlertLevelRequest;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register       */
      /* Monitor Events message and send it to the server.              */
      SetLinkLossAlertLevelRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetLinkLossAlertLevelRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetLinkLossAlertLevelRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      SetLinkLossAlertLevelRequest.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_SET_LINK_LOSS_ALERT_LEVEL;
      SetLinkLossAlertLevelRequest.MessageHeader.MessageLength   = PXPM_SET_LINK_LOSS_ALERT_LEVEL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetLinkLossAlertLevelRequest.MonitorEventHandlerID         = MonitorEventHandlerID;
      SetLinkLossAlertLevelRequest.RemoteDevice                  = BD_ADDR;
      SetLinkLossAlertLevelRequest.AlertLevel                    = AlertLevel;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetLinkLossAlertLevelRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PXPM_SET_LINK_LOSS_ALERT_LEVEL_RESPONSE_SIZE)
            ret_val = ((PXPM_Set_Link_Loss_Alert_Level_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

