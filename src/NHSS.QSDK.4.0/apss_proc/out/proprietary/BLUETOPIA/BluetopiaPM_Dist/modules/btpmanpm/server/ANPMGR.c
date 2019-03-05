/*****< anpmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPMGR - ANP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMANPM.h"            /* BTPM ANP Manager Prototypes/Constants.    */
#include "ANPMGR.h"              /* ANP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variables which are used to hold ANS Initialization Information.  */
static Word_t _SupportedNewAlertCategories;
static Word_t _SupportedUnReadAlertCategories;

   /* Variable which holds the GATT Connection Event Callback ID.       */
static unsigned int CallbackID;

   /* Variable which holds the current ANS Instance ID.                 */
static unsigned int ANSInstanceID;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _ANPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI ANS_Event_Callback(unsigned int BluetoothStackID, ANS_Event_Data_t *ANS_Event_Data, unsigned long CallbackParameter);

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
   ANPM_Update_Data_t *ANPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((GATT_Connection_Event_Data) && (GATT_Connection_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      ANPMUpdateData = NULL;

      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case etGATT_Connection_Device_Connection:
         case etGATT_Connection_Device_Disconnection:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Request_Data) && ((ANPMUpdateData = (ANPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(ANPM_Update_Data_t))) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               ANPMUpdateData->UpdateType                                     = utANPConnectionEvent;
               ANPMUpdateData->UpdateData.ConnectionEventData.Event_Data_Type = GATT_Connection_Event_Data->Event_Data_Type;
               ANPMUpdateData->UpdateData.ConnectionEventData.Event_Data_Size = GATT_Connection_Event_Data->Event_Data_Size;

               BTPS_MemCopy(&(ANPMUpdateData->UpdateData.ConnectionEventData.Event_Data), GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Request_Data, GATT_Connection_Event_Data->Event_Data_Size);
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(ANPMUpdateData)
      {
         if(!ANPM_NotifyUpdate(ANPMUpdateData))
            BTPS_FreeMemory((void *)ANPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function the function that is installed to process  */
   /* ANS Events from the stack.                                        */
static void BTPSAPI ANS_Event_Callback(unsigned int BluetoothStackID, ANS_Event_Data_t *ANS_Event_Data, unsigned long CallbackParameter)
{
   ANPM_Update_Data_t *ANPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((ANS_Event_Data) && (ANS_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      ANPMUpdateData = NULL;

      switch(ANS_Event_Data->Event_Data_Type)
      {
         case etANS_Server_Read_Client_Configuration_Request:
         case etANS_Server_Client_Configuration_Update:
         case etANS_Server_Control_Point_Command_Indication:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data) && ((ANPMUpdateData = (ANPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(ANPM_Update_Data_t))) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               ANPMUpdateData->UpdateType                                 = utANPEvent;
               ANPMUpdateData->UpdateData.ServerEventData.Event_Data_Type = ANS_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(ANPMUpdateData->UpdateData.ServerEventData.Event_Data), ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data, ANS_Event_Data->Event_Data_Size);
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(ANPMUpdateData)
      {
         if(!ANPM_NotifyUpdate(ANPMUpdateData))
            BTPS_FreeMemory((void *)ANPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the ANP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager ANP Manager  */
   /* Implementation.                                                   */
int _ANPM_Initialize(Word_t SupportedNewAlertCategories, Word_t SupportedUnReadAlertCategories)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANP Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */
      _SupportedNewAlertCategories    = SupportedNewAlertCategories;
      _SupportedUnReadAlertCategories = SupportedUnReadAlertCategories;

      /* Flag that this module is initialized.                          */
      Initialized                     = TRUE;

      ret_val                         = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the ANP   */
   /* Manager Implementation.  After this function is called the ANP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _ANPM_Initialize() function.  */
void _ANPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the ANP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * The last parameter to this function can be used to       */
   /*          specify the region in the GATT database that the ANS     */
   /*          Service will reside.                                     */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the ANP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _ANPM_SetBluetoothStackID(unsigned int BluetoothStackID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   int          Result;
   unsigned int ServiceID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Initialize the Identifiers.                                 */
         ANSInstanceID = 0;
         CallbackID    = 0;

         /* Stack has been powered up, so register an ANS Server        */
         /* Instance.                                                   */
         if(ServiceHandleRange)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to register ANS at 0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));

            /* Attempt to initialize the service at the specified handle*/
            /* range.                                                   */
            Result = ANS_Initialize_Service_Handle_Range(BluetoothStackID, ANS_Event_Callback, 0, &ServiceID, ServiceHandleRange);
         }
         else
            Result = ANS_Initialize_Service(BluetoothStackID, ANS_Event_Callback, 0, &ServiceID);

         if(Result > 0)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANS Service registered: GATT Service ID = %d\n", ServiceID));

            /* Save the ANP Instance ID.                                */
            ANSInstanceID = (unsigned int)Result;

            /* Set the Supported New Alert Categories.                  */
            if(_SupportedNewAlertCategories)
               Result = ANS_Set_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, _SupportedNewAlertCategories);
            else
               Result = 0;

            if(!Result)
            {
               /* Set the Supported Un-Read Alert Categories.           */
               if(_SupportedUnReadAlertCategories)
                  Result = ANS_Set_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, _SupportedUnReadAlertCategories);
               else
                  Result = 0;

               if(!Result)
               {
                  /* Register a GATT Connection Event Callback.         */
                  Result = GATT_Register_Connection_Events(BluetoothStackID, GATT_Connection_Event_Callback, 0);
                  if(Result > 0)
                  {
                     /* ANP Framework initialized successfully.         */
                     DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANP Framework Initialized\n"));

                     /* Save the GATT Connection Callback ID.           */
                     CallbackID        = (unsigned int)Result;

                     /* Save the Bluetooth Stack ID.                    */
                     _BluetoothStackID = BluetoothStackID;

                     /* Flag that no error has occurred.                */
                     Result            = 0;
                  }
                  else
                  {
                     /* Error initializing ANP Framework.               */
                     DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT Connection Events NOT registered: %d\n", Result));
                  }
               }
               else
               {
                  /* Error initializing ANP Framework.                  */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANS Supported Un-Read Alert Categories not set: %d\n", Result));
               }
            }
            else
            {
               /* Error initializing ANP Framework.                     */
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANS Supported New Alert Categories not set: %d\n", Result));
            }
         }
         else
         {
            /* Error initializing ANP Framework.                        */
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANP Service NOT registered: %d\n", Result));
         }

         /* Cleanup resources if an error occurred.                     */
         if(Result < 0)
         {
            if(ANSInstanceID)
            {
               ANS_Cleanup_Service(BluetoothStackID, ANSInstanceID);

               ANSInstanceID = 0;
            }

            if(CallbackID)
            {
               GATT_Un_Register_Connection_Events(BluetoothStackID, CallbackID);

               CallbackID = 0;
            }
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         ANS_Cleanup_Service(_BluetoothStackID, ANSInstanceID);
         GATT_Un_Register_Connection_Events(BluetoothStackID, CallbackID);

         CallbackID        = 0;
         ANSInstanceID     = 0;
         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for querying the number of  */
   /* attributes that are used by the ANS Service registered by this    */
   /* module.                                                           */
unsigned int _ANPM_Query_Number_Attributes(void)
{
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply call the Bluetopia function to query the number of         */
   /* attributes.                                                       */
   NumberOfAttributes = ANS_Query_Number_Attributes();

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", NumberOfAttributes));

   return(NumberOfAttributes);
}

   /* The following function is responsible for responding to a Read    */
   /* Client Configuration Request that was received earlier.  This     */
   /* function accepts as input the Transaction ID of the request and a */
   /* Boolean that indicates if notifications are enabled for this if   */
   /* TRUE.  This function returns zero if successful, or a negative    */
   /* return error code if there was an error.                          */
int _ANS_Read_Client_Configuration_Response(unsigned int TransactionID, Boolean_t NotificationsEnabled)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (TransactionID))
   {
      /* Respond to the read request.                                   */
      ret_val = ANS_Read_Client_Configuration_Response(_BluetoothStackID, ANSInstanceID, TransactionID, NotificationsEnabled);

      if(ret_val)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANS_Read_Client_Configuration_Response() returned %d\n", ret_val));

         if(ret_val == ANS_ERROR_INSUFFICIENT_RESOURCES)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a new Alert     */
   /* Notification to the specified connection.  This function accepts  */
   /* as input the ConnectionID of the remote client to send the        */
   /* notification to and the new alert data to notify.  This functions */
   /* returns ZERO if success or a negative error code.                 */
int _ANS_New_Alert_Notification(unsigned int ConnectionID, ANS_New_Alert_Data_t *NewAlertData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (ConnectionID))
   {
      /* Send the new alert notification to the specified client.       */
      ret_val = ANS_New_Alert_Notification(_BluetoothStackID, ANSInstanceID, ConnectionID, NewAlertData);
      if(ret_val)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANS_New_Alert_Notification() returned %d\n", ret_val));

         if(ret_val == ANS_ERROR_INSUFFICIENT_RESOURCES)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a Un-Read Alert */
   /* Notification to the specified connection.  This function accepts  */
   /* as input the ConnectionID of the remote client to send the        */
   /* notification to and the un-read alert data to notify.  This       */
   /* functions returns ZERO if success or a negative error code.       */
int _ANS_UnRead_Alert_Notification(unsigned int ConnectionID, ANS_Un_Read_Alert_Data_t *UnReadAlert)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (ConnectionID))
   {
      /* Send the un-read alert notification to the specified client.   */
      ret_val = ANS_Un_Read_Alert_Status_Notification(_BluetoothStackID, ANSInstanceID, ConnectionID, UnReadAlert);
      if(ret_val)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANS_Un_Read_Alert_Status_Notification() returned %d\n", ret_val));

         if(ret_val == ANS_ERROR_INSUFFICIENT_RESOURCES)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}
