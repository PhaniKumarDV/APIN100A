/*****< blpmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLPMGR - BLP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMBLPM.h"            /* BTPM BLP Manager Prototypes/Constants.    */
#include "BLPMGR.h"              /* BLP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _BLPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variable which holds the GATT Connection Event Callback ID.       */
static unsigned int CallbackID;

   /* Internal Function Prototypes.                                     */
static int MAPGATTErrorCode(int ErrorCodeToMap);

static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Client_Event_Callback(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);

   /* The following function is a utility function that is used to map a*/
   /* GATT error code to a PM error code.  The caller MUST ensure that  */
   /* an error has occurred before calling this function.               */
static int MAPGATTErrorCode(int ErrorCodeToMap)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: GATT Error Code = %d\n", ErrorCodeToMap));

   switch(ErrorCodeToMap)
   {
      case BTPS_ERROR_LOCAL_CONTROLLER_DOES_NOT_SUPPORT_LE:
      case BTPS_ERROR_FEATURE_NOT_AVAILABLE:
      case BTGATT_ERROR_NOT_INITIALIZED:
         ret_val = BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_NOT_SUPPORTED;
         break;
      case BTGATT_ERROR_INVALID_PARAMETER:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
      case BTGATT_ERROR_INVALID_SERVICE_TABLE_FORMAT:
         ret_val = BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_SERVICE_TABLE;
         break;
      case BTGATT_ERROR_INVALID_CONNECTION_ID:
         ret_val = BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_CONNECTION;
         break;
      case BTGATT_ERROR_INVALID_HANDLE_VALUE:
         ret_val = BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_ATTRIBUTE_HANDLE;
         break;
      default:
         ret_val = BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_OPERATION;
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: GATM Error Code = %d\n", ret_val));

   return(ret_val);
}

   /* The following declared type is the GATT connection event callback.*/
   /* This function will be called whenever a GATT connection Event     */
   /* occurs that is associated with the specified Bluetooth stack ID.  */
   /* This function passes to the caller the Bluetooth stack ID, the    */
   /* GATT event data that occurred and the GATT event callback         */
   /* parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the GATT event data ONLY*/
   /* in the context of this callback.  If the caller requires the data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another data buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It needs to be noted, however, that if the same      */
   /* callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the thread context of a thread that the*/
   /* user does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GATT Event will not be processed while this function call */
   /* is outstanding).                                                  */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by receiving GATT events.  A     */
   /*            deadlock WILL occur because NO GATT event callbacks    */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter)
{
   BLPM_Update_Data_t *BLPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((BluetoothStackID) && (GATT_Connection_Event_Data))
   {
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case etGATT_Connection_Server_Notification:
            /* Go ahead and dispatch the Server Notification            */
            /* information.                                             */
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
            {
               /* Go ahead and allocate the memory to hold the event.   */
               if((BLPMUpdateData = (BLPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(BLPM_Update_Data_t) + GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength)) != NULL)
               {
                  /* Initialize the response information.               */
                  BLPMUpdateData->UpdateType = utGATTNotificationEvent;

                  /* Format the event structure.                        */
                  BLPMUpdateData->UpdateData.GATTServerNotificationData = *(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data);

                  if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength)
                  {
                     BLPMUpdateData->UpdateData.GATTServerNotificationData.AttributeValue = ((Byte_t *)BLPMUpdateData) + sizeof(BLPM_Update_Data_t);

                     /* Copy the Notification data into the new buffer. */
                     BTPS_MemCopy(BLPMUpdateData->UpdateData.GATTServerNotificationData.AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength);
                  }
                  else
                     BLPMUpdateData->UpdateData.GATTServerNotificationData.AttributeValue = NULL;

                  /* If there is an event to dispatch, go ahead and     */
                  /* dispatch it.                                       */
                  if(!BLPM_NotifyUpdate(BLPMUpdateData))
                  {
                     /* There was an error with either parsing or       */
                     /* dispatching the event.  Free all memory         */
                     /* associated with this transaction.               */
                     BTPS_FreeMemory(BLPMUpdateData);
                  }
               }
            }
            break;
         case etGATT_Connection_Server_Indication:
            /* Go ahead and dispatch the Server indication information. */
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data)
            {
               /* Go ahead and allocate the memory to hold the event.   */
               if((BLPMUpdateData = (BLPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(BLPM_Update_Data_t) + GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength)) != NULL)
               {
                  /* Initialize the response information.               */
                  BLPMUpdateData->UpdateType = utGATTIndicationEvent;

                  /* Format the event structure.                        */
                  BLPMUpdateData->UpdateData.GATTServerIndicationData = *(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data);

                  if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength)
                  {
                     BLPMUpdateData->UpdateData.GATTServerIndicationData.AttributeValue = ((Byte_t *)BLPMUpdateData) + sizeof(BLPM_Update_Data_t);

                     /* Copy the Notification data into the new buffer. */
                     BTPS_MemCopy(BLPMUpdateData->UpdateData.GATTServerIndicationData.AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength);
                  }
                  else
                     BLPMUpdateData->UpdateData.GATTServerIndicationData.AttributeValue = NULL;

                  /* If there is an event to dispatch, go ahead and     */
                  /* dispatch it.                                       */
                  if(!BLPM_NotifyUpdate(BLPMUpdateData))
                  {
                     /* There was an error with either parsing or       */
                     /* dispatching the event.  Free all memory         */
                     /* associated with this transaction.               */
                     BTPS_FreeMemory(BLPMUpdateData);
                  }
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following declared type is the GATT Client event callback.    */
   /* This function will be called whenever a GATT Client Event occurs  */
   /* that is associated with the specified Bluetooth stack ID.  This   */
   /* function passes to the caller the Bluetooth stack ID, the GATT    */
   /* event data that occurred and the GATT event callback parameter    */
   /* that was specified when this Callback was installed.  The caller  */
   /* is free to use the contents of the GATT event data ONLY in the    */
   /* context of this callback.  If the caller requires the data for a  */
   /* longer period of time, then the callback function MUST copy the   */
   /* data into another data buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It needs to be noted, however, that if the same      */
   /* callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the thread context of a thread that the*/
   /* user does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GATT Event will not be processed while this function call */
   /* is outstanding).                                                  */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by receiving GATT events.  A     */
   /*            deadlock WILL occur because NO GATT event callbacks    */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI GATT_Client_Event_Callback(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   BLPM_Update_Data_t *BLPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((GATT_Client_Event_Data) && (GATT_Client_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      BLPMUpdateData = NULL;

      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
         case etGATT_Client_Write_Response:
         case etGATT_Client_Read_Response:
            /* Allocate memory to hold th e Event Data (we will process */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((GATT_Client_Event_Data->Event_Data_Size) && ((BLPMUpdateData = (BLPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(BLPM_Update_Data_t) + GATT_Client_Event_Data->Event_Data_Size)) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               BLPMUpdateData->UpdateType                                     = utGATTClientEvent;
               BLPMUpdateData->UpdateData.GATTClientEventData.Event_Data_Type = GATT_Client_Event_Data->Event_Data_Type;
               BLPMUpdateData->UpdateData.GATTClientEventData.Event_Data_Size = GATT_Client_Event_Data->Event_Data_Size;

               /* Copy over the data buffer if it exists.               */
               if(GATT_Client_Event_Data->Event_Data_Size)
               {
                  /* Since all event data items are a union of pointers,*/
                  /* just populate one of the pointers.                 */
                  BLPMUpdateData->UpdateData.GATTClientEventData.Event_Data.GATT_Request_Error_Data = (GATT_Request_Error_Data_t *)(((Byte_t *)BLPMUpdateData) + sizeof(BLPM_Update_Data_t));

                  BTPS_MemCopy((void *)BLPMUpdateData->UpdateData.GATTClientEventData.Event_Data.GATT_Request_Error_Data, (void *)GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data, GATT_Client_Event_Data->Event_Data_Size);
               }
               else
                  BLPMUpdateData->UpdateData.GATTClientEventData.Event_Data.GATT_Request_Error_Data = NULL;
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(BLPMUpdateData)
      {
         if(!BLPM_NotifyUpdate(BLPMUpdateData))
            BTPS_FreeMemory((void *)BLPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the BLP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager BLP Manager  */
   /* Implementation.                                                   */
int _BLPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing BLP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALREADY_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the BLP   */
   /* Manager Implementation.  After this function is called the BLP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _BLPM_Initialize() function.  */
void _BLPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the BLP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the BLP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _BLPM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         if((!_BluetoothStackID) && (!CallbackID))
         {
            /* Register a GATT Connection Event Callback.               */
            if((CallbackID = GATT_Register_Connection_Events(BluetoothStackID, GATT_Connection_Event_Callback, 0)) > 0)
            {
               /* ANP Framework initialized successfully.               */
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("BLP Framework Initialized\n"));

               /* Save the Bluetooth Stack ID.                          */
               _BluetoothStackID = BluetoothStackID;
            }
            else
            {
               /* Error initializing ANP Framework.                     */
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT Connection Events NOT registered: %d\n", CallbackID));

               CallbackID = 0;
            }
         }
         else
         {
            /* ANP Framework initialized successfully.                  */
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Cannot Re-initialize BLP Framework until clean-up\n"));
         }
      }
      else
      {
         if((_BluetoothStackID) && (CallbackID))
         {
            GATT_Un_Register_Connection_Events(_BluetoothStackID, CallbackID);

            /* Flag that the Bluetooth Stack ID is not longer valid.    */
            _BluetoothStackID = 0;

            CallbackID        = 0;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to read an Attribute Handle on a remote device.  The      */
   /* function accepts the Bluetooth Address of the remote device and   */
   /* the Attribute Handle of the attribute on the remote device to     */
   /* read.  This function returns a positive non-zero Transaction ID if*/
   /* successful, or a negative return error code if there was an error.*/
int _BLPM_Read_Value(BD_ADDR_t *RemoteServer, Word_t Handle)
{
   unsigned int ConnectionID;
   int          ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate the handle before submitting the request.                */
   if(Handle)
   {
      /* Validate that there is an active connection to this device.    */
      if((GATT_Query_Connection_ID(_BluetoothStackID, gctLE, *RemoteServer, &ConnectionID)) == 0)
      {
         /* Submit the request to GATT.                                 */
         if((ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, Handle, GATT_Client_Event_Callback, Handle)) <= 0)
            ret_val = MAPGATTErrorCode(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_ATTRIBUTE_HANDLE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to write to an Attribute Handle on a remote device.  The  */
   /* function accepts the Bluetooth Address of the remote device, the  */
   /* Attribute Handle of the attribute on the remote device to write,  */
   /* the length of the data, and a pointer to the data that will be    */
   /* written.  This function returns a positive non-zero Transaction ID*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
int _BLPM_Write_Value(BD_ADDR_t *RemoteServer, Word_t Handle, Word_t DataLength, Byte_t *Data)
{
   unsigned int ConnectionID;
   int          ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate the handle before submitting the request.                */
   if(Handle)
   {
      /* Validate that there is an active connection to this device.    */
      if((GATT_Query_Connection_ID(_BluetoothStackID, gctLE, *RemoteServer, &ConnectionID)) == 0)
      {
         /* Submit the request to GATT.                                 */
         if((ret_val = GATT_Write_Request(_BluetoothStackID, ConnectionID, Handle, DataLength, (void *)Data, GATT_Client_Event_Callback, Handle)) <= 0)
            ret_val = MAPGATTErrorCode(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_ATTRIBUTE_HANDLE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following is a utility function that is used to cancel an     */
   /* outstanding transaction.  The first parameter to this function is */
   /* the TransactionID of the transaction that is being canceled.  On  */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
int _BLPM_Cancel_Transaction(unsigned int TransactionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if an error occurred and the error code needs to be  */
   /* mapped to something different.                                    */
   if((ret_val = GATT_Cancel_Transaction(_BluetoothStackID, TransactionID)) < 0)
      ret_val = MAPGATTErrorCode(ret_val);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
