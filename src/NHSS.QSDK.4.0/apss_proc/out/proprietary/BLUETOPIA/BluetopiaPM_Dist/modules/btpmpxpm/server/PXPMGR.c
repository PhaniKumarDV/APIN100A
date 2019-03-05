/*****< PXPmgr.c >*************************************************************/
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
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMPXPM.h"            /* BTPM PXP Manager Prototypes/Constants.    */
#include "PXPMGR.h"              /* PXP Manager Impl. Prototypes/Constants.   */

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
   /* _PXPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI GATT_Client_Event_Callback(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static int MAPGATTErrorCode(int ErrorCodeToMap);

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
   PXPM_Update_Data_t *PXPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: PXPM\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((GATT_Client_Event_Data) && (GATT_Client_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      PXPMUpdateData = NULL;

      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
         case etGATT_Client_Write_Response:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data) && ((PXPMUpdateData = (PXPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PXPM_Update_Data_t))) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               PXPMUpdateData->UpdateType                                 = utGATTClientEvent;
               PXPMUpdateData->UpdateData.ClientEventData.Event_Data_Type = GATT_Client_Event_Data->Event_Data_Type;
               PXPMUpdateData->UpdateData.ClientEventData.Event_Data_Size = GATT_Client_Event_Data->Event_Data_Size;

               BTPS_MemCopy(&(PXPMUpdateData->UpdateData.ClientEventData.Event_Data), GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data, GATT_Client_Event_Data->Event_Data_Size);
            }
            break;
         case etGATT_Client_Read_Response:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data) && ((PXPMUpdateData = (PXPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PXPM_Update_Data_t) + GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength)) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               PXPMUpdateData->UpdateType                                 = utGATTClientEvent;
               PXPMUpdateData->UpdateData.ClientEventData.Event_Data_Type = GATT_Client_Event_Data->Event_Data_Type;
               PXPMUpdateData->UpdateData.ClientEventData.Event_Data_Size = GATT_Client_Event_Data->Event_Data_Size;

               BTPS_MemCopy(&(PXPMUpdateData->UpdateData.ClientEventData.Event_Data), GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data, GATT_Client_Event_Data->Event_Data_Size);

               /* Fix up the pointer to the data that was read.         */
               PXPMUpdateData->UpdateData.ClientEventData.Event_Data.GATT_Read_Response_Data.AttributeValue = (Byte_t *)(((Byte_t *)PXPMUpdateData) + ((unsigned int)sizeof(PXPM_Update_Data_t)));

               /* Copy the data that was read.                          */
               BTPS_MemCopy(PXPMUpdateData->UpdateData.ClientEventData.Event_Data.GATT_Read_Response_Data.AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(PXPMUpdateData)
      {
         if(!PXPM_NotifyUpdate(PXPMUpdateData))
            BTPS_FreeMemory((void *)PXPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: PXPM\n"));
}

   /* The following function is a utility function that is used to map a*/
   /* GATT error code to a PM error code.  The caller MUST ensure that  */
   /* an error has occurred before calling this function.               */
static int MAPGATTErrorCode(int ErrorCodeToMap)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: GATT Error Code = %d\n", ErrorCodeToMap));

   switch(ErrorCodeToMap)
   {
      case BTPS_ERROR_LOCAL_CONTROLLER_DOES_NOT_SUPPORT_LE:
      case BTPS_ERROR_FEATURE_NOT_AVAILABLE:
      case BTGATT_ERROR_NOT_INITIALIZED:
         ret_val = BTPM_ERROR_CODE_LOW_ENERGY_NOT_SUPPORTED;
         break;
      case BTGATT_ERROR_INVALID_PARAMETER:
      default:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: PXPM Error Code = %d\n", ret_val));

   return(ret_val);
}

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
      Initialized                     = TRUE;

      ret_val                         = 0;
   }
   else
      ret_val = 0;

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
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the PXP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the PXP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _PXPM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Save the Bluetooth Stack ID.                                */
         _BluetoothStackID = BluetoothStackID;
      }
      else
      {
         /* Stack has been shutdown.                                    */
         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the Connection ID for a specified LE connection. */
   /* The function accepts the BD_ADDR of the connection to query the   */
   /* Connection ID for and a pointer to store the Connection ID for.   */
   /* This functions returns ZERO on success or a negative error code.  */
int _PXPM_Query_Connection_ID(BD_ADDR_t BD_ADDR, unsigned int *ConnectionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Query the Connection ID.                                       */
      ret_val = GATT_Query_Connection_ID(_BluetoothStackID, gctLE, BD_ADDR, ConnectionID);

      /* Check to see if an error occurred and the error code needs to  */
      /* be mapped to something different.                              */
      if(ret_val < 0)
      {
         /* MAP the GATT error code to a PM error code.                 */
         ret_val = MAPGATTErrorCode(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to read a specified Attribute on the specified remote     */
   /* device.  The function accepts the ConnectionID of the Connection  */
   /* to read the value from, and the Attribute Handle of the attribute */
   /* on the remote device to read.  This function returns a positive   */
   /* non-zero value if successful, or a negative return error code if  */
   /* there was an error.                                               */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _PXPM_Read_Value(unsigned int ConnectionID, Word_t AttributeHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Reading Attribute Value 0x%02X\n", AttributeHandle));

      /* Read the GATT Value.                                           */
      ret_val = GATT_Read_Value_Request(_BluetoothStackID, ConnectionID, AttributeHandle, GATT_Client_Event_Callback, 0);

      /* Check to see if an error occurred and the error code needs to  */
      /* be mapped to something different.                              */
      if(ret_val < 0)
      {
         /* MAP the GATT error code to a PM error code.                 */
         ret_val = MAPGATTErrorCode(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to write a specified Attribute on the specified remote    */
   /* device.  The function accepts the ConnectionID of the Connection  */
   /* to write the value to, the Attribute Handle of the attribute on   */
   /* the remote device to write, the length of the data and a pointer  */
   /* to the data to write.  This function returns a positive non-zero  */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _PXPM_Write_Value(unsigned int ConnectionID, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Writing Attribute Value (Request) 0x%02X\n", AttributeHandle));

      ret_val = GATT_Write_Request(_BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)DataLength, (void *)Data, GATT_Client_Event_Callback, 0);

      /* Check to see if an error occurred and the error code needs to  */
      /* be mapped to something different.                              */
      if(ret_val < 0)
      {
         /* MAP the GATT error code to a PM error code.                 */
         ret_val = MAPGATTErrorCode(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to perform a write without response to a specified        */
   /* Attribute on the specified remote device.  The first parameter to */
   /* this function is the ConnectionID of the device to write to.  The */
   /* second parameter specifies the handle of the attribute that is to */
   /* be written.  The final two parameters specify the length and a    */
   /* pointer to the data that is to be written.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * No event is generated by this function.                  */
int _PXPM_Write_Value_Without_Response(unsigned int ConnectionID, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Writing Attribute Value (Command) 0x%02X\n", AttributeHandle));

       ret_val = GATT_Write_Without_Response_Request(_BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)DataLength, (void *)Data);

      /* Check to see if an error occurred and the error code needs to  */
      /* be mapped to something different.                              */
      if(ret_val < 0)
      {
         /* MAP the GATT error code to a PM error code.                 */
         ret_val = MAPGATTErrorCode(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* cancel an outstanding transaction.  The first parameter to this   */
   /* function is the TransactionID of the transaction that is being    */
   /* canceled.  On success this function will return ZERO or a negative*/
   /* error code on failure.                                            */
int _PXPM_Cancel_Transaction(unsigned int TransactionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Canceling Transaction %u\n", TransactionID));

      ret_val = GATT_Cancel_Transaction(_BluetoothStackID, TransactionID);

      /* Check to see if an error occurred and the error code needs to  */
      /* be mapped to something different.                              */
      if(ret_val < 0)
      {
         /* MAP the GATT error code to a PM error code.                 */
         ret_val = MAPGATTErrorCode(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to get  */
   /* the RSSI for the specified BD_ADDR (of connected LE Device).      */
int _PXPM_Get_Link_RSSI(BD_ADDR_t BD_ADDR, int *RSSI)
{
   int                             ret_val;
   Byte_t                          Status;
   Word_t                          ConnectionHandle;
   Word_t                          ConnectionHandleResult;
   SByte_t                         RSSIResult;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (RSSI))
      {
         /* Query the remote device properties for this device.         */
         if(!(ret_val = DEVM_QueryRemoteDeviceProperties(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties)))
         {
            /* If we fail to get the connection handle using the BD_ADDR*/
            /* try the resolvable BD_ADDR.                              */
            if(GAP_LE_Query_Connection_Handle(_BluetoothStackID, RemoteDeviceProperties.BD_ADDR, &ConnectionHandle))
            {
               /* Try to get the handle with the prior resolvable       */
               /* BD_ADDR.                                              */
               if(!GAP_LE_Query_Connection_Handle(_BluetoothStackID, RemoteDeviceProperties.PriorResolvableBD_ADDR, &ConnectionHandle))
                  ret_val = 0;
               else
                  ret_val = BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE;
            }
            else
               ret_val = 0;

            /* Continue only if no error has occurred.                  */
            if(!ret_val)
            {
               /* Get the RSSI of the connection.                       */
               if((!(ret_val = HCI_Read_RSSI(_BluetoothStackID, ConnectionHandle, &Status, &ConnectionHandleResult, &RSSIResult))) && (Status == HCI_ERROR_CODE_NO_ERROR))
               {
                  /* Finally return the RSSI for the connection.        */
                  *RSSI = (int)RSSIResult;
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("HCI_Read_RSSI: %d:0x%02X\n", ret_val, Status));

                  ret_val = BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE;
               }
            }
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

