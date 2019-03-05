/*****< antmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANTMGR - ANT Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/02/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMANTM.h"            /* BTPM ANT Manager Prototypes/Constants.    */
#include "ANTMMSG.h"             /* BTPM ANT Manager Message Formats.         */
#include "ANTMGR.h"              /* ANT Manager Impl. Prototypes/Constants.   */

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
   /* initialize the ANT Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager ANT Manager  */
   /* Implementation.                                                   */
int _ANTM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANT Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the ANT   */
   /* Manager Implementation.  After this function is called the ANT    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _ANTM_Initialize() function.  */
void _ANTM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for registering an ANTM     */
   /* client to receive ANT events from the PM server. This function    */
   /* returns a positive, non-zero value representing the Callback ID if*/
   /* successful and a negative error code if there was an error.       */
int _ANTM_Register_ANT_Events(void)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   ANTM_Register_ANT_Events_Request_t  RegisterANTEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&RegisterANTEventsRequest, 0, ANTM_REGISTER_ANT_EVENTS_REQUEST_SIZE);

      RegisterANTEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterANTEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterANTEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      RegisterANTEventsRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_REGISTER_ANT_EVENTS;
      RegisterANTEventsRequest.MessageHeader.MessageLength   = ANTM_REGISTER_ANT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterANTEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_REGISTER_ANT_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((ANTM_Register_ANT_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((ANTM_Register_ANT_Events_Response_t *)ResponseMessage)->EventHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for un-registering an ANTM  */
   /* client from receiving ANT events from the PM server. This function*/
   /* takes a Callback ID (returned from _ANTM_Register_ANT_Events) as a*/
   /* parameter. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
int _ANTM_Un_Register_ANT_Events(unsigned int CallbackID)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   ANTM_Un_Register_ANT_Events_Request_t  UnRegisterANTEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&UnRegisterANTEventsRequest, 0, ANTM_UN_REGISTER_ANT_EVENTS_REQUEST_SIZE);

      UnRegisterANTEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterANTEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterANTEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      UnRegisterANTEventsRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_UN_REGISTER_ANT_EVENTS;
      UnRegisterANTEventsRequest.MessageHeader.MessageLength   = ANTM_UN_REGISTER_ANT_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterANTEventsRequest.EventHandlerID                = CallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterANTEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_UN_REGISTER_ANT_EVENTS_RESPONSE_SIZE)
            ret_val = ((ANTM_Un_Register_ANT_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for assigning an ANT channel*/
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to register.  This function   */
   /* accepts as it's third argument, the channel type to be assigned to*/
   /* the channel.  This function accepts as it's fourth argument, the  */
   /* network number to be used for the channel.  Zero should be        */
   /* specified for this argument to use the default public network.    */
   /* This function accepts as it's fifth argument, the extended        */
   /* assignment to be used for the channel.  Zero should be specified  */
   /* for this argument if no extended capabilities are to be used.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ChannelType, unsigned int NetworkNumber, unsigned int ExtendedAssignment)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   ANTM_Assign_Channel_Request_t  AssignChannelRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&AssignChannelRequest, 0, ANTM_ASSIGN_CHANNEL_REQUEST_SIZE);

      AssignChannelRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      AssignChannelRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      AssignChannelRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      AssignChannelRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_ASSIGN_CHANNEL;
      AssignChannelRequest.MessageHeader.MessageLength   = ANTM_ASSIGN_CHANNEL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      AssignChannelRequest.EventHandlerID                = ANTManagerCallbackID;
      AssignChannelRequest.ChannelNumber                 = ChannelNumber;
      AssignChannelRequest.ChannelType                   = ChannelType;
      AssignChannelRequest.NetworkNumber                 = NetworkNumber;
      AssignChannelRequest.ExtendedAssignment            = ExtendedAssignment;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&AssignChannelRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_ASSIGN_CHANNEL_RESPONSE_SIZE)
            ret_val = ((ANTM_Assign_Channel_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for un-assigning an ANT     */
   /* channel on the local ANT+ system.  A channel must be unassigned   */
   /* before it can be reassigned using the ANTM_Assign_Channel() API.  */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to un-assign.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
int _ANTM_Un_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   ANTM_Un_Assign_Channel_Request_t  UnAssignChannelRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&UnAssignChannelRequest, 0, ANTM_UN_ASSIGN_CHANNEL_REQUEST_SIZE);

      UnAssignChannelRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnAssignChannelRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnAssignChannelRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      UnAssignChannelRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_UN_ASSIGN_CHANNEL;
      UnAssignChannelRequest.MessageHeader.MessageLength   = ANTM_UN_ASSIGN_CHANNEL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnAssignChannelRequest.EventHandlerID                = ANTManagerCallbackID;
      UnAssignChannelRequest.ChannelNumber                 = ChannelNumber;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnAssignChannelRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_UN_ASSIGN_CHANNEL_RESPONSE_SIZE)
            ret_val = ((ANTM_Un_Assign_Channel_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, the channel number to configure.  The ANT   */
   /* channel must be assigned using ANTM_Assign_Channel() before       */
   /* calling this function.  This function accepts as it's third       */
   /* argument, the device number to search for on the channel.  Zero   */
   /* should be specified for this argument to scan for any device      */
   /* number.  This function accepts as it's fourth argument, the device*/
   /* type to search for on the channel.  Zero should be specified for  */
   /* this argument to scan for any device type.  This function accepts */
   /* as it's fifth argument, the transmission type to search for on the*/
   /* channel.  Zero should be specified for this argument to scan for  */
   /* any transmission type.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
int _ANTM_Set_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   ANTM_Set_Channel_ID_Request_t  SetChannelIDRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetChannelIDRequest, 0, ANTM_SET_CHANNEL_ID_REQUEST_SIZE);

      SetChannelIDRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetChannelIDRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetChannelIDRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetChannelIDRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_CHANNEL_ID;
      SetChannelIDRequest.MessageHeader.MessageLength   = ANTM_SET_CHANNEL_ID_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetChannelIDRequest.EventHandlerID                = ANTManagerCallbackID;
      SetChannelIDRequest.ChannelNumber                 = ChannelNumber;
      SetChannelIDRequest.DeviceNumber                  = DeviceNumber;
      SetChannelIDRequest.DeviceType                    = DeviceType;
      SetChannelIDRequest.TransmissionType              = TransmissionType;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetChannelIDRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_ID_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Channel_ID_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the local ANT+ system.     */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel messaging period to   */
   /* set on the channel.  This function returns zero if successful,    */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
int _ANTM_Set_Channel_Period(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessagingPeriod)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   ANTM_Set_Channel_Period_Request_t  SetChannelPeriodRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetChannelPeriodRequest, 0, ANTM_SET_CHANNEL_PERIOD_REQUEST_SIZE);

      SetChannelPeriodRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetChannelPeriodRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetChannelPeriodRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetChannelPeriodRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_CHANNEL_PERIOD;
      SetChannelPeriodRequest.MessageHeader.MessageLength   = ANTM_SET_CHANNEL_PERIOD_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetChannelPeriodRequest.EventHandlerID                = ANTManagerCallbackID;
      SetChannelPeriodRequest.ChannelNumber                 = ChannelNumber;
      SetChannelPeriodRequest.MessagingPeriod               = MessagingPeriod;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetChannelPeriodRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_PERIOD_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Channel_Period_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument the     */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the search timeout to set on the  */
   /* channel.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will result in an infinite search timeout.  Specifying   */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
int _ANTM_Set_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   ANTM_Set_Channel_Search_Timeout_Request_t  SetChannelSearchTimeoutRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetChannelSearchTimeoutRequest, 0, ANTM_SET_CHANNEL_SEARCH_TIMEOUT_REQUEST_SIZE);

      SetChannelSearchTimeoutRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetChannelSearchTimeoutRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetChannelSearchTimeoutRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetChannelSearchTimeoutRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_TIMEOUT;
      SetChannelSearchTimeoutRequest.MessageHeader.MessageLength   = ANTM_SET_CHANNEL_SEARCH_TIMEOUT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetChannelSearchTimeoutRequest.EventHandlerID                = ANTManagerCallbackID;
      SetChannelSearchTimeoutRequest.ChannelNumber                 = ChannelNumber;
      SetChannelSearchTimeoutRequest.SearchTimeout                 = SearchTimeout;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetChannelSearchTimeoutRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_SEARCH_TIMEOUT_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Channel_Search_Timeout_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel.  This function accepts as it's first*/
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel frequency to set on   */
   /* the channel.  This function returns zero if successful, otherwise */
   /* this function returns a negative error code.                      */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (unsigned int ANTManagerCallbackID, 2400 +       */
   /*          RFFrequency) MHz.                                        */
int _ANTM_Set_Channel_RF_Frequency(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int RFFrequency)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   ANTM_Set_Channel_RF_Frequency_Request_t  SetChannelRFFrequencyRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetChannelRFFrequencyRequest, 0, ANTM_SET_CHANNEL_RF_FREQUENCY_REQUEST_SIZE);

      SetChannelRFFrequencyRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetChannelRFFrequencyRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetChannelRFFrequencyRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetChannelRFFrequencyRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_CHANNEL_RF_FREQUENCY;
      SetChannelRFFrequencyRequest.MessageHeader.MessageLength   = ANTM_SET_CHANNEL_RF_FREQUENCY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetChannelRFFrequencyRequest.EventHandlerID                = ANTManagerCallbackID;
      SetChannelRFFrequencyRequest.ChannelNumber                 = ChannelNumber;
      SetChannelRFFrequencyRequest.RFFrequency                   = RFFrequency;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetChannelRFFrequencyRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_RF_FREQUENCY_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Channel_RF_Frequency_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the network */
   /* key for an ANT channel.  This function accepts as it's first      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, a pointer to the ANT network key  */
   /* to set on the channel.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
int _ANTM_Set_Network_Key(unsigned int ANTManagerCallbackID, unsigned int NetworkNumber, ANT_Network_Key_t NetworkKey)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   ANTM_Set_Network_Key_Request_t  SetNetworkKeyRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetNetworkKeyRequest, 0, ANTM_SET_NETWORK_KEY_REQUEST_SIZE);

      SetNetworkKeyRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetNetworkKeyRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetNetworkKeyRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetNetworkKeyRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_NETWORK_KEY;
      SetNetworkKeyRequest.MessageHeader.MessageLength   = ANTM_SET_NETWORK_KEY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetNetworkKeyRequest.EventHandlerID                = ANTManagerCallbackID;
      SetNetworkKeyRequest.NetworkNumber                 = NetworkNumber;
      SetNetworkKeyRequest.NetworkKey                    = NetworkKey;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetNetworkKeyRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_NETWORK_KEY_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Network_Key_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the transmit*/
   /* power on the local ANT system.  This function accepts as it's     */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument the transmit power to set on the device.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Set_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int TransmitPower)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   ANTM_Set_Transmit_Power_Request_t  SetTransmitPowerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetTransmitPowerRequest, 0, ANTM_SET_TRANSMIT_POWER_REQUEST_SIZE);

      SetTransmitPowerRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetTransmitPowerRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetTransmitPowerRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetTransmitPowerRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_TRANSMIT_POWER;
      SetTransmitPowerRequest.MessageHeader.MessageLength   = ANTM_SET_TRANSMIT_POWER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetTransmitPowerRequest.EventHandlerID                = ANTManagerCallbackID;
      SetTransmitPowerRequest.TransmitPower                 = TransmitPower;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetTransmitPowerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_TRANSMIT_POWER_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Transmit_Power_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to add to the */
   /* list.  This function accepts as it's third argument, the device   */
   /* number to add to the list.  This function accepts as it's fourth  */
   /* argument, the device type to add to the list.  This function      */
   /* accepts as it's fifth argument, the transmission type to add to   */
   /* the list.  This function accepts as it's sixth argument, the the  */
   /* list index to overwrite with the updated entry.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Add_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType, unsigned int ListIndex)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   ANTM_Add_Channel_ID_Request_t  AddChannelIDRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&AddChannelIDRequest, 0, ANTM_ADD_CHANNEL_ID_REQUEST_SIZE);

      AddChannelIDRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      AddChannelIDRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      AddChannelIDRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      AddChannelIDRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_ADD_CHANNEL_ID;
      AddChannelIDRequest.MessageHeader.MessageLength   = ANTM_ADD_CHANNEL_ID_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      AddChannelIDRequest.EventHandlerID                = ANTManagerCallbackID;
      AddChannelIDRequest.ChannelNumber                 = ChannelNumber;
      AddChannelIDRequest.DeviceNumber                  = DeviceNumber;
      AddChannelIDRequest.DeviceType                    = DeviceType;
      AddChannelIDRequest.TransmissionType              = TransmissionType;
      AddChannelIDRequest.ListIndex                     = ListIndex;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&AddChannelIDRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_ADD_CHANNEL_ID_RESPONSE_SIZE)
            ret_val = ((ANTM_Add_Channel_ID_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* on which the list should be configured.  This function accepts as */
   /* it's third argument, the size of the list.  This function accepts */
   /* as it's fourth argument, the list type.  Zero should be specified */
   /* to configure the list for inclusion, and one should be specified  */
   /* to configure the list for exclusion.  This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Configure_Inclusion_Exclusion_List(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ListSize, unsigned int Exclude)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   ANTM_Configure_Incl_Excl_List_Request_t  ConfigureInclusionExclusionListRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ConfigureInclusionExclusionListRequest, 0, ANTM_CONFIGURE_INCL_EXCL_LIST_REQUEST_SIZE);

      ConfigureInclusionExclusionListRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConfigureInclusionExclusionListRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConfigureInclusionExclusionListRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      ConfigureInclusionExclusionListRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_CONFIGURE_INCL_EXCL_LIST;
      ConfigureInclusionExclusionListRequest.MessageHeader.MessageLength   = ANTM_CONFIGURE_INCL_EXCL_LIST_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConfigureInclusionExclusionListRequest.EventHandlerID                = ANTManagerCallbackID;
      ConfigureInclusionExclusionListRequest.ChannelNumber                 = ChannelNumber;
      ConfigureInclusionExclusionListRequest.ListSize                      = ListSize;
      ConfigureInclusionExclusionListRequest.Exclude                       = Exclude;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConfigureInclusionExclusionListRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_CONFIGURE_INCL_EXCL_LIST_RESPONSE_SIZE)
            ret_val = ((ANTM_Configure_Incl_Excl_List_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the transmit power level for the  */
   /* specified channel.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_Channel_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int TransmitPower)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   ANTM_Set_Channel_Transmit_Power_Request_t  SetChannelTransmitPowerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetChannelTransmitPowerRequest, 0, ANTM_SET_CHANNEL_TRANSMIT_POWER_REQUEST_SIZE);

      SetChannelTransmitPowerRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetChannelTransmitPowerRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetChannelTransmitPowerRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetChannelTransmitPowerRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_CHANNEL_TRANSMIT_POWER;
      SetChannelTransmitPowerRequest.MessageHeader.MessageLength   = ANTM_SET_CHANNEL_TRANSMIT_POWER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetChannelTransmitPowerRequest.EventHandlerID                = ANTManagerCallbackID;
      SetChannelTransmitPowerRequest.ChannelNumber                 = ChannelNumber;
      SetChannelTransmitPowerRequest.TransmitPower                 = TransmitPower;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetChannelTransmitPowerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_TRANSMIT_POWER_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Channel_Transmit_Power_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to   */
   /* configure.  This function accepts as it's third argument, the     */
   /* search timeout to set on the channel.  This function returns zero */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will result in an   */
   /*          infinite low priority search timeout.                    */
int _ANTM_Set_Low_Priority_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int                                                          ret_val;
   BTPM_Message_t                                              *ResponseMessage;
   ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Request_t  SetLowPriorityChannelSearchTimeoutRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetLowPriorityChannelSearchTimeoutRequest, 0, ANTM_SET_LOW_PRIORITY_CHANNEL_SCAN_SEARCH_TIMEOUT_REQUEST_SIZE);

      SetLowPriorityChannelSearchTimeoutRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetLowPriorityChannelSearchTimeoutRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetLowPriorityChannelSearchTimeoutRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetLowPriorityChannelSearchTimeoutRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_LOW_PRIORITY_CHANNEL_SEARCH_TIMEOUT;
      SetLowPriorityChannelSearchTimeoutRequest.MessageHeader.MessageLength   = ANTM_SET_LOW_PRIORITY_CHANNEL_SCAN_SEARCH_TIMEOUT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetLowPriorityChannelSearchTimeoutRequest.EventHandlerID                = ANTManagerCallbackID;
      SetLowPriorityChannelSearchTimeoutRequest.ChannelNumber                 = ChannelNumber;
      SetLowPriorityChannelSearchTimeoutRequest.SearchTimeout                 = SearchTimeout;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetLowPriorityChannelSearchTimeoutRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_LOW_PRIORITY_CHANNEL_SCAN_SEARCH_TIMEOUT_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function configures the   */
   /* channel ID in the same way as ANTM_Set_Channel_ID(), except it    */
   /* uses the two LSB of the device's serial number as the device's    */
   /* number.  This function accepts as it's first argument the Callback*/
   /* ID that was returned from a successful call                       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  The ANT channel*/
   /* must be assigned using ANTM_Assign_Channel() before calling this  */
   /* function.  This function accepts as it's third argument, the      */
   /* device type to search for on the channel.  Zero should be         */
   /* specified for this argument to scan for any device type.  This    */
   /* function accepts as it's fourth argument, the transmission type to*/
   /* search for on the channel.  Zero should be specified for this     */
   /* argument to scan for any transmission type.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Set_Serial_Number_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   ANTM_Set_Serial_Number_Channel_ID_Request_t  SetSerialNumberChannelIDRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetSerialNumberChannelIDRequest, 0, ANTM_SET_SERIAL_NUMBER_CHANNEL_ID_REQUEST_SIZE);

      SetSerialNumberChannelIDRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetSerialNumberChannelIDRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetSerialNumberChannelIDRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetSerialNumberChannelIDRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_SERIAL_NUMBER_CHANNEL_ID;
      SetSerialNumberChannelIDRequest.MessageHeader.MessageLength   = ANTM_SET_SERIAL_NUMBER_CHANNEL_ID_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetSerialNumberChannelIDRequest.EventHandlerID                = ANTManagerCallbackID;
      SetSerialNumberChannelIDRequest.ChannelNumber                 = ChannelNumber;
      SetSerialNumberChannelIDRequest.DeviceType                    = DeviceType;
      SetSerialNumberChannelIDRequest.TransmissionType              = TransmissionType;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetSerialNumberChannelIDRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_SERIAL_NUMBER_CHANNEL_ID_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Serial_Number_Channel_ID_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument whether or not to enable extended Rx messages.    */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Enable_Extended_Messages(unsigned int ANTManagerCallbackID, Boolean_t Enable)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   ANTM_Enable_Extended_Messages_Request_t  EnableExtendedMessagesRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&EnableExtendedMessagesRequest, 0, ANTM_ENABLE_EXTENDED_MESSAGES_REQUEST_SIZE);

      EnableExtendedMessagesRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      EnableExtendedMessagesRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      EnableExtendedMessagesRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      EnableExtendedMessagesRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_ENABLE_EXTENDED_MESSAGES;
      EnableExtendedMessagesRequest.MessageHeader.MessageLength   = ANTM_ENABLE_EXTENDED_MESSAGES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      EnableExtendedMessagesRequest.EventHandlerID                = ANTManagerCallbackID;
      EnableExtendedMessagesRequest.Enable                        = Enable;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableExtendedMessagesRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_ENABLE_EXTENDED_MESSAGES_RESPONSE_SIZE)
            ret_val = ((ANTM_Enable_Extended_Messages_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for enabling or disabling   */
   /* the LED on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, whether or not to enable the LED.  This     */
   /* function returns zero if successful, otherwise this function      */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Enable_LED(unsigned int ANTManagerCallbackID, Boolean_t Enable)
{
   int                        ret_val;
   BTPM_Message_t            *ResponseMessage;
   ANTM_Enable_LED_Request_t  EnableLEDRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&EnableLEDRequest, 0, ANTM_ENABLE_LED_REQUEST_SIZE);

      EnableLEDRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      EnableLEDRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      EnableLEDRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      EnableLEDRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_ENABLE_LED;
      EnableLEDRequest.MessageHeader.MessageLength   = ANTM_ENABLE_LED_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      EnableLEDRequest.EventHandlerID                = ANTManagerCallbackID;
      EnableLEDRequest.Enable                        = Enable;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableLEDRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_ENABLE_LED_RESPONSE_SIZE)
            ret_val = ((ANTM_Enable_LED_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the local ANT+ system.  This function accepts as */
   /* it's only argument the Callback ID that was returned from a       */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
int _ANTM_Enable_Crystal(unsigned int ANTManagerCallbackID)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   ANTM_Enable_Crystal_Request_t  EnableCrystalRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&EnableCrystalRequest, 0, ANTM_ENABLE_CRYSTAL_REQUEST_SIZE);

      EnableCrystalRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      EnableCrystalRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      EnableCrystalRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      EnableCrystalRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_ENABLE_CRYSTAL;
      EnableCrystalRequest.MessageHeader.MessageLength   = ANTM_ENABLE_CRYSTAL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      EnableCrystalRequest.EventHandlerID                = ANTManagerCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&EnableCrystalRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_ENABLE_CRYSTAL_RESPONSE_SIZE)
            ret_val = ((ANTM_Enable_Crystal_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the bitmask of extended */
   /* Rx messages that shall be enabled or disabled.  This function     */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Configure_Extended_Messages(unsigned int ANTManagerCallbackID, unsigned int EnabledExtendedMessagesMask)
{
   int                                             ret_val;
   BTPM_Message_t                                 *ResponseMessage;
   ANTM_Extended_Messages_Configuration_Request_t  ExtendedMessagesConfigurationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ExtendedMessagesConfigurationRequest, 0, ANTM_EXTENDED_MESSAGES_CONFIGURATION_REQUEST_SIZE);

      ExtendedMessagesConfigurationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ExtendedMessagesConfigurationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ExtendedMessagesConfigurationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      ExtendedMessagesConfigurationRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_EXTENDED_MESSAGES_CONFIGURATION;
      ExtendedMessagesConfigurationRequest.MessageHeader.MessageLength   = ANTM_EXTENDED_MESSAGES_CONFIGURATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ExtendedMessagesConfigurationRequest.EventHandlerID                = ANTManagerCallbackID;
      ExtendedMessagesConfigurationRequest.RxMessagesMask                = EnabledExtendedMessagesMask;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ExtendedMessagesConfigurationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_EXTENDED_MESSAGES_CONFIGURATION_RESPONSE_SIZE)
            ret_val = ((ANTM_Extended_Messages_Configuration_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel.  This function accepts  */
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to configure. */
   /* This function accepts as it's third, fourth, and fifth arguments, */
   /* the three operating agility frequencies to set.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANTM_Assign_Channel.  Frequency agility      */
   /*          should NOT be used with shared, Tx only, or Rx only      */
   /*          channels.                                                */
int _ANTM_Configure_Frequency_Agility(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int FrequencyAgility1, unsigned int FrequencyAgility2, unsigned int FrequencyAgility3)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   ANTM_Configure_Frequency_Agility_Request_t  ConfigureFrequencyAgilityRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ConfigureFrequencyAgilityRequest, 0, ANTM_CONFIGURE_FREQUENCY_AGILITY_REQUEST_SIZE);

      ConfigureFrequencyAgilityRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConfigureFrequencyAgilityRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConfigureFrequencyAgilityRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      ConfigureFrequencyAgilityRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_CONFIGURE_FREQUENCY_AGILITY;
      ConfigureFrequencyAgilityRequest.MessageHeader.MessageLength   = ANTM_CONFIGURE_FREQUENCY_AGILITY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConfigureFrequencyAgilityRequest.EventHandlerID                = ANTManagerCallbackID;
      ConfigureFrequencyAgilityRequest.ChannelNumber                 = ChannelNumber;
      ConfigureFrequencyAgilityRequest.FrequencyAgility1             = FrequencyAgility1;
      ConfigureFrequencyAgilityRequest.FrequencyAgility2             = FrequencyAgility2;
      ConfigureFrequencyAgilityRequest.FrequencyAgility3             = FrequencyAgility3;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConfigureFrequencyAgilityRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_CONFIGURE_FREQUENCY_AGILITY_RESPONSE_SIZE)
            ret_val = ((ANTM_Configure_Frequency_Agility_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the local ANT+ system.  This      */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search threshold to set.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
int _ANTM_Set_Proximity_Search(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchThreshold)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   ANTM_Set_Proximity_Search_Request_t  SetProximitySearchRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetProximitySearchRequest, 0, ANTM_SET_PROXIMITY_SEARCH_REQUEST_SIZE);

      SetProximitySearchRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetProximitySearchRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetProximitySearchRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetProximitySearchRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_PROXIMITY_SEARCH;
      SetProximitySearchRequest.MessageHeader.MessageLength   = ANTM_SET_PROXIMITY_SEARCH_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetProximitySearchRequest.EventHandlerID                = ANTManagerCallbackID;
      SetProximitySearchRequest.ChannelNumber                 = ChannelNumber;
      SetProximitySearchRequest.SearchThreshold               = SearchThreshold;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetProximitySearchRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_PROXIMITY_SEARCH_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Proximity_Search_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search priority to set.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_Channel_Search_Priority(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchPriority)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   ANTM_Set_Channel_Search_Priority_Request_t  SetChannelSearchPriorityRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetChannelSearchPriorityRequest, 0, ANTM_SET_CHANNEL_SEARCH_PRIORITY_REQUEST_SIZE);

      SetChannelSearchPriorityRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetChannelSearchPriorityRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetChannelSearchPriorityRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetChannelSearchPriorityRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_PRIORITY;
      SetChannelSearchPriorityRequest.MessageHeader.MessageLength   = ANTM_SET_CHANNEL_SEARCH_PRIORITY_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetChannelSearchPriorityRequest.EventHandlerID                = ANTManagerCallbackID;
      SetChannelSearchPriorityRequest.ChannelNumber                 = ChannelNumber;
      SetChannelSearchPriorityRequest.SearchPriority                = SearchPriority;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetChannelSearchPriorityRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_CHANNEL_SEARCH_PRIORITY_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_Channel_Search_Priority_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the local ANT+ system.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the descriptor string type to    */
   /* set.  This function accepts as it's third argument, the           */
   /* NULL-terminated descriptor string to be set.  This function       */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_USB_Descriptor_String(unsigned int ANTManagerCallbackID, unsigned int StringNumber, char *DescriptorString)
{
   int                                       ret_val;
   unsigned int                              StringLength;
   BTPM_Message_t                           *ResponseMessage;
   ANTM_Set_USB_Descriptor_String_Request_t *SetUSBDescriptorStringRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if((Initialized) && (DescriptorString))
   {
      /* First Calculate the string length.                             */
      StringLength = BTPS_StringLength(DescriptorString);

      /* Now allocate the message.                                      */
      if((SetUSBDescriptorStringRequest = (ANTM_Set_USB_Descriptor_String_Request_t *)BTPS_AllocateMemory(ANTM_SET_USB_DESCRIPTOR_STRING_REQUEST_SIZE(StringLength))) != NULL)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(SetUSBDescriptorStringRequest, 0, ANTM_SET_USB_DESCRIPTOR_STRING_REQUEST_SIZE(StringLength));

         SetUSBDescriptorStringRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetUSBDescriptorStringRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetUSBDescriptorStringRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         SetUSBDescriptorStringRequest->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_USB_DESCRIPTOR_STRING;
         SetUSBDescriptorStringRequest->MessageHeader.MessageLength   = ANTM_SET_USB_DESCRIPTOR_STRING_REQUEST_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE;

         SetUSBDescriptorStringRequest->EventHandlerID                = ANTManagerCallbackID;
         SetUSBDescriptorStringRequest->StringNumber                  = StringNumber;
         SetUSBDescriptorStringRequest->DescriptorStringLength        = StringLength;

         BTPS_MemCopy(SetUSBDescriptorStringRequest->DescriptorString, DescriptorString, StringLength);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SetUSBDescriptorStringRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_USB_DESCRIPTOR_STRING_RESPONSE_SIZE)
               ret_val = ((ANTM_Set_USB_Descriptor_String_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }

         BTPS_FreeMemory(SetUSBDescriptorStringRequest);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* Control Message API.                                              */

   /* The following function is responsible for resetting the ANT module*/
   /* on the local ANT+ system.  This function accepts as it's only     */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  A delay of at least 500ms is     */
   /* suggested after calling this function to allow time for the module*/
   /* to reset.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
int _ANTM_Reset_System(unsigned int ANTManagerCallbackID)
{
   int                          ret_val;
   BTPM_Message_t              *ResponseMessage;
   ANTM_Reset_System_Request_t  ResetSystemRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&ResetSystemRequest, 0, ANTM_RESET_SYSTEM_REQUEST_SIZE);

      ResetSystemRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ResetSystemRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ResetSystemRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      ResetSystemRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_RESET_SYSTEM;
      ResetSystemRequest.MessageHeader.MessageLength   = ANTM_RESET_SYSTEM_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResetSystemRequest.EventHandlerID                = ANTManagerCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ResetSystemRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_RESET_SYSTEM_RESPONSE_SIZE)
            ret_val = ((ANTM_Reset_System_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for opening an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  The channel    */
   /* specified must have been assigned and configured before calling   */
   /* this function.  This function returns zero if successful,         */
   /* otherwise this function returns a negative error code.            */
int _ANTM_Open_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                          ret_val;
   BTPM_Message_t              *ResponseMessage;
   ANTM_Open_Channel_Request_t  OpenChannelRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&OpenChannelRequest, 0, ANTM_OPEN_CHANNEL_REQUEST_SIZE);

      OpenChannelRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      OpenChannelRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      OpenChannelRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      OpenChannelRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_OPEN_CHANNEL;
      OpenChannelRequest.MessageHeader.MessageLength   = ANTM_OPEN_CHANNEL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      OpenChannelRequest.EventHandlerID                = ANTManagerCallbackID;
      OpenChannelRequest.ChannelNumber                 = ChannelNumber;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&OpenChannelRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_OPEN_CHANNEL_RESPONSE_SIZE)
            ret_val = ((ANTM_Open_Channel_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for closing an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the aetANTMChannelResponse event has been received */
   /*          with the Message_Code member specifying:                 */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
int _ANTM_Close_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                           ret_val;
   BTPM_Message_t               *ResponseMessage;
   ANTM_Close_Channel_Request_t  CloseChannelRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&CloseChannelRequest, 0, ANTM_CLOSE_CHANNEL_REQUEST_SIZE);

      CloseChannelRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      CloseChannelRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      CloseChannelRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      CloseChannelRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_CLOSE_CHANNEL;
      CloseChannelRequest.MessageHeader.MessageLength   = ANTM_CLOSE_CHANNEL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      CloseChannelRequest.EventHandlerID                = ANTManagerCallbackID;
      CloseChannelRequest.ChannelNumber                 = ChannelNumber;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&CloseChannelRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_CLOSE_CHANNEL_RESPONSE_SIZE)
            ret_val = ((ANTM_Close_Channel_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number that the request will be sent */
   /* to.  This function accepts as it's third argument, the message ID */
   /* being requested from the channel.  This function returns zero if  */
   /* successful, otherwise this function returns a negative error code.*/
int _ANTM_Request_Message(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessageID)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   ANTM_Request_Channel_Message_Request_t  RequestMessageRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&RequestMessageRequest, 0, ANTM_REQUEST_CHANNEL_MESSAGE_REQUEST_SIZE);

      RequestMessageRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RequestMessageRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RequestMessageRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      RequestMessageRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_REQUEST_CHANNEL_MESSAGE;
      RequestMessageRequest.MessageHeader.MessageLength   = ANTM_REQUEST_CHANNEL_MESSAGE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      RequestMessageRequest.EventHandlerID                = ANTManagerCallbackID;
      RequestMessageRequest.ChannelNumber                 = ChannelNumber;
      RequestMessageRequest.MessageID                     = MessageID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RequestMessageRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_REQUEST_CHANNEL_MESSAGE_RESPONSE_SIZE)
            ret_val = ((ANTM_Request_Channel_Message_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the local ANT+ system.  This function  */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to be*/
   /* opened.  The channel specified must have been assigned and        */
   /* configured as a SLAVE Rx ONLY channel before calling this         */
   /* function.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
int _ANTM_Open_Rx_Scan_Mode(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   ANTM_Open_Rx_Scan_Mode_Request_t  OpenRxScanModeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&OpenRxScanModeRequest, 0, ANTM_OPEN_RX_SCAN_MODE_REQUEST_SIZE);

      OpenRxScanModeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      OpenRxScanModeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      OpenRxScanModeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      OpenRxScanModeRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_OPEN_RX_SCAN_MODE;
      OpenRxScanModeRequest.MessageHeader.MessageLength   = ANTM_OPEN_RX_SCAN_MODE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      OpenRxScanModeRequest.EventHandlerID                = ANTManagerCallbackID;
      OpenRxScanModeRequest.ChannelNumber                 = ChannelNumber;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&OpenRxScanModeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_OPEN_RX_SCAN_MODE_RESPONSE_SIZE)
            ret_val = ((ANTM_Open_Rx_Scan_Mode_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for putting the ANT+ system */
   /* in ultra low-power mode.  This function accepts as it's only      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(unsigned int ANTManagerCallbackID, !MSGREADY) line*/
   /*          on the ANT chip to the appropriate value.                */
int _ANTM_Sleep_Message(unsigned int ANTManagerCallbackID)
{
   int                           ret_val;
   BTPM_Message_t               *ResponseMessage;
   ANTM_Sleep_Message_Request_t  SleepMessageRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SleepMessageRequest, 0, ANTM_SLEEP_MESSAGE_REQUEST_SIZE);

      SleepMessageRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SleepMessageRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SleepMessageRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SleepMessageRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SLEEP_MESSAGE;
      SleepMessageRequest.MessageHeader.MessageLength   = ANTM_SLEEP_MESSAGE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SleepMessageRequest.EventHandlerID                = ANTManagerCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SleepMessageRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SLEEP_MESSAGE_RESPONSE_SIZE)
            ret_val = ((ANTM_Sleep_Message_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* Data Message API.                                                 */

   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the local ANT+ system.  This function      */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be broadcast on.  This function accepts as it's     */
   /* third argument the length of the data to send.  This function     */
   /* accepts as it's fourth argument a pointer to a byte array of the  */
   /* broadcast data to send.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
int _ANTM_Send_Broadcast_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int                                 ret_val;
   BTPM_Message_t                     *ResponseMessage;
   ANTM_Send_Broadcast_Data_Request_t *SendBroadcastDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if((Initialized) && (Data))
   {
      /* Now allocate the message.                                      */
      if((SendBroadcastDataRequest = (ANTM_Send_Broadcast_Data_Request_t *)BTPS_AllocateMemory(ANTM_SEND_BROADCAST_DATA_REQUEST_SIZE(DataLength))) != NULL)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(SendBroadcastDataRequest, 0, ANTM_SEND_BROADCAST_DATA_REQUEST_SIZE(DataLength));

         SendBroadcastDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendBroadcastDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendBroadcastDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         SendBroadcastDataRequest->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SEND_BROADCAST_DATA;
         SendBroadcastDataRequest->MessageHeader.MessageLength   = ANTM_SEND_BROADCAST_DATA_REQUEST_SIZE(DataLength) - BTPM_MESSAGE_HEADER_SIZE;

         SendBroadcastDataRequest->EventHandlerID                = ANTManagerCallbackID;
         SendBroadcastDataRequest->ChannelNumber                 = ChannelNumber;
         SendBroadcastDataRequest->BroadcastDataLength           = DataLength;

         BTPS_MemCopy(SendBroadcastDataRequest, Data, DataLength);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendBroadcastDataRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SEND_BROADCAST_DATA_RESPONSE_SIZE)
               ret_val = ((ANTM_Send_Broadcast_Data_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }

         BTPS_FreeMemory(SendBroadcastDataRequest);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be sent on.  This function accepts as it's third    */
   /* argument the length of the data to send.  This function accepts as*/
   /* it's fourth argument, a pointer to a byte array of the            */
   /* acknowledged data to send.  This function returns zero if         */
   /* successful, otherwise this function returns a negative error code.*/
int _ANTM_Send_Acknowledged_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int                                    ret_val;
   BTPM_Message_t                        *ResponseMessage;
   ANTM_Send_Acknowledged_Data_Request_t *SendAcknowledgedDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if((Initialized) && (Data))
   {
      /* Now allocate the message.                                      */
      if((SendAcknowledgedDataRequest = (ANTM_Send_Acknowledged_Data_Request_t *)BTPS_AllocateMemory(ANTM_SEND_ACKNOWLEDGED_DATA_REQUEST_SIZE(DataLength))) != NULL)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(SendAcknowledgedDataRequest, 0, ANTM_SEND_ACKNOWLEDGED_DATA_REQUEST_SIZE(DataLength));

         SendAcknowledgedDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendAcknowledgedDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendAcknowledgedDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         SendAcknowledgedDataRequest->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SEND_ACKNOWLEDGED_DATA;
         SendAcknowledgedDataRequest->MessageHeader.MessageLength   = ANTM_SEND_ACKNOWLEDGED_DATA_REQUEST_SIZE(DataLength) - BTPM_MESSAGE_HEADER_SIZE;

         SendAcknowledgedDataRequest->EventHandlerID                = ANTManagerCallbackID;
         SendAcknowledgedDataRequest->ChannelNumber                 = ChannelNumber;
         SendAcknowledgedDataRequest->AcknowledgedDataLength        = DataLength;

         BTPS_MemCopy(SendAcknowledgedDataRequest->AcknowledgedData, Data, DataLength);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendAcknowledgedDataRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SEND_ACKNOWLEDGED_DATA_RESPONSE_SIZE)
               ret_val = ((ANTM_Send_Acknowledged_Data_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }

         BTPS_FreeMemory(SendAcknowledgedDataRequest);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the sequence / channel  */
   /* number that the data will be sent on.  The upper three bits of    */
   /* this argument are the sequence number, and the lower five bits are*/
   /* the channel number.  This function accepts as it's third argument */
   /* the length of the data to send.  This function accepts as it's    */
   /* fourth argument, a pointer to a byte array of the burst data to   */
   /* send.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
int _ANTM_Send_Burst_Transfer_Data(unsigned int ANTManagerCallbackID, unsigned int SequenceChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   ANTM_Send_Burst_Transfer_Data_Request_t *SendBurstTransferDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if((Initialized) && (Data))
   {
      /* Now allocate the message.                                      */
      if((SendBurstTransferDataRequest = (ANTM_Send_Burst_Transfer_Data_Request_t *)BTPS_AllocateMemory(ANTM_SEND_BURST_TRANSFER_DATA_REQUEST_SIZE(DataLength))) != NULL)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(SendBurstTransferDataRequest, 0, ANTM_SEND_BURST_TRANSFER_DATA_REQUEST_SIZE(DataLength));

         SendBurstTransferDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendBurstTransferDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendBurstTransferDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         SendBurstTransferDataRequest->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SEND_BURST_TRANSFER_DATA;
         SendBurstTransferDataRequest->MessageHeader.MessageLength   = ANTM_SEND_BURST_TRANSFER_DATA_REQUEST_SIZE(DataLength) - BTPM_MESSAGE_HEADER_SIZE;

         SendBurstTransferDataRequest->EventHandlerID                = ANTManagerCallbackID;
         SendBurstTransferDataRequest->SequenceChannelNumber         = SequenceChannelNumber;
         SendBurstTransferDataRequest->BurstTransferDataLength       = DataLength;

         BTPS_MemCopy(SendBurstTransferDataRequest->BurstTransferData, Data, DataLength);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendBurstTransferDataRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SEND_BURST_TRANSFER_DATA_RESPONSE_SIZE)
               ret_val = ((ANTM_Send_Burst_Transfer_Data_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }

         BTPS_FreeMemory(SendBurstTransferDataRequest);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* Test Mode Message API.                                            */

   /* The following function is responsible for putting the ANT+ system */
   /* in CW test mode.  This function accepts as it's only argument the */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
int _ANTM_Initialize_CW_Test_Mode(unsigned int ANTManagerCallbackID)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   ANTM_Initialize_CW_Test_Mode_Request_t  InitializeCWTestModeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&InitializeCWTestModeRequest, 0, ANTM_INITIALIZE_CW_TEST_MODE_REQUEST_SIZE);

      InitializeCWTestModeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      InitializeCWTestModeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      InitializeCWTestModeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      InitializeCWTestModeRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_INITIALIZE_CW_TEST_MODE;
      InitializeCWTestModeRequest.MessageHeader.MessageLength   = ANTM_INITIALIZE_CW_TEST_MODE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      InitializeCWTestModeRequest.EventHandlerID                = ANTManagerCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&InitializeCWTestModeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_INITIALIZE_CW_TEST_MODE_RESPONSE_SIZE)
            ret_val = ((ANTM_Initialize_CW_Test_Mode_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument the      */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the transmit power level to be used.  This       */
   /* function accepts as it's third argument, the RF frequency to be   */
   /* used.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANTM_Initialize_CW_Test_Mode().                  */
int _ANTM_Set_CW_Test_Mode(unsigned int ANTManagerCallbackID, unsigned int TxPower, unsigned int RFFrequency)
{
   int                              ret_val;
   BTPM_Message_t                  *ResponseMessage;
   ANTM_Set_CW_Test_Mode_Request_t  SetCWTestModeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a request message and*/
      /* send it to the server.                                         */
      BTPS_MemInitialize(&SetCWTestModeRequest, 0, ANTM_SET_CW_TEST_MODE_REQUEST_SIZE);

      SetCWTestModeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      SetCWTestModeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      SetCWTestModeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
      SetCWTestModeRequest.MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SET_CW_TEST_MODE;
      SetCWTestModeRequest.MessageHeader.MessageLength   = ANTM_SET_CW_TEST_MODE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      SetCWTestModeRequest.EventHandlerID                = ANTManagerCallbackID;
      SetCWTestModeRequest.TxPower                       = TxPower;
      SetCWTestModeRequest.RFFrequency                   = RFFrequency;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetCWTestModeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SET_CW_TEST_MODE_RESPONSE_SIZE)
            ret_val = ((ANTM_Set_CW_Test_Mode_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a raw ANT       */
   /* packet.  This function accepts as it's first argument, the        */
   /* Callback ID that was returned from a successful call to           */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int _ANTM_Send_Raw_Packet(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   ANTM_Send_Raw_Packet_Request_t *SendRawPacketRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if((Initialized) && (PacketData))
   {
      /* Now allocate the message.                                      */
      if((SendRawPacketRequest = (ANTM_Send_Raw_Packet_Request_t *)BTPS_AllocateMemory(ANTM_SEND_RAW_PACKET_REQUEST_SIZE(DataSize))) != NULL)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(SendRawPacketRequest, 0, ANTM_SEND_RAW_PACKET_REQUEST_SIZE(DataSize));

         SendRawPacketRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendRawPacketRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendRawPacketRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         SendRawPacketRequest->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SEND_RAW_PACKET;
         SendRawPacketRequest->MessageHeader.MessageLength   = ANTM_SEND_RAW_PACKET_REQUEST_SIZE(DataSize) - BTPM_MESSAGE_HEADER_SIZE;

         SendRawPacketRequest->EventHandlerID                = ANTManagerCallbackID;
         SendRawPacketRequest->PacketLength                  = DataSize;

         BTPS_MemCopy(SendRawPacketRequest->PacketData, PacketData, SendRawPacketRequest->PacketLength);

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendRawPacketRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= ANTM_SEND_RAW_PACKET_RESPONSE_SIZE)
               ret_val = ((ANTM_Send_Raw_Packet_Response_t *)ResponseMessage)->Status;
            else
               ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

            /* All finished with the message, go ahead and free it.     */
            MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
         }

         BTPS_FreeMemory(SendRawPacketRequest);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a raw ANT       */
   /* packet without waiting for the command to be queued for sending   */
   /* to the chip.  This function accepts as it's first argument,       */
   /* the Callback ID that was returned from a successful call to       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int _ANTM_Send_Raw_Packet_Async(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData)
{
   int                             ret_val;
   ANTM_Send_Raw_Packet_Request_t *SendRawPacketRequest;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if((Initialized) && (PacketData))
   {
      /* Now allocate the message.                                      */
      if((SendRawPacketRequest = (ANTM_Send_Raw_Packet_Request_t *)BTPS_AllocateMemory(ANTM_SEND_RAW_PACKET_REQUEST_SIZE(DataSize))) != NULL)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(SendRawPacketRequest, 0, ANTM_SEND_RAW_PACKET_REQUEST_SIZE(DataSize));

         SendRawPacketRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendRawPacketRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendRawPacketRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER;
         SendRawPacketRequest->MessageHeader.MessageFunction = ANTM_MESSAGE_FUNCTION_SEND_RAW_PACKET;
         SendRawPacketRequest->MessageHeader.MessageLength   = ANTM_SEND_RAW_PACKET_REQUEST_SIZE(DataSize) - BTPM_MESSAGE_HEADER_SIZE;

         SendRawPacketRequest->EventHandlerID                = ANTManagerCallbackID;
         SendRawPacketRequest->PacketLength                  = DataSize;

         BTPS_MemCopy(SendRawPacketRequest->PacketData, PacketData, SendRawPacketRequest->PacketLength);

         /* Message has been formatted, go ahead and send it off.       */
         ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendRawPacketRequest, 0, NULL);

         BTPS_FreeMemory(SendRawPacketRequest);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
