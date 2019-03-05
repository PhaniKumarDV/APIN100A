/*****< pasmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PASMGR - Phone Alert Status (PAS) Manager Implementation for Stonestreet  */
/*           One Bluetooth Protocol Stack Platform Manager.                   */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "SS1BTPAS.h"            /* PASS Service API Prototypes/Constants.    */

#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMPASM.h"            /* BTPM PAS Manager Prototypes/Constants.    */
#include "PASMGR.h"              /* PAS Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _PASM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variable which holds the current PASS Instance ID.                */
static unsigned int PASSInstanceID;

   /* Variables which hold the default values of the Alert Status and   */
   /* the Ringer Settings.  This is the value that is to be used before */
   /* the first values are explicitly set.                              */
static PASS_Alert_Status_t _DefaultAlertStatus;
static PASS_Ringer_Setting_t _DefaultRingerSetting;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI PASS_Event_Callback(unsigned int BluetoothStackID, PASS_Event_Data_t *PASS_Event_Data, unsigned long CallbackParameter);

   /* The following function the function that is installed to process  */
   /* CTS Events from the stack.                                        */
static void BTPSAPI PASS_Event_Callback(unsigned int BluetoothStackID, PASS_Event_Data_t *PASS_Event_Data, unsigned long CallbackParameter)
{
   PASM_Update_Data_t *PASMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((PASS_Event_Data) && (PASS_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      PASMUpdateData = NULL;

      switch(PASS_Event_Data->Event_Data_Type)
      {
         case etPASS_Server_Read_Client_Configuration_Request:
         case etPASS_Server_Client_Configuration_Update:
         case etPASS_Server_Ringer_Control_Command_Indication:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((PASS_Event_Data->Event_Data.PASS_Ringer_Control_Command_Data) && ((PASMUpdateData = (PASM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PASM_Update_Data_t))) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               PASMUpdateData->UpdateType                          = utPASSServerEvent;
               PASMUpdateData->UpdateData.PASSEventData.Event_Type = PASS_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(PASMUpdateData->UpdateData.PASSEventData.Event_Data), PASS_Event_Data->Event_Data.PASS_Ringer_Control_Command_Data, PASS_Event_Data->Event_Data_Size);
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(PASMUpdateData)
      {
         if(!PASM_NotifyUpdate(PASMUpdateData))
            BTPS_FreeMemory((void *)PASMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PAS Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAS Manager  */
   /* Implementation.                                                   */
int _PASM_Initialize(PASS_Ringer_Setting_t DefaultRingerSetting, PASS_Alert_Status_t *DefaultAlertStatus)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      /* Next, check to make sure the default parameters have been      */
      /* specified.                                                     */
      if(DefaultAlertStatus)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAS Manager (Imp)\n"));

         /* Flag that this module is initialized.                       */
         Initialized           = TRUE;

         /* Note the default settings that were specified.              */
         _DefaultRingerSetting = DefaultRingerSetting;
         _DefaultAlertStatus   = *DefaultAlertStatus;

         /* Flag success to the caller.                                 */
         ret_val               = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the PAS   */
   /* Manager Implementation.  After this function is called the PAS    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PASM_Initialize() function.  */
void _PASM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the PAS       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * The last parameter to this function can be used to       */
   /*          specify the region in the GATT database that the ANS     */
   /*          Service will reside.                                     */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the PAS Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _PASM_SetBluetoothStackID(unsigned int BluetoothStackID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   int          Result;
   unsigned int ServiceID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Stack has been powered up, so register an PASS Server       */
         /* Instance.                                                   */
         if(ServiceHandleRange)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to register PASS at 0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));

            Result = PASS_Initialize_Service_Handle_Range(BluetoothStackID, PASS_Event_Callback, 0, &ServiceID, ServiceHandleRange);
         }
         else
            Result = PASS_Initialize_Service(BluetoothStackID, PASS_Event_Callback, 0, &ServiceID);

         if(Result > 0)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASS Service registered: GATT Service ID = %u, Instance ID = %u\n", ServiceID, (unsigned int)Result));

            /* Save the PASS Instance ID.                               */
            PASSInstanceID = (unsigned int)Result;

            /* Attempt to set the Default Alert Status.                 */
            Result = PASS_Set_Alert_Status(BluetoothStackID, PASSInstanceID, _DefaultAlertStatus);
            if(!Result)
            {
               /* Attempt to set the Default Ringer Setting.            */
               Result = PASS_Set_Ringer_Setting(BluetoothStackID, PASSInstanceID, _DefaultRingerSetting);
               if(!Result)
               {
                  /* Save the Bluetooth Stack ID.                       */
                  _BluetoothStackID = BluetoothStackID;

                  /* Flag that no error has occurred.                   */
                  Result            = 0;
               }
               else
               {
                  /* Error initializing PASS Framework.                 */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Error - PASS_Set_Ringer_Setting(): %d\n", Result));
               }
            }
            else
            {
               /* Error initializing PASS Framework.                    */
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Error - PASS_Set_Alert_Status(): %d\n", Result));
            }

            /* Cleanup resources if an error occurred.                  */
            if(Result < 0)
            {
               if(PASSInstanceID)
               {
                  PASS_Cleanup_Service(BluetoothStackID, PASSInstanceID);

                  PASSInstanceID = 0;
               }
            }
         }
         else
         {
            /* Error initializing PASS Framework.                       */
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASS Service NOT registered: %d\n", Result));
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         PASS_Cleanup_Service(_BluetoothStackID, PASSInstanceID);

         PASSInstanceID    = 0;

         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for querying the number of  */
   /* attributes that are used by the PASS Service registered by this   */
   /* module.                                                           */
unsigned int _PASM_Query_Number_Attributes(void)
{
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply call the Bluetopia function to query the number of         */
   /* attributes.                                                       */
   NumberOfAttributes = PASS_Query_Number_Attributes();

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", NumberOfAttributes));

   return(NumberOfAttributes);
}

   /* The following function is responsible for querying the Connection */
   /* ID of a specified connection.  The first parameter is the BD_ADDR */
   /* of the connection to query the Connection ID.  The second         */
   /* parameter is a pointer to return the Connection ID if this        */
   /* function is successful.  This function returns a zero if          */
   /* successful or a negative return error code if an error occurs.    */
int _PASM_Query_Connection_ID(BD_ADDR_t BD_ADDR, unsigned int *ConnectionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ConnectionID))
      {
         /* Attempt to set the query the Connection ID.                 */
         ret_val = GATT_Query_Connection_ID(_BluetoothStackID, gctLE, BD_ADDR, ConnectionID);
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT_Query_Connection_ID() returned %d\n", ret_val));

            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for setting the Alert Status*/
   /* value.  The only parameter is the Alert Status to set as the      */
   /* current Alert Status.  This function returns a zero if successful */
   /* or a negative return error code if an error occurs.               */
int _PASM_Set_Alert_Status(PASS_Alert_Status_t *AlertStatus)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (PASSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if(AlertStatus)
      {
         /* Attempt to set the Alert Status in the specified PASS       */
         /* Instance.                                                   */
         ret_val = PASS_Set_Alert_Status(_BluetoothStackID, PASSInstanceID, *AlertStatus);
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASS_Set_Alert_Status() returned %d\n", ret_val));

            if(ret_val == PASS_ERROR_INSUFFICIENT_RESOURCES)
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for setting the Ringer      */
   /* Setting value.  The only parameter is the Ringer Setting to set as*/
   /* the current Ringer Setting.  This function returns a zero if      */
   /* successful or a negative return error code if an error occurs.    */
int _PASM_Set_Ringer_Setting(PASS_Ringer_Setting_t RingerSetting)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (PASSInstanceID))
   {
      /* Attempt to set the Ringer Setting in the specified PASS        */
      /* Instance.                                                      */
      ret_val = PASS_Set_Ringer_Setting(_BluetoothStackID, PASSInstanceID, RingerSetting);
      if(ret_val)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASS_Set_Ringer_Setting() returned %d\n", ret_val));

         if(ret_val == PASS_ERROR_INSUFFICIENT_RESOURCES)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for responding to a Read    */
   /* Client Configuration Request.  The first parameter to this        */
   /* function is the Transaction ID of the request.  The final         */
   /* parameter contains the Client Configuration to send to the remote */
   /* device.  This function returns a zero if successful or a negative */
   /* return error code if an error occurs.                             */
int _PASM_Read_Client_Configuration_Response(unsigned int TransactionID, Boolean_t NotificationsEnabled)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (PASSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if(TransactionID)
      {
         /* Attempt to set the Alert Status in the specified PASS       */
         /* Instance.                                                   */
         ret_val = PASS_Read_Client_Configuration_Response(_BluetoothStackID, PASSInstanceID, TransactionID, NotificationsEnabled);
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASS_Read_Client_Configuration_Response() returned %d\n", ret_val));

            if(ret_val == PASS_ERROR_INSUFFICIENT_RESOURCES)
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a notification  */
   /* of a specified characteristic to a specified remote device.  The  */
   /* first parameter to this function is the ConnectionID of the remote*/
   /* device to send the notification to.  The final parameter specifies*/
   /* the characteristic to notify.  This function returns a zero if    */
   /* successful or a negative return error code if an error occurs.    */
int _PASM_Send_Notification(unsigned int ConnectionID, PASS_Characteristic_Type_t CharacteristicType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (PASSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if(ConnectionID)
      {
         /* Attempt to set the Alert Status in the specified PASS       */
         /* Instance.                                                   */
         ret_val = PASS_Send_Notification(_BluetoothStackID, PASSInstanceID, ConnectionID, CharacteristicType);
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASS_Send_Notification() returned %d\n", ret_val));

            if(ret_val == PASS_ERROR_INSUFFICIENT_RESOURCES)
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

