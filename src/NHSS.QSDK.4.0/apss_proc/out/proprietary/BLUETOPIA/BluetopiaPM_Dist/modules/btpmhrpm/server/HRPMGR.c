/*****< hrpmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HRPMGR - HRP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHRPM.h"            /* BTPM HRP Manager Prototypes/Constants.    */
#include "HRPMGR.h"              /* HRP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following enumeration represents all transaction types that   */
   /* can occur in this module.                                         */
typedef enum
{
   ttGetBodySensorLocation,
   ttResetEnergyExpended,
   ttWriteMeasurementCCD
} Transaction_Type_t;

   /* The following structure is used to store transaction and response */
   /* information for the duration of the outstanding transaction.      */
typedef struct _tagTransaction_Entry_t
{
   Transaction_Type_t  TransactionType;
   unsigned int        ConnectionID;
   Word_t              AttributeHandle;
   void               *ResponseData;
   struct _tagTransaction_Entry_t *NextTransaction;
} Transaction_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _HRPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variable which holds the GATT Connection Event Callback ID.       */
static unsigned int CallbackID;

   /* Variable which holds a pointer to the first transaction in the    */
   /* outstanding transaction list.                                     */
static Transaction_Entry_t *TransactionList;

   /* Internal Function Prototypes.                                     */
static int MAPGATTErrorCode(int ErrorCodeToMap);

static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t *EntryToAdd);
static Transaction_Entry_t *SearchTransactionEntry(unsigned int ConnectionID, Word_t AttributeHandle);
static Transaction_Entry_t *DeleteTransactionEntry(unsigned int ConnectionID, Word_t AttributeHandle);
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);
static void FreeTransactionEntryList(void);

static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_HRP(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);

   /* The following function adds the specified Transaction Entry to the*/
   /* module's list.  This function will allocate and add a copy of the */
   /* entry to the list.  This function will return NULL if NO Entry was*/
   /* added.  This can occur if the element passed in was deemed        */
   /* invalid.                                                          */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the Connection ID and Attribute Handle fields are the  */
   /*            same as an entry already in the list.  When this       */
   /*            occurs, this function returns NULL.                    */
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t *EntryToAdd)
{
   Transaction_Entry_t *TransactionEntry;

   /* Iterate the list to check for any duplicate entries.              */
   TransactionEntry = TransactionList;

   while(TransactionEntry)
   {
      if((TransactionEntry->ConnectionID == EntryToAdd->ConnectionID) && (TransactionEntry->AttributeHandle == EntryToAdd->AttributeHandle))
      {
         /* A duplicate has been found.  Invalidate the Connection ID.  */
         EntryToAdd->ConnectionID = 0;

         break;
      }
      else
         TransactionEntry = TransactionEntry->NextTransaction;
   }

   /* If a duplicate was not found, then add the entry to the list.     */
   if(!TransactionEntry)
      TransactionEntry = (Transaction_Entry_t *)BSC_AddGenericListEntry(sizeof(Transaction_Entry_t), ekNone, 0, sizeof(Transaction_Entry_t), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransaction), (void **)(&TransactionList), ((void *)EntryToAdd));
   else
      TransactionEntry = NULL;

   return(TransactionEntry);
}

   /* The following function searches the module's Transaction Entry    */
   /* List for a Transaction Entry based on the specified Connection ID */
   /* and Attribute Handle.  This function returns NULL if the specified*/
   /* Entry was NOT present in the list.                                */
static Transaction_Entry_t *SearchTransactionEntry(unsigned int ConnectionID, Word_t AttributeHandle)
{
   Transaction_Entry_t *TransactionEntry;

   /* Iterate the list to search for the specified entry.               */
   TransactionEntry = TransactionList;

   while(TransactionEntry)
   {
      if((TransactionEntry->ConnectionID == ConnectionID) && (TransactionEntry->AttributeHandle == AttributeHandle))
      {
         /* The entry has been found.                                   */
         break;
      }
      else
         TransactionEntry = TransactionEntry->NextTransaction;
   }

   return(TransactionEntry);
}

   /* The following function searches the module's Transaction Entry    */
   /* List for the Transaction EntryEntry with the specified Connection */
   /* ID and Attribute Handle and removes it from the List.  This       */
   /* function returns NULL if the specified Entry was NOT present in   */
   /* the list.  The entry returned will have the Next Entry field set  */
   /* to NULL, and the caller is responsible for deleting the memory    */
   /* associated with this entry by calling                             */
   /* FreeTransactionEntryMemory().                                     */
static Transaction_Entry_t *DeleteTransactionEntry(unsigned int ConnectionID, Word_t AttributeHandle)
{
   Transaction_Entry_t *TransactionEntry;

   /* First, find the transaction entry.                                */
   if((TransactionEntry = SearchTransactionEntry(ConnectionID, AttributeHandle)) != NULL)
   {
      /* A transaction entry was found, just delete it.                 */
      TransactionEntry = BSC_DeleteGenericListEntry(ekEntryPointer, (void *)TransactionEntry, 0, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransaction), (void **)(&TransactionList));
   }

   return(TransactionEntry);
}

   /* This function frees the specified Transaction Entry member.  The  */
   /* Response Data will also be freed if the pointer is not NULL.  No  */
   /* check is done on this entry other than making sure it NOT NULL.   */
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Transaction Entry List.  The Response Data*/
   /* will for each Transaction Entry will also be freed if the pointer */
   /* is not NULL.  Upon return of this function, the Transaction List  */
   /* Pointer is set to NULL.                                           */
static void FreeTransactionEntryList(void)
{
   Transaction_Entry_t *TransactionEntry;

   /* Iterate the list to free all Response Data.                       */
   TransactionEntry = TransactionList;

   while(TransactionEntry)
   {
      if(TransactionEntry->ResponseData)
         BTPS_FreeMemory(TransactionEntry->ResponseData);

      TransactionEntry = TransactionEntry->NextTransaction;
   }

   BSC_FreeGenericListEntryList((void **)(&TransactionList), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransaction));
}

   /* The following function is a utility function that is used to map a*/
   /* GATT error code to a PM error code.  The caller MUST ensure that  */
   /* an error has occurred before calling this function.               */
static int MAPGATTErrorCode(int ErrorCodeToMap)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: GATT Error Code = %d\n", ErrorCodeToMap));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: GATM Error Code = %d\n", ret_val));

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
   HRPM_Update_Data_t *HRPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
               if((HRPMUpdateData = (HRPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HRPM_Update_Data_t) + GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength)) != NULL)
               {
                  /* Initialize the response information.               */
                  HRPMUpdateData->UpdateType = utGATTNotificationEvent;

                  /* Format the event structure.                        */
                  HRPMUpdateData->UpdateData.GATTServerNotificationData = *(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data);

                  if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength)
                  {
                     HRPMUpdateData->UpdateData.GATTServerNotificationData.AttributeValue = ((Byte_t *)HRPMUpdateData) + sizeof(HRPM_Update_Data_t);

                     /* Copy the Notification data into the new buffer. */
                     BTPS_MemCopy(HRPMUpdateData->UpdateData.GATTServerNotificationData.AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength);
                  }
                  else
                     HRPMUpdateData->UpdateData.GATTServerNotificationData.AttributeValue = NULL;

                  /* If there is an event to dispatch, go ahead and     */
                  /* dispatch it.                                       */
                  if(!HRPM_NotifyUpdate(HRPMUpdateData))
                  {
                     /* There was an error with either parsing or       */
                     /* dispatching the event.  Free all memory         */
                     /* associated with this transaction.               */
                     BTPS_FreeMemory(HRPMUpdateData);
                  }
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ClientEventCallback_HRP(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   Boolean_t            ResponseValid;
   HRPM_Update_Data_t  *HRPMUpdateData;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data) && (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data) && ((TransactionEntry = DeleteTransactionEntry(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID, (Word_t)CallbackParameter)) != NULL))
   {
      /* A valid transaction was found, so an event must be dispatched. */
      if((HRPMUpdateData = (HRPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(HRPM_Update_Data_t))) != NULL)
      {
         /* Initialize the response information.                        */
         HRPMUpdateData->UpdateType = utHRPCollectorEvent;
         ResponseValid              = FALSE;

         /* Determine the event that occurred.                          */
         switch(GATT_Client_Event_Data->Event_Data_Type)
         {
            case etGATT_Client_Error_Response:
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
               {
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttGetBodySensorLocation:
                        ResponseValid                                                                                         = TRUE;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventDataType                                           = cetGetBodySensorLocationResponse;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.GetBodySensorLocationResponse.TransactionData = (Get_Body_Sensor_Location_Transaction_Data_t *)TransactionEntry->ResponseData;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.GetBodySensorLocationResponse.ResponseStatus  = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode;
                        break;
                     case ttResetEnergyExpended:
                        ResponseValid                                                                                       = TRUE;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventDataType                                         = cetResetEnergyExpendedResponse;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.ResetEnergyExpendedResponse.TransactionData = (Reset_Energy_Expended_Transaction_Data_t *)TransactionEntry->ResponseData;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.ResetEnergyExpendedResponse.ResponseStatus  = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode;
                        break;
                     case ttWriteMeasurementCCD:
                        ResponseValid                                                                                       = TRUE;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventDataType                                         = cetWriteMeasurementCCDResponse;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.WriteMeasurementCCDResponse.TransactionData = (Write_Measurement_CCD_Transaction_Data_t *)TransactionEntry->ResponseData;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.WriteMeasurementCCDResponse.ResponseStatus  = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode;
                        break;
                     default:
                        break;
                  }
               }
               break;
            case etGATT_Client_Read_Response:
               if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
               {
                  /* If we know about this device and a callback        */
                  /* parameter exists, then check if we know what read  */
                  /* response this is.                                  */
                  if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength != 0)
                  {
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttGetBodySensorLocation:
                           ResponseValid                                                                                         = TRUE;
                           HRPMUpdateData->UpdateData.CollectorEventData.EventDataType                                           = cetGetBodySensorLocationResponse;
                           HRPMUpdateData->UpdateData.CollectorEventData.EventData.GetBodySensorLocationResponse.TransactionData = (Get_Body_Sensor_Location_Transaction_Data_t *)TransactionEntry->ResponseData;
                           HRPMUpdateData->UpdateData.CollectorEventData.EventData.GetBodySensorLocationResponse.ResponseStatus  = (Byte_t)HRS_Decode_Body_Sensor_Location(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, &(HRPMUpdateData->UpdateData.CollectorEventData.EventData.GetBodySensorLocationResponse.BodySensorLocation));
                           break;
                        default:
                           break;
                     }
                  }
               }
               break;
            case etGATT_Client_Write_Response:
               if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
               {
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttResetEnergyExpended:
                        ResponseValid                                                                                       = TRUE;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventDataType                                         = cetResetEnergyExpendedResponse;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.ResetEnergyExpendedResponse.TransactionData = (Reset_Energy_Expended_Transaction_Data_t *)TransactionEntry->ResponseData;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.ResetEnergyExpendedResponse.ResponseStatus  = 0;
                        break;
                     case ttWriteMeasurementCCD:
                        ResponseValid                                                                                       = TRUE;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventDataType                                         = cetWriteMeasurementCCDResponse;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.WriteMeasurementCCDResponse.TransactionData = (Write_Measurement_CCD_Transaction_Data_t *)TransactionEntry->ResponseData;
                        HRPMUpdateData->UpdateData.CollectorEventData.EventData.WriteMeasurementCCDResponse.ResponseStatus  = 0;
                        break;
                     default:
                        break;
                  }
               }
               break;
            default:
               break;
         }


         /* If there is an event to dispatch, go ahead and dispatch it. */
         if(!(ResponseValid) || !(HRPM_NotifyUpdate(HRPMUpdateData)))
         {
            /* There was an error with either parsing or dispatching the*/
            /* response.  Free all memory associated with this          */
            /* transaction.                                             */
            BTPS_FreeMemory((void *)HRPMUpdateData);

            if(TransactionEntry->ResponseData)
               BTPS_FreeMemory((void *)TransactionEntry->ResponseData);
         }

         /* Delete the transaction entry since it has been processed.   */
         FreeTransactionEntryMemory(TransactionEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HRP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HRP Manager  */
   /* Implementation.                                                   */
int _HRPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HRP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the HRP   */
   /* Manager Implementation.  After this function is called the HRP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HRPM_Initialize() function.  */
void _HRPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Free all outstanding transactions.                             */
      FreeTransactionEntryList();

      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the HRP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HRP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HRPM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         if(!(_BluetoothStackID) && !(CallbackID))
         {
            /* Register a GATT Connection Event Callback.               */
            if((CallbackID = GATT_Register_Connection_Events(BluetoothStackID, GATT_Connection_Event_Callback, 0)) > 0)
            {
               /* ANP Framework initialized successfully.               */
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("HRP Framework Initialized\n"));

               /* Save the Bluetooth Stack ID.                          */
               _BluetoothStackID = BluetoothStackID;
            }
            else
            {
               /* Error initializing ANP Framework.                     */
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT Connection Events NOT registered: %d\n", CallbackID));

               CallbackID = 0;
            }
         }
         else
         {
            /* ANP Framework initialized successfully.                  */
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Cannot Re-initialize HRP Framework until clean-up\n"));
         }
      }
      else
      {
         if((_BluetoothStackID) && (CallbackID))
         {
            GATT_Un_Register_Connection_Events(_BluetoothStackID, CallbackID);

            CallbackID        = 0;

            _BluetoothStackID = 0;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will parse a HRPM Heart Rate Measurement   */
   /* Event given a Heart Rate Measurement dispatched by this module.   */
   /* The first parameter is a pointer to the Heart Rate Measurement    */
   /* Event dispatched by this module.  The second parameter is a       */
   /* pointer to the Heart Rate Measurement Event to be dispatched by   */
   /* BTPMHRPM.  This function returns zero on success; otherwise, a    */
   /* negative value is returned.                                       */
   /* * NOTE * _HRPM_Free_Heart_Rate_Measurement_Event_RR_Interval must */
   /*          be called with a pointer to the formatted Heart Rate     */
   /*          Measurement Event after the event is no longer needed.   */
int _HRPM_Decode_Heart_Rate_Measurement_Event_To_Event(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent, HRPM_Heart_Rate_Measurement_Event_Data_t *HeartRateMeasurementEventEvent)
{
   int                                ret_val;
   HRS_Heart_Rate_Measurement_Data_t  HeartRateMeasurement;
   HRS_Heart_Rate_Measurement_Data_t *AllocHeartRateMeasurement;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize the Heart Rate Structure.                              */
   BTPS_MemInitialize(&HeartRateMeasurement, 0, sizeof(HRS_Heart_Rate_Measurement_Data_t));

   /* Decode the Heart Rate Measurement without RR Intervals.           */
   if((ret_val = HRS_Decode_Heart_Rate_Measurement(HeartRateMeasurementEvent->BufferLength, HeartRateMeasurementEvent->Buffer, &(HeartRateMeasurement))) == 0)
   {
      HeartRateMeasurementEventEvent->MeasurementFlags    = HeartRateMeasurement.Flags;
      HeartRateMeasurementEventEvent->HeartRate           = HeartRateMeasurement.Heart_Rate;
      HeartRateMeasurementEventEvent->EnergyExpended      = HeartRateMeasurement.Energy_Expended;
      HeartRateMeasurementEventEvent->NumberOfRRIntervals = HeartRateMeasurement.Number_Of_RR_Intervals;

      /* If there are RR Intervals, then allocate memory to hold them   */
      /* and parse them out of the notification.                        */
      if(HeartRateMeasurement.Number_Of_RR_Intervals)
      {
         /* Allocate the required memory for the RR Intervals.          */
         if((AllocHeartRateMeasurement = (HRS_Heart_Rate_Measurement_Data_t *)BTPS_AllocateMemory(HRS_HEART_RATE_MEASUREMENT_DATA_SIZE(HeartRateMeasurement.Number_Of_RR_Intervals))) != NULL)
         {
            /* Decode the Heart Rate Measurement.                       */
            if((ret_val = HRS_Decode_Heart_Rate_Measurement(HeartRateMeasurementEvent->BufferLength, HeartRateMeasurementEvent->Buffer, AllocHeartRateMeasurement)) == 0)
               HeartRateMeasurementEventEvent->RRIntervals = AllocHeartRateMeasurement->RR_Intervals;
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function will free any resources allocated by a call*/
   /* to _HRPM_Decode_Heart_Rate_Measurement_Event_To_Event.  The first */
   /* parameter is a pointer to the HRPM Heart Rate Measurement Event   */
   /* that was formatted.                                               */
void _HRPM_Free_Heart_Rate_Measurement_Event_RR_Interval(HRPM_Heart_Rate_Measurement_Event_Data_t *HeartRateMeasurementEventEvent)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((HeartRateMeasurementEventEvent->NumberOfRRIntervals) && (HeartRateMeasurementEventEvent->RRIntervals))
      BTPS_FreeMemory((void *)(((unsigned char *)(HeartRateMeasurementEventEvent->RRIntervals)) - BTPS_STRUCTURE_OFFSET(HRS_Heart_Rate_Measurement_Data_t, RR_Intervals)));

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will parse a HRPM Heart Rate Measurement   */
   /* Message given a Heart Rate Measurement dispatched by this module. */
   /* The first parameter is a pointer to the Heart Rate Measurement    */
   /* Event dispatched by this module.  This function returns a pointer */
   /* to a Heart Rate Measurement Message on success; otherwise, NULL is*/
   /* returned.                                                         */
   /* * NOTE * _HRPM_Free_Heart_Rate_Measurement_Message must be called */
   /*          with a pointer to the formatted Heart Rate Measurement   */
   /*          Event after the event is no longer needed.               */
HRPM_Heart_Rate_Measurement_Message_t *_HRPM_Decode_Heart_Rate_Measurement_Event_To_Message(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent)
{
   HRS_Heart_Rate_Measurement_Data_t      HeartRateMeasurement;
   HRS_Heart_Rate_Measurement_Data_t     *AllocHeartRateMeasurement;
   HRPM_Heart_Rate_Measurement_Message_t *ret_val = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize the Heart Rate Structure.                              */
   BTPS_MemInitialize(&HeartRateMeasurement, 0, sizeof(HRS_Heart_Rate_Measurement_Data_t));

   /* Decode the Heart Rate Measurement without RR Intervals.           */
   if(HRS_Decode_Heart_Rate_Measurement(HeartRateMeasurementEvent->BufferLength, HeartRateMeasurementEvent->Buffer, &(HeartRateMeasurement)) == 0)
   {
      /* Allocate a buffer to hold the message.                         */
      if((ret_val = BTPS_AllocateMemory(HRPM_HEART_RATE_MEASUREMENT_MESSAGE_SIZE(HeartRateMeasurement.Number_Of_RR_Intervals))) != NULL)
      {
         ret_val->MeasurementFlags    = HeartRateMeasurement.Flags;
         ret_val->HeartRate           = HeartRateMeasurement.Heart_Rate;
         ret_val->EnergyExpended      = HeartRateMeasurement.Energy_Expended;
         ret_val->NumberOfRRIntervals = HeartRateMeasurement.Number_Of_RR_Intervals;

         /* If there are RR Intervals, then allocate memory to hold them*/
         /* and parse them out of the notification.                     */
         if(HeartRateMeasurement.Number_Of_RR_Intervals)
         {
            /* Allocate the required memory for the RR Intervals.       */
            if((AllocHeartRateMeasurement = (HRS_Heart_Rate_Measurement_Data_t *)BTPS_AllocateMemory(HRS_HEART_RATE_MEASUREMENT_DATA_SIZE(HeartRateMeasurement.Number_Of_RR_Intervals))) != NULL)
            {
               /* Decode the Heart Rate Measurement.                    */
               if(HRS_Decode_Heart_Rate_Measurement(HeartRateMeasurementEvent->BufferLength, HeartRateMeasurementEvent->Buffer, AllocHeartRateMeasurement) == 0)
               {
                  /* Copy over the RR Intervals from the decoded Heart  */
                  /* Rate Measurement to the message.                   */
                  BTPS_MemCopy(ret_val->RRIntervals, AllocHeartRateMeasurement->RR_Intervals, HeartRateMeasurement.Number_Of_RR_Intervals * sizeof(Word_t));

                  /* Free the Heart Rate Measurement with RR Interval   */
                  /* data.                                              */
                  BTPS_FreeMemory(AllocHeartRateMeasurement);
               }
               else
               {
                  /* An error occurred, Free the message.               */
                  BTPS_FreeMemory(ret_val);

                  ret_val = NULL;
               }
            }
            else
            {
               /* An error occurred, Free the message.                  */
               BTPS_FreeMemory(ret_val);

               ret_val = NULL;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function will free any resources allocated by a call*/
   /* to _HRPM_Decode_Heart_Rate_Measurement_Event_To_Message.  The     */
   /* first parameter is a pointer to the HRPM Heart Rate Measurement   */
   /* Message that was formatted.                                       */
void _HRPM_Free_Heart_Rate_Measurement_Message(HRPM_Heart_Rate_Measurement_Message_t *HeartRateMeasurementMessage)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   BTPS_FreeMemory(HeartRateMeasurementMessage);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will submit a Get Body Sensor Location     */
   /* request to a remote sensor.  The first parameter is a pointer to a*/
   /* dynamically allocated Transaction structure that will be used to  */
   /* submit the request and provide information in the corresponding   */
   /* response.  The transaction data will be passed back in the        */
   /* corresponding event, and it is NOT freed internal to this module. */
   /* The second parameter is the Attribute Handle of the Body Sensor   */
   /* Location Characteristic on the remote sensor.  This function will */
   /* return zero on success; otherwise, a negative error code will be  */
   /* returned.                                                         */
int _HRPM_Get_Body_Sensor_Location(Get_Body_Sensor_Location_Transaction_Data_t *TransactionData, Word_t Handle)
{
   int                  ret_val;
   unsigned int         ConnectionID;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate that there is an active connection to this device.       */
   if((GATT_Query_Connection_ID(_BluetoothStackID, gctLE, TransactionData->RemoteSensor, &ConnectionID)) == 0)
   {
      /* Create new outstanding transaction entry.                      */
      TransactionEntry.TransactionType = ttGetBodySensorLocation;
      TransactionEntry.AttributeHandle = Handle;
      TransactionEntry.ConnectionID    = ConnectionID;
      TransactionEntry.ResponseData    = (void *)TransactionData;

      /* Add the entry to the list.                                     */
      if(AddTransactionEntry(&TransactionEntry) != NULL)
      {
         /* Submit the request to GATT.                                 */
         if((ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, Handle, GATT_ClientEventCallback_HRP, Handle)) > 0)
            ret_val = 0;
         else
         {
            ret_val = MAPGATTErrorCode(ret_val);

            /* Delete the transaction entry since the GATT command      */
            /* failed.                                                  */
            if((TransactionEntryPtr = DeleteTransactionEntry(ConnectionID, Handle)) != NULL)
               FreeTransactionEntryMemory(TransactionEntryPtr);
         }
      }
      else
      {
         /* Couldn't add to the list.  If the Connection ID is invalid, */
         /* then there is already an existing entry.                    */
         if(!TransactionEntry.ConnectionID)
            ret_val = BTPM_ERROR_CODE_HEART_RATE_BODY_SENSOR_LOCATION_OUTSTANDING;
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function will submit a Reset Energy Expended request*/
   /* to a remote sensor.  The first parameter is a pointer to a        */
   /* dynamically allocated Transaction structure that will be used to  */
   /* submit the request and provide information in the corresponding   */
   /* response.  The transaction data will be passed back in the        */
   /* corresponding event, and it is NOT freed internal to this module. */
   /* The second parameter is the Attribute Handle of the Heart Rate    */
   /* Control Point Characteristic on the remote sensor.  This function */
   /* will return zero on success; otherwise, a negative error code will*/
   /* be returned.                                                      */
int _HRPM_Reset_Energy_Expended(Reset_Energy_Expended_Transaction_Data_t *TransactionData, Word_t Handle)
{
   int                  ret_val;
   unsigned int         ConnectionID;
   NonAlignedByte_t     CommandBuffer[1];
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the command                                                */
   ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(CommandBuffer, HRS_HEART_RATE_CONTROL_POINT_RESET_ENERGY_EXPENDED);

   /* Validate that there is an active connection to this device.       */
   if((GATT_Query_Connection_ID(_BluetoothStackID, gctLE, TransactionData->RemoteSensor, &ConnectionID)) == 0)
   {
      /* Create new outstanding transaction entry.                      */
      TransactionEntry.TransactionType = ttResetEnergyExpended;
      TransactionEntry.AttributeHandle = Handle;
      TransactionEntry.ConnectionID    = ConnectionID;
      TransactionEntry.ResponseData    = (void *)TransactionData;

      if(AddTransactionEntry(&TransactionEntry) != NULL)
      {
         /* Submit the command to GATT.                                 */
         if((ret_val = GATT_Write_Request(_BluetoothStackID, ConnectionID, Handle, HRS_HEART_RATE_CONTROL_POINT_VALUE_LENGTH, CommandBuffer, GATT_ClientEventCallback_HRP, Handle)) > 0)
            ret_val = 0;
         else
         {
            ret_val = MAPGATTErrorCode(ret_val);

            /* Delete the transaction entry since the GATT command      */
            /* failed.                                                  */
            if((TransactionEntryPtr = DeleteTransactionEntry(ConnectionID, Handle)) != NULL)
               FreeTransactionEntryMemory(TransactionEntryPtr);
         }
      }
      else
      {
         /* Couldn't add to the list.  If the Connection ID is invalid, */
         /* then there is already an existing entry.                    */
         if(!TransactionEntry.ConnectionID)
            ret_val = BTPM_ERROR_CODE_HEART_RATE_RESET_ENERGY_EXPENDED_OUTSTANDING;
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function will submit a Write Measurement CCD request*/
   /* to a remote sensor.  The first parameter is a pointer to a        */
   /* dynamically allocated Transaction structure that will be used to  */
   /* submit the request and provide information in the corresponding   */
   /* response.  The transaction data will be passed back in the        */
   /* corresponding event, and it is NOT freed internal to this module. */
   /* The second parameter is the Attribute Handle of the Heart Rate    */
   /* Client Characteristic Descriptor on the remote sensor.  This      */
   /* function will return zero on success; otherwise, a negative error */
   /* code will be returned.                                            */
int _HRPM_Write_Measurement_CCD(Write_Measurement_CCD_Transaction_Data_t *TransactionData, Word_t Handle, Word_t ClientConfigurationValue)
{
   int                  ret_val;
   unsigned int         ConnectionID;
   NonAlignedWord_t     CommandBuffer[1];
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the command                                                */
   ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(CommandBuffer, ClientConfigurationValue);

   /* Validate that there is an active connection to this device.       */
   if((GATT_Query_Connection_ID(_BluetoothStackID, gctLE, TransactionData->RemoteSensor, &ConnectionID)) == 0)
   {
      /* Initialize the transaction entry.                              */
      TransactionEntry.TransactionType = ttWriteMeasurementCCD;
      TransactionEntry.AttributeHandle = Handle;
      TransactionEntry.ConnectionID    = ConnectionID;
      TransactionEntry.ResponseData    = (void *)TransactionData;

      /* Add the entry to the list.                                     */
      if(AddTransactionEntry(&TransactionEntry) != NULL)
      {
         /* Submit the command to GATT.                                 */
         if((ret_val = GATT_Write_Request(_BluetoothStackID, ConnectionID, Handle, sizeof(NonAlignedWord_t), CommandBuffer, GATT_ClientEventCallback_HRP, Handle)) > 0)
            ret_val = 0;
         else
         {
            ret_val = MAPGATTErrorCode(ret_val);

            /* Delete the transaction entry since the GATT command      */
            /* failed.                                                  */
            if((TransactionEntryPtr = DeleteTransactionEntry(ConnectionID, Handle)) != NULL)
               FreeTransactionEntryMemory(TransactionEntryPtr);
         }
      }
      else
      {
         /* Couldn't add to the list.  This error message will never    */
         /* make it to the client, so it doesn't matter if another CCD  */
         /* is outstanding.                                             */
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}
