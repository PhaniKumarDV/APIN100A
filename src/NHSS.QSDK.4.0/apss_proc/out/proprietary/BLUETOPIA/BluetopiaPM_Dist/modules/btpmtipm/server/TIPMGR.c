/*****< tipmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMGR - TIP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMTIPM.h"            /* BTPM TIP Manager Prototypes/Constants.    */
#include "TIPMGR.h"              /* TIP Manager Impl. Prototypes/Constants.   */
#include "TIPMGRI.h"             /* TIP Manager Platform Implementation.      */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Varaible which holds the currently supported roles of this TIP    */
   /* modules.                                                          */
static unsigned int SupportedRoles;

   /* Variable which holds the current CTS Instance ID.                 */
static unsigned int CTSInstanceID;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _TIPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variable which holds the GATT Connection Events callback ID return*/
   /* when registering for events.                                      */
static unsigned int GATTConnectionEventsCallbackID;

   /* Varaible which indicates how many devices have had notifications  */
   /* enabled.                                                          */
static unsigned int NumberOfEnabledNotifications;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI CTS_Event_Callback(unsigned int BluetoothStackID, CTS_Event_Data_t *CTS_Event_Data, unsigned long CallbackParameter);

static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);

static void BTPSAPI GATT_Client_Event_Callback(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);

   /* The following function the function that is installed to process  */
   /* CTS Events from the stack.                                        */
static void BTPSAPI CTS_Event_Callback(unsigned int BluetoothStackID, CTS_Event_Data_t *CTS_Event_Data, unsigned long CallbackParameter)
{
   TIPM_Update_Data_t *TIPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((CTS_Event_Data) && (CTS_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      TIPMUpdateData = NULL;

      switch(CTS_Event_Data->Event_Data_Type)
      {
         case etCTS_Server_Read_Client_Configuration_Request:
         case etCTS_Server_Update_Client_Configuration_Request:
         case etCTS_Server_Read_Current_Time_Request:
         case etCTS_Server_Read_Reference_Time_Information_Request:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((CTS_Event_Data->Event_Data.CTS_Read_Client_Configuration_Data) && ((TIPMUpdateData = (TIPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(TIPM_Update_Data_t))) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               TIPMUpdateData->UpdateType                                    = utCTSServerEvent;
               TIPMUpdateData->UpdateData.CTSServerEventData.Event_Data_Type = CTS_Event_Data->Event_Data_Type;
               TIPMUpdateData->UpdateData.CTSServerEventData.Event_Data_Size = CTS_Event_Data->Event_Data_Size;

               BTPS_MemCopy(&(TIPMUpdateData->UpdateData.CTSServerEventData.Event_Data), CTS_Event_Data->Event_Data.CTS_Read_Client_Configuration_Data, CTS_Event_Data->Event_Data_Size);
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(TIPMUpdateData)
      {
         if(!TIPM_NotifyUpdate(TIPMUpdateData))
            BTPS_FreeMemory((void *)TIPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is installed to       */
   /* process GATT connection events.                                   */
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter)
{
   TIPM_Update_Data_t *TIPMUpdateData = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((GATT_Connection_Event_Data) && (GATT_Connection_Event_Data->Event_Data_Size))
   {
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         /* We need to capture TIP notifications.                       */
         case etGATT_Connection_Server_Notification:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
            {
               if((TIPMUpdateData = (TIPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(TIPM_Update_Data_t) + GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength)) != NULL)
               {
                  /* Copy the event data into the update structure.     */
                  TIPMUpdateData->UpdateType                                                                  = utGATTConnectionEvent;
                  TIPMUpdateData->UpdateData.GATTConnectionEventData.Event_Data_Type                          = GATT_Connection_Event_Data->Event_Data_Type;
                  TIPMUpdateData->UpdateData.GATTConnectionEventData.Event_Data_Size                          = GATT_Connection_Event_Data->Event_Data_Size;
                  TIPMUpdateData->UpdateData.GATTConnectionEventData.Event_Data.GATT_Server_Notification_Data = *(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data);

                  /* Check if there is any extra data to copy.          */
                  if(TIPMUpdateData->UpdateData.GATTConnectionEventData.Event_Data.GATT_Server_Notification_Data.AttributeValueLength)
                  {
                     /* Move the value pointer to the end of the        */
                     /* structure.                                      */
                     TIPMUpdateData->UpdateData.GATTConnectionEventData.Event_Data.GATT_Server_Notification_Data.AttributeValue = ((Byte_t *)TIPMUpdateData) + sizeof(TIPM_Update_Data_t);

                     /* Copy the extra data into the structure.         */
                     BTPS_MemCopy(TIPMUpdateData->UpdateData.GATTConnectionEventData.Event_Data.GATT_Server_Notification_Data.AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength);
                  }
               }
            }

            break;

         /* We don't need any other connection events.                  */
         default:
            ;
      }

      if(TIPMUpdateData)
      {
         if(!TIPM_NotifyUpdate(TIPMUpdateData))
            BTPS_FreeMemory(TIPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

}

   /* The following function the function that is installed to process  */
   /* GATT Client Events from the stack.                                */
static void BTPSAPI GATT_Client_Event_Callback(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   TIPM_Update_Data_t *TIPMUpdateData = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify the data is semi-valid.                                    */
   if((GATT_Client_Event_Data) && (GATT_Client_Event_Data->Event_Data_Size))
   {
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Read_Response:
            /* Verify the data is valid.                                */
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT_Read_Response Event:\n"));

               /* Allocate the update data structure.                   */
               if((TIPMUpdateData = (TIPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(TIPM_Update_Data_t) + GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength)) != NULL)
               {
                  /* Copy the event data into the update structure.     */
                  TIPMUpdateData->UpdateType                                                        = utGATTClientEvent;
                  TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data_Type                    = GATT_Client_Event_Data->Event_Data_Type;
                  TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data_Size                    = GATT_Client_Event_Data->Event_Data_Size;
                  TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data.GATT_Read_Response_Data = *(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data);

                  /* Check if there is any extra data to copy.          */
                  if(TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data.GATT_Read_Response_Data.AttributeValueLength)
                  {
                     /* Move the value pointer to the end of the        */
                     /* structure.                                      */
                     TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data.GATT_Read_Response_Data.AttributeValue = ((Byte_t *)TIPMUpdateData) + sizeof(TIPM_Update_Data_t);

                     /* Copy the extra data into the structure.         */
                     BTPS_MemCopy(TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data.GATT_Read_Response_Data.AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);
                  }
               }
            }

            break;
         case etGATT_Client_Write_Response:
         case etGATT_Client_Error_Response:
            /* Verify the data is valid.                                */
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Handling GATT Client Event: %d\n", GATT_Client_Event_Data->Event_Data_Type));
               /* Allocate the update data structure.                   */
               if((TIPMUpdateData = (TIPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(TIPM_Update_Data_t))) != NULL)
               {
                  /* Copy the event information into the update.        */
                  TIPMUpdateData->UpdateType                                     = utGATTClientEvent;
                  TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data_Type = GATT_Client_Event_Data->Event_Data_Type;
                  TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data_Size = GATT_Client_Event_Data->Event_Data_Size;

                  /* Copy the event data into the update structure.     */
                  BTPS_MemCopy(&(TIPMUpdateData->UpdateData.GATTClientEventData.Event_Data), GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data, GATT_Client_Event_Data->Event_Data_Size);
               }
            }

            break;

         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled or Unknown GATT Client Event: %d\n", GATT_Client_Event_Data->Event_Data_Type));
      }

      if(TIPMUpdateData)
      {
         if(!TIPM_NotifyUpdate(TIPMUpdateData))
            BTPS_FreeMemory(TIPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the TIP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager TIP Manager  */
   /* Implementation.                                                   */
int _TIPM_Initialize(unsigned int SupportedTIPRoles)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing TIP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      /* Note the supported roles.                                      */
      SupportedRoles = SupportedTIPRoles;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the TIP   */
   /* Manager Implementation.  After this function is called the TIP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _TIPM_Initialize() function.  */
void _TIPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized = FALSE;

      SupportedRoles = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the TIP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * The last parameter to this function can be used to       */
   /*          specify the region in the GATT database that the ANS     */
   /*          Service will reside.                                     */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the TIP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _TIPM_SetBluetoothStackID(unsigned int BluetoothStackID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   int                               Result;
   unsigned int                      ServiceID;
   CTS_Local_Time_Information_Data_t LocalTimeInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Initialize the Identifiers.                                 */
         CTSInstanceID = 0;

         /* Only register the service is we support Server role.        */
         if(SupportedRoles & TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_SERVER)
         {
            /* Stack has been powered up, so register an CTS Server     */
            /* Instance.                                                */
            if(ServiceHandleRange)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to register CTS at 0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));

               /* Attempt to initialize the service at the specified    */
               /* handle range.                                         */
               Result = CTS_Initialize_Service_Handle_Range(BluetoothStackID, CTS_Event_Callback, 0, &ServiceID, ServiceHandleRange);
            }
            else
               Result = CTS_Initialize_Service(BluetoothStackID, CTS_Event_Callback, 0, &ServiceID);

            if(Result > 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS Service registered: GATT Service ID = %u, Instance ID = %u\n", ServiceID, (unsigned int)Result));

               /* Save the CTS Instance ID.                             */
               CTSInstanceID                      = (unsigned int)Result;

               /* Configure some default Local Time Information.        */
               LocalTimeInfo.Daylight_Saving_Time = doUnknown;
               LocalTimeInfo.Time_Zone            = tzUTCUnknown;

               /* Attempt to set the local time information.            */
               Result = CTS_Set_Local_Time_Information(BluetoothStackID, CTSInstanceID, &LocalTimeInfo);
               if((!Result) || (Result == BTPS_ERROR_FEATURE_NOT_AVAILABLE))
               {
                  /* Save the Bluetooth Stack ID.                       */
                  _BluetoothStackID = BluetoothStackID;

                  /* Flag that no error has occurred.                   */
                  Result            = 0;
               }
               else
               {
                  /* Error initializing CTS Framework.                  */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Error - CTS_Set_Local_Time_Information(): %d\n", Result));
               }
            }
            else
            {
               /* Error initializing CTS Framework.                     */
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS Service NOT registered: %d\n", Result));
            }

            /* Cleanup resources if an error occurred.                  */
            if(Result < 0)
            {
               if(CTSInstanceID)
               {
                  CTS_Cleanup_Service(BluetoothStackID, CTSInstanceID);

                  CTSInstanceID = 0;
               }
            }
         }
         else
         {
            /* Just note success.                                       */
            _BluetoothStackID = BluetoothStackID;
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         if(SupportedRoles & TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_SERVER)
            CTS_Cleanup_Service(_BluetoothStackID, CTSInstanceID);

         CTSInstanceID     = 0;

         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for querying the number of  */
   /* attributes that are used by the CTS Service registered by this    */
   /* module.                                                           */
unsigned int _TIPM_Query_Number_Attributes(void)
{
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply call the Bluetopia function to query the number of         */
   /* attributes.                                                       */
   NumberOfAttributes = CTS_Query_Number_Attributes();

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", NumberOfAttributes));

   return(NumberOfAttributes);
}

   /* The following function is provided to allow a mechanism to to     */
   /* fetch the current time of the current platform (and formats it    */
   /* into the format expected by the Current Time Service).  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _TIPM_Get_Current_Time(CTS_Current_Time_Data_t *CurrentTime, unsigned long AdjustMask)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Make sure the input parameter appears to be semi-valid.        */
      if(CurrentTime)
      {
         /* All that is left to do is to fetch the current time.        */
         if(_TIPM_Get_Current_Platform_Time(CurrentTime, AdjustMask))
            ret_val = 0;
         else
            ret_val = BTPM_ERROR_CODE_TIME_UNABLE_TO_READ_TIME;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for querying the Connection */
   /* ID of a specified connection.  The first parameter is the BD_ADDR */
   /* of the connection to query the Connection ID.  The second         */
   /* parameter is a pointer to return the Connection ID if this        */
   /* function is successful.  This function returns a zero if          */
   /* successful or a negative return error code if an error occurs.    */
int _TIPM_Query_Connection_ID(BD_ADDR_t BD_ADDR, unsigned int *ConnectionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT_Query_Connection_ID() returned %d\n", ret_val));

            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for responding to a Read    */
   /* Current Time Request that was received earlier.  This function    */
   /* accepts as input the Transaction ID of the request, an optional   */
   /* Error Code to respond with an error, and a optional pointer to a  */
   /* structure containing the current time.  This function returns zero*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If ErrorCode is equal to ZERO then Current_Time MUST     */
   /*          point to a structure containing the current time to      */
   /*          respond to the request.                                  */
   /* * NOTE * If ErrorCode is NON-ZERO then an error response will be  */
   /*          sent to the request.                                     */
int _TIPM_CTS_Current_Time_Read_Request_Response(unsigned int TransactionID, Byte_t ErrorCode, CTS_Current_Time_Data_t *Current_Time)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (CTSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if(TransactionID)
      {
         /* Respond to the read request.                                */
         if(ErrorCode)
            ret_val = CTS_Current_Time_Read_Request_Error_Response(_BluetoothStackID, TransactionID, ErrorCode);
         else
         {
            if(Current_Time)
               ret_val = CTS_Current_Time_Read_Request_Response(_BluetoothStackID, TransactionID, Current_Time);
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }

         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("%s() returned %d\n", (ErrorCode)?"CTS_Current_Time_Read_Request_Error_Response":"CTS_Current_Time_Read_Request_Response", ret_val));

            if(ret_val == CTS_ERROR_INSUFFICIENT_RESOURCES)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for setting the Local Time  */
   /* Information stored in the device.  This function accepts as input */
   /* a pointer to the Local Time Information to set.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_CTS_Set_Local_Time_Information(CTS_Local_Time_Information_Data_t *Local_Time)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (CTSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if(Local_Time)
      {
         /* Set the Local Time Information for the registered CTS       */
         /* Instance.                                                   */
         ret_val = CTS_Set_Local_Time_Information(_BluetoothStackID, CTSInstanceID, Local_Time);
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS_Set_Local_Time_Information() returned %d\n", ret_val));

            /* If this feature is for some reason compiled out of the   */
            /* CTS library we will return success but no remote devices */
            /* will be able to read the Local Time Information.         */
            if(ret_val == BTPS_ERROR_FEATURE_NOT_AVAILABLE)
               ret_val = 0;
            else
            {
               if(ret_val == CTS_ERROR_INSUFFICIENT_RESOURCES)
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for responding to a Read    */
   /* Reference Time Information Request that was received earlier.     */
   /* This function accepts as input the Transaction ID of the request, */
   /* an optional Error Code to respond with an error, and a optional   */
   /* pointer to a structure containing the Reference Time Information. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If ErrorCode is equal to ZERO Reference_Time Current_Time*/
   /*          MUST point to a structure containing the Reference Time  */
   /*          Information to respond to the request.                   */
   /* * NOTE * If ErrorCode is NON-ZERO then an error response will be  */
   /*          sent to the request.                                     */
int _TIPM_CTS_Reference_Time_Information_Read_Request_Response(unsigned int TransactionID, Byte_t ErrorCode, CTS_Reference_Time_Information_Data_t *Reference_Time)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (CTSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if(TransactionID)
      {
         /* Respond to the read request.                                */
         if(ErrorCode)
            ret_val = CTS_Reference_Time_Information_Read_Request_Error_Response(_BluetoothStackID, TransactionID, ErrorCode);
         else
         {
            if(Reference_Time)
               ret_val = CTS_Reference_Time_Information_Read_Request_Response(_BluetoothStackID, TransactionID, Reference_Time);
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }

         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("%s() returned %d\n", (ErrorCode)?"CTS_Reference_Time_Information_Read_Request_Error_Response":"CTS_Reference_Time_Information_Read_Request_Response", ret_val));

            if(ret_val == CTS_ERROR_INSUFFICIENT_RESOURCES)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for responding to a Read    */
   /* Client Configuration Request that was received earlier.  This     */
   /* function accepts as input the Transaction ID of the request and   */
   /* the Client Configuration to respond with.  This function returns  */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int _TIPM_CTS_Read_Client_Configuration_Response(unsigned int TransactionID, Word_t ClientConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (CTSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if(TransactionID)
      {
         /* Respond to the read request.                                */
         ret_val = CTS_Read_Client_Configuration_Response(_BluetoothStackID, CTSInstanceID, TransactionID, ClientConfiguration);
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS_Read_Client_Configuration_Response() returned %d\n", ret_val));

            if(ret_val == CTS_ERROR_INSUFFICIENT_RESOURCES)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a Current Time  */
   /* Notification to a specified connection.  This function accepts as */
   /* input the Connection ID of the connection to the device to notify */
   /* and the Current Time that is to be notified.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_CTS_Notify_Current_Time(unsigned int ConnectionID, CTS_Current_Time_Data_t *Current_Time)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (CTSInstanceID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (Current_Time))
      {
         /* Attempt to send the notification to the specified device.   */
         ret_val = CTS_Notify_Current_Time(_BluetoothStackID, CTSInstanceID, ConnectionID, Current_Time);
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS_Notify_Current_Time() returned %d\n", ret_val));

            if(ret_val == CTS_ERROR_INSUFFICIENT_RESOURCES)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a get current   */
   /* time request.  This function accepts as input the Connection ID   */
   /* of the connection to the device and the handle of the current     */
   /* time characteristic.  This function returns a positive value      */
   /* representing the transaction ID of the request, or a negative     */
   /* return error code if there was an error.                          */
int _TIPM_Get_Current_Time_Request(unsigned int ConnectionID, Word_t CurrentTimeHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (CurrentTimeHandle))
      {
         if((ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, CurrentTimeHandle, GATT_Client_Event_Callback, 0)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* time notifications.  This function accepts as input the Connection*/
   /* ID of the connection to the device and the handle of the current  */
   /* time client configuration characteristic decriptor.  This function*/
   /* returns a positive value representing the transaction ID of the   */
   /* request, or a negative return error code if there was an error.   */
int _TIPM_Enable_Time_Notifications(unsigned int ConnectionID, Word_t CurrentTimeCCCD, Boolean_t Enable)
{
   int              ret_val = 0;
   Word_t           CCCDValue;
   NonAlignedWord_t Buffer;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (CurrentTimeCCCD))
      {
         /* Check if we are enabled or disabling.                       */
         if(Enable)
         {
            CCCDValue = GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;

            /* If we are not tracking connection events, we need to to  */
            /* receive notifications.                                   */
            if(!GATTConnectionEventsCallbackID)
            {
               if((ret_val = GATT_Register_Connection_Events(_BluetoothStackID, GATT_Connection_Event_Callback, 0)) > 0)
               {
                  GATTConnectionEventsCallbackID = ret_val;
                  ret_val                        = 0;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
         }
         else
            CCCDValue = 0;

         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, CCCDValue);

         /* Now submit the request if there has not been an error.      */
         if(!ret_val)
            ret_val = GATT_Write_Request(_BluetoothStackID, ConnectionID, CurrentTimeCCCD, sizeof(Buffer), &Buffer, GATT_Client_Event_Callback, 0);

         if(ret_val < 0)
         {
            /* If we just registered a callback, we should un-register. */
            if((!NumberOfEnabledNotifications) && (Enable))
               GATT_Un_Register_Connection_Events(_BluetoothStackID, GATTConnectionEventsCallbackID);

            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
         else
            NumberOfEnabledNotifications += Enable?1:-1;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a get local     */
   /* time information request.  This function accepts as input the     */
   /* Connection ID of the connection to the device and the handle of   */
   /* the local time information characteristic.  This function returns */
   /* a positive value representing the transaction ID of the request,  */
   /* or a negative return error code if there was an error.            */
int _TIPM_Get_Local_Time_Information(unsigned int ConnectionID, Word_t LocalTimeInformationHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (LocalTimeInformationHandle))
      {
         if((ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, LocalTimeInformationHandle, GATT_Client_Event_Callback, 0)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a get time      */
   /* accuracy request.  This function accepts as input the Connection  */
   /* ID of the connection to the device and the handle of the reference*/
   /* time information characteristic.  This function returns a positive*/
   /* value representing the transaction ID of the request, or a        */
   /* negative return error code if there was an error.                 */
int _TIPM_Get_Time_Accuracy(unsigned int ConnectionID, Word_t ReferenceTimeInformationHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (ReferenceTimeInformationHandle))
      {
         if((ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, ReferenceTimeInformationHandle, GATT_Client_Event_Callback, 0)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a get next DST  */
   /* change request.  This function accepts as input the Connection    */
   /* ID of the connection to the device and the handle of the time     */
   /* with DST characteristic.  This function returns a positive value  */
   /* representing the transaction ID of the request, or a negative     */
   /* return error code if there was an error.                          */
int _TIPM_Get_Next_DST_Change_Information(unsigned int ConnectionID, Word_t TimeWithDSTHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (TimeWithDSTHandle))
      {
         if((ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, TimeWithDSTHandle, GATT_Client_Event_Callback, 0)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for getting the reference   */
   /* time update state.  This function accepts as input the Connection */
   /* ID of the connection to the device and the handle of the time     */
   /* update state characteristic.  This function returns a positive    */
   /* value representing the transaction ID of the request, or a        */
   /* negative return error code if there was an error.                 */
int _TIPM_Get_Reference_Time_Update_State(unsigned int ConnectionID, Word_t TimeUpdateStateHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (TimeUpdateStateHandle))
      {
         if((ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, TimeUpdateStateHandle, GATT_Client_Event_Callback, 0)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for requesting a reference  */
   /* time update.  This function accepts as input the Connection ID of */
   /* the connection to the device and the handle of the time update    */
   /* control point characteristic.  This function returns a positive   */
   /* value representing number of bytes written, or a negative return  */
   /* error code if there was an error.                                 */
int _TIPM_Request_Reference_Time_Update(unsigned int ConnectionID, Word_t TimeUpdateControlPointHandle)
{
   int    ret_val;
   Byte_t AttributeValue;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ConnectionID) && (TimeUpdateControlPointHandle))
      {
         AttributeValue = RTUS_TIME_UPDATE_CONTROL_POINT_GET_REFERENCE_UPDATE;

         if((ret_val = GATT_Write_Without_Response_Request(_BluetoothStackID, ConnectionID, TimeUpdateControlPointHandle, 1, (void *)&AttributeValue)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Current Time      */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the CTS_Current_Time_Data        */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Current_Time(unsigned int ValueLength, Byte_t *Value, CTS_Current_Time_Data_t *CTSCurrentTime)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ValueLength) && (Value) && (CTSCurrentTime))
      {
         if((ret_val = CTS_Decode_Current_Time(ValueLength, Value, CTSCurrentTime)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Local Time        */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the CTS_Local_Time_Data structure*/
   /* in which to store the decoded information. This function returns  */
   /* zero if successful and a negative return error code if there was  */
   /* an error.                                                         */
int _TIPM_Decode_Local_Time_Information(unsigned int ValueLength, Byte_t *Value, CTS_Local_Time_Information_Data_t *CTSLocalTime)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ValueLength) && (Value) && (CTSLocalTime))
      {
         if((ret_val = CTS_Decode_Local_Time_Information(ValueLength, Value, CTSLocalTime)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Reference Time    */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the CTS_Reference_Time_Data      */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Reference_Time_Information(unsigned int ValueLength, Byte_t *Value, CTS_Reference_Time_Information_Data_t *CTSReferenceTime)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ValueLength) && (Value) && (CTSReferenceTime))
      {
         if((ret_val = CTS_Decode_Reference_Time_Information(ValueLength, Value, CTSReferenceTime)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Time With DST     */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the NDCS_Time_With_DST_Data      */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Time_With_DST(unsigned int ValueLength, Byte_t *Value, NDCS_Time_With_Dst_Data_t *NDCSTimeWithDST)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ValueLength) && (Value) && (NDCSTimeWithDST))
      {
         if((ret_val = NDCS_Decode_Time_With_Dst(ValueLength, Value, NDCSTimeWithDST)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Time Update State */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the RTUS_Time_Update_State_Data  */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Time_Update_State(unsigned int ValueLength, Byte_t *Value, RTUS_Time_Update_State_Data_t *RTUSTimeUpdateState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Verify that the input parameters are semi-valid.               */
      if((ValueLength) && (Value) && (RTUSTimeUpdateState))
      {
         if((ret_val = RTUS_Decode_Time_Update_State(ValueLength, Value, RTUSTimeUpdateState)) < 0)
         {
            if(ret_val == BTPS_ERROR_INVALID_PARAMETER)
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            else
               ret_val = BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

