/*****< pbamgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PBAMGR - Phone Book Access Manager Implementation for Stonestreet One     */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMPBAM.h"            /* BTPM HFRE Manager Prototypes/Constants.   */
#include "PBAMMSG.h"             /* BTPM HFRE Manager Message Formats.        */
#include "PBAMGR.h"              /* HFR Manager Impl. Prototypes/Constants.   */

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
   /* initialize the Phone Book Access Manager implementation. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Phone Book Access Manager implementation.                 */
int _PBAM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Phone Book Access Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* Phone Book Access Manager implementation. After this function     */
   /* is called the Phone Book Access Manager implementation will no    */
   /* longer operate until it is initialized again via a call to the    */
   /* _PBAM_Initialize() function.                                      */
void _PBAM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Phone Book Access device. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error. This function accepts the connection  */
   /* information for the remote device (address and server port). This */
   /* function accepts the connection flags to apply to control how the */
   /* connection is made regarding encryption and/or authentication.    */
int _PBAM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags)
{
   int                                   ret_val;
   BTPM_Message_t                       *ResponseMessage;
   PBAM_Connect_Remote_Device_Request_t  ConnectRemoteDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Check to make sure that the input parameters appear to         */
      /* semi-valid.                                                    */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort))
      {
         /* All that we really need to do is to build a Connect Remote  */
         /* Device message and send it to the server.                   */
         BTPS_MemInitialize(&ConnectRemoteDeviceRequest, 0, PBAM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE);

         ConnectRemoteDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectRemoteDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         ConnectRemoteDeviceRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE;
         ConnectRemoteDeviceRequest.MessageHeader.MessageLength   = PBAM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectRemoteDeviceRequest.ConnectionFlags               = ConnectionFlags;
         ConnectRemoteDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;
         ConnectRemoteDeviceRequest.RemoteServerPort              = RemoteServerPort;

         /* Message has been formatted, go ahead and send it off.       */
         if(!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectRemoteDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage)) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid.  If it is, go ahead and note the returned      */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE)
               ret_val = ((PBAM_Connect_Remote_Device_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Phone Book Access*/
   /* connection that was previously opened by a Successful call to     */
   /* _PBAM_Connect_Remote_Device() function.  This function accepts as */
   /* input the device address of the local connection which should     */
   /* close its active connection.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
int _PBAM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress)
{
   int                               ret_val;
   BTPM_Message_t                   *ResponseMessage;
   PBAM_Disconnect_Device_Request_t  DisconnectDeviceRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* All that we really need to do is to build a Disconnect      */
         /* Device message and send it to the server.                   */
         BTPS_MemInitialize(&DisconnectDeviceRequest, 0, sizeof(PBAM_DISCONNECT_DEVICE_REQUEST_SIZE));

         DisconnectDeviceRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         DisconnectDeviceRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisconnectDeviceRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         DisconnectDeviceRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_DISCONNECT_DEVICE;
         DisconnectDeviceRequest.MessageHeader.MessageLength   = PBAM_DISCONNECT_DEVICE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         DisconnectDeviceRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&DisconnectDeviceRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_DISCONNECT_DEVICE_RESPONSE_SIZE)
               ret_val = ((PBAM_Disconnect_Device_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding PBAP Profile Client Request.  This function accepts   */
   /* the device address of the device that is to have the PBAP         */
   /* operation aborted.  This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that a PBAP Profile Client Request that is to be aborted */
   /*          may have completed before the server was able to Abort   */
   /*          the request.  In either case, the caller will be notified*/
   /*          via PBAP Profile Callback of the status of the previous  */
   /*          Request.                                                 */
int _PBAM_Abort_Request(BD_ADDR_t RemoteDeviceAddress)
{
   int                  ret_val;
   BTPM_Message_t      *ResponseMessage;
   PBAM_Abort_Request_t AbortRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* All that we really need to do is to build an abort request  */
         /* message and send it to the server.                          */
         BTPS_MemInitialize(&AbortRequest, 0, PBAM_ABORT_REQUEST_SIZE);

         AbortRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         AbortRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         AbortRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         AbortRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_ABORT;
         AbortRequest.MessageHeader.MessageLength   = PBAM_ABORT_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         AbortRequest.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&AbortRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_ABORT_REQUEST_RESPONSE_SIZE)
               ret_val = ((PBAM_Abort_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a PBAP Pull Phone Book request to*/
   /* the specified remote PBAP Server.  The first parameter specifies  */
   /* the remote, connected, device to issue the request on.  The       */
   /* PhoneBookNamePath parameter contains the name/path of the phone   */
   /* book being requested by this pull phone book operation.  This     */
   /* value can be NULL if a phone book size request is being performed.*/
   /* The FilterLow parameter contains the lower 32 bits of the 64-bit  */
   /* filter attribute.  The FilterHigh parameter contains the higher 32*/
   /* bits of the 64-bit filter attribute.  The Format parameter is an  */
   /* enumeration which specifies the vCard format requested in this    */
   /* pull phone book request.  If pfDefault is specified then the      */
   /* format will not be included in the request (note that the server  */
   /* will default to pfvCard21 in this case).  The MaxListCount        */
   /* parameter is an unsigned integer that specifies the maximum number*/
   /* of entries the client can handle.  A value of 65535 means that the*/
   /* number of entries is not restricted.  A MaxListCount of ZERO (0)  */
   /* indicates that this is a request for the number of used indexes in*/
   /* the Phonebook specified by the PhoneBookNamePath parameter.  The  */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this PullPhonebookRequest.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP profile server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP profile server successfully */
   /*          executed the request.                                    */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current request is completed (this is   */
   /*          signified by receiving a confirmation event in the PBAP  */
   /*          profile event callback that was registered when the PBAM */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_Phone_Book_Request(BD_ADDR_t RemoteDeviceAddress, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t Format, Word_t MaxListCount, Word_t ListStartOffset)
{
   int                             ret_val;
   BTPM_Message_t                 *ResponseMessage;
   PBAM_Pull_Phone_Book_Request_t *PullPhoneBookRequest;
   unsigned int                    StringLength;
   unsigned int                    MessageLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         if(PhoneBookNamePath)
            StringLength = BTPS_StringLength(PhoneBookNamePath) + 1;
         else
            StringLength = 0;

         MessageLength = PBAM_PULL_PHONE_BOOK_REQUEST_SIZE(StringLength);

         if((PullPhoneBookRequest = (PBAM_Pull_Phone_Book_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a Pull Phone   */
            /* Book message and send it to the server.                  */
            BTPS_MemInitialize(PullPhoneBookRequest, 0, PBAM_PULL_PHONE_BOOK_REQUEST_SIZE(StringLength));

            PullPhoneBookRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            PullPhoneBookRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            PullPhoneBookRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            PullPhoneBookRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK;
            PullPhoneBookRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            PullPhoneBookRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            PullPhoneBookRequest->FilterLow                     = FilterLow;
            PullPhoneBookRequest->FilterHigh                    = FilterHigh;
            PullPhoneBookRequest->VCardFormat                   = Format;
            PullPhoneBookRequest->MaxListCount                  = MaxListCount;
            PullPhoneBookRequest->ListStartOffset               = ListStartOffset;

            if(StringLength)
            {
               BTPS_MemCopy(PullPhoneBookRequest->PhoneBookNamePath, PhoneBookNamePath, StringLength);

               PullPhoneBookRequest->PhoneBookNamePathSize = StringLength;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)PullPhoneBookRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_RESPONSE_SIZE)
                  ret_val = ((PBAM_Pull_Phone_Book_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a PBAP Pull Phone Book request to*/
   /* the specified remote PBAP server to request the size of the remote*/
   /* phone book. The Bluetooth Address parameter specifies the PBAP    */
   /* client. This function returns zero if successful or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current request is completed.           */
int _PBAM_Pull_Phone_Book_Size(BD_ADDR_t RemoteDeviceAddress)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   PBAM_Pull_Phone_Book_Size_Request_t  PullPhoneBookSize;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* All that we really need to do is to build a Pull Phone Book */
         /* Size message and send it to the server.                     */
         BTPS_MemInitialize(&PullPhoneBookSize, 0, PBAM_PULL_PHONE_BOOK_SIZE_REQUEST_SIZE);

         PullPhoneBookSize.MessageHeader.AddressID       = MSG_GetServerAddressID();
         PullPhoneBookSize.MessageHeader.MessageID       = MSG_GetNextMessageID();
         PullPhoneBookSize.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         PullPhoneBookSize.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE;
         PullPhoneBookSize.MessageHeader.MessageLength   = PBAM_PULL_PHONE_BOOK_SIZE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         PullPhoneBookSize.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&PullPhoneBookSize, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) == PBAM_PULL_PHONE_BOOK_SIZE_RESPONSE_SIZE)
               ret_val = ((PBAM_Pull_Phone_Book_Size_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a PBAP Set Phonebook Request to  */
   /* the specified remote PBAP Server.  The first parameter specifies  */
   /* the remote, connected, device address of the device to issue the  */
   /* request on.  The PathOption parameter contains an enumerated value*/
   /* that indicates the type of path change to request.  The FolderName*/
   /* parameter contains the folder name to include with this Set       */
   /* Phonebook request.  This value can be NULL if no name is required */
   /* for the selected PathOption.  See the PBAP specification for more */
   /* information.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
int _PBAM_Set_Phone_Book_Request(BD_ADDR_t RemoteDeviceAddress, PBAM_Set_Path_Option_t PathOption, char *FolderName)
{
   int                            ret_val;
   BTPM_Message_t                *ResponseMessage;
   PBAM_Set_Phone_Book_Request_t *SetPhoneBookRequest;
   unsigned int                   StringLength;
   unsigned int                   MessageLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         if(FolderName)
            StringLength = BTPS_StringLength(FolderName) + 1;
         else
            StringLength = 0;

         MessageLength = PBAM_SET_PHONE_BOOK_REQUEST_SIZE(StringLength);

         if((SetPhoneBookRequest = (PBAM_Set_Phone_Book_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a Set Phone    */
            /* Book message and send it to the server.                  */
            BTPS_MemInitialize(SetPhoneBookRequest, 0, PBAM_SET_PHONE_BOOK_REQUEST_SIZE(StringLength));

            SetPhoneBookRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SetPhoneBookRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SetPhoneBookRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            SetPhoneBookRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK;
            SetPhoneBookRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            SetPhoneBookRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            SetPhoneBookRequest->PathOption                    = PathOption;

            if(StringLength)
            {
               BTPS_MemCopy(SetPhoneBookRequest->FolderName, FolderName, StringLength);

               SetPhoneBookRequest->FolderNameSize = StringLength;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SetPhoneBookRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_SIZE_RESPONSE_SIZE)
                  ret_val = ((PBAM_Set_Phone_Book_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a PBAP Pull vCard Listing Request*/
   /* to the specified remote PBAP Server. The RemoteDeviceAddress      */
   /* parameter specifies the connected device for the request. The     */
   /* PhonebookPath Parameter specifies the name of the phonebook to    */
   /* pull the listing from. The ListOrder parameter is an enumerated   */
   /* type that determines the optionally requested order of listing.   */
   /* Using 'loDefault' will prevent the field from being added. The    */
   /* SearchAttribute parameter is an enumerated type that specifies    */
   /* the requested attribute to be used as a search filter. The        */
   /* SearchValue contains an optional ASCII string that contains the   */
   /* requested search value. If this is NULL, it will be excluded. The */
   /* MaxListCount is an unsigned integer that represents the maximum   */
   /* number of list entries to be returned. The ListStartOffset        */
   /* parameter specifies the index requested. This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
int _PBAM_Pull_vCard_Listing(BD_ADDR_t RemoteDeviceAddress, char *PhonebookPath, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   PBAM_Pull_vCard_Listing_Request_t *PullvCardListingRequest;
   unsigned int                       PhonebookPathLength;
   unsigned int                       SearchValueLength;
   unsigned int                       MessageLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         if(PhonebookPath)
            PhonebookPathLength = BTPS_StringLength(PhonebookPath) + 1;
         else
            PhonebookPathLength = 0;

         if(SearchValue)
            SearchValueLength = BTPS_StringLength(SearchValue) + 1;
         else
            SearchValueLength = 0;

         MessageLength = PBAM_PULL_VCARD_LISTING_REQUEST_SIZE(PhonebookPathLength, SearchValueLength);

         if((PullvCardListingRequest = (PBAM_Pull_vCard_Listing_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a Pull vCard   */
            /* Listing message and send it to the server.               */
            BTPS_MemInitialize(PullvCardListingRequest, 0, PBAM_PULL_VCARD_LISTING_REQUEST_SIZE(PhonebookPathLength, SearchValueLength));

            PullvCardListingRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            PullvCardListingRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            PullvCardListingRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            PullvCardListingRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING;
            PullvCardListingRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            PullvCardListingRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            PullvCardListingRequest->ListOrder                     = ListOrder;
            PullvCardListingRequest->SearchAttribute               = SearchAttribute;
            PullvCardListingRequest->MaxListCount                  = MaxListCount;
            PullvCardListingRequest->ListStartOffset               = ListStartOffset;

            if(PhonebookPathLength)
            {
               BTPS_MemCopy(PullvCardListingRequest->VariableData, PhonebookPath, PhonebookPathLength);

               PullvCardListingRequest->PhonebookPathSize = PhonebookPathLength;
            }

            if(SearchValueLength)
            {
               BTPS_MemCopy(PullvCardListingRequest->VariableData+PhonebookPathLength, SearchValue, SearchValueLength);

               PullvCardListingRequest->SearchValueSize = SearchValueLength;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)PullvCardListingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_LISTING_RESPONSE_SIZE)
                  ret_val = ((PBAM_Pull_vCard_Listing_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function generates a PBAP Pull vCard Entry Request  */
   /* to the specified remote PBAP Server. The RemoteDeviceAddress      */
   /* Parameter specifies the connected device for the request. The     */
   /* vCardName parameter is an ASCII string representing the name of   */
   /* the vCard to be pulled in the request. The FilterLow parameter    */
   /* contains the lower 32 bits of the 64-bit filter attribute. The    */
   /* FilterHigh parameter contains the higher 32 bits of the 64-bit    */
   /* filter attribute. The Format parameter is an enumeration which    */
   /* specifies the format of the vCard requested. This function returns*/
   /* zero if successful and a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
int _PBAM_Pull_vCard(BD_ADDR_t RemoteDeviceAddress, char *VCardName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat)
{
   int                        ret_val;
   BTPM_Message_t            *ResponseMessage;
   PBAM_Pull_vCard_Request_t *PullvCardRequest;
   unsigned int               StringLength;
   unsigned int               MessageLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         if(VCardName)
            StringLength = BTPS_StringLength(VCardName) + 1;
         else
            StringLength = 0;

         MessageLength = PBAM_PULL_VCARD_REQUEST_SIZE(StringLength);

         if((PullvCardRequest = (PBAM_Pull_vCard_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a Set Phone    */
            /* Book message and send it to the server.                  */
            BTPS_MemInitialize(PullvCardRequest, 0, PBAM_PULL_VCARD_REQUEST_SIZE(StringLength));

            PullvCardRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            PullvCardRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            PullvCardRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            PullvCardRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_VCARD;
            PullvCardRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            PullvCardRequest->RemoteDeviceAddress           = RemoteDeviceAddress;
            PullvCardRequest->FilterLow                     = FilterLow;
            PullvCardRequest->FilterHigh                    = FilterHigh;
            PullvCardRequest->VCardFormat                   = VCardFormat;

            if(StringLength)
            {
               BTPS_MemCopy(PullvCardRequest->VCardName, VCardName, StringLength);

               PullvCardRequest->VCardNameSize = StringLength;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)PullvCardRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_RESPONSE_SIZE)
                  ret_val = ((PBAM_Pull_vCard_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function wraps PBAP Set Phone Book Requests in order*/
   /* to supply an absolute path to change to. The RemoteDeviceAddress  */
   /* parameter specifies the connected device for the request. The     */
   /* AbsolutePath parameter is an ASCII string containing the path to  */
   /* set the phone book to. This function returns zero if successful   */
   /* and a negative return error code if there was and error.          */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * If there is an error while processing the series of      */
   /*          requests, a petPhoneBookSetEvent will be sent containing */
   /*          the path before the failure occurred. This will can be   */
   /*          assumed to be the current path.                          */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
int _PBAM_Set_Phone_Book_Absolute(BD_ADDR_t RemoteDeviceAddress, char *Path)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   PBAM_Set_Phone_Book_Absolute_Request_t *SetPhoneBookAbsoluteRequest;
   unsigned int                            StringLength;
   unsigned int                            MessageLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         if(Path)
            StringLength = BTPS_StringLength(Path) + 1;
         else
            StringLength = 0;

         MessageLength = PBAM_SET_PHONE_BOOK_ABSOLUTE_REQUEST_SIZE(StringLength);

         if((SetPhoneBookAbsoluteRequest = (PBAM_Set_Phone_Book_Absolute_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a Set Phone    */
            /* Book message and send it to the server.                  */
            BTPS_MemInitialize(SetPhoneBookAbsoluteRequest, 0, PBAM_SET_PHONE_BOOK_ABSOLUTE_REQUEST_SIZE(StringLength));

            SetPhoneBookAbsoluteRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SetPhoneBookAbsoluteRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SetPhoneBookAbsoluteRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            SetPhoneBookAbsoluteRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_ABSOLUTE;
            SetPhoneBookAbsoluteRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            SetPhoneBookAbsoluteRequest->RemoteDeviceAddress           = RemoteDeviceAddress;

            if(StringLength)
            {
               BTPS_MemCopy(SetPhoneBookAbsoluteRequest->Path, Path, StringLength);

               SetPhoneBookAbsoluteRequest->PathSize = StringLength;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SetPhoneBookAbsoluteRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_ABSOLUTE_RESPONSE_SIZE)
                  ret_val = ((PBAM_Set_Phone_Book_Absolute_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Register a PBAP Server Port. The ServerPort parameter specifies   */
   /* the RFCOMM port number on which to open the server. The           */
   /* SupportedRepositories parameter is a bit mask of the supported    */
   /* local contact database types. The IncomingConnectionFlags         */
   /* parameter is a bitmask that determine how to handle incoming      */
   /* connection requests to the server port. The ServiceName parameter */
   /* is the service name to insert into the SDP record for the         */
   /* server. The EventCallback parameter is the callback function      */
   /* that will receive asynchronous events for this server. The        */
   /* CallbackParameter will be passed to the EventCallback when events */
   /* are dispatched. On success, this function returns a positive,     */
   /* non-zero value representing the ServerID for the newly opened     */
   /* server. On failure, this function returns a negative error code.  */
   /* * NOTE * Supplying a ServerPort of 0 will cause this function to  */
   /*          automatically pick an available port number.             */
int _PBAM_Register_Server(unsigned int ServerPort, unsigned int SupportedRepositories, unsigned long IncomingConnectionFlags, char *ServiceName)
{
   int                             ret_val;
   unsigned int                    StringLength;
   unsigned int                    MessageLength;
   BTPM_Message_t                 *ResponseMessage;
   PBAM_Register_Server_Request_t *RegisterServerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if((ServerPort) && (ServiceName))
      {
         StringLength = BTPS_StringLength(ServiceName) + 1;

         MessageLength = PBAM_REGISTER_SERVER_REQUEST_SIZE(StringLength);

         if((RegisterServerRequest = (PBAM_Register_Server_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a request      */
            /* message and send it to the server.                       */
            BTPS_MemInitialize(RegisterServerRequest, 0, MessageLength);

            RegisterServerRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            RegisterServerRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            RegisterServerRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            RegisterServerRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_REGISTER_SERVER;
            RegisterServerRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            RegisterServerRequest->ServerPort                    = ServerPort;
            RegisterServerRequest->SupportedRepositories         = SupportedRepositories;
            RegisterServerRequest->IncomingConnectionFlags       = IncomingConnectionFlags;

            if(StringLength)
            {
               BTPS_MemCopy(RegisterServerRequest->ServiceName, ServiceName, StringLength);

               RegisterServerRequest->ServiceNameLength = StringLength;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)RegisterServerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_REGISTER_SERVER_RESPONSE_SIZE)
                  ret_val = ((PBAM_Register_Server_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Unregisters a previously opened PBAP server port. The ServerID    */
   /* parameter is the ID of the server returned from a successful      */
   /* call to _PBAM_Register_Server(). This fuction returns zero if     */
   /* successful or a negative return error code if there was an error. */
int _PBAM_Un_Register_Server(unsigned int ServerID)
{
   int                                ret_val;
   BTPM_Message_t                    *ResponseMessage;
   PBAM_Un_Register_Server_Request_t  UnRegisterServerRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ServerID)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&UnRegisterServerRequest, 0, PBAM_UN_REGISTER_SERVER_REQUEST_SIZE);

         UnRegisterServerRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         UnRegisterServerRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         UnRegisterServerRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         UnRegisterServerRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_UN_REGISTER_SERVER;
         UnRegisterServerRequest.MessageHeader.MessageLength   = PBAM_UN_REGISTER_SERVER_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         UnRegisterServerRequest.ServerID                      = ServerID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&UnRegisterServerRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_UN_REGISTER_SERVER_RESPONSE_SIZE)
               ret_val = ((PBAM_Un_Register_Server_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to an outstanding connection request to the local         */
   /* server. The ConnectionID is the indentifier of the connection     */
   /* request returned in a petConnectionRequest event. The Accept      */
   /* parameter indicates whether the connection should be accepted or  */
   /* rejected. This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _PBAM_Connection_Request_Response(unsigned int ConnectionID, Boolean_t Accept)
{
   int                                         ret_val;
   BTPM_Message_t                             *ResponseMessage;
   PBAM_Connection_Request_Response_Request_t  ConnectionRequestResposneRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&ConnectionRequestResposneRequest, 0, PBAM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE);

         ConnectionRequestResposneRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         ConnectionRequestResposneRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionRequestResposneRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         ConnectionRequestResposneRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE;
         ConnectionRequestResposneRequest.MessageHeader.MessageLength   = PBAM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ConnectionRequestResposneRequest.ConnectionID                  = ConnectionID;
         ConnectionRequestResposneRequest.Accept                        = Accept;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&ConnectionRequestResposneRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE)
               ret_val = ((PBAM_Connection_Request_Response_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Close an active connection to a local PBAP Server instance. The   */
   /* ConnectionID parameter is the identifier of the connection        */
   /* returned in a petConnected event. This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _PBAM_Close_Server_Connection(unsigned int ConnectionID)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   PBAM_Close_Server_Connection_Request_t  CloseServerConnectionRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&CloseServerConnectionRequest, 0, PBAM_CLOSE_SERVER_CONNECTION_REQUEST_SIZE);

         CloseServerConnectionRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         CloseServerConnectionRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         CloseServerConnectionRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         CloseServerConnectionRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_CLOSE_SERVER_CONNECTION;
         CloseServerConnectionRequest.MessageHeader.MessageLength   = PBAM_CLOSE_SERVER_CONNECTION_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         CloseServerConnectionRequest.ConnectionID                  = ConnectionID;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&CloseServerConnectionRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_CLOSE_SERVER_CONNECTION_RESPONSE_SIZE)
               ret_val = ((PBAM_Close_Server_Connection_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received etPullPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the active connection */
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes. If the request */
   /* event indicated the 'mch' phonebook, the NewMissedCalls parameter */
   /* should be a pointer to the value of the number of missed calls    */
   /* since the last 'mch' pull request. If it is not an 'mch' request, */
   /* this parameter should be set to NULL. The BufferSize parameter    */
   /* indicates the amount of data in the buffer to be sent. The Buffer */
   /* parameter is a pointer to the phone book data to send. The Final  */
   /* parameter should be set to FALSE if there is more data to be sent */
   /* after this buffer or TRUE if there is no more data. This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _PBAM_Send_Phone_Book(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int                             ret_val;
   unsigned int                    MessageLength;
   BTPM_Message_t                 *ResponseMessage;
   PBAM_Send_Phone_Book_Request_t *SendPhoneBookRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         MessageLength = PBAM_SEND_PHONE_BOOK_REQUEST_SIZE(BufferSize);

         if((SendPhoneBookRequest = (PBAM_Send_Phone_Book_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a request      */
            /* message and send it to the server.                       */
            BTPS_MemInitialize(SendPhoneBookRequest, 0, MessageLength);

            SendPhoneBookRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendPhoneBookRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendPhoneBookRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            SendPhoneBookRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK;
            SendPhoneBookRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            SendPhoneBookRequest->ConnectionID                  = ConnectionID;
            SendPhoneBookRequest->ResponseStatusCode            = ResponseStatusCode;
            SendPhoneBookRequest->IncludeMissedCalls            = NewMissedCalls?TRUE:FALSE;
            SendPhoneBookRequest->NewMissedCalls                = NewMissedCalls?*NewMissedCalls:0;
            SendPhoneBookRequest->ResponseStatusCode            = ResponseStatusCode;
            SendPhoneBookRequest->Final                         = Final;

            if(BufferSize)
            {
               BTPS_MemCopy(SendPhoneBookRequest->Buffer, Buffer, BufferSize);

               SendPhoneBookRequest->BufferSize = BufferSize;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendPhoneBookRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SEND_PHONE_BOOK_RESPONSE_SIZE)
                  ret_val = ((PBAM_Send_Phone_Book_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received etPullPhoneBookSize event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode          */
   /* parameter is one of the defined _PBAM Response Status codes. The  */
   /* PhonebookSize parameter indicates the number of entries in the    */
   /* requested phone book. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int _PBAM_Send_Phone_Book_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int PhoneBookSize)
{
   int                                  ret_val;
   BTPM_Message_t                      *ResponseMessage;
   PBAM_Send_Phone_Book_Size_Request_t  SendPhoneBookSizeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&SendPhoneBookSizeRequest, 0, PBAM_SEND_PHONE_BOOK_SIZE_REQUEST_SIZE);

         SendPhoneBookSizeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendPhoneBookSizeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendPhoneBookSizeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         SendPhoneBookSizeRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK_SIZE;
         SendPhoneBookSizeRequest.MessageHeader.MessageLength   = PBAM_SEND_PHONE_BOOK_SIZE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendPhoneBookSizeRequest.ConnectionID                  = ConnectionID;
         SendPhoneBookSizeRequest.ResponseStatusCode            = ResponseStatusCode;
         SendPhoneBookSizeRequest.PhoneBookSize                 = PhoneBookSize;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendPhoneBookSizeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SEND_PHONE_BOOK_SIZE_RESPONSE_SIZE)
               ret_val = ((PBAM_Send_Phone_Book_Size_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petSetPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes. This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PBAM_Set_Phone_Book_Response(unsigned int ConnectionID, unsigned int ResponseStatusCode)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   PBAM_Set_Phone_Book_Response_Request_t  SetPhoneBookResponseRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&SetPhoneBookResponseRequest, 0, PBAM_SET_PHONE_BOOK_RESPONSE_REQUEST_SIZE);

         SetPhoneBookResponseRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SetPhoneBookResponseRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SetPhoneBookResponseRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         SetPhoneBookResponseRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_RESPONSE;
         SetPhoneBookResponseRequest.MessageHeader.MessageLength   = PBAM_SET_PHONE_BOOK_RESPONSE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SetPhoneBookResponseRequest.ConnectionID                  = ConnectionID;
         SetPhoneBookResponseRequest.ResponseStatusCode            = ResponseStatusCode;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SetPhoneBookResponseRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_RESPONSE_RESPONSE_SIZE)
               ret_val = ((PBAM_Set_Phone_Book_Response_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petPullvCardListing event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes.  If the request*/
   /* event indicated the 'mch' phonebook, the NewMissedCalls parameter */
   /* should be a pointer to the value of the number of missed calls    */
   /* since the last 'mch' pull request. If it is not an 'mch' request, */
   /* this parameter should be set to NULL. The BufferSize parameter    */
   /* indicates the amount of data in the buffer to be sent. The Buffer */
   /* parameter is a pointer to the vCardListing data to send. The Final*/
   /* parameter should be set to FALSE if there is more data to be sent */
   /* after this buffer or TRUE if there is no more data. This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _PBAM_Send_vCard_Listing(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int                                ret_val;
   unsigned int                       MessageLength;
   BTPM_Message_t                    *ResponseMessage;
   PBAM_Send_vCard_Listing_Request_t *SendvCardListingRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         MessageLength = PBAM_SEND_VCARD_LISTING_REQUEST_SIZE(BufferSize);

         if((SendvCardListingRequest = (PBAM_Send_vCard_Listing_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a request      */
            /* message and send it to the server.                       */
            BTPS_MemInitialize(SendvCardListingRequest, 0, MessageLength);

            SendvCardListingRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendvCardListingRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendvCardListingRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            SendvCardListingRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING;
            SendvCardListingRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            SendvCardListingRequest->ConnectionID                  = ConnectionID;
            SendvCardListingRequest->ResponseStatusCode            = ResponseStatusCode;
            SendvCardListingRequest->IncludeMissedCalls            = NewMissedCalls?TRUE:FALSE;
            SendvCardListingRequest->NewMissedCalls                = NewMissedCalls?*NewMissedCalls:0;
            SendvCardListingRequest->Final                         = Final;

            if(BufferSize)
            {
               BTPS_MemCopy(SendvCardListingRequest->Buffer, Buffer, BufferSize);

               SendvCardListingRequest->BufferSize = BufferSize;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendvCardListingRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_LISTING_RESPONSE_SIZE)
                  ret_val = ((PBAM_Send_vCard_Listing_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petPullvCardListingSize          */
   /* event. The ConnectionID parameter is the identifier of the        */
   /* activer connection returned in a petConnected event. The          */
   /* ResponseStatusCode parameter is one of the defined _PBAM Response */
   /* Status codes. The vCardListingSize parameter indicates the number */
   /* of vCard entries in the current/specfied folder. This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PBAM_Send_vCard_Listing_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int vCardListingSize)
{
   int                                     ret_val;
   BTPM_Message_t                         *ResponseMessage;
   PBAM_Send_vCard_Listing_Size_Request_t  SendvCardListingSizeRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         /* All that we really need to do is to build a request message */
         /* and send it to the server.                                  */
         BTPS_MemInitialize(&SendvCardListingSizeRequest, 0, PBAM_SEND_VCARD_LISTING_SIZE_REQUEST_SIZE);

         SendvCardListingSizeRequest.MessageHeader.AddressID       = MSG_GetServerAddressID();
         SendvCardListingSizeRequest.MessageHeader.MessageID       = MSG_GetNextMessageID();
         SendvCardListingSizeRequest.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         SendvCardListingSizeRequest.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING_SIZE;
         SendvCardListingSizeRequest.MessageHeader.MessageLength   = PBAM_SEND_VCARD_LISTING_SIZE_REQUEST_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         SendvCardListingSizeRequest.ConnectionID                  = ConnectionID;
         SendvCardListingSizeRequest.ResponseStatusCode            = ResponseStatusCode;
         SendvCardListingSizeRequest.vCardListingSize              = vCardListingSize;

         /* Message has been formatted, go ahead and send it off.       */
         if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)&SendvCardListingSizeRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
         {
            /* Response received, go ahead and see if the return value  */
            /* is valid. If it is, go ahead and note the returned       */
            /* status.                                                  */
            if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_LISTING_SIZE_RESPONSE_SIZE)
               ret_val = ((PBAM_Send_vCard_Listing_Size_Response_t *)ResponseMessage)->Status;
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

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petPullvCard event. The          */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes.  The BufferSize*/
   /* parameter indicates the amount of data in the buffer to be        */
   /* sent. The Buffer parameter is a pointer to the vCard data to      */
   /* send. The Final parameter should be set to FALSE if there is more */
   /* data to be sent after this buffer or TRUE if there is no more     */
   /* data. This function returns zero if successful and a negative     */
   /* return error code if there was an error.                          */
int _PBAM_Send_vCard(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int                        ret_val;
   unsigned int               MessageLength;
   BTPM_Message_t            *ResponseMessage;
   PBAM_Send_vCard_Request_t *SendvCardRequest;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First check to make sure that this module has been initialized.   */
   if(Initialized)
   {
      /* Verify that the input parameters appear to be semi-valid.      */
      if(ConnectionID)
      {
         MessageLength = PBAM_SEND_VCARD_REQUEST_SIZE(BufferSize);

         if((SendvCardRequest = (PBAM_Send_vCard_Request_t *)BTPS_AllocateMemory(MessageLength)) != NULL)
         {
            /* All that we really need to do is to build a request      */
            /* message and send it to the server.                       */
            BTPS_MemInitialize(SendvCardRequest, 0, MessageLength);

            SendvCardRequest->MessageHeader.AddressID       = MSG_GetServerAddressID();
            SendvCardRequest->MessageHeader.MessageID       = MSG_GetNextMessageID();
            SendvCardRequest->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            SendvCardRequest->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SEND_VCARD;
            SendvCardRequest->MessageHeader.MessageLength   = MessageLength - BTPM_MESSAGE_HEADER_SIZE;

            SendvCardRequest->ConnectionID                  = ConnectionID;
            SendvCardRequest->ResponseStatusCode            = ResponseStatusCode;
            SendvCardRequest->Final                         = Final;

            if(BufferSize)
            {
               BTPS_MemCopy(SendvCardRequest->Buffer, Buffer, BufferSize);

               SendvCardRequest->BufferSize = BufferSize;
            }

            /* Message has been formatted, go ahead and send it off.    */
            if((!(ret_val = MSG_SendMessageResponse((BTPM_Message_t *)SendvCardRequest, MAXIMUM_GENERAL_MESSAGE_RESPONSE_TIMEOUT, &ResponseMessage))) && (ResponseMessage))
            {
               /* Response received, go ahead and see if the return     */
               /* value is valid. If it is, go ahead and note the       */
               /* returned status.                                      */
               if(BTPM_MESSAGE_SIZE(ResponseMessage->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_RESPONSE_SIZE)
                  ret_val = ((PBAM_Send_vCard_Response_t *)ResponseMessage)->Status;
               else
                  ret_val = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

               /* All finished with the message, go ahead and free it.  */
               MSG_FreeReceivedMessageGroupHandlerMessage(ResponseMessage);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
