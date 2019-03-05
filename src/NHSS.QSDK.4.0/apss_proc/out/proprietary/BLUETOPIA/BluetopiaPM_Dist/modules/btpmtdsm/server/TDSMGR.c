/*****< tdsmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSMGR - 3D Sync Manager Implementation for Stonestreet One Bluetooth     */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMTDSM.h"            /* BTPM TDS Manager Prototypes/Constants.    */
#include "TDSMMSG.h"             /* BTPM TDS Manager Message Formats.         */
#include "TDSMGR.h"              /* TDS Manager Impl. Prototypes/Constants.   */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

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
   /* _HDSM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

static TDSM_Initialization_Info_t _TDSMInitializationInfo;

static DWord_t SDPHandle;

static TDS_Connectionless_Slave_Broadcast_Data_t CurrentCSBData;
static TDSM_Current_Broadcast_Information_t      CurrentInformation;

static unsigned long CurrentFlags;

static Mutex_t BroadcastInformationMutex;

#define TDSMGR_FLAGS_CURRENTLY_BROADCASTING                    0x00000001
#define TDSMGR_FLAGS_SYNC_TRAIN_CURRENTLY_ON                   0x00000002
#define TDSMGR_FLAGS_BROADCAST_3D                              0x00000004
#define TDSMGR_FLAGS_WAITING_CLOCK_CAPTURE                     0x00000008

#define TDS_CALLBACK_MUTEX_TIMEOUT                             1000

//#define _TDS_Start_Triggered_Clock_Capture TDS_Start_Triggered_Clock_Capture
//#define _TDS_Disable_Triggered_Clock_Capture TDS_Disable_Triggered_Clock_Capture

static unsigned int TimerID;

static int MapTDSErrorCode(int TDSErrorCode);

static int _TDS_Start_Triggered_Clock_Capture(unsigned int BluetoothStackID, Byte_t Cycles);
static int _TDS_Disable_Triggered_Clock_Capture(unsigned int BluetoothStackID);

static void ClockTimerCallback(unsigned int BluetoothStackID, unsigned int TimerID, unsigned long Parameter);

static void TDS_Event_Callback(unsigned int BluetoothStackID, TDS_Event_Data_t *EventData, unsigned long CallbackParameter);

   /* Maps an error code returned by the Bluetopia TDS profiles to a PM */
   /* error code.                                                       */
static int MapTDSErrorCode(int TDSErrorCode)
{
   int ret_val;

   switch(TDSErrorCode)
   {
      case BTTDS_ERROR_INVALID_PARAMETER:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
      case BTTDS_ERROR_NOT_INITIALIZED:
         ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;
         break;
      case BTTDS_ERROR_INVALID_BLUETOOTH_STACK_ID:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
      case BTTDS_ERROR_INSUFFICIENT_RESOURCES:
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         break;
      default:
         ret_val = BTPM_ERROR_CODE_3D_SYNC_UNKNOWN_ERROR;
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Error Code: %d\n", TDSErrorCode));
         break;
   }

   return(ret_val);
}

   /* Utility function to start clock capture. This will either start   */
   /* clock capture on the chip or start a timer for simulated capture  */
   /* depending on the module's Initialization Information.             */
static int _TDS_Start_Triggered_Clock_Capture(unsigned int BluetoothStackID, Byte_t Cycles)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(_TDSMInitializationInfo.SimulatedClockCapture)
   {
      /* Start a timer such that we simulated the estimated triggered   */
      /* clock capture timing given the frame rate and frames per event.*/
      if((ret_val = BSC_StartTimer(BluetoothStackID, (Cycles * 1000)/_TDSMInitializationInfo.SimulatedFrameRate, ClockTimerCallback, 0)) > 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Timer Started: %u\n", TimerID));

         TimerID = (unsigned int)ret_val;
         ret_val = 0;
      }
   }
   else
      ret_val = TDS_Start_Triggered_Clock_Capture(BluetoothStackID, Cycles);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return ret_val;
}

   /* Utility function to stop the current clock capture method.        */
static int _TDS_Disable_Triggered_Clock_Capture(unsigned int BluetoothStackID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(_TDSMInitializationInfo.SimulatedClockCapture)
   {
      BSC_StopTimer(BluetoothStackID, TimerID);
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Timer Stopped: %u\n", TimerID));

      TimerID = 0;
      ret_val = 0;
   }
   else
      ret_val = TDS_Disable_Triggered_Clock_Capture(BluetoothStackID);

   return ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Callback function for the timer used when simulating Triggered    */
   /* Clock Capture.                                                    */
static void ClockTimerCallback(unsigned int BluetoothStackID, unsigned int TimerID, unsigned long Parameter)
{
   int     Result;
   Byte_t  Status;
   Word_t  Handle;
   Word_t  Accuracy;
   DWord_t Clock;
   TDS_Event_Data_t EventData;
   TDS_Display_Triggered_Clock_Capture_Data_t Buffer;

   Result = HCI_Read_Clock(BluetoothStackID, 0, 0, &Status, &Handle, &Clock, &Accuracy);

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Clock Capture: %u\n", Clock));

   if((!Result) && (!Status))
   {
      EventData.Event_Data_Type                                     = etTDS_Display_Triggered_Clock_Capture;
      EventData.Event_Data_Size                                     = TDS_DISPLAY_TRIGGERED_CLOCK_CAPTURE_DATA_SIZE;
      EventData.Event_Data.TDS_Display_Triggered_Clock_Capture_Data = &Buffer;
      Buffer.NativeBluetoothClock                                   = Clock;
      Buffer.SlotOffset                                             = 0;

      TDS_Event_Callback(BluetoothStackID, &EventData, 0);

      if(BTPS_WaitMutex(BroadcastInformationMutex, BTPS_INFINITE_WAIT))
      {
         if((CurrentFlags & TDSMGR_FLAGS_BROADCAST_3D) && (CurrentFlags & TDSMGR_FLAGS_CURRENTLY_BROADCASTING))
         {
            if((Result = BSC_StartTimer(BluetoothStackID, (1000 * CurrentInformation.SyncsPerClockCapture)/_TDSMInitializationInfo.SimulatedFrameRate, ClockTimerCallback, 0)) > 0)
            {
               TimerID = (unsigned int)Result;
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Timer Startd: %u\n", TimerID));
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("waht: %d\n", Result));
         }

         BTPS_ReleaseMutex(BroadcastInformationMutex);
      }
   }
}

static void TDS_Event_Callback(unsigned int BluetoothStackID, TDS_Event_Data_t *EventData, unsigned long CallbackParameter)
{
   DWord_t                                    ClockDifference;
   DWord_t                                    Clock;
   TDSM_Update_Data_t                        *TDSMUpdateData;
   TDS_Connectionless_Slave_Broadcast_Data_t  NewBroadcastData;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if(EventData)
   {
      TDSMUpdateData = NULL;

      switch(EventData->Event_Data_Type)
      {
         case etTDS_Display_Connection_Announcement:
         case etTDS_Display_Synchronization_Train_Complete:
         case etTDS_Display_Channel_Map_Change:
            if(EventData->Event_Data_Size)
            {
               if((EventData->Event_Data.TDS_Display_Connection_Announcement_Data) && (TDSMUpdateData = (TDSM_Update_Data_t *)BTPS_AllocateMemory(sizeof(TDSM_Update_Data_t))) != NULL)
               {
                  /* Note the event type and copy the data.             */
                  TDSMUpdateData->UpdateType                        = utTDSEvent;
                  TDSMUpdateData->UpdateData.TDSEventData.EventType = EventData->Event_Data_Type;

                  BTPS_MemCopy(&(TDSMUpdateData->UpdateData.TDSEventData.EventData), EventData->Event_Data.TDS_Display_Connection_Announcement_Data, EventData->Event_Data_Size);
               }
            }
            break;
         case etTDS_Display_CSB_Supervision_Timeout:
            if(CurrentFlags & TDSMGR_FLAGS_BROADCAST_3D)
               _TDS_Disable_Triggered_Clock_Capture(BluetoothStackID);

            CurrentFlags &= ~((unsigned long)TDSMGR_FLAGS_CURRENTLY_BROADCASTING);

            /* Note our clock information is no longer valid.        */
            CurrentCSBData.FrameSyncInstant        = 0;
            CurrentCSBData.ClockPhase              = 0;
            CurrentCSBData.FrameSyncPeriod         = 0;
            CurrentCSBData.FrameSyncPeriodFraction = 0;

            /* Reset the LLS Open Offset back to 2D mode for any     */
            /* future broadcasts.                                    */
            CurrentCSBData.LeftLensShutterOpenOffset = TDS_BROADCAST_MESSAGE_LEFT_LENS_OPEN_2D_MODE;

            if((TDSMUpdateData = (TDSM_Update_Data_t *)BTPS_AllocateMemory(sizeof(TDSM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the data.                */
               TDSMUpdateData->UpdateType                        = utTDSEvent;
               TDSMUpdateData->UpdateData.TDSEventData.EventType = EventData->Event_Data_Type;
            }
            break;
         case etTDS_Display_Slave_Page_Response_Timeout:
            if((TDSMUpdateData = (TDSM_Update_Data_t *)BTPS_AllocateMemory(sizeof(TDSM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the data.                */
               TDSMUpdateData->UpdateType                        = utTDSEvent;
               TDSMUpdateData->UpdateData.TDSEventData.EventType = EventData->Event_Data_Type;
            }
            break;
         case etTDS_Display_Triggered_Clock_Capture:
            if((EventData->Event_Data_Size) && (EventData->Event_Data.TDS_Display_Triggered_Clock_Capture_Data))
            {
               if(BTPS_WaitMutex(BroadcastInformationMutex, TDS_CALLBACK_MUTEX_TIMEOUT))
               {
                  /* If we don't have a clock value yet, just set the   */
                  /* current clock.                                     */
                  if(!CurrentCSBData.FrameSyncInstant)
                  {
                     CurrentCSBData.FrameSyncInstant = EventData->Event_Data.TDS_Display_Triggered_Clock_Capture_Data->NativeBluetoothClock >> 1;
                     CurrentCSBData.ClockPhase       = EventData->Event_Data.TDS_Display_Triggered_Clock_Capture_Data->SlotOffset;
                  }
                  else
                  {
                     /* Note the current data so we can modify it.      */
                     NewBroadcastData.VideoMode = CurrentInformation.VideoMode;

                     /* Modify the new data.                            */
                     Clock = EventData->Event_Data.TDS_Display_Triggered_Clock_Capture_Data->NativeBluetoothClock >> 1;
                     ClockDifference                          = (Clock*(625) + (DWord_t)EventData->Event_Data.TDS_Display_Triggered_Clock_Capture_Data->SlotOffset) - (CurrentCSBData.FrameSyncInstant * (625) + (DWord_t)CurrentCSBData.ClockPhase);
                     NewBroadcastData.FrameSyncPeriod         = ClockDifference / CurrentInformation.SyncsPerClockCapture;
                     NewBroadcastData.FrameSyncPeriodFraction = ((ClockDifference % CurrentInformation.SyncsPerClockCapture) * 256) / CurrentInformation.SyncsPerClockCapture;
                     NewBroadcastData.FrameSyncInstant        = Clock;
                     NewBroadcastData.ClockPhase              = EventData->Event_Data.TDS_Display_Triggered_Clock_Capture_Data->SlotOffset;

                     NewBroadcastData.LeftLensShutterOpenOffset   = 0 + CurrentInformation.LeftLensShutterOpenOffset;
                     NewBroadcastData.LeftLensShutterCloseOffset  = (Word_t)((NewBroadcastData.FrameSyncPeriod >> 1) - 1 + CurrentInformation.LeftLensShutterCloseOffset);
                     NewBroadcastData.RightLensShutterOpenOffset  = (Word_t)((NewBroadcastData.FrameSyncPeriod >> 1) + CurrentInformation.RightLensShutterOpenOffset);
                     NewBroadcastData.RightLensShutterCloseOffset = (Word_t)(NewBroadcastData.FrameSyncPeriod - 1 + CurrentInformation.RightLensShutterCloseOffset);


                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Instant: %u\n", NewBroadcastData.FrameSyncInstant));
                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Phase: %u\n", NewBroadcastData.ClockPhase));
                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Period: %u\n", NewBroadcastData.FrameSyncPeriod));
                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Fraction: %u\n", NewBroadcastData.FrameSyncPeriodFraction));
                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("LLS Open: %u\n", NewBroadcastData.LeftLensShutterOpenOffset));
                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("LLS Close: %u\n", NewBroadcastData.LeftLensShutterCloseOffset));
                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("RLS Open: %u\n", NewBroadcastData.RightLensShutterOpenOffset));
                     DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("RLS Close: %u\n", NewBroadcastData.RightLensShutterCloseOffset));

                     /* If we are waiting to for clock to switch to 3D, */
                     /* set the LLS offset to make the switch.          */
                     if(CurrentFlags & TDSMGR_FLAGS_WAITING_CLOCK_CAPTURE)
                        CurrentFlags &= ~((unsigned long)TDSMGR_FLAGS_WAITING_CLOCK_CAPTURE);

                     if(CurrentFlags & TDSMGR_FLAGS_BROADCAST_3D)
                     {
                        /* Attempt to set the new data.                 */
                        if(!TDS_Set_Connectionless_Slave_Broadcast_Data(BluetoothStackID, &NewBroadcastData))
                        {
                           CurrentCSBData                             = NewBroadcastData;
                           CurrentInformation.LastKnownPeriod         = NewBroadcastData.FrameSyncPeriod;
                           CurrentInformation.LastKnownPeriodFraction = NewBroadcastData.FrameSyncPeriodFraction;
                        }
                     }
                  }

                  BTPS_ReleaseMutex(BroadcastInformationMutex);
               }
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      if(TDSMUpdateData)
      {
         if(!TDSM_NotifyUpdate(TDSMUpdateData))
            BTPS_FreeMemory((void *)TDSMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the 3D Sync Manager implementation.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* 3D Sync Manager Implementation.                                   */
int _TDSM_Initialize(TDSM_Initialization_Info_t *TDSInitializationInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Headset Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */

      /* Only initialize if either Audio Gateway or Headset is valid.   */
      if(TDSInitializationInfo)
      {
         if((BroadcastInformationMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Note the initialization info.                               */
            _TDSMInitializationInfo = *TDSInitializationInfo;

            /* Set up our default information.                             */
            BTPS_MemInitialize(&CurrentInformation, 0, sizeof(CurrentInformation));
            CurrentInformation.SyncsPerClockCapture = _TDSMInitializationInfo.DefaultSyncsPerClockCapture;

            BTPS_MemInitialize(&CurrentCSBData, 0, sizeof(CurrentCSBData));

            CurrentFlags = 0;

            /* Flag that this module is initialized.                       */
            Initialized = TRUE;

            ret_val     = 0;
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;
      }
      else
         ret_val = BTPM_ERROR_CODE_3D_SYNC_INVALID_INITIALIZATION_DATA;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* 3D Sync Manager implementation.  After this function is called the*/
   /* 3D Sync Manager implementation will no longer operate until it is */
   /* initialized again via a call to the _TDSM_Initialize() function.  */
void _TDSM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      if(_BluetoothStackID)
      {
         BTPS_CloseMutex(BroadcastInformationMutex);

         if(SDPHandle)
            TDS_Un_Register_SDP_Record(_BluetoothStackID, SDPHandle);

         TDS_Close_Display_Server(_BluetoothStackID);

         SDPHandle = 0;
      }

      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the 3D Sync   */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the 3D Sync Manager with the    */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _TDSM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   int                      Result;
   Byte_t                   LegacyEIRData[5];
   Byte_t                   EIRData[2];
   Byte_t                  *RawEIRData;
   Byte_t                   Status;
   SByte_t                  TxPower;
   unsigned int             DataSize;
   DEVM_Tag_Length_Value_t  TLVData[3];

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", BluetoothStackID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         if((Result = TDS_Open_Display_Server(BluetoothStackID, 0, TDS_Event_Callback, 0)) == 0)
         {
            if((Result = TDS_Register_Display_SDP_Record(BluetoothStackID, _TDSMInitializationInfo.ServiceName, &SDPHandle)) == 0)
            {
               /* Try to read the Tx Power.                             */
               if(((Result = HCI_Read_Inquiry_Response_Transmit_Power_Level(BluetoothStackID, &Status, &TxPower)) == 0) && (Status == HCI_ERROR_CODE_NO_ERROR))
               {
                  /* Format the LTV Data.                               */
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&LegacyEIRData[0], HCI_LMP_COMPID_MANUFACTURER_NAME_BROADCOM);
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LegacyEIRData[2], 0);
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LegacyEIRData[3], 0x01);
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LegacyEIRData[4], _TDSMInitializationInfo.PathLossThreshold);

                  TLVData[0].DataType   = HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_MANUFACTURER_SPECIFIC;
                  TLVData[0].DataLength = 5;
                  TLVData[0].DataBuffer = LegacyEIRData;

                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], _TDSMInitializationInfo.EIRFlags);
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], _TDSMInitializationInfo.PathLossThreshold);

                  TLVData[1].DataType   = HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_3D_INFORMATION_DATA;
                  TLVData[1].DataLength = 2;
                  TLVData[1].DataBuffer = EIRData;

                  TLVData[2].DataType   = HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_TX_POWER_LEVEL;
                  TLVData[2].DataLength = 1;
                  TLVData[2].DataBuffer = (Byte_t *)&TxPower;

                  /* Determine how much memory we need for the EIR Data.*/
                  if((Result = DEVM_ConvertParsedTLVDataToRaw(3, TLVData, 0, NULL)) > 0)
                  {
                     DataSize = (unsigned int)Result;

                     /* Allocate the EIR Data buffer.                   */
                     if((RawEIRData = BTPS_AllocateMemory(DataSize)) != NULL)
                     {
                        /* Convert the LTV data to a buffer.            */
                        if((Result = DEVM_ConvertParsedTLVDataToRaw(3, TLVData, DataSize, RawEIRData)) > 0)
                        {
                           /* Add the EIR Data to used.                 */
                           if((Result = DEVM_AddLocalEIRData(DataSize, RawEIRData)) == 0)
                           {
                              _BluetoothStackID = BluetoothStackID;

                              /* Set up the default CSB data.           */
                              CurrentCSBData.FrameSyncInstant          = 0;
                              CurrentCSBData.ClockPhase                = 0;
                              CurrentCSBData.FrameSyncPeriod           = 0;
                              CurrentCSBData.FrameSyncPeriodFraction   = 0;

                              CurrentCSBData.LeftLensShutterOpenOffset = TDS_BROADCAST_MESSAGE_LEFT_LENS_OPEN_2D_MODE;
                           }
                        }

                        BTPS_FreeMemory(RawEIRData);
                     }
                     else
                        Result = -1;
                  }
               }
               else
               {
                  /* Flag an error if the command was successful but had*/
                  /* a bad status.                                      */
                  if(!Result)
                     Result = -1;
               }
            }

            /* If there was an error, clean up anything we previously   */
            /* registered.                                              */
            if(Result < 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Error occurred during setup: %d\n", Result));

               if(SDPHandle)
                  TDS_Un_Register_SDP_Record(BluetoothStackID, SDPHandle);

               TDS_Close_Display_Server(BluetoothStackID);

               SDPHandle = 0;
            }
         }
         else
            Initialized = FALSE;
      }
      else
      {
         if(SDPHandle)
            TDS_Un_Register_SDP_Record(BluetoothStackID, SDPHandle);

         TDS_Close_Display_Server(BluetoothStackID);

         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will configure the Synchronization Train   */
   /* parameters on the Bluetooth controller.  The SyncTrainParams      */
   /* parameter is a pointer to the Synchronization Train parameters to */
   /* be set.  The IntervalResult parameter is a pointer to the variable*/
   /* that will be populated with the Synchronization Interval chosen   */
   /* by the Bluetooth controller.  This function will return zero on   */
   /* success; otherwise, a negative error value will be returned.      */
   /* * NOTE * The Timout value in the                                  */
   /*          TDS_Synchronization_Train_Parameters_t structure must be */
   /*          a value of at least 120 seconds, or the function call    */
   /*          will fail.                                               */
int _TDSM_Write_Synchronization_Train_Parameters(TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult)
{
   int                                    ret_val;
   TDS_Synchronization_Train_Parameters_t Params;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      if(SyncTrainParams)
      {
         Params.MinInterval = SyncTrainParams->MinInterval;
         Params.MaxInterval = SyncTrainParams->MaxInterval;
         Params.Timeout     = SyncTrainParams->Timeout;
         Params.ServiceData = SyncTrainParams->ServiceData;

         ret_val = TDS_Write_Synchronization_Train_Parameters(_BluetoothStackID, &Params, IntervalResult);

         if(ret_val < 0)
            ret_val = MapTDSErrorCode(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));
   return(ret_val);
}

   /* The following function will enable the Synchronization Train.     */
   /* This function will return zero on success; otherwise, a negative  */
   /* error value will be returned.                                     */
   /* * NOTE * The TDSM_Write_Synchronization_Train_Parameters function */
   /*          should be called at least once after initializing the    */
   /*          stack and before calling this function.                  */
   /* * NOTE * The tetTDSM_Display_Synchronization_Train_Complete event */
   /*          will be triggered when the Synchronization Train         */
   /*          completes.  This function can be called again at this    */
   /*          time to restart the Synchronization Train.               */
int _TDSM_Start_Synchronization_Train(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      ret_val = TDS_Start_Synchronization_Train(_BluetoothStackID);

      if(ret_val < 0)
         ret_val = MapTDSErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));
   return(ret_val);
}

   /* The following function will configure and enable the              */
   /* Connectionless Slave Broadcast channel on the Bluetooth           */
   /* controller.  The ConnectionlessSlaveBroadcastParams parameter is  */
   /* a pointer to the Connectionless Slave Broadcast parameters to be  */
   /* set.  The IntervalResult parameter is a pointer to the variable   */
   /* that will be populated with the Broadcast Interval chosen by the  */
   /* Bluetooth controller.  This function will return zero on success; */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The MinInterval value should be greater than or equal to */
   /*          50 milliseconds, and the MaxInterval value should be less*/
   /*          than or equal to 100 milliseconds; otherwise, the        */
   /*          function will fail.                                      */
int _TDSM_Enable_Connectionless_Slave_Broadcast(TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult)
{
   int                                             ret_val;
   TDS_Connectionless_Slave_Broadcast_Parameters_t Params;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      if(BTPS_WaitMutex(BroadcastInformationMutex, BTPS_INFINITE_WAIT))
      {
         if((ret_val = TDS_Set_Connectionless_Slave_Broadcast_Data(_BluetoothStackID, &CurrentCSBData)) == 0)
         {
            Params.MinInterval        = ConnectionlessSlaveBroadcastParams->MinInterval;
            Params.MaxInterval        = ConnectionlessSlaveBroadcastParams->MaxInterval;
            Params.SupervisionTimeout = ConnectionlessSlaveBroadcastParams->SupervisionTimeout;
            Params.LowPower           = ConnectionlessSlaveBroadcastParams->LowPowerEnabled?TDS_CSB_LOW_POWER_ALLOWED:TDS_CSB_LOW_POWER_NOT_ALLOWED;

            //TDS_CSB_ENABLE_EDR_PACKETS(Params.PacketType);
            Params.PacketType = (HCI_PACKET_ACL_TYPE_DM1 | TDS_CSB_PACKET_TYPE_EDR_NOT_ALLOWED);

            if((ret_val = TDS_Enable_Connectionless_Slave_Broadcast(_BluetoothStackID, &Params, IntervalResult)) == 0)
            {
               /* If we're in 3D, start the clock capture. */
               if(CurrentFlags & TDSMGR_FLAGS_BROADCAST_3D)
                  ret_val = _TDS_Start_Triggered_Clock_Capture(_BluetoothStackID, CurrentInformation.SyncsPerClockCapture);

               if(!ret_val)
               {
                  CurrentFlags |= TDSMGR_FLAGS_CURRENTLY_BROADCASTING;

                  /* If we are broadcasting 3D, we will be waiting for  */
                  /* clock values before switching the mode over. Flag  */
                  /* this.                                              */
                  if(CurrentFlags & TDSMGR_FLAGS_BROADCAST_3D)
                     CurrentFlags |= TDSMGR_FLAGS_WAITING_CLOCK_CAPTURE;
               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Start Capture Error: %d\n", ret_val));
            }
         else
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("enable connectionless error: %d\n", ret_val));
         }
         else
         DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("set connectionless error: %d\n", ret_val));

         BTPS_ReleaseMutex(BroadcastInformationMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

      if(ret_val < 0)
         ret_val = MapTDSErrorCode(ret_val);

   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));
   return(ret_val);
}

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.                           */
   /* * NOTE * Calling this function will terminate the Synchronization */
   /*          Train (if it is currently enabled).                      */
int _TDSM_Disable_Connectionless_Slave_Broadcast(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      if((ret_val = TDS_Disable_Connectionless_Slave_Broadcast(_BluetoothStackID)) == 0)
      {
         if((ret_val = _TDS_Disable_Triggered_Clock_Capture(_BluetoothStackID)) == 0)
         {
            /* Note our clock information is no longer valid.           */
            CurrentCSBData.FrameSyncInstant        = 0;
            CurrentCSBData.ClockPhase              = 0;
            CurrentCSBData.FrameSyncPeriod         = 0;
            CurrentCSBData.FrameSyncPeriodFraction = 0;

            /* Reset the LLS Open Offset back to 2D mode for any future */
            /* broadcasts.                                              */
            CurrentCSBData.LeftLensShutterOpenOffset = TDS_BROADCAST_MESSAGE_LEFT_LENS_OPEN_2D_MODE;

            CurrentFlags &= ~((unsigned long)(TDSMGR_FLAGS_CURRENTLY_BROADCASTING | TDSMGR_FLAGS_WAITING_CLOCK_CAPTURE));
         }
      }

      if(ret_val < 0)
         ret_val = MapTDSErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));
   return(ret_val);
}

   /* The following function is used to get the current information     */
   /* being used int the synchronization broadcasts.  The               */
   /* CurrentBroadcastInformation parameter to a structure in which the */
   /* current information will be placed.  This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
int _TDSM_Get_Current_Broadcast_Information(TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      if(CurrentBroadcastInformation)
      {
         if(BTPS_WaitMutex(BroadcastInformationMutex, BTPS_INFINITE_WAIT))
         {
            *CurrentBroadcastInformation                       = CurrentInformation;
            (*CurrentBroadcastInformation).CurrentBroadcasting = (CurrentFlags & TDSMGR_FLAGS_CURRENTLY_BROADCASTING)?TRUE:FALSE;

            BTPS_ReleaseMutex(BroadcastInformationMutex);

            ret_val = 0;
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));
   return 0;
}

   /* The following function is used to update the information          */
   /* being sent in the synchronization broadcasts.  The                */
   /* BroadcastInformationUpdate parameter is a pointer to a structure  */
   /* which contains the information to update.  This function returns  */
   /* zero if successful and a negative return error code if there was  */
   /* an error.                                                         */
int _TDSM_Update_Broadcast_Information(TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate)
{
   int ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Make sure the update data is semi-valid.                       */
      if((BroadcastInformationUpdate) && (!(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_LLS_OPEN_OFFSET) || (BroadcastInformationUpdate->LeftLensShutterOpenOffset >= 0)) && (!(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_RLS_CLOSE_OFFSET) || (BroadcastInformationUpdate->RightLensShutterCloseOffset <= 0)))
      {
         /* If no fields were specified to update just return success.  */
         if(BroadcastInformationUpdate->UpdateFlags)
         {
            /* If we were successful updating the clock or didn't need  */
            /* to, grab the mutex to update any necessary information.  */
            if(BTPS_WaitMutex(BroadcastInformationMutex, BTPS_INFINITE_WAIT))
            {
               /* Update the clock capture frequency if requested and we*/
               /* are currently broadcasting.                           */
               if((BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_SYNCS_PER_CLOCK_CAPTURE) && (CurrentFlags & TDSMGR_FLAGS_CURRENTLY_BROADCASTING))
               {
                  if((ret_val = _TDS_Disable_Triggered_Clock_Capture(_BluetoothStackID)) == 0)
                  {
                     if((ret_val = _TDS_Start_Triggered_Clock_Capture(_BluetoothStackID, BroadcastInformationUpdate->SyncsPerClockCapture)) == 0)
                        CurrentInformation.SyncsPerClockCapture = BroadcastInformationUpdate->SyncsPerClockCapture;
                  }

                  /* Map any lower level TDS error codes.               */
                  if(ret_val < 0)
                     ret_val = MapTDSErrorCode(ret_val);
               }

               /* If we are broadcasting and the want to switch 3D/2D   */
               /* mode, we need to start/stop clock capture.            */
               if((!ret_val) && (BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_BROADCAST_3D) && (CurrentFlags & TDSMGR_FLAGS_CURRENTLY_BROADCASTING))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_VERBOSE), ("Update 3D: %d,%08lX\n",BroadcastInformationUpdate->Broadcast3D, CurrentFlags));
                  if((BroadcastInformationUpdate->Broadcast3D) && !(CurrentFlags & TDSMGR_FLAGS_BROADCAST_3D))
                  {
                     ret_val = _TDS_Start_Triggered_Clock_Capture(_BluetoothStackID, CurrentInformation.SyncsPerClockCapture);
                  }
                  else
                  {
                     if((!BroadcastInformationUpdate->Broadcast3D) && (CurrentFlags & TDSMGR_FLAGS_BROADCAST_3D))
                     {
                        if((ret_val = _TDS_Disable_Triggered_Clock_Capture(_BluetoothStackID)) == 0)
                        {
                           /* Note 2D Mode.                             */
                           CurrentCSBData.LeftLensShutterOpenOffset  = TDS_BROADCAST_MESSAGE_LEFT_LENS_OPEN_2D_MODE;

                           /* Clear out the old timing information.     */
                           CurrentCSBData.FrameSyncInstant        = 0;
                           CurrentCSBData.ClockPhase              = 0;
                           CurrentCSBData.FrameSyncPeriod         = 0;
                           CurrentCSBData.FrameSyncPeriodFraction = 0;

                           /* Update the currently broadcasting data to */
                           /* 2D mode.                                  */
                           ret_val = TDS_Set_Connectionless_Slave_Broadcast_Data(_BluetoothStackID, &CurrentCSBData);
                        }
                     }
                  }
               }

               if(!ret_val)
               {
                  if(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_BROADCAST_3D)
                  {
                     if(BroadcastInformationUpdate->Broadcast3D)
                        CurrentFlags |= (unsigned long)TDSMGR_FLAGS_BROADCAST_3D;
                     else
                        CurrentFlags &= ~((unsigned long)TDSMGR_FLAGS_BROADCAST_3D);
                  }
                  if(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_VIDEO_MODE)
                     CurrentInformation.VideoMode = BroadcastInformationUpdate->VideoMode;
                  if(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_LLS_OPEN_OFFSET)
                        CurrentInformation.LeftLensShutterOpenOffset = BroadcastInformationUpdate->LeftLensShutterOpenOffset;
                  if(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_LLS_CLOSE_OFFSET)
                     CurrentInformation.LeftLensShutterCloseOffset = BroadcastInformationUpdate->LeftLensShutterCloseOffset;
                  if(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_RLS_OPEN_OFFSET)
                     CurrentInformation.RightLensShutterOpenOffset = BroadcastInformationUpdate->RightLensShutterOpenOffset;
                  if(BroadcastInformationUpdate->UpdateFlags & TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_RLS_CLOSE_OFFSET)
                     CurrentInformation.RightLensShutterCloseOffset = BroadcastInformationUpdate->RightLensShutterCloseOffset;
               }

               BTPS_ReleaseMutex(BroadcastInformationMutex);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_3D_SYNC | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));
   return(ret_val);
}


