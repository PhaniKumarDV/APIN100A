/*****< antmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANTMGR - ANT Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/30/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMANTM.h"            /* BTPM ANT Manager Prototypes/Constants.    */
#include "ANTMGR.h"              /* ANT Manager Impl. Prototypes/Constants.   */

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
   /* _ANTM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variable which holds the Initialization Flags to pass down to the */
   /* ANT module.                                                       */
static unsigned long InitializationFlags;

   /* Internal Function Prototypes.                                     */
static int ConvertANTToPMError(int BluetopiaErrorCode);

static void BTPSAPI ANT_Event_Callback(unsigned int BluetoothStackID, ANT_Event_Data_t *ANT_Event_Data, unsigned long CallbackParameter);

   /* The following function is a utility function which is used to     */
   /* convert a Bluetopia error code into a PM error code.              */
static int ConvertANTToPMError(int BluetopiaErrorCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: Bluetopia error = %d\n", BluetopiaErrorCode));

   switch(BluetopiaErrorCode)
   {
      default:
      case BTANT_ERROR_INVALID_PARAMETER:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
      case BTANT_ERROR_INVALID_BLUETOOTH_STACK_ID:
      case BTANT_ERROR_NOT_INITIALIZED:
         ret_val = BTPM_ERROR_CODE_ANT_NOT_INITIALIZED;
         break;
      case BTANT_ERROR_INSUFFICIENT_RESOURCES:
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         break;
      case BTANT_ERROR_UNABLE_TO_SEND_ANT_MESSAGE:
         ret_val = BTPM_ERROR_CODE_ANT_UNABLE_TO_SEND_ANT_MESSAGE;
         break;
      case BTANT_ERROR_ANT_MESSAGE_RESPONSE_ERROR:
         ret_val = BTPM_ERROR_CODE_ANT_MESSAGE_RESPONSE_ERROR;
         break;
      case BTANT_ERROR_INVALID_NETWORK_KEY:
         ret_val = BTPM_ERROR_CODE_ANT_INVALID_NETWORK_KEY;
         break;
      case BTANT_ERROR_FEATURE_NOT_ENABLED:
         ret_val = BTPM_ERROR_CODE_ANT_PLUS_NOT_CURRENTLY_ENABLED;
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function the function that is installed to process  */
   /* ANT Events from the stack.                                        */
static void BTPSAPI ANT_Event_Callback(unsigned int BluetoothStackID, ANT_Event_Data_t *ANT_Event_Data, unsigned long CallbackParameter)
{
   ANTM_Update_Data_t *ANTMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((ANT_Event_Data) && (ANT_Event_Data->Event_Data_Size))
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Event: %u, Event Length: %u\n", (unsigned int)ANT_Event_Data->Event_Data_Type, (unsigned int)ANT_Event_Data->Event_Data_Size));

      /* Flag that there is no Event to dispatch.                       */
      ANTMUpdateData = NULL;

      switch(ANT_Event_Data->Event_Data_Type)
      {
         case etStartup_Message:
         case etChannel_Response:
         case etChannel_Status:
         case etChannel_ID:
         case etANT_Version:
         case etCapabilities:
         case etPacket_Broadcast_Data:
         case etPacket_Acknowledged_Data:
         case etPacket_Burst_Data:
         case etPacket_Extended_Broadcast_Data:
         case etPacket_Extended_Acknowledged_Data:
         case etPacket_Extended_Burst_Data:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((ANT_Event_Data->Event_Data.ANT_Startup_Message_Event_Data) && ((ANTMUpdateData = (ANTM_Update_Data_t *)BTPS_AllocateMemory(sizeof(ANTM_Update_Data_t))) != NULL))
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               ANTMUpdateData->UpdateType                              = utANTEvent;
               ANTMUpdateData->UpdateData.ANTEventData.Event_Data_Type = ANT_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(ANTMUpdateData->UpdateData.ANTEventData.Event_Data), ANT_Event_Data->Event_Data.ANT_Startup_Message_Event_Data, ANT_Event_Data->Event_Data_Size);
            }
            break;
         case etRaw_Packet_Data:
            /* Make sure that we have event data.                       */
            if(ANT_Event_Data->Event_Data.ANT_Raw_Packet_Data_Event_Data)
            {
               /* Attempt to allocate update data large enough.         */
               if((ANTMUpdateData = (ANTM_Update_Data_t *)BTPS_AllocateMemory(sizeof(ANTM_Update_Data_t) + ANT_Event_Data->Event_Data.ANT_Raw_Packet_Data_Event_Data->PacketLength)) != NULL)
               {
                  ANTMUpdateData->UpdateType                                                                     = utANTEvent;
                  ANTMUpdateData->UpdateData.ANTEventData.Event_Data_Type                                        = etRaw_Packet_Data;
                  ANTMUpdateData->UpdateData.ANTEventData.Event_Data.ANT_Raw_Packet_Data_Event_Data.PacketLength = ANT_Event_Data->Event_Data.ANT_Raw_Packet_Data_Event_Data->PacketLength;
                  ANTMUpdateData->UpdateData.ANTEventData.Event_Data.ANT_Raw_Packet_Data_Event_Data.PacketData   = ((Byte_t*)ANTMUpdateData) + sizeof(ANTM_Update_Data_t);

                  BTPS_MemCopy(ANTMUpdateData->UpdateData.ANTEventData.Event_Data.ANT_Raw_Packet_Data_Event_Data.PacketData, ANT_Event_Data->Event_Data.ANT_Raw_Packet_Data_Event_Data->PacketData, ANT_Event_Data->Event_Data.ANT_Raw_Packet_Data_Event_Data->PacketLength);
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(ANTMUpdateData)
      {
         if(!ANTM_NotifyUpdate(ANTMUpdateData))
            BTPS_FreeMemory((void *)ANTMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the ANT Manager Implementation.  The function accpets  */
   /* as a parameter the flags with which to initialize the ANT module. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error initializing the Bluetopia       */
   /* Platform Manager ANT Manager Implementation.                      */
int _ANTM_Initialize(unsigned long Flags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANT Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized         = TRUE;
      InitializationFlags = Flags;

      ret_val             = 0;
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
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the ANT       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the ANT Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _ANTM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Stack has been powered up, so register an ANS Server        */
         /* Instance.                                                   */
         Result = ANT_Initialize(BluetoothStackID, InitializationFlags, ANT_Event_Callback, 0);
         if(!Result)
         {
            /* ANT Framework initialized successfully.                  */
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANT Framework Initialized: %lu\n", InitializationFlags));

            /* Save the Bluetooth Stack ID.                             */
            _BluetoothStackID = BluetoothStackID;

            /* Flag that no error has occurred.                         */
            Result            = 0;
         }
         else
         {
            /* Error initializing ANT Framework.                        */
            DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANT_Initialize(): %d\n", Result));
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         ANT_Cleanup(_BluetoothStackID);

         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Configuration Message API.                                        */

   /* The following function is responsible for assigning an ANT channel*/
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument, the channel number to register.  This function accepts  */
   /* as it's second argument, the channel type to be assigned to the   */
   /* channel.  This function accepts as it's third argument, the       */
   /* network number to be used for the channel.  Zero should be        */
   /* specified for this argument to use the default public network.    */
   /* This function accepts as it's fourth argument, the extended       */
   /* assignment to be used for the channel.  Zero should be specified  */
   /* for this argument if no extended capabilities are to be used.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Assign_Channel(unsigned int ChannelNumber, unsigned int ChannelType, unsigned int NetworkNumber, unsigned int ExtendedAssignment)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Assign_Channel(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)ChannelType, (Byte_t)NetworkNumber, (Byte_t)ExtendedAssignment);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for un-assigning an ANT     */
   /* channel on the local ANT+ system.  A channel must be unassigned   */
   /* before it can be reassigned using the ANTM_Assign_Channel() API.  */
   /* This function accepts as it's only argument, the channel number to*/
   /* un-assign.  This function returns zero if successful, otherwise   */
   /* this function returns a negative error code.                      */
int _ANTM_Un_Assign_Channel(unsigned int ChannelNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Unassign_Channel(_BluetoothStackID, (Byte_t)ChannelNumber);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function accepts as it's  */
   /* first argument, the channel number to configure.  The ANT channel */
   /* must be assigned using ANTM_Assign_Channel() before calling this  */
   /* function.  This function accepts as it's second argument, the     */
   /* device number to search for on the channel.  Zero should be       */
   /* specified for this argument to scan for any device number.  This  */
   /* function accepts as it's third argument, the device type to search*/
   /* for on the channel.  Zero should be specified for this argument to*/
   /* scan for any device type.  This function accepts as it's fourth   */
   /* argument, the transmission type to search for on the channel.     */
   /* Zero should be specified for this argument to scan for any        */
   /* transmission type.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
int _ANTM_Set_Channel_ID(unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Channel_ID(_BluetoothStackID, (Byte_t)ChannelNumber, (Word_t)DeviceNumber, (Byte_t)DeviceType, (Byte_t)TransmissionType);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the local ANT+ system.     */
   /* This function accepts as it's first argument, the channel number  */
   /* to configure.  This function accepts as it's second argument, the */
   /* channel messaging period to set on the channel.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
int _ANTM_Set_Channel_Period(unsigned int ChannelNumber, unsigned int MessagingPeriod)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Channel_Period(_BluetoothStackID, (Byte_t)ChannelNumber, (Word_t)MessagingPeriod);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument, the    */
   /* channel number to configure.  This function accepts as it's second*/
   /* argument, the search timeout to set on the channel.  This function*/
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will result in an infinite search timeout.  Specifying   */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
int _ANTM_Set_Channel_Search_Timeout(unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Channel_Search_Timeout(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)SearchTimeout);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel.  This function accepts as it's first*/
   /* argument, the channel number to configure.  This function accepts */
   /* as it's second argument, the channel frequency to set on the      */
   /* channel.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (2400 + RFFrequency) MHz.                        */
int _ANTM_Set_Channel_RF_Frequency(unsigned int ChannelNumber, unsigned int RFFrequency)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Channel_RF_Frequency(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)RFFrequency);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the network */
   /* key for an ANT channel.  This function accepts as it's first      */
   /* argument, the channel number to configure.  This function accepts */
   /* as it's second argument, a pointer to the ANT network key to set  */
   /* on the channel.  This function returns zero if successful,        */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
int _ANTM_Set_Network_Key(unsigned int NetworkNumber, ANT_Network_Key_t NetworkKey)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Network_Key(_BluetoothStackID, (Byte_t)NetworkNumber, NetworkKey);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the transmit*/
   /* power on the local ANT system.  This function accepts as it's only*/
   /* argument, the transmit power to set on the device.  This function */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
int _ANTM_Set_Transmit_Power(unsigned int TransmitPower)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Transmit_Power(_BluetoothStackID, (Byte_t)TransmitPower);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function accepts*/
   /* as it's first argument, the channel number to add to the list.    */
   /* This function accepts as it's second argument, the device number  */
   /* to add to the list.  This function accepts as it's third argument,*/
   /* the device type to add to the list.  This function accepts as it's*/
   /* fourth argument, the transmission type to add to the list.  This  */
   /* function accepts as it's fifth argument, the the list index to    */
   /* overwrite with the updated entry.  This function returns zero if  */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Add_Channel_ID(unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType, unsigned int ListIndex)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Add_Channel_ID(_BluetoothStackID, (Byte_t)ChannelNumber, (Word_t)DeviceNumber, (Byte_t)DeviceType, (Byte_t)TransmissionType, (Byte_t)ListIndex);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the local ANT+ system.  This        */
   /* function accepts as it's first argument, the channel number on    */
   /* which the list should be configured.  This function accepts as    */
   /* it's second argument, the size of the list.  This function accepts*/
   /* as it's third argument, the list type.  Zero should be specified  */
   /* to configure the list for inclusion, and one should be specified  */
   /* to configure the list for exclusion.  This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Configure_Inclusion_Exclusion_List(unsigned int ChannelNumber, unsigned int ListSize, unsigned int Exclude)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Config_List(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)ListSize, (Byte_t)Exclude);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel.  This function accepts as it's first    */
   /* argument, the channel number to configure.  This function accepts */
   /* as it's second argument, the transmit power level for the         */
   /* specified channel.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_Channel_Transmit_Power(unsigned int ChannelNumber, unsigned int TransmitPower)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Channel_Transmit_Power(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)TransmitPower);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument, the channel number to configure.  */
   /* This function accepts as it's second argument, the search timeout */
   /* to set on the channel.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will result in an   */
   /*          infinite low priority search timeout.                    */
int _ANTM_Set_Low_Priority_Channel_Search_Timeout(unsigned int ChannelNumber, unsigned int SearchTimeout)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Low_Priority_Channel_Search_Timeout(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)SearchTimeout);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function configures the   */
   /* channel ID in the same way as ANTM_Set_Channel_ID(), except it    */
   /* uses the two LSB of the device's serial number as the device's    */
   /* number.  This function accepts as it's first argument, the channel*/
   /* number to configure.  The ANT channel must be assigned using      */
   /* ANTM_Assign_Channel() before calling this function.  This function*/
   /* accepts as it's second argument, the device type to search for on */
   /* the channel.  Zero should be specified for this argument to scan  */
   /* for any device type.  This function accepts as it's third         */
   /* argument, the transmission type to search for on the channel.     */
   /* Zero should be specified for this argument to scan for any        */
   /* transmission type.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Set_Serial_Number_Channel_ID(unsigned int ChannelNumber, unsigned int DeviceType, unsigned int TransmissionType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Serial_Number_Channel_ID(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)DeviceType, (Byte_t)TransmissionType);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the local ANT+ system. */
   /* This function accepts as it's only argument, whether or not to    */
   /* enable extended Rx messages.  This function returns zero if       */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Enable_Extended_Messages(unsigned int Enable)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Enable_Extended_Messages(_BluetoothStackID, (Byte_t)Enable);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* the LED on the local ANT+ system.  This function accepts as it's  */
   /* first argument, whether or not to enable the LED.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Enable_LED(unsigned int Enable)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Enable_LED(_BluetoothStackID, (Byte_t)Enable);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the local ANT+ system.  This function returns    */
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
int _ANTM_Enable_Crystal(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Crystal_Enable(_BluetoothStackID);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the local ANT+ system.  This function */
   /* accepts as it's first argument, the bitmask of extended Rx        */
   /* messages that shall be enabled or disabled.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Configure_Extended_Messages(unsigned int EnabledExtendedMessagesMask)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Lib_Config(_BluetoothStackID, (Byte_t)EnabledExtendedMessagesMask);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel.  This function accepts  */
   /* as it's first argument, the channel number to configure.  This    */
   /* function accepts as it's second, third, and fourth arguments, the */
   /* three operating agility frequencies to set.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANTM_Assign_Channel.  Frequency agility      */
   /*          should NOT be used with shared, Tx only, or Rx only      */
   /*          channels.                                                */
int _ANTM_Configure_Frequency_Agility(unsigned int ChannelNumber, unsigned int FrequencyAgility1, unsigned int FrequencyAgility2, unsigned int FrequencyAgility3)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Config_Frequency_Agility(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)FrequencyAgility1, (Byte_t)FrequencyAgility2, (Byte_t)FrequencyAgility3);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the local ANT+ system.  This      */
   /* function accepts as it's first argument, the channel number to    */
   /* configure.  This function accepts as it's second argument, the    */
   /* search threshold to set.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
int _ANTM_Set_Proximity_Search(unsigned int ChannelNumber, unsigned int SearchThreshold)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Proximity_Search(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)SearchThreshold);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the local ANT+ system.  This        */
   /* function accepts as it's first argument, the channel number to    */
   /* configure.  This function accepts as it's second argument, the    */
   /* search priority to set.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_Channel_Search_Priority(unsigned int ChannelNumber, unsigned int SearchPriority)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_Channel_Search_Priority(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)SearchPriority);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the local ANT+ system.  This function accepts*/
   /* as it's first argument, the descriptor string type to set.  This  */
   /* function accepts as it's second argument, the NULL-terminated     */
   /* descriptor string to be set.  This function returns zero if       */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_USB_Descriptor_String(unsigned int StringNumber, char *DescriptorString)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_USB_Descriptor_String(_BluetoothStackID, (Byte_t)StringNumber, DescriptorString);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Control Message API.                                              */

   /* The following function is responsible for resetting the ANT module*/
   /* on the local ANT+ system.  A delay of at least 500ms is suggested */
   /* after calling this function to allow time for the module to reset.*/
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Reset_System(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Reset_System(_BluetoothStackID);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for opening an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument, the channel number to be opened.  The channel specified */
   /* must have been assigned and configured before calling this        */
   /* function.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
int _ANTM_Open_Channel(unsigned int ChannelNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Open_Channel(_BluetoothStackID, (Byte_t)ChannelNumber);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for closing an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument, the channel number to be opened.  This function returns */
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the aetANTMChannelResponse event has been received */
   /*          with the Message_Code member specifying:                 */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
int _ANTM_Close_Channel(unsigned int ChannelNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Close_Channel(_BluetoothStackID, (Byte_t)ChannelNumber);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument, the channel number  */
   /* that the request will be sent to.  This function accepts as it's  */
   /* second argument, the message ID being requested from the channel. */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Request_Message(unsigned int ChannelNumber, unsigned int MessageID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Request_Message(_BluetoothStackID, (Byte_t)ChannelNumber, (Byte_t)MessageID);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the local ANT+ system.  This function  */
   /* accepts as it's first argument, the channel number to be opened.  */
   /* The channel specified must have been assigned and configured as a */
   /* SLAVE Rx ONLY channel before calling this function.  This function*/
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
int _ANTM_Open_Rx_Scan_Mode(unsigned int ChannelNumber)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Open_Rx_Scan_Mode(_BluetoothStackID, (Byte_t)ChannelNumber);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for putting the ANT+ system */
   /* in ultra low-power mode.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(!MSGREADY) line on the ANT chip to the appropriate*/
   /*          value.                                                   */
int _ANTM_Sleep_Message(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Sleep_Message(_BluetoothStackID);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Data Message API.                                                 */

   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the local ANT+ system.  This function      */
   /* accepts as it's first argument, the channel number that the data  */
   /* will be broadcast on.  This function accepts as it's second       */
   /* argument the length of the data to send.  This function accepts as*/
   /* it's third argument a pointer to a byte array of the broadcast    */
   /* data to send.  This function returns zero if successful, otherwise*/
   /* this function returns a negative error code.                      */
int _ANTM_Send_Broadcast_Data(unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (DataLength >= ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE) && (Data))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Send_Broadcast(_BluetoothStackID, (Byte_t)ChannelNumber, Data);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument, the channel number that the data  */
   /* will be sent on.  This function accepts as it's second argument   */
   /* the length of the data to send.  This function accepts as it's    */
   /* third argument, a pointer to a byte array of the acknowledged data*/
   /* to send.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
int _ANTM_Send_Acknowledged_Data(unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (DataLength >= ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE) && (Data))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Send_Acknowledged(_BluetoothStackID, (Byte_t)ChannelNumber, Data);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument, the sequence / channel number that*/
   /* the data will be sent on.  The upper three bits of this argument  */
   /* are the sequence number, and the lower five bits are the channel  */
   /* number.  This function accepts as it's second argument the length */
   /* of the data to send.  This function accepts as it's third         */
   /* argument, a pointer to a byte array of the burst data to send.    */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Send_Burst_Transfer_Data(unsigned int SequenceChannelNumber, unsigned int DataLength, Byte_t *Data)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID) && (DataLength >= ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE) && (Data))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Send_Burst_Transfer(_BluetoothStackID, (Byte_t)SequenceChannelNumber, Data);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Test Mode Message API.                                            */

   /* The following function is responsible for putting the ANT+ system */
   /* in CW test mode.  This function returns zero if successful,       */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
int _ANTM_Initialize_CW_Test_Mode(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Initialize_CW_Test_Mode(_BluetoothStackID);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument, the     */
   /* transmit power level to be used.  This function accepts as it's   */
   /* second argument, the RF frequency to be used.  This function      */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANTM_Initialize_CW_Test_Mode().                  */
int _ANTM_Set_CW_Test_Mode(unsigned int TxPower, unsigned int RFFrequency)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Set_CW_Test_Mode(_BluetoothStackID, (Byte_t)TxPower, (Byte_t)RFFrequency);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a raw ANT       */
   /* packet.  This function accepts as it's first argument, the size   */
   /* of the packet data buffer.  This function accepts as it's second  */
   /* argument, a pointer to a buffer containing the ANT packet to be   */
   /* sent. This function returns zero if successful, otherwise this    */
   /* function returns a negative error code.                           */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int _ANTM_Send_Raw_Packet(unsigned int DataSize, Byte_t *PacketData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (_BluetoothStackID))
   {
      /* Send the ANT Command.                                          */
      ret_val = ANT_Send_Raw_Packet(_BluetoothStackID, DataSize, PacketData);

      /* Check to see if an error occurred.                             */
      if(ret_val != 0)
      {
         /* Convert the ANT error code to a PM error code.              */
         ret_val = ConvertANTToPMError(ret_val);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_ANT_PLUS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}
