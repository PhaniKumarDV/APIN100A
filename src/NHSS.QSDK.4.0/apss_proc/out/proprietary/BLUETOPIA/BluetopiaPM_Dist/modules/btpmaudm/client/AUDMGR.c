/*****< audmgr.c >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDMGR - Audio Manager Implementation for Stonestreet One Bluetooth       */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMAUDM.h"            /* BTPM Audio Manager Prototypes/Constants.  */
#include "AUDMGR.h"              /* Audio Manager Impl. Prototypes/Constants. */
#include "AUDMUTIL.h"            /* Audio Manager Util. Prototypes/Constants. */

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
   /* initialize the Audio Manager Implementation.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Audo Manager Implementation.                                      */
int _AUDM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Audio Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the Audio */
   /* Manager Implementation.  After this function is called the Audio  */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _AUDM_Initialize() function.  */
void _AUDM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Audio Stream or*/
   /* Remote Control connection.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _AUDM_Connection_Request_Response(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   AUDM_Connection_Request_Response_Request_t  ConnectionRequestResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Disconnect Audio   */
      /* Stream message and send it to the server.                      */
      BTPS_MemInitialize(&ConnectionRequestResponseRequest, 0, sizeof(ConnectionRequestResponseRequest));

      ConnectionRequestResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ConnectionRequestResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ConnectionRequestResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      ConnectionRequestResponseRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
      ConnectionRequestResponseRequest.MessageHeader.MessageLength   = AUDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ConnectionRequestResponseRequest.RequestType                   = RequestType;
      ConnectionRequestResponseRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      ConnectionRequestResponseRequest.Accept                        = Accept;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
            ret_val = ((AUDM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect an Audio Stream to a remote device.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Connect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned long StreamFlags)
{
   int                                  ret_val;
   BD_ADDR_t                            NULL_BD_ADDR;
   BTPM_Message_t                      *ResponseMessage;
   AUDM_Connect_Audio_Stream_Request_t  ConnectAudioStreamRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up a NULL BD_ADDR.                                         */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(!COMPARE_BD_ADDR(NULL_BD_ADDR, RemoteDeviceAddress))
      {
         /* All that we really need to do is to build a Connect Audio   */
         /* Stream message and send it to the server.                   */
         BTPS_MemInitialize(&ConnectAudioStreamRequest, 0, sizeof(ConnectAudioStreamRequest));

         ConnectAudioStreamRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectAudioStreamRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectAudioStreamRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         ConnectAudioStreamRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CONNECT_AUDIO_STREAM;
         ConnectAudioStreamRequest.MessageHeader.MessageLength   = AUDM_CONNECT_AUDIO_STREAM_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectAudioStreamRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectAudioStreamRequest.StreamType                    = StreamType;
         ConnectAudioStreamRequest.StreamFlags                   = StreamFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectAudioStreamRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_CONNECT_AUDIO_STREAM_RESPONSE_SIZE)
               ret_val = ((AUDM_Connect_Audio_Stream_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Audio Stream.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Disconnect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   AUDM_Disconnect_Audio_Stream_Request_t  DisconnectAudioStreamRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Disconnect Audio   */
      /* Stream message and send it to the server.                      */
      BTPS_MemInitialize(&DisconnectAudioStreamRequest, 0, sizeof(DisconnectAudioStreamRequest));

      DisconnectAudioStreamRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      DisconnectAudioStreamRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      DisconnectAudioStreamRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      DisconnectAudioStreamRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_DISCONNECT_AUDIO_STREAM;
      DisconnectAudioStreamRequest.MessageHeader.MessageLength   = AUDM_DISCONNECT_AUDIO_STREAM_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DisconnectAudioStreamRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      DisconnectAudioStreamRequest.StreamType                    = StreamType;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectAudioStreamRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_DISCONNECT_AUDIO_STREAM_RESPONSE_SIZE)
            ret_val = ((AUDM_Disconnect_Audio_Stream_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Audio   */
   /* sessions of the specified stream type (specified by the first     */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer). The next parameter is       */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful. The    */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters). This function returns a non-negative   */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer. This    */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _AUDM_Query_Audio_Connected_Devices(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                                           ret_val;
   BTPM_Message_t                               *ResponseMessage;
   AUDM_Query_Audio_Connected_Devices_Request_t  QueryConnectedAudioDevices;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || ((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)))
      {
         /* All that we really need to do is to build a Query Connected */
         /* Remote Control Devices message and send it to the server.   */
         QueryConnectedAudioDevices.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedAudioDevices.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedAudioDevices.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         QueryConnectedAudioDevices.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_CONNECTED_DEVICES;
         QueryConnectedAudioDevices.MessageHeader.MessageLength   = AUDM_QUERY_AUDIO_CONNECTED_DEVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryConnectedAudioDevices.StreamType                    = StreamType;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedAudioDevices, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_CONNECTED_DEVICES_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_CONNECTED_DEVICES_RESPONSE_SIZE(((AUDM_Query_Audio_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)))
            {
               if(!(ret_val = ((AUDM_Query_Audio_Connected_Devices_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = ((AUDM_Query_Audio_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                  /* If the caller specified a buffer to receive the    */
                  /* list into, go ahead and copy it over.              */
                  if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
                  {
                     /* If there are more entries in the returned list  */
                     /* than the buffer specified, we need to truncate  */
                     /* it.                                             */
                     if(MaximumRemoteDeviceListEntries >= ((AUDM_Query_Audio_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)
                        MaximumRemoteDeviceListEntries = ((AUDM_Query_Audio_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                     BTPS_MemCopy(RemoteDeviceAddressList, ((AUDM_Query_Audio_Connected_Devices_Response_t *)ResponseMessage)->DeviceConnectedList, MaximumRemoteDeviceListEntries*sizeof(BD_ADDR_t));

                     /* Flag how many devices that we returned.         */
                     ret_val = (int)MaximumRemoteDeviceListEntries;
                  }
                  else
                     ret_val = 0;
               }
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream state of the     */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream State of the Audio Stream (if*/
   /* this function is successful).                                     */
int _AUDM_Query_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t *StreamState)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   AUDM_Query_Audio_Stream_State_Request_t  QueryAudioStreamStateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(StreamState)
      {
         /* All that we really need to do is to build a Query Audio     */
         /* Stream State Message and send it to the server.             */
         BTPS_MemInitialize(&QueryAudioStreamStateRequest, 0, sizeof(QueryAudioStreamStateRequest));

         QueryAudioStreamStateRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryAudioStreamStateRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryAudioStreamStateRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         QueryAudioStreamStateRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_STATE;
         QueryAudioStreamStateRequest.MessageHeader.MessageLength   = AUDM_QUERY_AUDIO_STREAM_STATE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryAudioStreamStateRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         QueryAudioStreamStateRequest.StreamType                    = StreamType;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryAudioStreamStateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_STREAM_STATE_RESPONSE_SIZE)
            {
               /* Check to see if the response indicates success.       */
               if(!(ret_val = ((AUDM_Query_Audio_Stream_State_Response_t *)ResponseMessage)->Status))
               {
                  /* Note the Stream state.                             */
                  *StreamState = ((AUDM_Query_Audio_Stream_State_Response_t *)ResponseMessage)->StreamState;
               }
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream format of the    */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream Format of the Audio Stream   */
   /* (if this function is successful).                                 */
int _AUDM_Query_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int                                      ret_val;
   BTPM_Message_t                          *ResponseMessage;
   AUDM_Query_Audio_Stream_Format_Request_t  QueryAudioStreamFormatRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(StreamFormat)
      {
         /* All that we really need to do is to build a Query Audio     */
         /* Stream Format Message and send it to the server.            */
         BTPS_MemInitialize(&QueryAudioStreamFormatRequest, 0, sizeof(QueryAudioStreamFormatRequest));

         QueryAudioStreamFormatRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryAudioStreamFormatRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryAudioStreamFormatRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         QueryAudioStreamFormatRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_FORMAT;
         QueryAudioStreamFormatRequest.MessageHeader.MessageLength   = AUDM_QUERY_AUDIO_STREAM_FORMAT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         QueryAudioStreamFormatRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         QueryAudioStreamFormatRequest.StreamType                    = StreamType;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryAudioStreamFormatRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_STREAM_FORMAT_RESPONSE_SIZE)
            {
               /* Check to see if the response indicates success.       */
               if(!(ret_val = ((AUDM_Query_Audio_Stream_Format_Response_t *)ResponseMessage)->Status))
               {
                  /* Note the Stream format.                            */
                  *StreamFormat = ((AUDM_Query_Audio_Stream_Format_Response_t *)ResponseMessage)->StreamFormat;
               }
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to start/suspend the specified Audio Stream.  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Change_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   AUDM_Change_Audio_Stream_State_Request_t  ChangeAudioStreamStateRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Change Audio Stream*/
      /* State message and send it to the server.                       */
      BTPS_MemInitialize(&ChangeAudioStreamStateRequest, 0, sizeof(ChangeAudioStreamStateRequest));

      ChangeAudioStreamStateRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ChangeAudioStreamStateRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ChangeAudioStreamStateRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      ChangeAudioStreamStateRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE;
      ChangeAudioStreamStateRequest.MessageHeader.MessageLength   = AUDM_CHANGE_AUDIO_STREAM_STATE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ChangeAudioStreamStateRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      ChangeAudioStreamStateRequest.StreamType                    = StreamType;
      ChangeAudioStreamStateRequest.StreamState                   = StreamState;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeAudioStreamStateRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_CHANGE_AUDIO_STREAM_STATE_RESPONSE_SIZE)
            ret_val = ((AUDM_Change_Audio_Stream_State_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the current format used by the specified Audio  */
   /* Stream.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Change_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int                                        ret_val;
   BTPM_Message_t                            *ResponseMessage;
   AUDM_Change_Audio_Stream_Format_Request_t  ChangeAudioStreamFormatRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear valid.           */
      if(StreamFormat)
      {
         /* All that we really need to do is to build a Change Audio    */
         /* Stream Format message and send it to the server.            */
         BTPS_MemInitialize(&ChangeAudioStreamFormatRequest, 0, sizeof(ChangeAudioStreamFormatRequest));

         ChangeAudioStreamFormatRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ChangeAudioStreamFormatRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ChangeAudioStreamFormatRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         ChangeAudioStreamFormatRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT;
         ChangeAudioStreamFormatRequest.MessageHeader.MessageLength   = AUDM_CHANGE_AUDIO_STREAM_FORMAT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ChangeAudioStreamFormatRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ChangeAudioStreamFormatRequest.StreamType                    = StreamType;
         ChangeAudioStreamFormatRequest.StreamFormat                  = *StreamFormat;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeAudioStreamFormatRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_CHANGE_AUDIO_STREAM_FORMAT_RESPONSE_SIZE)
               ret_val = ((AUDM_Change_Audio_Stream_Format_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream configuration of */
   /* the specified Audio Stream.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* The final parameter will hold the Audio Stream Configuration of   */
   /* the Audio Stream (if this function is successful).                */
int _AUDM_Query_Audio_Stream_Configuration(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Configuration_t *StreamConfiguration)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   AUDM_Query_Audio_Stream_Configuration_Request_t  QueryAudioStreamConfigurationRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Query Audio Stream */
      /* Configuration message and send it to the server.               */
      BTPS_MemInitialize(&QueryAudioStreamConfigurationRequest, 0, sizeof(QueryAudioStreamConfigurationRequest));

      QueryAudioStreamConfigurationRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      QueryAudioStreamConfigurationRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      QueryAudioStreamConfigurationRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      QueryAudioStreamConfigurationRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_CONFIGURATION;
      QueryAudioStreamConfigurationRequest.MessageHeader.MessageLength   = AUDM_QUERY_AUDIO_STREAM_CONFIGURATION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      QueryAudioStreamConfigurationRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
      QueryAudioStreamConfigurationRequest.StreamType                    = StreamType;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryAudioStreamConfigurationRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_STREAM_CONFIGURATION_RESPONSE_SIZE)
         {
            /* Check to see if the response indicates success.          */
            if(!(ret_val = ((AUDM_Query_Audio_Stream_Configuration_Response_t *)ResponseMessage)->Status))
            {
               /* Note the Stream Configuration.                        */
               *StreamConfiguration = ((AUDM_Query_Audio_Stream_Configuration_Response_t *)ResponseMessage)->StreamConfiguration;
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for incoming Audio*/
   /* Stream and Remote Control connections.  This function returns zero*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
int _AUDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int                                              ret_val;
   BTPM_Message_t                                  *ResponseMessage;
   AUDM_Change_Incoming_Connection_Flags_Request_t  ChangeIncomingConnectionFlagsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Start Audio Stream */
      /* message and send it to the server.                             */
      ChangeIncomingConnectionFlagsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS;
      ChangeIncomingConnectionFlagsRequest.MessageHeader.MessageLength   = AUDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ChangeIncomingConnectionFlagsRequest.ConnectionFlags               = ConnectionFlags;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ChangeIncomingConnectionFlagsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE)
            ret_val = ((AUDM_Change_Incoming_Connection_Flags_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          _AUDM_Query_Audio_Stream_Configuration() function.       */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _AUDM_Send_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int AudioManagerDataEventCallbackID, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame)
{
   int                                     ret_val;
   AUDM_Send_Encoded_Audio_Data_Request_t *SendEncodedAudioDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((RawAudioDataFrameLength) && (RawAudioDataFrame))
      {
         if((SendEncodedAudioDataRequest = (AUDM_Send_Encoded_Audio_Data_Request_t *)BTPS_AllocateMemory(AUDM_SEND_ENCODED_AUDIO_DATA_REQUEST_SIZE(RawAudioDataFrameLength))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendEncodedAudioDataRequest, 0, AUDM_SEND_ENCODED_AUDIO_DATA_REQUEST_SIZE(0));

            SendEncodedAudioDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendEncodedAudioDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendEncodedAudioDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
            SendEncodedAudioDataRequest->MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_SEND_ENCODED_AUDIO_DATA;
            SendEncodedAudioDataRequest->MessageHeader.MessageLength   = AUDM_SEND_ENCODED_AUDIO_DATA_REQUEST_SIZE(RawAudioDataFrameLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendEncodedAudioDataRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SendEncodedAudioDataRequest->StreamEventsHandlerID         = AudioManagerDataEventCallbackID;
            SendEncodedAudioDataRequest->RawAudioDataFrameLength       = RawAudioDataFrameLength;

            BTPS_MemCopy(SendEncodedAudioDataRequest->RawAudioDataFrame, RawAudioDataFrame, RawAudioDataFrameLength);

            /* Message has been formatted, go ahead and send it off.    */
            /* * NOTE * There is NO Response for this message, so we    */
            /*          are not going to wait for one.                  */
            ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendEncodedAudioDataRequest, 0, NULL);

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendEncodedAudioDataRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          _AUDM_Query_Audio_Stream_Configuration() function.       */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _AUDM_Send_RTP_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int AudioManagerDataEventCallbackID, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo)
{
   int                                         ret_val;
   AUDM_Send_RTP_Encoded_Audio_Data_Request_t *SendRTPEncodedAudioDataRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if((RawAudioDataFrameLength) && (RawAudioDataFrame) && (RTPHeaderInfo))
      {
         if((SendRTPEncodedAudioDataRequest = (AUDM_Send_RTP_Encoded_Audio_Data_Request_t *)BTPS_AllocateMemory(AUDM_SEND_RTP_ENCODED_AUDIO_DATA_REQUEST_SIZE(RawAudioDataFrameLength))) != NULL)
         {
            /* All that we really need to do is to build the message and*/
            /* send it to the server.                                   */
            BTPS_MemInitialize(SendRTPEncodedAudioDataRequest, 0, AUDM_SEND_RTP_ENCODED_AUDIO_DATA_REQUEST_SIZE(0));

            SendRTPEncodedAudioDataRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendRTPEncodedAudioDataRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendRTPEncodedAudioDataRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
            SendRTPEncodedAudioDataRequest->MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_SEND_RTP_ENCODED_AUDIO_DATA;
            SendRTPEncodedAudioDataRequest->MessageHeader.MessageLength   = AUDM_SEND_RTP_ENCODED_AUDIO_DATA_REQUEST_SIZE(RawAudioDataFrameLength) - BTPM_MESSAGE_HEADER_SIZE;

            SendRTPEncodedAudioDataRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SendRTPEncodedAudioDataRequest->StreamEventsHandlerID         = AudioManagerDataEventCallbackID;
            SendRTPEncodedAudioDataRequest->Flags                         = Flags;
            SendRTPEncodedAudioDataRequest->RTPHeaderInfo                 = *RTPHeaderInfo;
            SendRTPEncodedAudioDataRequest->RawAudioDataFrameLength       = RawAudioDataFrameLength;

            BTPS_MemCopy(SendRTPEncodedAudioDataRequest->RawAudioDataFrame, RawAudioDataFrame, RawAudioDataFrameLength);

            /* Message has been formatted, go ahead and send it off.    */
            /* * NOTE * There is NO Response for this message, so we    */
            /*          are not going to wait for one.                  */
            ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendRTPEncodedAudioDataRequest, 0, NULL);

            /* Free the memory that was allocated for the packet.       */
            BTPS_FreeMemory(SendRTPEncodedAudioDataRequest);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control session to a remote device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _AUDM_Connect_Remote_Control(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags)
{
   int                                    ret_val;
   BD_ADDR_t                              NULL_BD_ADDR;
   BTPM_Message_t                        *ResponseMessage;
   AUDM_Connect_Remote_Control_Request_t  ConnectRemoteControlRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up a NULL BD_ADDR.                                         */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(!COMPARE_BD_ADDR(NULL_BD_ADDR, RemoteDeviceAddress))
      {
         /* All that we really need to do is to build a Connect Remote  */
         /* Control message and send it to the server.                  */
         BTPS_MemInitialize(&ConnectRemoteControlRequest, 0, sizeof(ConnectRemoteControlRequest));

         ConnectRemoteControlRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteControlRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteControlRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         ConnectRemoteControlRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL;
         ConnectRemoteControlRequest.MessageHeader.MessageLength   = AUDM_CONNECT_REMOTE_CONTROL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectRemoteControlRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectRemoteControlRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteControlRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_CONNECT_REMOTE_CONTROL_RESPONSE_SIZE)
               ret_val = ((AUDM_Connect_Remote_Control_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Remote Control        */
   /* session.  This function returns zero if successful, or a negative */
   /* return error code if there was an error.                          */
int _AUDM_Disconnect_Remote_Control(BD_ADDR_t RemoteDeviceAddress)
{
   int                                       ret_val;
   BTPM_Message_t                           *ResponseMessage;
   AUDM_Disconnect_Remote_Control_Request_t  DisconnectRemoteControlRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Disconnect Remote  */
      /* Control message and send it to the server.                     */
      BTPS_MemInitialize(&DisconnectRemoteControlRequest, 0, sizeof(DisconnectRemoteControlRequest));

      DisconnectRemoteControlRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      DisconnectRemoteControlRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      DisconnectRemoteControlRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      DisconnectRemoteControlRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL;
      DisconnectRemoteControlRequest.MessageHeader.MessageLength   = AUDM_DISCONNECT_REMOTE_CONTROL_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DisconnectRemoteControlRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectRemoteControlRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_DISCONNECT_REMOTE_CONTROL_RESPONSE_SIZE)
            ret_val = ((AUDM_Disconnect_Remote_Control_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Remote  */
   /* Control Target or Controller sessions (specified by the first     */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer). The next parameter is       */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful. The    */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters). This function returns a non-negative   */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer. This    */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _AUDM_Query_Remote_Control_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                                                    ret_val;
   BTPM_Message_t                                        *ResponseMessage;
   AUDM_Query_Remote_Control_Connected_Devices_Request_t  QueryConnectedRemoteControlDevices;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || ((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)))
      {
         /* All that we really need to do is to build a Query Connected */
         /* Remote Control Devices message and send it to the server.   */
         QueryConnectedRemoteControlDevices.MessageHeader.AddressID       = MSG_GetServerAddressID();
         QueryConnectedRemoteControlDevices.MessageHeader.MessageID       = MSG_GetNextMessageID();
         QueryConnectedRemoteControlDevices.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         QueryConnectedRemoteControlDevices.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES;
         QueryConnectedRemoteControlDevices.MessageHeader.MessageLength   = AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&QueryConnectedRemoteControlDevices, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if((BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_RESPONSE_SIZE(0)) && (BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_RESPONSE_SIZE(((AUDM_Query_Remote_Control_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)))
            {
               if(!(ret_val = ((AUDM_Query_Remote_Control_Connected_Devices_Response_t *)ResponseMessage)->Status))
               {
                  /* Response successful, go ahead and note the results.*/
                  if(TotalNumberConnectedDevices)
                     *TotalNumberConnectedDevices = ((AUDM_Query_Remote_Control_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                  /* If the caller specified a buffer to receive the    */
                  /* list into, go ahead and copy it over.              */
                  if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
                  {
                     /* If there are more entries in the returned list  */
                     /* than the buffer specified, we need to truncate  */
                     /* it.                                             */
                     if(MaximumRemoteDeviceListEntries >= ((AUDM_Query_Remote_Control_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected)
                        MaximumRemoteDeviceListEntries = ((AUDM_Query_Remote_Control_Connected_Devices_Response_t *)ResponseMessage)->NumberDevicesConnected;

                     BTPS_MemCopy(RemoteDeviceAddressList, ((AUDM_Query_Remote_Control_Connected_Devices_Response_t *)ResponseMessage)->DeviceConnectedList, MaximumRemoteDeviceListEntries*sizeof(BD_ADDR_t));

                     /* Flag how many devices that we returned.         */
                     ret_val = (int)MaximumRemoteDeviceListEntries;
                  }
                  else
                     ret_val = 0;
               }
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Remote Control Command to the remote Device.  This function       */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* _AUDM_Register_Remote_Control_Event_Callback() function), followed*/
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Response Timeout (in milliseconds), followed by a */
   /* pointer to the actual Remote Control Message to send.  This       */
   /* function returns a positive, value if successful or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Transaction ID of the Remote Control Event that was  */
   /*          submitted.                                               */
int _AUDM_Send_Remote_Control_Command(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   AUDM_Send_Remote_Control_Command_Request_t *SendRemoteControlCommandRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if(CommandData)
      {
         /* First, determine how much space is required to hold the     */
         /* message.                                                    */
         if((ret_val = ConvertDecodedAVRCPCommandToStream(CommandData, 0, NULL)) > 0)
         {
            if((SendRemoteControlCommandRequest = (AUDM_Send_Remote_Control_Command_Request_t *)BTPS_AllocateMemory(AUDM_SEND_REMOTE_CONTROL_COMMAND_REQUEST_SIZE(ret_val))) != NULL)
            {
               /* All that we really need to do is to build the message */
               /* and send it to the server.                            */
               BTPS_MemInitialize(SendRemoteControlCommandRequest, 0, AUDM_SEND_REMOTE_CONTROL_COMMAND_REQUEST_SIZE(0));

               SendRemoteControlCommandRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               SendRemoteControlCommandRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SendRemoteControlCommandRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
               SendRemoteControlCommandRequest->MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_COMMAND;
               SendRemoteControlCommandRequest->MessageHeader.MessageLength   = AUDM_SEND_REMOTE_CONTROL_COMMAND_REQUEST_SIZE(ret_val) - BTPM_MESSAGE_HEADER_SIZE;

               SendRemoteControlCommandRequest->RemoteControlEventsHandlerID  = AudioManagerRemoteControlEventCallbackID;
               SendRemoteControlCommandRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               SendRemoteControlCommandRequest->ResponseTimeout               = ResponseTimeout;
               SendRemoteControlCommandRequest->MessageType                   = CommandData->MessageType;
               SendRemoteControlCommandRequest->MessageDataLength             = (unsigned int)ret_val;

               if((ret_val = ConvertDecodedAVRCPCommandToStream(CommandData, SendRemoteControlCommandRequest->MessageDataLength, SendRemoteControlCommandRequest->MessageData)) > 0)
               {
                  /* Message has been formatted, go ahead and send it   */
                  /* off.                                               */
                  if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendRemoteControlCommandRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
                  {
                     /* Response received, go ahead and see if the      */
                     /* return value is valid.  If it is, go ahead and  */
                     /* note the returned status.                       */
                     if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_SEND_REMOTE_CONTROL_COMMAND_RESPONSE_SIZE)
                     {
                        if(!(ret_val = ((AUDM_Send_Remote_Control_Command_Response_t *)ResponseMessage)->Status))
                           ret_val = (int)(((AUDM_Send_Remote_Control_Command_Response_t *)ResponseMessage)->TransactionID);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                     /* All finished with the message, go ahead and free*/
                     /* it.                                             */
                     MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
                  }
               }

               /* Free the memory that was allocated for the packet.    */
               BTPS_FreeMemory(SendRemoteControlCommandRequest);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Remote Control Response to the remote Device.  This function      */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* _AUDM_Register_Remote_Control_Event_Callback() function), followed*/
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Transaction ID of the Remote Control Event,       */
   /* followed by a pointer to the actual Remote Control Response       */
   /* Message to send.  This function returns zero if successful or a   */
   /* negative return error code if there was an error.                 */
int _AUDM_Send_Remote_Control_Response(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   AUDM_Send_Remote_Control_Response_Request_t *SendRemoteControlResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure that input parameters appear to be    */
      /* semi-valid.                                                    */
      if(ResponseData)
      {
         /* First, determine how much space is required to hold the     */
         /* message.                                                    */
         if((ret_val = ConvertDecodedAVRCPResponseToStream(ResponseData, 0, NULL)) > 0)
         {
            if((SendRemoteControlResponseRequest = (AUDM_Send_Remote_Control_Response_Request_t *)BTPS_AllocateMemory(AUDM_SEND_REMOTE_CONTROL_RESPONSE_REQUEST_SIZE(ret_val))) != NULL)
            {
               /* All that we really need to do is to build the message */
               /* and send it to the server.                            */
               BTPS_MemInitialize(SendRemoteControlResponseRequest, 0, AUDM_SEND_REMOTE_CONTROL_RESPONSE_REQUEST_SIZE(0));

               SendRemoteControlResponseRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
               SendRemoteControlResponseRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SendRemoteControlResponseRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
               SendRemoteControlResponseRequest->MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_RESPONSE;
               SendRemoteControlResponseRequest->MessageHeader.MessageLength   = AUDM_SEND_REMOTE_CONTROL_RESPONSE_REQUEST_SIZE(ret_val) - BTPM_MESSAGE_HEADER_SIZE;

               SendRemoteControlResponseRequest->RemoteControlEventsHandlerID  = AudioManagerRemoteControlEventCallbackID;
               SendRemoteControlResponseRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
               SendRemoteControlResponseRequest->TransactionID                 = TransactionID;
               SendRemoteControlResponseRequest->MessageType                   = ResponseData->MessageType;
               SendRemoteControlResponseRequest->MessageDataLength             = (unsigned int)ret_val;

               if((ret_val = ConvertDecodedAVRCPResponseToStream(ResponseData, SendRemoteControlResponseRequest->MessageDataLength, SendRemoteControlResponseRequest->MessageData)) > 0)
               {
                  /* Message has been formatted, go ahead and send it   */
                  /* off.                                               */
                  if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendRemoteControlResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
                  {
                     /* Response received, go ahead and see if the      */
                     /* return value is valid.  If it is, go ahead and  */
                     /* note the returned status.                       */
                     if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_SEND_REMOTE_CONTROL_RESPONSE_RESPONSE_SIZE)
                        ret_val = ((AUDM_Send_Remote_Control_Response_Response_t *)ResponseMessage)->Status;
                     else
                        ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

                     /* All finished with the message, go ahead and free*/
                     /* it.                                             */
                     MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
                  }
               }

               /* Free the memory that was allocated for the packet.    */
               BTPS_FreeMemory(SendRemoteControlResponseRequest);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Audio Manager    */
   /* Service.  This Callback will be dispatched by the Audio Manager   */
   /* when various Audio Manager Events occur.  This function returns a */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the        */
   /*          Stream Event Handler ID.  This value can be passed to the*/
   /*          AUDM_Un_Register_Stream_Events() function to Un-Register */
   /*          the Event Handler.                                       */
int _AUDM_Register_Stream_Events(void)
{
   int                                          ret_val;
   BTPM_Message_t                              *ResponseMessage;
   AUDM_Register_Audio_Stream_Events_Request_t  RegisterAudioStreamEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register Audio     */
      /* Stream Events message and send it to the server.               */
      RegisterAudioStreamEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterAudioStreamEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterAudioStreamEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      RegisterAudioStreamEventsRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_EVENTS;
      RegisterAudioStreamEventsRequest.MessageHeader.MessageLength   = AUDM_REGISTER_AUDIO_STREAM_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterAudioStreamEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_REGISTER_AUDIO_STREAM_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((AUDM_Register_Audio_Stream_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((AUDM_Register_Audio_Stream_Events_Response_t *)ResponseMessage)->StreamEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Handler   */
   /* (registered via a successful call to the                          */
   /* AUDM_RegisterStreamEvents() function).  This function accepts as  */
   /* input the Stream Event Handler ID (return value from              */
   /* AUDM_Register_Stream_Events() function).                          */
int _AUDM_Un_Register_Stream_Events(unsigned int StreamEventsHandlerID)
{
   int                                             ret_val;
   BTPM_Message_t                                 *ResponseMessage;
   AUDM_Un_Register_Audio_Stream_Events_Request_t  UnRegisterAudioStreamEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register Audio */
      /* Stream Events message and send it to the server.               */
      UnRegisterAudioStreamEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterAudioStreamEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterAudioStreamEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      UnRegisterAudioStreamEventsRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_EVENTS;
      UnRegisterAudioStreamEventsRequest.MessageHeader.MessageLength   = AUDM_UN_REGISTER_AUDIO_STREAM_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterAudioStreamEventsRequest.StreamEventsHandlerID         = StreamEventsHandlerID;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterAudioStreamEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_UN_REGISTER_AUDIO_STREAM_EVENTS_RESPONSE_SIZE)
            ret_val = ((AUDM_Un_Register_Audio_Stream_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Audio     */
   /* Manager Service to explicitly process Data (either Source or      */
   /* Sink).  This Callback will be dispatched by the Audio Manager when*/
   /* various Audio Manager Events occur.  This function accepts Audio  */
   /* Stream Type to register.  This function returns a positive        */
   /* (non-zero) value if successful, or a negative return error code if*/
   /* there was an error.                                               */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _AUDM_Send_Encoded_Audio_Data() function to send data    */
   /*          for the Audio Source).                                   */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Audio Stream Type.                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _AUDM_Un_Register_Stream_Data_Events() function to       */
   /*          un-register the callback from this module.               */
int _AUDM_Register_Stream_Data_Events(AUD_Stream_Type_t StreamType)
{
   int                                               ret_val;
   BTPM_Message_t                                   *ResponseMessage;
   AUDM_Register_Audio_Stream_Data_Events_Request_t  RegisterAudioStreamDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register Audio     */
      /* Stream Data Events message and send it to the server.          */
      BTPS_MemInitialize(&RegisterAudioStreamDataEventsRequest, 0, sizeof(RegisterAudioStreamDataEventsRequest));

      RegisterAudioStreamDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterAudioStreamDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterAudioStreamDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      RegisterAudioStreamDataEventsRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_DATA;
      RegisterAudioStreamDataEventsRequest.MessageHeader.MessageLength   = AUDM_REGISTER_AUDIO_STREAM_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      RegisterAudioStreamDataEventsRequest.StreamType                    = StreamType;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterAudioStreamDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_REGISTER_AUDIO_STREAM_DATA_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((AUDM_Register_Audio_Stream_Data_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((AUDM_Register_Audio_Stream_Data_Events_Response_t *)ResponseMessage)->StreamDataEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Data Event      */
   /* Callback (registered via a successful call to the                 */
   /* _AUDM_Register_Stream_Data_Events() function).  This function     */
   /* accepts as input the Audio Manager Event Callback ID (return value*/
   /* from _AUDM_Register_Stream_Data_Events() function).               */
int _AUDM_Un_Register_Stream_Data_Events(unsigned int AudioManagerDataCallbackID)
{
   int                                                  ret_val;
   BTPM_Message_t                                      *ResponseMessage;
   AUDM_Un_Register_Audio_Stream_Data_Events_Request_t  UnRegisterAudioStreamDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register Audio */
      /* Stream Data Events message and send it to the server.          */
      UnRegisterAudioStreamDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterAudioStreamDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterAudioStreamDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      UnRegisterAudioStreamDataEventsRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_DATA;
      UnRegisterAudioStreamDataEventsRequest.MessageHeader.MessageLength   = AUDM_UN_REGISTER_AUDIO_STREAM_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterAudioStreamDataEventsRequest.StreamDataEventsHandlerID     = AudioManagerDataCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterAudioStreamDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_UN_REGISTER_AUDIO_STREAM_DATA_EVENTS_RESPONSE_SIZE)
            ret_val = ((AUDM_Un_Register_Audio_Stream_Data_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback with the Audio Manager      */
   /* Service to explicitly process Remote Control Data (either         */
   /* Controller or Target).  This function accepts the Service Type    */
   /* (Target or Controller) of the events that are to be registered    */
   /* for.  This function returns a positive (non-zero) value if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _AUDM_Send_Remote_Control_Command() or                   */
   /*          _AUDM_Send_Remote_Control_Response() functions to send   */
   /*          Remote Control Events.                                   */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Service Type.                                   */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _AUDM_Un_Register_Remote_Control_Event_Callback()        */
   /*          function to un-register the callback from this module.   */
int _AUDM_Register_Remote_Control_Event_Callback(unsigned int ServiceType)
{
   int                                                 ret_val;
   BTPM_Message_t                                     *ResponseMessage;
   AUDM_Register_Remote_Control_Data_Events_Request_t  RegisterRemoteControlDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Register Remote    */
      /* Control Data Events message and send it to the server.         */
      BTPS_MemInitialize(&RegisterRemoteControlDataEventsRequest, 0, sizeof(RegisterRemoteControlDataEventsRequest));

      RegisterRemoteControlDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      RegisterRemoteControlDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      RegisterRemoteControlDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      RegisterRemoteControlDataEventsRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REGISTER_REMOTE_CONTROL_DATA;
      RegisterRemoteControlDataEventsRequest.MessageHeader.MessageLength   = AUDM_REGISTER_REMOTE_CONTROL_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      RegisterRemoteControlDataEventsRequest.ServiceType                   = ServiceType;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&RegisterRemoteControlDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_REGISTER_REMOTE_CONTROL_DATA_EVENTS_RESPONSE_SIZE)
         {
            if(!(ret_val = ((AUDM_Register_Remote_Control_Data_Events_Response_t *)ResponseMessage)->Status))
               ret_val = (int)(((AUDM_Register_Remote_Control_Data_Events_Response_t *)ResponseMessage)->RemoteControlEventsHandlerID);
         }
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Remote Control  */
   /* Event Callback (registered via a successful call to the           */
   /* _AUDM_Register_Remote_Control_Event_Callback() function).  This   */
   /* function accepts as input the Audio Manager Remote Control Event  */
   /* Callback ID (return value from                                    */
   /* _AUDM_Register_Remote_Control_Event_Callback() function).         */
int _AUDM_Un_Register_Remote_Control_Event_Callback(unsigned int AudioManagerRemoteControlCallbackID)
{
   int                                                    ret_val;
   BTPM_Message_t                                        *ResponseMessage;
   AUDM_Un_Register_Remote_Control_Data_Events_Request_t  UnRegisterRemoteControlDataEventsRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build an Un-Register Remote*/
      /* Control Data Events message and send it to the server.         */
      UnRegisterRemoteControlDataEventsRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      UnRegisterRemoteControlDataEventsRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      UnRegisterRemoteControlDataEventsRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      UnRegisterRemoteControlDataEventsRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_UN_REGISTER_REMOTE_CONTROL_DATA;
      UnRegisterRemoteControlDataEventsRequest.MessageHeader.MessageLength   = AUDM_UN_REGISTER_REMOTE_CONTROL_DATA_EVENTS_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      UnRegisterRemoteControlDataEventsRequest.RemoteControlEventsHandlerID  = AudioManagerRemoteControlCallbackID;

      /* Message has been formatted, go ahead and send it off.          */
      if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterRemoteControlDataEventsRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_UN_REGISTER_REMOTE_CONTROL_DATA_EVENTS_RESPONSE_SIZE)
            ret_val = ((AUDM_Un_Register_Remote_Control_Data_Events_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control Browsing session to a remote  */
   /* device.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Connect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags)
{
   int                                             ret_val;
   BD_ADDR_t                                       NULL_BD_ADDR;
   BTPM_Message_t                                 *ResponseMessage;
   AUDM_Connect_Remote_Control_Browsing_Request_t  ConnectRemoteControlBrowsingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up a NULL BD_ADDR.                                         */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if(!COMPARE_BD_ADDR(NULL_BD_ADDR, RemoteDeviceAddress))
      {
         /* All that we really need to do is to build a Connect Remote  */
         /* Control Browsing message and send it to the server.         */
         BTPS_MemInitialize(&ConnectRemoteControlBrowsingRequest, 0, sizeof(ConnectRemoteControlBrowsingRequest));

         ConnectRemoteControlBrowsingRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteControlBrowsingRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteControlBrowsingRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         ConnectRemoteControlBrowsingRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL_BROWSING;
         ConnectRemoteControlBrowsingRequest.MessageHeader.MessageLength   = AUDM_CONNECT_REMOTE_CONTROL_BROWSING_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectRemoteControlBrowsingRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectRemoteControlBrowsingRequest.ConnectionFlags               = ConnectionFlags;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteControlBrowsingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_CONNECT_REMOTE_CONTROL_BROWSING_RESPONSE_SIZE)
               ret_val = ((AUDM_Connect_Remote_Control_Browsing_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* Browsing session.  This function returns zero if successful, or a */
   /* negative return error code if there was an error.                 */
int _AUDM_Disconnect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress)
{
   int                                                ret_val;
   BTPM_Message_t                                    *ResponseMessage;
   AUDM_Disconnect_Remote_Control_Browsing_Request_t  DisconnectRemoteControlBrowsingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* All that we really need to do is to build a Disconnect Remote  */
      /* Control message and send it to the server.                     */
      BTPS_MemInitialize(&DisconnectRemoteControlBrowsingRequest, 0, sizeof(DisconnectRemoteControlBrowsingRequest));

      DisconnectRemoteControlBrowsingRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
      DisconnectRemoteControlBrowsingRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
      DisconnectRemoteControlBrowsingRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      DisconnectRemoteControlBrowsingRequest.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL_BROWSING;
      DisconnectRemoteControlBrowsingRequest.MessageHeader.MessageLength   = AUDM_DISCONNECT_REMOTE_CONTROL_BROWSING_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      DisconnectRemoteControlBrowsingRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

      /* Message has been formatted, go ahead and send it off.          */
      if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectRemoteControlBrowsingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
      {
         /* Response received, go ahead and see if the return value is  */
         /* valid.  If it is, go ahead and note the returned status.    */
         if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= AUDM_DISCONNECT_REMOTE_CONTROL_BROWSING_RESPONSE_SIZE)
            ret_val = ((AUDM_Disconnect_Remote_Control_Browsing_Response_t *)ResponseMessage)->Status;
         else
            ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

         /* All finished with the message, go ahead and free it.        */
         MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

