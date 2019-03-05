/*****< btpmrscm.c >***********************************************************/
/*      Copyright 2013 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMRSCM - Running Speed and Cadence Manager for Stonestreet One Bluetooth*/
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/18/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"            /* BTPS Protocol Stack Prototypes/Constants.  */
#include "BTPSKRNL.h"           /* BTPS Kernel Prototypes/Constants.          */

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */

#include "BTPMRSCM.h"           /* BTPM RSC Manager Prototypes/Constants.     */
#include "RSCMAPI.h"            /* RSC Manager Prototypes/Constants.          */
#include "RSCMMSG.h"            /* BTPM RSC Manager Message Formats.          */

#include "SS1BTRSC.h"           /* Bluetooth RSC Service Prototypes/Constants.*/

#include "SS1BTPM.h"            /* BTPM Main Prototypes and Constants.        */
#include "BTPMERR.h"            /* BTPM Error Prototypes/Constants.           */
#include "BTPMCFG.h"            /* BTPM Configuration Settings/Constants.     */

   /* The following defines the RSCM LE Configuration File Section Name.*/
#define RSCM_LE_CONFIGURATION_FILE_SECTION_NAME                   "RSCM-Collector"

   /* The following defines the Maximum Key Size that is used in the    */
   /* RSCM LE Configuration File.                                       */
#define RSCM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH             (4+(BD_ADDR_SIZE*2))

   /* The following defines the RSCM LE Configuration File Maximum Line */
   /* Length.                                                           */
#define RSCM_LE_CONFIGURATION_FILE_MAXIMUM_LINE_LENGTH            ((BTPM_CONFIGURATION_SETTINGS_MAXIMUM_FILE_LINE_LENGTH/2)-RSCM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH)

   /* The following define the Key Names that are used with the RSCM LE */
   /* Configuration File.                                               */
#define RSCM_LE_KEY_NAME_DEVICE_INFORMATION                       "DI-%02X%02X%02X%02X%02X%02X"

#define RSCM_PROCEDURE_TIMEOUT                                    30000

typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   RSCM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following structure holds the information for tracking        */
   /* registered callbacks.                                             */
typedef struct _tagCallback_Entry_t
{
   unsigned int                 CallbackID;
   Callback_Info_t              CallbackInfo;
   struct _tagCallback_Entry_t *NextCallbackEntryPtr;
} Callback_Entry_t;

   /* Enum which tracks the current state of configuring a device.      */
typedef enum
{
   csNotConfigured,
   csEnablingMeasurements,
   csEnablingControlPoint,
   csGettingFeatures,
   csGettingSupportedLocations,
   csConfigured,

   /* NOTE: These state must indicate un-configuring.                   */
   csDisablingControlPoint,
   csDisablingMeasurements
} Configuration_State_t;

   /* Enum which tracks what the current Control Point procedure is for */
   /* a remote sensor.                                                  */
typedef enum
{
   cpNone,
   cpUpdateCumulative,
   cpUpdateLocation,
   cpGetSupportedLocations,
   cpStartSensorCalibration
} Current_Procedure_t;

   /* The following structure holds the information for tracking a known*/
   /* remote sensor.                                                    */
typedef struct _tagDevice_Entry_t
{
   BD_ADDR_t                  BluetoothAddress;
   unsigned long              Flags;
   Configuration_State_t      ConfigurationState;
   RSCS_Client_Information_t  ClientInformation;
   unsigned int               CurrentProcedureID;
   Current_Procedure_t        CurrentProcedure;
   unsigned int               ProcedureTimerID;
   RSCM_Sensor_Location_t     PendingLocation;
   DWord_t                    PendingCumulative;
   unsigned long              ExtraCharacteristics;
   unsigned long              SupportedFeatures;
   DWord_t                    SupportedSensorLocations;
   struct _tagDevice_Entry_t *NextDeviceEntryPtr;
} Device_Entry_t;

#define DEVICE_ENTRY_FLAGS_STORE_STATE                         0x00000001
#define DEVICE_ENTRY_FLAGS_SKIP_SUPPORTED_LOCATIONS            0x00000002

   /* Enum which tracks the type of an outstanding GATT transaction.    */
typedef enum
{
   ttEnableMeasurementCCCD,
   ttDisableMeasurementCCCD,
   ttEnableControlCCCD,
   ttDisableControlCCCD,
   ttReadFeatures,
   ttReadLocation,
   ttWriteControlPoint
} Transaction_Type_t;

   /* The following structure holds the information for tracking an     */
   /* outstanding GATT transaction.                                     */
typedef struct _tagTransaction_Entry_t
{
   Transaction_Type_t              TransactionType;
   unsigned int                    TransactionID;
   unsigned int                    GATMTransactionID;
   unsigned int                    ClientID;
   BD_ADDR_t                       BluetoothAddress;
   struct _tagTransaction_Entry_t *NextTransactionEntryPtr;
} Transaction_Entry_t;

   /* Enum which is used to indicated which characteristic a UUID refers*/
   /* to.                                                               */
typedef enum
{
   ruMeasurements,
   ruFeatures,
   ruLocation,
   ruControlPoint,
   ruUnknown
} RSC_UUID_t;

   /* The following structure defines the format of the data about a    */
   /* device that will be written to the settings file.                 */
typedef struct _tagStored_Device_Information_t
{
   Boolean_t     Configured;
   unsigned long SupportedFeatures;
   DWord_t       SupportedLocations;
} Stored_Device_Information_t;

#define RSCM_LE_DEVICE_INFORMATION_VALUE_SIZE                  (sizeof(Stored_Device_Information_t))


   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Procedure ID.    */
static unsigned int NextProcedureID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextTransactionID;

   /* Variable which holds the Callback ID obtained from GATM.          */
static unsigned int GATMCallbackID;

   /* Variable which holds a pointer to the first element of the        */
   /* Callback List (which holds registered event callbacks).           */
static Callback_Entry_t *CallbackEntryList;

   /* Variable which holds a pointer to the first element of the Device */
   /* Information List (which holds connected RSC LE devices).          */
static Device_Entry_t *DeviceEntryList;

   /* Variable which holds a pointer to the first transaction in the    */
   /* outstanding transaction list.                                     */
static Transaction_Entry_t *TransactionEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);
static unsigned int GetNextProcedureID(void);
static unsigned int GetNextTransactionID(void);

static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **ListHead, Callback_Entry_t *EntryToAdd);
static Callback_Entry_t *SearchCallbackEntryByClientID(Callback_Entry_t **ListHead, unsigned int ClientID);
static Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID);
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);
static void FreeCallbackEntryList(Callback_Entry_t **ListHead);

static Device_Entry_t *AddDeviceEntry(Device_Entry_t **ListHead, Device_Entry_t *EntryToAdd);
static Device_Entry_t *SearchDeviceEntry(Device_Entry_t **ListHead, BD_ADDR_t *BluetoothAddress);
static Device_Entry_t *SearchDeviceEntryByProcedureID(Device_Entry_t **ListHead, unsigned int ProcedureID);
static Device_Entry_t *DeleteDeviceEntry(Device_Entry_t **ListHead, BD_ADDR_t *BluetoothAddress);
static void FreeDeviceEntryMemory(Device_Entry_t *EntryToFree);
static void FreeDeviceEntryList(Device_Entry_t **ListHead);

static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd);
static Transaction_Entry_t *SearchTransactionEntry(Transaction_Entry_t **ListHead, BD_ADDR_t *BluetoothAddress, Transaction_Type_t TransactionType, unsigned int ClientID);
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int TransactionID);
static Transaction_Entry_t *DeleteTransactionEntryByGATMTransactionID(Transaction_Entry_t **ListHead, unsigned int GATTTransactionID);
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead);

static void DispatchRSCEvent(Callback_Info_t *CallbackInfo, RSCM_Event_Data_t *EventData, BTPM_Message_t *Message, Boolean_t LockHeld);
static void DispatchRSCBroadcastEvent(RSCM_Event_Data_t *EventData, BTPM_Message_t *Message, Boolean_t LocalOnly);

static void DispatchConnectedEvent(Device_Entry_t *Device);
static void DispatchDisconnectedEvent(Device_Entry_t *Device);
static void DispatchConfigurationStatusChangedEvent(Device_Entry_t *Device, unsigned int Status);
static void DispatchMeasurementEvent(Device_Entry_t *Device, RSCM_Measurement_Data_t *MeasurementData);
static void DispatchSensorLocationResponseEvent(unsigned int ClientID, Device_Entry_t *Device, unsigned int Status, RSCM_Sensor_Location_t Location);
static void DispatchCumulativeValueUpdatedEvent(Device_Entry_t *Device);
static void DispatchSensorLocationUpdatedEvent(Device_Entry_t *Device);
static void DispatchProcedureCompleteEvent(Device_Entry_t *Device, unsigned int Status, unsigned int ResponseCode);

static Boolean_t ReloadRSCDeviceFromFile(Device_Entry_t *Device, BD_ADDR_t BluetoothAddress);
static void StoreRSCMDeviceToFile(Device_Entry_t *Device);
static void ClearRSCMDeviceFromFile(BD_ADDR_t BluetoothAddress);

static void DeleteTransactionsForDevice(BD_ADDR_t RemoteDeviceAddress);

static unsigned long ConvertLocationListToMask(RSCS_SCCP_Supported_Sensor_Locations_t *SupportedSensorLocations);

static RSC_UUID_t DetermineRSCUUID(GATT_UUID_t UUID);
static Word_t FindCCCD(GATT_Characteristic_Information_t *Characteristic);
static Device_Entry_t *DiscoverRSCSensor(BD_ADDR_t *BluetoothAddress);

static int ProcessRegisterCollectorEvents(unsigned int ClientID, RSCM_Event_Callback_t CallbackFunction, void *CallbackParameter);
static void ProcessUnRegisterCollectorEvents(unsigned int ClientID, unsigned int CollectorCallbackID);
static int ProcessQueryConnectedSensors(unsigned int ClientID, unsigned int MaximumRemoteDeviceListEntries, RSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices);
static int ProcessConfigureRemoteSensor(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned long Flags);
static int ProcessUnConfigureRemoteSensor(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress);
static int ProcessGetConnectedSensorInfo(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, RSCM_Connected_Sensor_t *DeviceInfo);
static int ProcessGetSensorLocation(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress);
static int ProcessUpdateCumulativeValue(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue);
static int ProcessUpdateSensorLocation(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, RSCM_Sensor_Location_t Location);
static int ProcessStartSensorCalibration(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress);

static void ProcessRegisterCollectorEventsMessage(RSCM_Register_Collector_Events_Request_t *Message);
static void ProcessUnRegisterCollectorEventsMessage(RSCM_Un_Register_Collector_Events_Request_t *Message);
static void ProcessQueryConnectedSensorsMessage(RSCM_Query_Connected_Sensors_Request_t *Message);
static void ProcessConfigureRemoteSensorMessage(RSCM_Configure_Remote_Sensor_Request_t *Message);
static void ProcessUnConfigureRemoteSensorMessage(RSCM_Un_Configure_Remote_Sensor_Request_t *Message);
static void ProcessGetConnectedSensorInfoMessage(RSCM_Get_Connected_Sensor_Info_Request_t *Message);
static void ProcessGetSensorLocationMessage(RSCM_Get_Sensor_Location_Request_t *Message);
static void ProcessUpdateCumulativeValueMessage(RSCM_Update_Cumulative_Value_Request_t *Message);
static void ProcessUpdateSensorLocationMessage(RSCM_Update_Sensor_Location_Request_t *Message);
static void ProcessStartSensorCalibrationMessage(RSCM_Start_Sensor_Calibration_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessRSCMConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRSCMDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRSCMDevicePaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRSCMDeviceUnPaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRemoteDeviceDeletedEvent(BD_ADDR_t RemoteDeviceAddress);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessConfigurationResponse(GATM_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry, Boolean_t Success);
static void ProcessReadLocationResponse(GATM_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry, Boolean_t Success);
static void ProcessControlPointErrorResponse(GATM_Error_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry);
static void ProcessGATTNotification(GATM_Handle_Value_Data_Event_Data_t *EventData);
static void ProcessGATTIndication(GATM_Handle_Value_Data_Event_Data_t *EventData);

static Boolean_t BTPSAPI ProcedureTimerCallback(unsigned int TimerID, void *CallbackParameter);
static void BTPSAPI GATMEventCallback(GATM_Event_Data_t *EventData, void *CallbackParameter);

static void BTPSAPI BTPMDispatchCallback_RSCM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_TMR(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI RSCManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the Callback List.                                           */
static unsigned int GetNextCallbackID(void)
{
   ++NextCallbackID;

   if(NextCallbackID & 0x80000000)
      NextCallbackID = 1;

   return(NextCallbackID);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Procedure ID that can be used to add an entry    */
   /* into the Procedure List.                                           */
static unsigned int GetNextProcedureID(void)
{
   ++NextProcedureID;

   if(NextProcedureID & 0x80000000)
      NextProcedureID = 1;

   return(NextProcedureID);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Transaction ID that can be used to add an entry    */
   /* into the Transaction List.                                           */
static unsigned int GetNextTransactionID(void)
{
   ++NextTransactionID;

   if(NextTransactionID & 0x80000000)
      NextTransactionID = 1;

   return(NextTransactionID);
}

   /* The following function adds the specified Callback Entry to the   */
   /* module's list.  This function will allocate and add a copy of the */
   /* entry to the list.  This function will return NULL if NO Entry was*/
   /* added.  This can occur if the element passed in was deemed        */
   /* invalid.                                                          */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the Callback ID field is the same as an entry already  */
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **ListHead, Callback_Entry_t *EntryToAdd)
{
   return((Callback_Entry_t *)BSC_AddGenericListEntry(sizeof(Callback_Entry_t), ekUnsignedInteger, STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), sizeof(Callback_Entry_t), STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntryPtr), (void **)ListHead, ((void *)EntryToAdd)));
}

   /* The following function searches the module's Callback Entry List  */
   /* for a Callback Entry based on the specified Client ID.  This      */
   /* function returns NULL if either the Client ID is invalid, or the  */
   /* specified Entry was NOT present in the list.                      */
static Callback_Entry_t *SearchCallbackEntryByClientID(Callback_Entry_t **ListHead, unsigned int ClientID)
{
   return((Callback_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&ClientID, STRUCTURE_OFFSET(Callback_Entry_t, CallbackInfo) + STRUCTURE_OFFSET(Callback_Info_t, ClientID), STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntryPtr), (void **)ListHead));
}

   /* The following function searches the module's Callback Entry List  */
   /* for the Callback Entry with the specified Callback ID and removes */
   /* it from the List.  This function returns NULL if either the       */
   /* Callback ID is invalid, or the specified Entry was NOT present in */
   /* the list.  The entry returned will have the Next Entry field set  */
   /* to NULL, and the caller is responsible for deleting the memory    */
   /* associated with this entry by calling FreeCallbackEntryMemory().  */
static Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID)
{
   return((Callback_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntryPtr), (void **)ListHead));
}

   /* This function frees the specified Callback Entry member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Callback Entry List.  Upon return of this */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeCallbackEntryList(Callback_Entry_t **ListHead)
{
   Callback_Entry_t *CallbackEntry;
   Callback_Entry_t *tmpCallbackEntry;

   if(ListHead)
   {
      CallbackEntry = *ListHead;

      while(CallbackEntry)
      {
         tmpCallbackEntry = CallbackEntry;
         CallbackEntry    = CallbackEntry->NextCallbackEntryPtr;

         FreeCallbackEntryMemory(tmpCallbackEntry);
      }

      *ListHead = NULL;
   }
}

   /* The following function adds the specified Device Entry to the     */
   /* module's list.  This function allocate and add a copy of the      */
   /* entry to the list.  This function will return NULL if NO Entry    */
   /* was added.  This can occur if the element passed in was deemed    */
   /* invalid.                                                          */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Device_Entry_t *AddDeviceEntry(Device_Entry_t **ListHead, Device_Entry_t *EntryToAdd)
{
   return((Device_Entry_t *)BSC_AddGenericListEntry(sizeof(Device_Entry_t), ekBD_ADDR_t, STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), sizeof(Device_Entry_t), STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr), (void **)ListHead, ((void *)EntryToAdd)));
}

   /* The following function searches the module's Device Entry List for*/
   /* a Device Entry based on the specified Bluetooth Address.  This    */
   /* function returns NULL if either the Bluetooth Device Address is   */
   /* invalid, or the specified Entry was NOT present in the list.      */
static Device_Entry_t *SearchDeviceEntry(Device_Entry_t **ListHead, BD_ADDR_t *BluetoothAddress)
{
   return((Device_Entry_t *)BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)BluetoothAddress, STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr), (void *)ListHead));
}

   /* The following function searches the module's Device Entry List    */
   /* for a Device Entry based on the specified ProcedureID.  This      */
   /* function returns NULL if either the ProcedureID is invalid, or the*/
   /* specified Entry was NOT present in the list.                      */
static Device_Entry_t *SearchDeviceEntryByProcedureID(Device_Entry_t **ListHead, unsigned int ProcedureID)
{
   Device_Entry_t *D;
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));
   D = (Device_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&ProcedureID, STRUCTURE_OFFSET(Device_Entry_t, CurrentProcedureID), STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr), (void *)ListHead);
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
   return D;
}

   /* The following function searches the module's Device Entry List for*/
   /* the Device Entry with the specified Bluetooth Address and removes */
   /* it from the List.  This function returns NULL if either the       */
   /* Bluetooth Device Address is invalid, or the specified Entry was   */
   /* NOT present in the list.  The entry returned will have the Next   */
   /* Entry field set to NULL, and the caller is responsible for        */
   /* deleting the memory associated with this entry by calling         */
   /* FreeDeviceEntryMemory().                                          */
static Device_Entry_t *DeleteDeviceEntry(Device_Entry_t **ListHead, BD_ADDR_t *BluetoothAddress)
{
   return((Device_Entry_t *)BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)BluetoothAddress, STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr), (void *)ListHead));
}

   /* This function frees the specified Device Entry member.  No check  */
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeDeviceEntryMemory(Device_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Device Entry Entry List.  Upon return of  */
   /* this function, the Head Pointer is set to NULL.                   */
static void FreeDeviceEntryList(Device_Entry_t **ListHead)
{
   Device_Entry_t *DeviceEntry;
   Device_Entry_t *tmpDeviceEntry;

   if(ListHead)
   {
      DeviceEntry = *ListHead;

      while(DeviceEntry)
      {
         tmpDeviceEntry = DeviceEntry;
         DeviceEntry    = DeviceEntry->NextDeviceEntryPtr;

         FreeDeviceEntryMemory(tmpDeviceEntry);
      }

      *ListHead = NULL;
   }
}

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
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd)
{
   return((Transaction_Entry_t *)BSC_AddGenericListEntry(sizeof(Transaction_Entry_t), ekUnsignedInteger, STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), sizeof(Transaction_Entry_t), STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntryPtr), (void **)ListHead, ((void *)EntryToAdd)));
}

   /* The following function searches the module's Transaction Entry    */
   /* List for a Transaction Entry based on the specified Bluetooth     */
   /* Address, Transaction Type, and Callback ID pair.  This function   */
   /* returns NULL if either the Bluetooth Device Address is invalid, or*/
   /* the specified Entry was NOT present in the list.                  */
static Transaction_Entry_t *SearchTransactionEntry(Transaction_Entry_t **ListHead, BD_ADDR_t *BluetoothAddress, Transaction_Type_t TransactionType, unsigned int ClientID)
{
   Transaction_Entry_t *ret_val = NULL;

   if(ListHead)
   {
      if(BluetoothAddress)
      {
         for(ret_val = *ListHead; ret_val; ret_val = ret_val->NextTransactionEntryPtr)
         {
            if(COMPARE_BD_ADDR(ret_val->BluetoothAddress, *BluetoothAddress) && (ret_val->TransactionType == TransactionType) && (ret_val->ClientID == ClientID))
               break;
         }
      }
   }

   return(ret_val);
}

   /* The following function searches the module's Transaction Entry    */
   /* List for the Transaction Entry with the specified Transaction ID  */
   /* and removes it from the List.  This function returns NULL if the  */
   /* specified Entry was NOT present in the list.  The entry returned  */
   /* will have the Next Entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeTransactionEntryMemory().                             */
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int TransactionID)
{
   return((Transaction_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&TransactionID, STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntryPtr), (void **)ListHead));
}

   /* The following function searches the module's Transaction Entry    */
   /* List for the Transaction Entry with the specified GATT Transaction*/
   /* ID and removes it from the List.  This function returns NULL if   */
   /* the specified Entry was NOT present in the list.  The entry       */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionEntryMemory().                    */
static Transaction_Entry_t *DeleteTransactionEntryByGATMTransactionID(Transaction_Entry_t **ListHead, unsigned int GATMTransactionID)
{
   return((Transaction_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&GATMTransactionID, STRUCTURE_OFFSET(Transaction_Entry_t, GATMTransactionID), STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntryPtr), (void **)ListHead));
}

   /* This function frees the specified Transaction Entry member.  No   */
   /* check is done on this entry other than making sure it NOT NULL.   */
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Transaction Entry List.  Upon return of   */
   /* this function, the Transaction List Pointer is set to NULL.       */
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead)
{
   Transaction_Entry_t *TransactionEntry;
   Transaction_Entry_t *tmpTransactionEntry;

   if(ListHead)
   {
      TransactionEntry = *ListHead;

      while(TransactionEntry)
      {
         tmpTransactionEntry = TransactionEntry;
         TransactionEntry    = TransactionEntry->NextTransactionEntryPtr;

         FreeTransactionEntryMemory(tmpTransactionEntry);
      }

      *ListHead = NULL;
   }
}

   /* The following function is a utility function that will dispatch a */
   /* RSC event to the specified callback.                              */
   /* * NOTE * If the LockHeld parameter is TRUE, this function will    */
   /*          release the lock to dispatch the event and re-acquire it */
   /*          upon completion. If FALSE, it will assume the caller has */
   /*          already released the lock.                               */
static void DispatchRSCEvent(Callback_Info_t *CallbackInfo, RSCM_Event_Data_t *EventData, BTPM_Message_t *Message, Boolean_t LockHeld)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));
   if((EventData) && (Message) && (CallbackInfo))
   {
      if(CallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         if(LockHeld)
            DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(CallbackInfo->EventCallback)
               (*CallbackInfo->EventCallback)(EventData, CallbackInfo->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         /* Re-acquire the Lock.                                        */
         if(LockHeld)
            DEVM_AcquireLock();
      }
      else
      {
         Message->MessageHeader.AddressID = CallbackInfo->ClientID;
         MSG_SendMessage(Message);
      }
   }
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified RSC Data event to the correct registered   */
   /* RSC Data Event Callback.                                          */
   /* * NOTE * This function should be called with the RSC Manager Lock */
   /*          held.  Upon exit from this function it will free the RSC */
   /*          Manager Lock.                                            */
static void DispatchRSCBroadcastEvent(RSCM_Event_Data_t *EventData, BTPM_Message_t *Message, Boolean_t LocalOnly)
{
   unsigned int      Index;
   unsigned int      Index1;
   unsigned int      ServerID;
   unsigned int      NumberCallbacks;
   Callback_Info_t   CallbackInfoArray[16];
   Callback_Info_t  *CallbackInfoArrayPtr;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CallbackEntryList) && (EventData) && (Message))
   {
      CallbackEntry   = CallbackEntryList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      while(CallbackEntry)
      {
         if(((!LocalOnly) && (CallbackEntry->CallbackInfo.ClientID != ServerID)) || (CallbackEntry->CallbackInfo.EventCallback))
            NumberCallbacks++;

         CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
      }

      if(NumberCallbacks)
      {
         if(NumberCallbacks <= (sizeof(CallbackInfoArray)/sizeof(Callback_Info_t)))
            CallbackInfoArrayPtr = CallbackInfoArray;
         else
            CallbackInfoArrayPtr = BTPS_AllocateMemory((NumberCallbacks*sizeof(Callback_Info_t)));

         /* Make sure that we have memory to copy the Callback List     */
         /* into.                                                       */
         if(CallbackInfoArrayPtr)
         {
            CallbackEntry   = CallbackEntryList;
            NumberCallbacks = 0;

            while(CallbackEntry)
            {
               if(((!LocalOnly) && (CallbackEntry->CallbackInfo.ClientID != ServerID)) || (CallbackEntry->CallbackInfo.EventCallback))
                  CallbackInfoArrayPtr[NumberCallbacks++] = CallbackEntry->CallbackInfo;

               CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
            }

            /* Release the Lock because we have already built the       */
            /* Callback Array.                                          */
            DEVM_ReleaseLock();

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               if(CallbackInfoArrayPtr[Index].ClientID != ServerID)
               {
                  /* Make sure we have not already dispatched to this   */
                  /* client.                                            */
                  for(Index1=0;Index1<Index;Index1++)
                  {
                     if(CallbackInfoArrayPtr[Index].ClientID == CallbackInfoArrayPtr[Index1].ClientID)
                        break;
                  }

                  if(Index == Index1)
                     DispatchRSCEvent(&CallbackInfoArray[Index], EventData, Message, FALSE);
               }
               else
                  DispatchRSCEvent(&CallbackInfoArray[Index], EventData, Message, FALSE);

               Index++;
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will dispatch a connected event for the    */
   /* specified device.                                                 */
static void DispatchConnectedEvent(Device_Entry_t *Device)
{
   RSCM_Event_Data_t        EventData;
   RSCM_Connected_Message_t Message;

   if(Device)
   {
      /* Format the event.                                              */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                                    = retRSCConnected;
      EventData.EventLength                                                  = RSCM_CONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.ConnectedEventData.RemoteDeviceAddress             = Device->BluetoothAddress;
      EventData.EventData.ConnectedEventData.SupportedOptionalCharateristics = Device->ExtraCharacteristics;
      EventData.EventData.ConnectedEventData.Configured                      = (Device->ConfigurationState == csConfigured);

      /* Now format the IPC Message.                                    */
      BTPS_MemInitialize(&Message, 0, RSCM_CONNECTED_MESSAGE_SIZE);

      Message.MessageHeader.AddressID         = 0;
      Message.MessageHeader.MessageID         = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup      = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction   = RSCM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength     = RSCM_CONNECTED_MESSAGE_SIZE;

      Message.RemoteDeviceAddress             = Device->BluetoothAddress;
      Message.SupportedOptionalCharateristics = Device->ExtraCharacteristics;
      Message.Configured                      = (Device->ConfigurationState == csConfigured);

      DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, FALSE);
   }
}

   /* The following function will dispatch a disconnected event for the */
   /* specified device.                                                 */
static void DispatchDisconnectedEvent(Device_Entry_t *Device)
{
   RSCM_Event_Data_t        EventData;
   RSCM_Disconnected_Message_t Message;

   if(Device)
   {
      /* Format the event.                                              */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                           = retRSCDisconnected;
      EventData.EventLength                                         = RSCM_DISCONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Device->BluetoothAddress;

      /* Now format the IPC Message.                                    */
      BTPS_MemInitialize(&Message, 0, RSCM_DISCONNECTED_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction = RSCM_MESSAGE_FUNCTION_DISCONNECTED;
      Message.MessageHeader.MessageLength   = RSCM_DISCONNECTED_MESSAGE_SIZE;

      Message.RemoteDeviceAddress           = Device->BluetoothAddress;

      DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, FALSE);
   }
}

   /* The following function will dispatch a configuration status       */
   /* changed event for the specified device.                           */
static void DispatchConfigurationStatusChangedEvent(Device_Entry_t *Device, unsigned int Status)
{
   RSCM_Event_Data_t                           EventData;
   RSCM_Configuration_Status_Changed_Message_t Message;

   if(Device)
   {
      /* Format the event.                                              */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                           = retRSCConfigurationStatusChanged;
      EventData.EventLength                                         = RSCM_CONFIGURATION_STATUS_CHANGED_EVENT_DATA_SIZE;

      EventData.EventData.ConfigurationStatusChangedEventData.RemoteDeviceAddress = Device->BluetoothAddress;
      EventData.EventData.ConfigurationStatusChangedEventData.Configured          = (Device->ConfigurationState == csConfigured);
      EventData.EventData.ConfigurationStatusChangedEventData.Status              = Status;

      /* Now format the IPC Message.                                    */
      BTPS_MemInitialize(&Message, 0, RSCM_CONFIGURATION_STATUS_CHANGED_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction = RSCM_MESSAGE_FUNCTION_CONFIGURATION_STATUS_CHANGED;
      Message.MessageHeader.MessageLength   = RSCM_CONFIGURATION_STATUS_CHANGED_MESSAGE_SIZE;

      Message.RemoteDeviceAddress           = Device->BluetoothAddress;
      Message.Configured                    = (Device->ConfigurationState == csConfigured);
      Message.Status                        = Status;

      DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, FALSE);
   }
}

   /* The following function will dispatch a measurement event for the  */
   /* specified device.                                                 */
static void DispatchMeasurementEvent(Device_Entry_t *Device, RSCM_Measurement_Data_t *MeasurementData)
{
   RSCM_Event_Data_t           EventData;
   RSCM_Measurement_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Device)
   {
      /* Format the event.                                              */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                          = retRSCMeasurement;
      EventData.EventLength                                        = RSCM_MEASUREMENT_EVENT_DATA_SIZE;

      EventData.EventData.MeasurementEventData.RemoteDeviceAddress = Device->BluetoothAddress;
      EventData.EventData.MeasurementEventData.MeasurementData     = *MeasurementData;

      /* Now format the IPC Message.                                    */
      BTPS_MemInitialize(&Message, 0, RSCM_MEASUREMENT_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction = RSCM_MESSAGE_FUNCTION_MEASUREMENT;
      Message.MessageHeader.MessageLength   = RSCM_MEASUREMENT_MESSAGE_SIZE;

      Message.RemoteDeviceAddress           = Device->BluetoothAddress;
      Message.MeasurementData               = *MeasurementData;

      DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, FALSE);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will dispatch a sensor location response   */
   /* event for the specified device.                                   */
static void DispatchSensorLocationResponseEvent(unsigned int ClientID, Device_Entry_t *Device, unsigned int Status, RSCM_Sensor_Location_t Location)
{
   Callback_Entry_t                        *CallbackEntry;
   RSCM_Event_Data_t                        EventData;
   RSCM_Sensor_Location_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Device)
   {
      /* Format the event.                                           */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                                     = retRSCSensorLocationResponse;
      EventData.EventLength                                                   = RSCM_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE;

      EventData.EventData.SensorLocationResponseEventData.RemoteDeviceAddress = Device->BluetoothAddress;
      EventData.EventData.SensorLocationResponseEventData.Status              = Status;
      EventData.EventData.SensorLocationResponseEventData.SensorLocation      = Location;

      /* Now format the IPC Message.                                 */
      BTPS_MemInitialize(&Message, 0, RSCM_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction = RSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_RESPONSE;
      Message.MessageHeader.MessageLength   = RSCM_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE;

      Message.RemoteDeviceAddress           = Device->BluetoothAddress;
      Message.Status                        = Status;
      Message.SensorLocation                = Location;

      /* Determine whether we want to send this to a single client or   */
      /* broadcast it to all local callbacks.                           */
      if(ClientID == MSG_GetServerAddressID())
         DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, TRUE);
      else
      {
         /* Make sure this client has registered.                       */
         if((CallbackEntry = SearchCallbackEntryByClientID(&CallbackEntryList, ClientID)) != NULL)
            DispatchRSCEvent(&CallbackEntry->CallbackInfo, &EventData, (BTPM_Message_t *)&Message, TRUE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will dispatch a cumulative value updated   */
   /* event for the specified device.                                   */
static void DispatchCumulativeValueUpdatedEvent(Device_Entry_t *Device)
{
   RSCM_Event_Data_t                       EventData;
   RSCM_Cumulative_Value_Updated_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Device)
   {
      /* Format the event.                                              */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                                     = retRSCCumulativeValueUpdated;
      EventData.EventLength                                                   = RSCM_CUMULATIVE_VALUE_UPDATED_EVENT_DATA_SIZE;

      EventData.EventData.CumulativeValueUpdatedEventData.RemoteDeviceAddress = Device->BluetoothAddress;
      EventData.EventData.CumulativeValueUpdatedEventData.CumulativeValue     = Device->PendingCumulative;

      /* Now format the IPC Message.                                    */
      BTPS_MemInitialize(&Message, 0, RSCM_CUMULATIVE_VALUE_UPDATED_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction = RSCM_MESSAGE_FUNCTION_CUMULATIVE_VALUE_UPDATED;
      Message.MessageHeader.MessageLength   = RSCM_CUMULATIVE_VALUE_UPDATED_MESSAGE_SIZE;

      Message.RemoteDeviceAddress           = Device->BluetoothAddress;
      Message.CumulativeValue               = Device->PendingCumulative;

      /* Clear the pending information.                                 */
      Device->PendingCumulative = 0;

      DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, FALSE);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will dispatch a sensor location updated    */
   /* event for the specified device.                                   */
static void DispatchSensorLocationUpdatedEvent(Device_Entry_t *Device)
{
   RSCM_Event_Data_t                      EventData;
   RSCM_Sensor_Location_Updated_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Device)
   {
      /* Format the event.                                              */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                                    = retRSCSensorLocationUpdated;
      EventData.EventLength                                                  = RSCM_SENSOR_LOCATION_UPDATED_EVENT_DATA_SIZE;

      EventData.EventData.SensorLocationUpdatedEventData.RemoteDeviceAddress = Device->BluetoothAddress;
      EventData.EventData.SensorLocationUpdatedEventData.SensorLocation      = Device->PendingLocation;

      /* Now format the IPC Message.                                    */
      BTPS_MemInitialize(&Message, 0, RSCM_SENSOR_LOCATION_UPDATED_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction = RSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_UPDATED;
      Message.MessageHeader.MessageLength   = RSCM_SENSOR_LOCATION_UPDATED_MESSAGE_SIZE;

      Message.RemoteDeviceAddress           = Device->BluetoothAddress;
      Message.SensorLocation                = Device->PendingLocation;

      DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, FALSE);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will dispatch a procedure completed event  */
   /* for the specified device.                                         */
static void DispatchProcedureCompleteEvent(Device_Entry_t *Device, unsigned int Status, unsigned int ResponseCode)
{
   RSCM_Event_Data_t           EventData;
   RSCM_Procedure_Complete_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Device)
   {
      /* Format the event.                                              */
      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                                = retRSCProcedureComplete;
      EventData.EventLength                                              = RSCM_PROCEDURE_COMPLETE_EVENT_DATA_SIZE;

      EventData.EventData.ProcedureCompleteEventData.RemoteDeviceAddress = Device->BluetoothAddress;
      EventData.EventData.ProcedureCompleteEventData.ProcedureID         = Device->CurrentProcedureID;
      EventData.EventData.ProcedureCompleteEventData.Status              = Status;
      EventData.EventData.ProcedureCompleteEventData.ResponseErrorCode   = ResponseCode;

      /* Now format the IPC Message.                                    */
      BTPS_MemInitialize(&Message, 0, RSCM_PROCEDURE_COMPLETE_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER;
      Message.MessageHeader.MessageFunction = RSCM_MESSAGE_FUNCTION_PROCEDURE_COMPLETE;
      Message.MessageHeader.MessageLength   = RSCM_PROCEDURE_COMPLETE_MESSAGE_SIZE;

      Message.RemoteDeviceAddress           = Device->BluetoothAddress;
      Message.ProcedureID                   = Device->CurrentProcedureID;
      Message.Status                        = Status;
      Message.ResponseErrorCode             = ResponseCode;

      /* Clear the device entry since the procedure is complete.        */
      Device->CurrentProcedureID = 0;
      Device->CurrentProcedure   = cpNone;

      if(Device->ProcedureTimerID)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Stopping Timer: %u\n", Device->ProcedureTimerID));
         TMR_StopTimer(Device->ProcedureTimerID);
         Device->ProcedureTimerID = 0;
      }

      /* Now broadcast the event.                                       */
      DispatchRSCBroadcastEvent(&EventData, (BTPM_Message_t *)&Message, FALSE);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will attempt to load information for       */
   /* the specified device from the LE settings file. If the Device     */
   /* parameter is specified, this function will populate the stored    */
   /* information in the device entry. This function returns TRUE if and*/
   /* entry for the device address was present or FALSE otherwise.      */
static Boolean_t ReloadRSCDeviceFromFile(Device_Entry_t *Device, BD_ADDR_t BluetoothAddress)
{
   char                        KeyName[RSCM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Boolean_t                   ret_val                                                = FALSE;
   Stored_Device_Information_t StoredDeviceInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(!COMPARE_NULL_BD_ADDR(BluetoothAddress))
   {
      sprintf(KeyName, RSCM_LE_KEY_NAME_DEVICE_INFORMATION, BluetoothAddress.BD_ADDR5, BluetoothAddress.BD_ADDR4, BluetoothAddress.BD_ADDR3, BluetoothAddress.BD_ADDR2, BluetoothAddress.BD_ADDR1, BluetoothAddress.BD_ADDR0);

      if(SET_ReadBinaryData(RSCM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (unsigned char *)&StoredDeviceInformation, RSCM_LE_DEVICE_INFORMATION_VALUE_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == RSCM_LE_DEVICE_INFORMATION_VALUE_SIZE)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Loaded: %s, %08lX, %08lX\n", StoredDeviceInformation.Configured ? "TRUE" : "FALSE", StoredDeviceInformation.SupportedFeatures, (unsigned long)StoredDeviceInformation.SupportedLocations));

         ret_val = TRUE;

         if(Device)
         {
            Device->Flags |= DEVICE_ENTRY_FLAGS_STORE_STATE;

            if(StoredDeviceInformation.Configured)
            {
               Device->ConfigurationState       = csConfigured;
               Device->SupportedFeatures        = StoredDeviceInformation.SupportedFeatures;
               Device->SupportedSensorLocations = StoredDeviceInformation.SupportedLocations;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static void StoreRSCMInformation(BD_ADDR_t BD_ADDR, Stored_Device_Information_t *StoredDeviceInformation)
{
   char KeyName[RSCM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];

   if(StoredDeviceInformation)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Storing Info: %s, %08lX, %08lX\n", StoredDeviceInformation->Configured ? "TRUE" : "FALSE", StoredDeviceInformation->SupportedFeatures, (unsigned long)StoredDeviceInformation->SupportedLocations));
      sprintf(KeyName, RSCM_LE_KEY_NAME_DEVICE_INFORMATION, BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

      /* Write out the information structure.                           */
      SET_WriteBinaryData(RSCM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (unsigned char *)StoredDeviceInformation, RSCM_LE_DEVICE_INFORMATION_VALUE_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
   }
}

   /* The following function whill store information about the specified*/
   /* device to persistant storage.                                     */
static void StoreRSCMDeviceToFile(Device_Entry_t *Device)
{
   Stored_Device_Information_t StoredDeviceInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((Device) && (Device->Flags & DEVICE_ENTRY_FLAGS_STORE_STATE) && (!COMPARE_NULL_BD_ADDR(Device->BluetoothAddress)))
   {
      StoredDeviceInformation.Configured         = (Device->ConfigurationState == csConfigured);
      StoredDeviceInformation.SupportedFeatures  = Device->SupportedFeatures;
      StoredDeviceInformation.SupportedLocations = Device->SupportedSensorLocations;

      StoreRSCMInformation(Device->BluetoothAddress, &StoredDeviceInformation);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function will clear any stored information for the  */
   /* given device.                                                     */
static void ClearRSCMDeviceFromFile(BD_ADDR_t BluetoothAddress)
{
   char KeyName[RSCM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(!COMPARE_NULL_BD_ADDR(BluetoothAddress))
   {
      sprintf(KeyName, RSCM_LE_KEY_NAME_DEVICE_INFORMATION, BluetoothAddress.BD_ADDR5, BluetoothAddress.BD_ADDR4, BluetoothAddress.BD_ADDR3, BluetoothAddress.BD_ADDR2, BluetoothAddress.BD_ADDR1, BluetoothAddress.BD_ADDR0);

      /* Simply write a NULL buffer to clear the key entry.             */
      SET_WriteBinaryData(RSCM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* delete the transaction entries for a specified device for all     */
   /* outstanding GATT transactions.                                    */
static void DeleteTransactionsForDevice(BD_ADDR_t RemoteDeviceAddress)
{
   Transaction_Entry_t *TransactionEntry;
   Transaction_Entry_t *tmpTransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      TransactionEntry = TransactionEntryList;

      while(TransactionEntry)
      {
         if(COMPARE_BD_ADDR(TransactionEntry->BluetoothAddress, RemoteDeviceAddress))
         {
            if((TransactionEntry = DeleteTransactionEntry(&TransactionEntryList, TransactionEntry->TransactionID)) != NULL)
            {
               tmpTransactionEntry = TransactionEntry->NextTransactionEntryPtr;
               FreeTransactionEntryMemory(TransactionEntry);
               TransactionEntry    = tmpTransactionEntry;
            }
         }
         else
            TransactionEntry = TransactionEntry->NextTransactionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility to convert a list of supported*/
   /* sensor locations values into a bitmask.                           */
static unsigned long ConvertLocationListToMask(RSCS_SCCP_Supported_Sensor_Locations_t *SupportedSensorLocations)
{
   int           Index;
   unsigned long Mask=0;

   for(Index=0;Index<SupportedSensorLocations->NumberOfSensorLocations;Index++)
   {
      switch(SupportedSensorLocations->SensorLocations[Index])
      {
         case RSCS_SENSOR_LOCATION_TOP_OF_SHOE:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_TOP_OF_SHOE;
            break;
         case RSCS_SENSOR_LOCATION_IN_SHOE:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_IN_SHOE;
            break;
         case RSCS_SENSOR_LOCATION_HIP:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_HIP;
            break;
         case RSCS_SENSOR_LOCATION_FRONT_WHEEL:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_FRONT_WHEEL;
            break;
         case RSCS_SENSOR_LOCATION_LEFT_CRANK:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_LEFT_CRANK;
            break;
         case RSCS_SENSOR_LOCATION_RIGHT_CRANK:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_RIGHT_CRANK;
            break;
         case RSCS_SENSOR_LOCATION_LEFT_PEDAL:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_LEFT_PEDAL;
            break;
         case RSCS_SENSOR_LOCATION_RIGHT_PEDAL:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_RIGHT_PEDAL;
            break;
         case RSCS_SENSOR_LOCATION_FRONT_HUB:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_FRONT_HUB;
            break;
         case RSCS_SENSOR_LOCATION_REAR_DROPOUT:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_REAR_DROPOUT;
            break;
         case RSCS_SENSOR_LOCATION_CHAINSTAY:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_CHAINSTAY;
            break;
         case RSCS_SENSOR_LOCATION_REAR_WHEEL:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_REAR_WHEEL;
            break;
         case RSCS_SENSOR_LOCATION_REAR_HUB:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_REAR_HUB;
            break;
         case RSCS_SENSOR_LOCATION_CHEST:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_CHEST;
            break;
         default:
         case RSCS_SENSOR_LOCATION_OTHER:
            Mask |= RSCM_SUPPORTED_SENSOR_LOCATION_OTHER;
            break;
      }
   }

   return(Mask);
}

   /* This function will take a GATT UUID from a discovered RSC service */
   /* and attempt to determine the characteristic the UUID represents.  */
static RSC_UUID_t DetermineRSCUUID(GATT_UUID_t UUID)
{
   RSC_UUID_t Type = ruUnknown;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(UUID.UUID_Type == guUUID_16)
   {
      if(RSCS_COMPARE_RSC_MEASUREMENT_UUID_TO_UUID_16(UUID.UUID.UUID_16))
         Type = ruMeasurements;
      else
      {
         if(RSCS_COMPARE_RSC_FEATURE_UUID_TO_UUID_16(UUID.UUID.UUID_16))
            Type = ruFeatures;
         else
         {
            if(RSCS_COMPARE_SENSOR_LOCATION_UUID_TO_UUID_16(UUID.UUID.UUID_16))
               Type = ruLocation;
            else
            {
               if(RSCS_COMPARE_SC_CONTROL_POINT_UUID_TO_UUID_16(UUID.UUID.UUID_16))
                  Type = ruControlPoint;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Type));

   return(Type);
}

   /* The function is a utility the locate the handle of the Client     */
   /* Characteristic Descriptor for a given characteristic.             */
static Word_t FindCCCD(GATT_Characteristic_Information_t *Characteristic)
{
   Word_t       ret_val = 0;
   GATT_UUID_t  UUID;
   unsigned int Index;

   for(Index=0;Index<Characteristic->NumberOfDescriptors;Index++)
   {
      UUID = Characteristic->DescriptorList[Index].Characteristic_Descriptor_UUID;

      if((UUID.UUID_Type == guUUID_16) && (GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16)))
      {
         ret_val = Characteristic->DescriptorList[Index].Characteristic_Descriptor_Handle;

         break;
      }
   }

   return(ret_val);
}

   /* This function is used to discover the RSC Sensor service on a     */
   /* remote device and add a tracking structure to the internal list of*/
   /* the service is discovered.                                        */
static Device_Entry_t *DiscoverRSCSensor(BD_ADDR_t *BluetoothAddress)
{
   int                                       Result;
   GATT_UUID_t                               UUID;
   unsigned int                              ServiceDataSize;
   unsigned int                              Index;
   unsigned int                              CharacteristicIndex;
   unsigned char                            *ServiceData;
   Device_Entry_t                            DeviceEntry;
   Device_Entry_t                           *DeviceEntryPtr = NULL;
   DEVM_Parsed_Services_Data_t               ParsedGATTData;
   GATT_Characteristic_Information_t        *CharacteristicInformation;
   GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Get the buffer size required to parse the service data.           */
   if((Result = DEVM_QueryRemoteDeviceServices(*BluetoothAddress, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 0, NULL, &ServiceDataSize)) == 0)
   {
      /* Allocate the buffer required to parse the service data         */
      if((ServiceData = (unsigned char *)BTPS_AllocateMemory(ServiceDataSize)) != NULL)
      {
         /* Query the services for the remote device.                   */
         if((Result = DEVM_QueryRemoteDeviceServices(*BluetoothAddress, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, ServiceDataSize, ServiceData, NULL)) > 0)
         {
            /* Convert the Raw GATT Stream to a Parsed GATT Stream.     */
            if((Result = DEVM_ConvertRawServicesStreamToParsedServicesData(ServiceDataSize, ServiceData, &ParsedGATTData)) == 0)
            {
               /* Iterate the services on the remote device.            */
               for(Index=0;Index<ParsedGATTData.NumberServices;Index++)
               {
                  UUID = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID;

                  /* Check for RSC.                                     */
                  if((UUID.UUID_Type == guUUID_16) && (RSCS_COMPARE_RSCS_SERVICE_UUID_TO_UUID_16(UUID.UUID.UUID_16)))
                  {
                     /* Initalize the device entry.                  */
                     BTPS_MemInitialize(&DeviceEntry, 0, sizeof(Device_Entry_t));
                     DeviceEntry.BluetoothAddress = *BluetoothAddress;

                     GATTServiceDiscoveryIndicationData = &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index]);

                     /* Iterate the RSC characteristics.             */
                     for(CharacteristicIndex=0;CharacteristicIndex<GATTServiceDiscoveryIndicationData->NumberOfCharacteristics;CharacteristicIndex++)
                     {
                        CharacteristicInformation = &(GATTServiceDiscoveryIndicationData->CharacteristicInformationList[CharacteristicIndex]);
                        UUID                      = CharacteristicInformation->Characteristic_UUID;

                        switch(DetermineRSCUUID(UUID))
                        {
                           case ruMeasurements:
                              if((DeviceEntry.ClientInformation.RSC_Measurement_Client_Configuration = FindCCCD(CharacteristicInformation)) > 0)
                                 DeviceEntry.ClientInformation.RSC_Measurement = CharacteristicInformation->Characteristic_Handle;
                              break;
                           case ruFeatures:
                              DeviceEntry.ClientInformation.RSC_Feature = CharacteristicInformation->Characteristic_Handle;
                              break;
                           case ruLocation:
                              DeviceEntry.ClientInformation.Sensor_Location = CharacteristicInformation->Characteristic_Handle;
                              DeviceEntry.ExtraCharacteristics             |= RSCM_SUPPORTED_CHARACTERISTIC_SENSOR_LOCATION;
                              break;
                           case ruControlPoint:
                              if((DeviceEntry.ClientInformation.SC_Control_Point_Client_Configuration = FindCCCD(CharacteristicInformation)) > 0)
                              {
                                 DeviceEntry.ClientInformation.SC_Control_Point = CharacteristicInformation->Characteristic_Handle;
                                 DeviceEntry.ExtraCharacteristics              |= RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT;
                              }
                              break;
                           case ruUnknown:
                           default:
                              DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown characteristic in RSC Service\n"));
                        }
                     }

                     /* The Measurement and Features characteristics    */
                     /* MUST be present.                                */
                     if((DeviceEntry.ClientInformation.RSC_Measurement) && (DeviceEntry.ClientInformation.RSC_Feature))
                     {
                        /* Attempt to start tracking the device.        */
                        if((DeviceEntryPtr = AddDeviceEntry(&DeviceEntryList, &DeviceEntry)) != NULL)
                        {
                           /* Attempt to reload any stored information  */
                           /* about this device.                        */
                           ReloadRSCDeviceFromFile(DeviceEntryPtr, DeviceEntryPtr->BluetoothAddress);

                           /* Now dispatch the connected event.         */
                           DispatchConnectedEvent(DeviceEntryPtr);
                        }
                     }

                     /* Exit loop after parsing RSC information.        */
                     break;
                  }
               }

               /* All finished with the parsed data, so free it.        */
               DEVM_FreeParsedServicesData(&ParsedGATTData);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_ConvertRawServicesStreamToParsedServicesData returned %d.\n", Result));
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
         }

         /* Free the previously allocated buffer holding service data   */
         /* information.                                                */
         BTPS_FreeMemory(ServiceData);
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FAILURE), ("Allocation request for Service Data failed, size = %u.\n", ServiceDataSize));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit %p\n", DeviceEntryPtr));

   return(DeviceEntryPtr);
}

   /* The following function is a utility to process a register         */
   /* collector events request from both remote IPC clients and local   */
   /* API calls.                                                        */
static int ProcessRegisterCollectorEvents(unsigned int ClientID, RSCM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   Callback_Entry_t CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the callback entry.                                        */
   BTPS_MemInitialize(&CallbackEntry, 0, sizeof(CallbackEntry));

   CallbackEntry.CallbackID                     = GetNextCallbackID();
   CallbackEntry.CallbackInfo.ClientID          = ClientID;
   CallbackEntry.CallbackInfo.EventCallback     = CallbackFunction;
   CallbackEntry.CallbackInfo.CallbackParameter = CallbackParameter;

   /* Simply add the entry.                                             */
   if(AddCallbackEntry(&CallbackEntryList, &CallbackEntry) != NULL)
      ret_val = CallbackEntry.CallbackID;
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a un-register      */
   /* collector events request from both remote IPC clients and local   */
   /* API calls.                                                        */
static void ProcessUnRegisterCollectorEvents(unsigned int ClientID, unsigned int CollectorCallbackID)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply delete the callback from the list.                         */
   if(((CallbackEntry = DeleteCallbackEntry(&CallbackEntryList, CollectorCallbackID)) != NULL) && (CallbackEntry->CallbackInfo.ClientID == ClientID))
      FreeCallbackEntryMemory(CallbackEntry);

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility to process a query connected  */
   /* sensors request from both remote IPC clients and local API calls. */
static int ProcessQueryConnectedSensors(unsigned int ClientID, unsigned int MaximumRemoteDeviceListEntries, RSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int             ret_val;
   unsigned int    DevicesCopied = 0;
   unsigned int    TotalDevices  = 0;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Start at the beginning of the device list.                        */
   DeviceEntry = DeviceEntryList;

   /* Check the the parameters seem valid.                              */
   if((MaximumRemoteDeviceListEntries && ConnectedDeviceList) || (TotalNumberConnectedDevices))
   {
      /* Loop through the list.                                         */
      while(DeviceEntry)
      {
         /* Check if we have a buffer to copy.                          */
         if(ConnectedDeviceList)
         {
            /* Check if we have room in the buffer.                     */
            if(DevicesCopied < MaximumRemoteDeviceListEntries)
            {
               ConnectedDeviceList[DevicesCopied].Configured                      = (DeviceEntry->ConfigurationState == csConfigured);
               ConnectedDeviceList[DevicesCopied].RemoteDeviceAddress             = DeviceEntry->BluetoothAddress;
               ConnectedDeviceList[DevicesCopied].SupportedOptionalCharateristics = DeviceEntry->ExtraCharacteristics;
               ConnectedDeviceList[DevicesCopied].SupportedFeatures               = DeviceEntry->SupportedFeatures;
               ConnectedDeviceList[DevicesCopied].SupportedSensorLocations        = DeviceEntry->SupportedSensorLocations;

               DevicesCopied++;
            }
            else
            {
               /* If we are out of room and don't need to count all the */
               /* devices, break out now.                               */
               if(!TotalNumberConnectedDevices)
                  break;
            }
         }

         TotalDevices++;

         DeviceEntry = DeviceEntry->NextDeviceEntryPtr;
      }

      if(TotalNumberConnectedDevices)
         *TotalNumberConnectedDevices = TotalDevices;

      ret_val = DevicesCopied;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a configre remote  */
   /* sensor request from both remote IPC clients and local API calls.  */
static int ProcessConfigureRemoteSensor(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned long Flags)
{
   int                  ret_val;
   Device_Entry_t      *DeviceEntry;
   NonAlignedWord_t     Buffer;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that the parameters seem valid.                             */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Make sure the device is connected.                             */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL)
      {
         /* Only submit a transaction if we are not configured.         */
         if(DeviceEntry->ConfigurationState == csNotConfigured)
         {
            /* Format the initial transaction.                          */
            BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

            TransactionEntry.TransactionID    = GetNextTransactionID();
            TransactionEntry.BluetoothAddress = RemoteDeviceAddress;
            TransactionEntry.TransactionType  = ttEnableMeasurementCCCD;

            /* Attempt to add the entry.                                */
            if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
            {
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

               /* Now attempt to actually submit the write.             */
               if((ret_val = GATM_WriteValue(GATMCallbackID, RemoteDeviceAddress, DeviceEntry->ClientInformation.RSC_Measurement_Client_Configuration, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Buffer)) > 0)
               {
                  /* Note the transaction ID and update the new state.  */
                  TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                  DeviceEntry->ConfigurationState        = csEnablingMeasurements;

                  if(Flags & RSCM_CONFIGURE_FLAG_SKIP_SUPPORTED_SENSOR_LOCATIONS)
                     DeviceEntry->Flags |= (unsigned long)DEVICE_ENTRY_FLAGS_SKIP_SUPPORTED_LOCATIONS;
                  else
                     DeviceEntry->Flags &= ~((unsigned long)DEVICE_ENTRY_FLAGS_SKIP_SUPPORTED_LOCATIONS);

                  ret_val = 0;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
         {
            /* If we are already confiured, let the called know,        */
            /* otherwise they will be notified whenever the ongoing     */
            /* configuration completes.                                 */
            if(DeviceEntry->ConfigurationState == csConfigured)
               ret_val = BTPM_ERROR_CODE_RSC_ALREADY_CONFIGURED;
            else
            {
               /* We could also be un-configuring.                      */
               if(DeviceEntry->ConfigurationState > csConfigured)
                  ret_val = BTPM_ERROR_CODE_RSC_CURRENTLY_UN_CONFIGURING;
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Already Configuring: %d\n", DeviceEntry->ConfigurationState));
                  ret_val = 0;
               }
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_RSC_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a un-configure     */
   /* remote sensor request from both remote IPC clients and local API  */
   /* calls.                                                            */
static int ProcessUnConfigureRemoteSensor(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress)
{
   int                  ret_val;
   Word_t               Handle;
   Device_Entry_t      *DeviceEntry;
   NonAlignedWord_t     Buffer;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that the parameters seem valid.                             */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Make sure the device is connected.                             */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL)
      {
         /* Only submit a transaction if we are not configured.         */
         if(DeviceEntry->ConfigurationState == csConfigured)
         {
            /* Format the initial transaction.                          */
            BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

            TransactionEntry.TransactionID    = GetNextTransactionID();
            TransactionEntry.BluetoothAddress = RemoteDeviceAddress;
            TransactionEntry.TransactionType  = (DeviceEntry->ExtraCharacteristics & RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT)?ttDisableControlCCCD:ttDisableMeasurementCCCD;

            Handle                            = (DeviceEntry->ExtraCharacteristics & RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT)?DeviceEntry->ClientInformation.SC_Control_Point_Client_Configuration:DeviceEntry->ClientInformation.RSC_Measurement_Client_Configuration;

            /* Attempt to add the entry.                                */
            if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
            {
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, 0);

               /* Now attempt to actually submit the write.             */
               if((ret_val = GATM_WriteValue(GATMCallbackID, RemoteDeviceAddress, Handle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Buffer)) > 0)
               {
                  /* Note the transaction ID and update the new state.  */
                  TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                  DeviceEntry->ConfigurationState = (TransactionEntryPtr->TransactionType == ttDisableControlCCCD)?csDisablingControlPoint:csDisablingMeasurements;

                  ret_val = 0;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
         {
            /* If we are already unconfigured or are configuring, we    */
            /* need an error.                                           */
            if(DeviceEntry->ConfigurationState == csNotConfigured)
               ret_val = BTPM_ERROR_CODE_RSC_NOT_CONFIGURED;
            else
            {
               /* If we are configuring, throw in error.                */
               if(DeviceEntry->ConfigurationState < csConfigured)
                  ret_val = BTPM_ERROR_CODE_RSC_CURRENTLY_CONFIGURING;
               else
                  ret_val = 0;
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_RSC_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a get connected    */
   /* sensor info request from both remote IPC clients and local API    */
   /* calls.                                                            */
static int ProcessGetConnectedSensorInfo(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, RSCM_Connected_Sensor_t *DeviceInfo)
{
   int             ret_val;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (DeviceInfo))
   {
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL)
      {
         DeviceInfo->RemoteDeviceAddress             = DeviceEntry->BluetoothAddress;
         DeviceInfo->Configured                      = (DeviceEntry->ConfigurationState == csConfigured);
         DeviceInfo->SupportedOptionalCharateristics = DeviceEntry->ExtraCharacteristics;
         DeviceInfo->SupportedFeatures               = DeviceEntry->SupportedFeatures;
         DeviceInfo->SupportedSensorLocations        = DeviceEntry->SupportedSensorLocations;

         ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_RSC_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a get sensor       */
   /* location request from both remote IPC clients and local API calls.*/
static int ProcessGetSensorLocation(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress)
{
   int                  ret_val;
   Device_Entry_t      *DeviceEntry;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      if(((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL) && (DeviceEntry->ConfigurationState == csConfigured))
      {
         /* Make sure the device supports this operation.               */
         if(DeviceEntry->ExtraCharacteristics & RSCM_SUPPORTED_CHARACTERISTIC_SENSOR_LOCATION)
         {
            /* Make sure the same client has not already submitted this */
            /* request.                                                 */
            if((SearchTransactionEntry(&TransactionEntryList, &DeviceEntry->BluetoothAddress, ttReadLocation, ClientID)) == NULL)
            {
               /* Format the transaction.                               */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID    = GetNextTransactionID();
               TransactionEntry.TransactionType  = ttReadLocation;
               TransactionEntry.BluetoothAddress = DeviceEntry->BluetoothAddress;
               TransactionEntry.ClientID         = ClientID;

               /* Now attempt to add the transaction.                   */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Attempt to make the GATT Read Request.             */
                  if((ret_val = GATM_ReadValue(GATMCallbackID, DeviceEntry->BluetoothAddress, DeviceEntry->ClientInformation.Sensor_Location, 0, TRUE)) > 0)
                  {
                     TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                     ret_val                                = 0;
                  }
                  else
                  {
                     /* Remove the transaction from the list.           */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_RSC_SAME_REQUEST_ALREADY_OUTSTANDING;
         }
         else
            ret_val = BTPM_ERROR_CODE_RSC_REMOTE_FEATURE_NOT_SUPPORTED;
      }
      else
      {
         /* Check if we are not connected or not configured.            */
         if(DeviceEntry)
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONFIGURED;
         else
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONNECTED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a update cumulative*/
   /* value request from both remote IPC clients and local API calls.   */
static int ProcessUpdateCumulativeValue(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue)
{
   int                                  ret_val;
   Byte_t                               Buffer[16];
   unsigned int                         ProcedureID;
   unsigned int                         BufferLength;
   Device_Entry_t                      *DeviceEntry;
   Transaction_Entry_t                  TransactionEntry;
   Transaction_Entry_t                 *TransactionEntryPtr;
   RSCS_SC_Control_Point_Format_Data_t  CommandData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters seem valid.                              */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Check that the device is connected.                            */
      if(((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL) && (DeviceEntry->ConfigurationState == csConfigured))
      {
         /* Make sure the device supports this operation.               */
         if(DeviceEntry->ExtraCharacteristics & RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT)
         {
            /* Make sure that there is not already an ongoing procedure.*/
            if(DeviceEntry->CurrentProcedure == cpNone)
            {
               /* Format the transaction.                               */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID    = GetNextTransactionID();
               TransactionEntry.TransactionType  = ttWriteControlPoint;
               TransactionEntry.BluetoothAddress = DeviceEntry->BluetoothAddress;
               TransactionEntry.ClientID         = ClientID;

               /* Now attempt to add the transaction.                   */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Format the Control Point Command.                  */
                  CommandData.CommandType                      = sccSetCumulativeValue;
                  CommandData.FilterParameters.CumulativeValue = CumulativeValue;

                  BufferLength = sizeof(Buffer);

                  if((ret_val = RSCS_Format_SC_Control_Point_Command(&CommandData, &BufferLength, Buffer)) == 0)
                  {
                     /* Attempt to make the GATT Write Request.         */
                     if((ret_val = GATM_WriteValue(GATMCallbackID, DeviceEntry->BluetoothAddress, DeviceEntry->ClientInformation.SC_Control_Point, BufferLength, Buffer)) > 0)
                     {
                        TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                        ProcedureID                            = GetNextProcedureID();

                        /* Start a timer to time out the procedure if no*/
                        /* response is received.                        */
                        if((ret_val = TMR_StartTimer((void *)ProcedureID, ProcedureTimerCallback, RSCM_PROCEDURE_TIMEOUT)) > 0)
                        {
                           DeviceEntry->CurrentProcedureID = ProcedureID;
                           DeviceEntry->CurrentProcedure   = cpUpdateCumulative;
                           DeviceEntry->PendingCumulative  = CumulativeValue;
                           DeviceEntry->ProcedureTimerID   = (unsigned int)ret_val;
                           ret_val                         = DeviceEntry->CurrentProcedureID;
                        }
                        else
                        {
                           /* Remove the transaction from the list.     */
                           if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                              FreeTransactionEntryMemory(TransactionEntryPtr);
                        }
                     }
                     else
                     {
                        /* Remove the transaction from the list.        */
                        if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                           FreeTransactionEntryMemory(TransactionEntryPtr);
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_RSC_CONTROL_POINT_PROCEDURE_IN_PROGRESS;
         }
         else
            ret_val = BTPM_ERROR_CODE_RSC_REMOTE_FEATURE_NOT_SUPPORTED;
      }
      else
      {
         /* Check if we are not connected or not configured.            */
         if(DeviceEntry)
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONFIGURED;
         else
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONNECTED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a update sensor    */
   /* location request from both remote IPC clients and local API calls.*/
static int ProcessUpdateSensorLocation(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, RSCM_Sensor_Location_t Location)
{
   int                                  ret_val;
   Byte_t                               Buffer[16];
   unsigned int                         BufferLength;
   unsigned int                         ProcedureID;
   Device_Entry_t                      *DeviceEntry;
   Transaction_Entry_t                  TransactionEntry;
   Transaction_Entry_t                 *TransactionEntryPtr;
   RSCS_SC_Control_Point_Format_Data_t  CommandData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters seem valid.                              */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Check that the device is connected.                            */
      if(((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL) && (DeviceEntry->ConfigurationState == csConfigured))
      {
         /* Make sure the device supports this operation.               */
         if(DeviceEntry->ExtraCharacteristics & RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT)
         {
            /* Make sure that there is not already an ongoing procedure.*/
            if(DeviceEntry->CurrentProcedure == cpNone)
            {
               /* Format the transaction.                               */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID    = GetNextTransactionID();
               TransactionEntry.TransactionType  = ttWriteControlPoint;
               TransactionEntry.BluetoothAddress = DeviceEntry->BluetoothAddress;
               TransactionEntry.ClientID         = ClientID;

               /* Now attempt to add the transaction.                   */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Format the Control Point Command.                  */
                  CommandData.CommandType                     = sccUpdateSensorLocation;
                  CommandData.FilterParameters.SensorLocation = Location;

                  BufferLength = sizeof(Buffer);

                  if((ret_val = RSCS_Format_SC_Control_Point_Command(&CommandData, &BufferLength, Buffer)) == 0)
                  {
                     /* Attempt to make the GATT Write Request.         */
                     if((ret_val = GATM_WriteValue(GATMCallbackID, DeviceEntry->BluetoothAddress, DeviceEntry->ClientInformation.SC_Control_Point, BufferLength, Buffer)) > 0)
                     {
                        TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                        ProcedureID                            = GetNextProcedureID();

                        /* Start a timer to time out the procedure if no*/
                        /* response is received.                        */
                        if((ret_val = TMR_StartTimer((void *)ProcedureID, ProcedureTimerCallback, RSCM_PROCEDURE_TIMEOUT)) > 0)
                        {
                           DeviceEntry->CurrentProcedureID = ProcedureID;
                           DeviceEntry->CurrentProcedure   = cpUpdateLocation;
                           DeviceEntry->PendingLocation    = Location;
                           DeviceEntry->ProcedureTimerID   = (unsigned int)ret_val;
                           ret_val                         = DeviceEntry->CurrentProcedureID;
                        }
                        else
                        {
                           /* Remove the transaction from the list.     */
                           if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                              FreeTransactionEntryMemory(TransactionEntryPtr);
                        }
                     }
                     else
                     {
                        /* Remove the transaction from the list.        */
                        if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                           FreeTransactionEntryMemory(TransactionEntryPtr);
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_RSC_CONTROL_POINT_PROCEDURE_IN_PROGRESS;
         }
         else
            ret_val = BTPM_ERROR_CODE_RSC_REMOTE_FEATURE_NOT_SUPPORTED;
      }
      else
      {
         /* Check if we are not connected or not configured.            */
         if(DeviceEntry)
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONFIGURED;
         else
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONNECTED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility to process a start sensor     */
   /* calibration request from both remote IPC clients and local API    */
   /* calls.                                                            */
static int ProcessStartSensorCalibration(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress)
{
   int                                  ret_val;
   Byte_t                               Buffer[16];
   unsigned int                         BufferLength;
   unsigned int                         ProcedureID;
   Device_Entry_t                      *DeviceEntry;
   Transaction_Entry_t                  TransactionEntry;
   Transaction_Entry_t                 *TransactionEntryPtr;
   RSCS_SC_Control_Point_Format_Data_t  CommandData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters seem valid.                              */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Check that the device is connected.                            */
      if(((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL) && (DeviceEntry->ConfigurationState == csConfigured))
      {
         /* Make sure the device supports this operation.               */
         if(DeviceEntry->ExtraCharacteristics & RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT)
         {
            /* Make sure that there is not already an ongoing procedure.*/
            if(DeviceEntry->CurrentProcedure == cpNone)
            {
               /* Format the transaction.                               */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID    = GetNextTransactionID();
               TransactionEntry.TransactionType  = ttWriteControlPoint;
               TransactionEntry.BluetoothAddress = DeviceEntry->BluetoothAddress;
               TransactionEntry.ClientID         = ClientID;

               /* Now attempt to add the transaction.                   */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Format the Control Point Command.                  */
                  CommandData.CommandType                     = sccStartSensorCalibration;

                  BufferLength = sizeof(Buffer);

                  if((ret_val = RSCS_Format_SC_Control_Point_Command(&CommandData, &BufferLength, Buffer)) == 0)
                  {
                     /* Attempt to make the GATT Write Request.         */
                     if((ret_val = GATM_WriteValue(GATMCallbackID, DeviceEntry->BluetoothAddress, DeviceEntry->ClientInformation.SC_Control_Point, BufferLength, Buffer)) > 0)
                     {
                        TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                        ProcedureID                            = GetNextProcedureID();

                        /* Start a timer to time out the procedure if no*/
                        /* response is received.                        */
                        if((ret_val = TMR_StartTimer((void *)ProcedureID, ProcedureTimerCallback, RSCM_PROCEDURE_TIMEOUT)) > 0)
                        {
                           DeviceEntry->CurrentProcedureID = ProcedureID;
                           DeviceEntry->CurrentProcedure   = cpStartSensorCalibration;
                           DeviceEntry->ProcedureTimerID   = (unsigned int)ret_val;
                           ret_val                         = DeviceEntry->CurrentProcedureID;
                        }
                        else
                        {
                           /* Remove the transaction from the list.     */
                           if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                              FreeTransactionEntryMemory(TransactionEntryPtr);
                        }
                     }
                     else
                     {
                        /* Remove the transaction from the list.        */
                        if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                           FreeTransactionEntryMemory(TransactionEntryPtr);
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_RSC_CONTROL_POINT_PROCEDURE_IN_PROGRESS;
         }
         else
            ret_val = BTPM_ERROR_CODE_RSC_REMOTE_FEATURE_NOT_SUPPORTED;
      }
      else
      {
         /* Check if we are not connected or not configured.            */
         if(DeviceEntry)
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONFIGURED;
         else
            ret_val = BTPM_ERROR_CODE_RSC_NOT_CONNECTED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function processes the specified Register Collector */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterCollectorEventsMessage(RSCM_Register_Collector_Events_Request_t *Message)
{
   int                                       Result;
   RSCM_Register_Collector_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessRegisterCollectorEvents(Message->MessageHeader.AddressID, NULL, NULL);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = RSCM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status     = 0;
         ResponseMessage.CallbackID = Result;
      }
      else
      {
         ResponseMessage.Status     = Result;
         ResponseMessage.CallbackID = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified UnRegister         */
   /* Collector Events Request Message and responds to the message      */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterCollectorEventsMessage(RSCM_Un_Register_Collector_Events_Request_t *Message)
{
   RSCM_Un_Register_Collector_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      ProcessUnRegisterCollectorEvents(Message->MessageHeader.AddressID, Message->CallbackID);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = 0;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Connected    */
   /* Sensors Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessQueryConnectedSensorsMessage(RSCM_Query_Connected_Sensors_Request_t *Message)
{
   int                                      Result;
   unsigned int                             TotalDevices = 0;
   unsigned int                             NumberDevices;
   RSCM_Query_Connected_Sensors_Response_t  StaticMessage;
   RSCM_Query_Connected_Sensors_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* First determine how many devices are connected.                */
      if((Result = ProcessQueryConnectedSensors(Message->MessageHeader.AddressID, 0, NULL, &TotalDevices)) == 0)
      {
         /* If the client only wants total devices, just return that    */
         /* information.                                                */
         if(!Message->MaximumRemoteDeviceListEntries)
         {
            BTPS_MemInitialize(&StaticMessage, 0, RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0));

            StaticMessage.MessageHeader               = Message->MessageHeader;
            StaticMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
            StaticMessage.MessageHeader.MessageLength = RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
            StaticMessage.Status                      = 0;
            StaticMessage.NumberDevices               = 0;
            StaticMessage.TotalNumberConnectedDevices = TotalDevices;

            MSG_SendMessage((BTPM_Message_t *)&StaticMessage);
         }
         else
         {
            /* Determine how many devices to allocated.                 */
            NumberDevices = (TotalDevices > Message->MaximumRemoteDeviceListEntries)?Message->MaximumRemoteDeviceListEntries:TotalDevices;

            /* Now allocate a message to hold the number of devices.       */
            if((ResponseMessage = (RSCM_Query_Connected_Sensors_Response_t *)BTPS_AllocateMemory(RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(NumberDevices))) != NULL)
            {
               BTPS_MemInitialize(ResponseMessage, 0, RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(NumberDevices));

               /* Now copy the devices into the allocated message.         */
               if((Result = ProcessQueryConnectedSensors(Message->MessageHeader.AddressID, NumberDevices, ResponseMessage->ConnectedSensors, &ResponseMessage->TotalNumberConnectedDevices)) >= 0)
               {
                  /* Format and send the response message.              */
                  ResponseMessage->MessageHeader               = Message->MessageHeader;
                  ResponseMessage->MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
                  ResponseMessage->MessageHeader.MessageLength = RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(NumberDevices) - BTPM_MESSAGE_HEADER_SIZE;
                  ResponseMessage->Status                      = 0;
                  ResponseMessage->NumberDevices               = Result;

                  MSG_SendMessage((BTPM_Message_t *)ResponseMessage);
               }
               else
               {
                  /* Format and send and error message.                 */
                  BTPS_MemInitialize(&StaticMessage, 0, RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0));

                  StaticMessage.MessageHeader               = Message->MessageHeader;
                  StaticMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
                  StaticMessage.MessageHeader.MessageLength = RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
                  StaticMessage.Status                      = Result;

                  MSG_SendMessage((BTPM_Message_t *)&StaticMessage);
               }

               BTPS_FreeMemory(ResponseMessage);
            }
            else
            {
               /* Format and send and error message.                    */
               BTPS_MemInitialize(&StaticMessage, 0, RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0));

               StaticMessage.MessageHeader               = Message->MessageHeader;
               StaticMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
               StaticMessage.MessageHeader.MessageLength = RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
               StaticMessage.Status                      = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

               MSG_SendMessage((BTPM_Message_t *)&StaticMessage);
            }
         }
      }
      else
      {
         /* Format and send and error message.                          */
         BTPS_MemInitialize(&StaticMessage, 0, RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0));

         StaticMessage.MessageHeader               = Message->MessageHeader;
         StaticMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
         StaticMessage.MessageHeader.MessageLength = RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
         StaticMessage.Status                      = Result;

         MSG_SendMessage((BTPM_Message_t *)&StaticMessage);
      }

   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Configure Remote   */
   /* Sensor Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessConfigureRemoteSensorMessage(RSCM_Configure_Remote_Sensor_Request_t *Message)
{
   int                                     Result;
   RSCM_Configure_Remote_Sensor_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessConfigureRemoteSensor(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->Flags);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified UnConfigure Remote */
   /* Sensor Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessUnConfigureRemoteSensorMessage(RSCM_Un_Configure_Remote_Sensor_Request_t *Message)
{
   int                                        Result;
   RSCM_Un_Configure_Remote_Sensor_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessUnConfigureRemoteSensor(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_UN_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_UN_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Connected      */
   /* Sensor Information Request Message and responds to the message    */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessGetConnectedSensorInfoMessage(RSCM_Get_Connected_Sensor_Info_Request_t *Message)
{
   int                                       Result;
   RSCM_Connected_Sensor_t                   Sensor;
   RSCM_Get_Connected_Sensor_Info_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessGetConnectedSensorInfo(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, &Sensor);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_GET_CONNECTED_SENSOR_INFO_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_GET_CONNECTED_SENSOR_INFO_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      if(!Result)
         ResponseMessage.ConnectedSensorInfo = Sensor;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Sensor Location*/
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessGetSensorLocationMessage(RSCM_Get_Sensor_Location_Request_t *Message)
{
   int                                 Result;
   RSCM_Get_Sensor_Location_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessGetSensorLocation(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_GET_SENSOR_LOCATION_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_GET_SENSOR_LOCATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Update Cumulative  */
   /* Value Request Message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessUpdateCumulativeValueMessage(RSCM_Update_Cumulative_Value_Request_t *Message)
{
   int                                     Result;
   RSCM_Update_Cumulative_Value_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessUpdateCumulativeValue(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->CumulativeValue);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_UPDATE_CUMULATIVE_VALUE_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_UPDATE_CUMULATIVE_VALUE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Update Sensor      */
   /* Location Request Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessUpdateSensorLocationMessage(RSCM_Update_Sensor_Location_Request_t *Message)
{
   int                                    Result;
   RSCM_Update_Sensor_Location_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessUpdateSensorLocation(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->SensorLocation);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_UPDATE_SENSOR_LOCATION_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_UPDATE_SENSOR_LOCATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Update Sensor      */
   /* Location Request Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the RSC Manager Lock */
   /*          held.                                                    */
static void ProcessStartSensorCalibrationMessage(RSCM_Start_Sensor_Calibration_Request_t *Message)
{
   int                                      Result;
   RSCM_Start_Sensor_Calibration_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Result = ProcessStartSensorCalibration(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, RSCM_START_SENSOR_CALIBRATION_RESPONSE_SIZE);

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = RSCM_START_SENSOR_CALIBRATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the RSC Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case RSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
               ProcessRegisterCollectorEventsMessage((RSCM_Register_Collector_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Register Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterCollectorEventsMessage((RSCM_Un_Register_Collector_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Connected Sensors Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_QUERY_CONNECTED_SENSORS_REQUEST_SIZE)
               ProcessQueryConnectedSensorsMessage((RSCM_Query_Connected_Sensors_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure Remote Sensor Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE)
               ProcessConfigureRemoteSensorMessage((RSCM_Configure_Remote_Sensor_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Configure Remote Sensor Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_UN_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE)
               ProcessUnConfigureRemoteSensorMessage((RSCM_Un_Configure_Remote_Sensor_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Connected Sensor Info Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_GET_CONNECTED_SENSOR_INFO_REQUEST_SIZE)
               ProcessGetConnectedSensorInfoMessage((RSCM_Get_Connected_Sensor_Info_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Sensor Location Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_GET_SENSOR_LOCATION_REQUEST_SIZE)
               ProcessGetSensorLocationMessage((RSCM_Get_Sensor_Location_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Cumulative Value Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_UPDATE_CUMULATIVE_VALUE_REQUEST_SIZE)
               ProcessUpdateCumulativeValueMessage((RSCM_Update_Cumulative_Value_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Sensor Location Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_UPDATE_SENSOR_LOCATION_REQUEST_SIZE)
               ProcessUpdateSensorLocationMessage((RSCM_Update_Sensor_Location_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case RSCM_MESSAGE_FUNCTION_START_SENSOR_CALIBRATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Sensor Location Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= RSCM_START_SENSOR_CALIBRATION_REQUEST_SIZE)
               ProcessStartSensorCalibrationMessage((RSCM_Start_Sensor_Calibration_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));

            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   Callback_Entry_t *CallbackEntry;
   Callback_Entry_t *tmpCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Loop through the callback list and delete all callback         */
      /* registered for this client.                                    */
      CallbackEntry = CallbackEntryList;

      while(CallbackEntry)
      {
         /* Check if this callback was registered by the client.        */
         if(CallbackEntry->CallbackInfo.ClientID == ClientID)
         {
            /* Note the next entry, since we are going to free this one.*/
            tmpCallbackEntry = CallbackEntry->NextCallbackEntryPtr;

            /* Go ahead and delete the entry.                           */
            if((CallbackEntry = DeleteCallbackEntry(&CallbackEntryList, CallbackEntry->CallbackID)) != NULL)
               FreeCallbackEntryMemory(CallbackEntry);

            /* Move to the next entry.                                  */
            CallbackEntry = tmpCallbackEntry;
         }
         else
            CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible RSCM Connection.                               */
   /* * NOTE * This function *MUST* be called with the RSCM Manager Lock*/
   /*          held.                                                    */
static void ProcessRSCMConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Update Device Entry with service discovery information.        */
      if((DeviceEntry = DiscoverRSCSensor(&(RemoteDeviceProperties->BD_ADDR))) != NULL)
      {
         /* If we are paired, flag that we want to save this devices    */
         /* state.                                                      */
         if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)
            DeviceEntry->Flags |= DEVICE_ENTRY_FLAGS_STORE_STATE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible RSCM Connection.                               */
   /* * NOTE * This function *MUST* be called with the RSCM Manager Lock*/
   /*          held.                                                    */
static void ProcessRSCMDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Delete the device if it is known.                              */
      if((DeviceEntry = DeleteDeviceEntry(&DeviceEntryList, &(RemoteDeviceProperties->BD_ADDR))) != NULL)
      {
         /* Dispatch a disconnected event.                              */
         DispatchDisconnectedEvent(DeviceEntry);

         FreeDeviceEntryMemory(DeviceEntry);
      }

      /* Now make sure we remove any transactions for this device.      */
      DeleteTransactionsForDevice(RemoteDeviceProperties->BD_ADDR);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible RSCM Device Paired Event.                      */
   /* * NOTE * This function *MUST* be called with the RSCM Manager Lock*/
   /*          held.                                                    */
static void ProcessRSCMDevicePaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(RemoteDeviceProperties)
   {
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &(RemoteDeviceProperties->BD_ADDR))) != NULL)
      {
         /* Note that we want to store any configuration changes in the */
         /* settings file.                                              */
         DeviceEntry->Flags |= DEVICE_ENTRY_FLAGS_STORE_STATE;

         /* Go ahead and store information for this device, even though */
         /* we are probably not configured.                             */
         StoreRSCMDeviceToFile(DeviceEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible RSCM Device Un-Paired Event.                   */
   /* * NOTE * This function *MUST* be called with the RSCM Manager Lock*/
   /*          held.                                                    */
static void ProcessRSCMDeviceUnPaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(RemoteDeviceProperties)
   {
      /* If we're tracking a connection to the device, note that we no  */
      /* longer want to save any updates.                               */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &(RemoteDeviceProperties->BD_ADDR))) != NULL)
         DeviceEntry->Flags &= ~((unsigned long)DEVICE_ENTRY_FLAGS_STORE_STATE);

      /* Delete the device from storage.                                */
      if(ReloadRSCDeviceFromFile(NULL, RemoteDeviceProperties->BD_ADDR))
         ClearRSCMDeviceFromFile(RemoteDeviceProperties->BD_ADDR);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the RSCM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t      *DeviceEntry;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(RemoteDeviceProperties)
   {
      /* First clear out any storage for the old address.               */
      if(ReloadRSCDeviceFromFile(NULL, RemoteDeviceProperties->PriorResolvableBD_ADDR))
         ClearRSCMDeviceFromFile(RemoteDeviceProperties->PriorResolvableBD_ADDR);

      /* Next, see if we were tracking the device and update it         */
      /* accordingly.                                                   */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &(RemoteDeviceProperties->PriorResolvableBD_ADDR))) != NULL)
      {
         /* Store the new address.                                      */
         DeviceEntry->BluetoothAddress = RemoteDeviceProperties->BD_ADDR;

         /* Update the file if necessary.                               */
         StoreRSCMDeviceToFile(DeviceEntry);
      }

      /* Now walk the list of transactions to update any transaction for*/
      /* this address.                                                  */
      TransactionEntry = TransactionEntryList;

      while(TransactionEntry)
      {
         if(COMPARE_BD_ADDR(TransactionEntry->BluetoothAddress, RemoteDeviceProperties->PriorResolvableBD_ADDR))
            TransactionEntry->BluetoothAddress = RemoteDeviceProperties->BD_ADDR;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Deleted Event.                            */
   /* * NOTE * This function *MUST* be called with the RSCM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDeviceDeletedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Check if this device was connected.                            */
      /* * NOTE * You should have to be disconnected before you can     */
      /*          delete a device.                                      */
      if((DeviceEntry = DeleteDeviceEntry(&DeviceEntryList, &RemoteDeviceAddress)) != NULL)
      {
         DispatchDisconnectedEvent(DeviceEntry);
         FreeDeviceEntryMemory(DeviceEntry);

         /* Make sure we remove all transactions for this device.       */
         DeleteTransactionsForDevice(RemoteDeviceAddress);
      }

      /* Delete the device from the file if necessary.                  */
      if(ReloadRSCDeviceFromFile(NULL, RemoteDeviceAddress))
         ClearRSCMDeviceFromFile(RemoteDeviceAddress);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the RSCM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long RequiredConnectionFlags;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

         /* Process the LE Connection State Change Event if necessary.  */
         if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
         {
            /* We need to be connected and know the services to consider*/
            /* the connection to this device.                           */
            RequiredConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN);

            /* Check to see if this connection is a candidate.          */
            if((RemoteDeviceProperties->RemoteDeviceFlags & RequiredConnectionFlags) == RequiredConnectionFlags)
            {
               /* Process a possible RSCM Connection.                   */
               ProcessRSCMConnection(RemoteDeviceProperties);
            }
            else
            {
               /* Process a possible disconnection.                     */
               ProcessRSCMDisconnection(RemoteDeviceProperties);

               /* If we no longer know services, any previous saved     */
               /* state is likely invalid, so delete it.                */
               if(!(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN))
               {
                  if(ReloadRSCDeviceFromFile(NULL, RemoteDeviceProperties->BD_ADDR))
                     ClearRSCMDeviceFromFile(RemoteDeviceProperties->BD_ADDR);
               }
            }
         }
      }

      /* Check if our pairing status has changed.                       */
      if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE)
      {
         /* If we are paired, note that we want to track any changes for*/
         /* the device in persistant storage.                           */
         if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)
            ProcessRSCMDevicePaired(RemoteDeviceProperties);
         else
            ProcessRSCMDeviceUnPaired(RemoteDeviceProperties);
      }

      /* Check if the address changed.                                  */
      if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
         ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* This function is a utility to process a GATM event in response to */
   /* transaction that is a part of the configuration procedure. This   */
   /* function will start the next transaction in the configuration if  */
   /* necessary.                                                        */
static void ProcessConfigurationResponse(GATM_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry, Boolean_t Success)
{
   int                    Result;
   Byte_t                *BufferPtr              = NULL;
   Byte_t                 OpCode;
   Word_t                 Handle;
   Boolean_t              Submit                 = TRUE;
   unsigned int           BufferLength;
   Device_Entry_t        *DeviceEntry;
   NonAlignedWord_t       CCDBuffer;
   Transaction_Entry_t    NewTransactionEntry;
   Transaction_Entry_t   *NewTransactionEntryPtr;
   Configuration_State_t  NewConfigurationState;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((EventData) && (TransactionEntry))
   {
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &TransactionEntry->BluetoothAddress)) != NULL)
      {
         if(Success)
         {
            BTPS_MemInitialize(&NewTransactionEntry, 0, sizeof(NewTransactionEntry));

            NewTransactionEntry.TransactionID    = GetNextTransactionID();
            NewTransactionEntry.BluetoothAddress = DeviceEntry->BluetoothAddress;

            switch(TransactionEntry->TransactionType)
            {
               case ttEnableMeasurementCCCD:
                  NewTransactionEntry.TransactionType = ttReadFeatures;
                  Handle                              = DeviceEntry->ClientInformation.RSC_Feature;
                  NewConfigurationState               = csGettingFeatures;
                  break;
               case ttReadFeatures:
                  DeviceEntry->SupportedFeatures = (unsigned long)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(EventData->EventData.ReadResponseEventData.Value);

                  if(DeviceEntry->ExtraCharacteristics & RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT)
                  {
                     /* Format the command to enable Control Point      */
                     /* indications.                                    */
                     ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCDBuffer, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE);

                     NewTransactionEntry.TransactionType = ttEnableControlCCCD;
                     Handle                              = DeviceEntry->ClientInformation.SC_Control_Point_Client_Configuration;
                     NewConfigurationState               = csEnablingControlPoint;
                     BufferPtr                           = (Byte_t *)&CCDBuffer;
                     BufferLength                        = NON_ALIGNED_WORD_SIZE;
                  }
                  else
                  {
                     /* There are no more transaction left without      */
                     /* a control point feature, so dispatch the        */
                     /* configured event.                               */
                     DeviceEntry->ConfigurationState = csConfigured;

                     DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_SUCCESS);

                     /* Save the new state information if necessary.    */
                     if(DeviceEntry->Flags & DEVICE_ENTRY_FLAGS_STORE_STATE)
                        StoreRSCMDeviceToFile(DeviceEntry);

                     Submit = FALSE;
                  }
                  break;
               case ttEnableControlCCCD:
                  /* Determine if the device supports multiple sensor   */
                  /* locations.                                         */
                  if((DeviceEntry->SupportedFeatures & RSCM_FEATURE_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED) && !(DeviceEntry->Flags & DEVICE_ENTRY_FLAGS_SKIP_SUPPORTED_LOCATIONS))
                  {
                     /* Format the command to write to the control      */
                     /* point.                                          */
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&OpCode, RSCS_SC_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS);

                     NewTransactionEntry.TransactionType = ttWriteControlPoint;
                     Handle                              = DeviceEntry->ClientInformation.SC_Control_Point;
                     NewConfigurationState               = csGettingSupportedLocations;
                     BufferPtr                           = &OpCode;
                     BufferLength                        = NON_ALIGNED_BYTE_SIZE;
                  }
                  else
                  {
                     /* Nothing left to do.                             */
                     DeviceEntry->ConfigurationState = csConfigured;

                     DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_SUCCESS);

                     /* Save the new state information if necessary.    */
                     if(DeviceEntry->Flags & DEVICE_ENTRY_FLAGS_STORE_STATE)
                        StoreRSCMDeviceToFile(DeviceEntry);

                     Submit = FALSE;
                  }
                  break;
               case ttDisableControlCCCD:
                  /* Simply disable measurement notifications.          */
                  ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&CCDBuffer, 0);

                  NewTransactionEntry.TransactionType = ttDisableMeasurementCCCD;
                  Handle                              = DeviceEntry->ClientInformation.RSC_Measurement_Client_Configuration;
                  NewConfigurationState               = csDisablingMeasurements;
                  BufferPtr                           = (Byte_t *)&CCDBuffer;
                  BufferLength                        = NON_ALIGNED_WORD_SIZE;
                  break;
               case ttDisableMeasurementCCCD:
                  /* This is the last disable state.                    */

                  DeviceEntry->ConfigurationState = csNotConfigured;
                  DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_SUCCESS);

                  /* Save the new state information if necessary.       */
                  if(DeviceEntry->Flags & DEVICE_ENTRY_FLAGS_STORE_STATE)
                     StoreRSCMDeviceToFile(DeviceEntry);

                  Submit = FALSE;
                  break;
               default:
                  Submit = FALSE;
                  break;
            }

            if(Submit)
            {
               if((NewTransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &NewTransactionEntry)) != NULL)
               {
                  if(BufferPtr)
                     Result = GATM_WriteValue(GATMCallbackID, DeviceEntry->BluetoothAddress, Handle, BufferLength, BufferPtr);
                  else
                     Result = GATM_ReadValue(GATMCallbackID, DeviceEntry->BluetoothAddress, Handle, 0, TRUE);

                  if(Result > 0)
                  {
                     NewTransactionEntryPtr->GATMTransactionID = (unsigned int)Result;
                     DeviceEntry->ConfigurationState           = NewConfigurationState;
                  }
                  else
                  {
                     if((NewTransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, NewTransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(NewTransactionEntryPtr);
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

               if(Result < 0)
               {
                  DeviceEntry->ConfigurationState = csNotConfigured;
                  DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_GATT_OPERATION_FAILURE);
               }
            }
         }
         else
         {
            /* Operation Failed. Revert state and announce a critical   */
            /* error.                                                   */
            DeviceEntry->ConfigurationState = csNotConfigured;
            DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_GATT_OPERATION_FAILURE);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* This function is a utility to process a GATT Read Response for the*/
   /* Sensor Location characteristic.                                   */
static void ProcessReadLocationResponse(GATM_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry, Boolean_t Success)
{
   Byte_t          Location;
   unsigned int    Status;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters are valid.                               */
   if((EventData) && (TransactionEntry))
   {
      /* Check that we are connected to the device.                     */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &TransactionEntry->BluetoothAddress)) != NULL)
      {
         /* Check whether the operation was successful.                 */
         if(Success)
         {
            if(EventData->EventData.ReadResponseEventData.ValueLength == BYTE_SIZE)
            {
               Location = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(EventData->EventData.ReadResponseEventData.Value);

               if(RSCS_SENSOR_LOCATION_VALID(Location))
                  DispatchSensorLocationResponseEvent(TransactionEntry->ClientID, DeviceEntry, RSCM_GET_SENSOR_LOCATION_STATUS_SUCCESS, (RSCM_Sensor_Location_t)Location);
               else
                  DispatchSensorLocationResponseEvent(TransactionEntry->ClientID, DeviceEntry, RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_UNKNOWN, 0);
            }
            else
               DispatchSensorLocationResponseEvent(TransactionEntry->ClientID, DeviceEntry, RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_UNKNOWN, 0);
         }
         else
         {
            switch(EventData->EventData.ErrorResponseEventData.ErrorType)
            {
               case retProtocolTimeout:
                  Status = RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_TIMEOUT;
                  break;
               case retErrorResponse:
                  switch(EventData->EventData.ErrorResponseEventData.AttributeProtocolErrorCode)
                  {
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION:
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION:
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
                        Status = RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_SECURITY;
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES:
                        Status = RSCM_GET_SENSOR_LOCATION_STATUS_INSUFFICIENT_RESOURCES;
                        break;
                     default:
                        Status = RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_UNKNOWN;
                  }
                  break;
               default:
                  Status = RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_UNKNOWN;
                  break;
            }

            DispatchSensorLocationResponseEvent(TransactionEntry->ClientID, DeviceEntry, Status, 0);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* This function is a utility to process an GATT Error Response      */
   /* returned when attempted to write to the Control Point on a remote */
   /* sensor.                                                           */
static void ProcessControlPointErrorResponse(GATM_Error_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry)
{
   unsigned int    Status;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters are valid.                               */
   if((EventData) && (TransactionEntry))
   {
      /* Check that we are connected to the device.                     */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &TransactionEntry->BluetoothAddress)) != NULL)
      {
         /* Determine if this is a configuration event or if it was     */
         /* explicitly requested.                                       */
         if(DeviceEntry->ConfigurationState == csConfigured)
         {
            switch(EventData->ErrorType)
            {
               case retProtocolTimeout:
                  Status = RSCM_PROCEDURE_STATUS_FAILURE_TIMEOUT;
                  break;
               case retErrorResponse:
                  switch(EventData->AttributeProtocolErrorCode)
                  {
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION:
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION:
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
                        Status = RSCM_PROCEDURE_STATUS_FAILURE_SECURITY;
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES:
                        Status = RSCM_PROCEDURE_STATUS_INSUFFICIENT_RESOURCES;
                        break;
                     default:
                        Status = RSCM_PROCEDURE_STATUS_FAILURE_UNKNOWN;
                  }
                  break;
               default:
                  Status = RSCM_PROCEDURE_STATUS_FAILURE_UNKNOWN;
                  break;
            }

            /* Dispatch the event.                                      */
            /* * NOTE * This function will clear the current procedure  */
            /*          from the device entry.                          */
            DispatchProcedureCompleteEvent(DeviceEntry, Status, 0);
         }
         else
         {
            /* We are configuring. Clear the state and announce failure.*/
            DeviceEntry->ConfigurationState = csNotConfigured;
            DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_GATT_OPERATION_FAILURE);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* This function is a utility to process any GATT Handle Value       */
   /* Notifications that are recieved.                                  */
static void ProcessGATTNotification(GATM_Handle_Value_Data_Event_Data_t *EventData)
{
   Device_Entry_t              *DeviceEntry;
   RSCM_Measurement_Data_t      MeasurementData;
   RSCS_RSC_Measurement_Data_t  RSCSMeasurementData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters are valid.                               */
   if(EventData)
   {
      /* Check that we are connected to the device.                     */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &EventData->RemoteDeviceAddress)) != NULL)
      {
         if(RSCS_Decode_RSC_Measurement(EventData->AttributeValueLength, EventData->AttributeValue, &RSCSMeasurementData) == 0)
         {
            MeasurementData.Flags                       = RSCSMeasurementData.Flags;

            if(DeviceEntry->SupportedFeatures & RSCM_FEATURE_WALKING_OR_RUNNING_STATUS_SUPPORTED)
               MeasurementData.Flags |= RSCM_MEASUREMENT_WALKING_RUNNING_BIT_VALID;

            MeasurementData.Instantaneous_Speed         = RSCSMeasurementData.Instantaneous_Speed;
            MeasurementData.Instantaneous_Cadence       = RSCSMeasurementData.Instantaneous_Cadence;
            MeasurementData.Instantaneous_Stride_Length = RSCSMeasurementData.Instantaneous_Stride_Length;
            MeasurementData.Total_Distance              = RSCSMeasurementData.Total_Distance;

            DispatchMeasurementEvent(DeviceEntry, &MeasurementData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* This function is a utility to process any GATT Handle Value       */
   /* Indications that are received.                                    */
static void ProcessGATTIndication(GATM_Handle_Value_Data_Event_Data_t *EventData)
{
   int Result;
   Device_Entry_t *DeviceEntry;
   RSCS_SC_Control_Point_Response_Data_t ResponseData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &EventData->RemoteDeviceAddress)) != NULL)
   {
      if(EventData->AttributeHandle == DeviceEntry->ClientInformation.SC_Control_Point)
      {
         /* Now try to decode the result.                               */
         if((Result = RSCS_Decode_SC_Control_Point_Response(EventData->AttributeValueLength, EventData->AttributeValue, &ResponseData)) == 0)
         {
            /* Check if this is a user-requested procedure or part of   */
            /* the configuration process.                               */
            if(DeviceEntry->ConfigurationState == csConfigured)
            {
               /* Check if the response is successful.                  */
               if(ResponseData.ResponseOpCode == RSCS_SC_CONTROL_POINT_RESPONSE_CODE_SUCCESS)
               {
                  /* First dispatch the correct status event for the    */
                  /* procedure type.                                    */
                  switch(DeviceEntry->CurrentProcedure)
                  {
                     case cpUpdateCumulative:
                        if(ResponseData.RequestOpCode == RSCS_SC_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE)
                        {
                           DispatchCumulativeValueUpdatedEvent(DeviceEntry);
                           Result = RSCM_PROCEDURE_STATUS_SUCCESS;
                        }
                        else
                           Result = RSCM_PROCEDURE_STATUS_FAILURE_INVALID_RESPONSE;
                        break;
                     case cpUpdateLocation:
                        if(ResponseData.RequestOpCode == RSCS_SC_CONTROL_POINT_OPCODE_UPDATE_SENSOR_LOCATION)
                        {
                           DispatchSensorLocationUpdatedEvent(DeviceEntry);
                           Result = RSCM_PROCEDURE_STATUS_SUCCESS;
                        }
                        else
                           Result = RSCM_PROCEDURE_STATUS_FAILURE_INVALID_RESPONSE;
                        break;
                     case cpStartSensorCalibration:
                        if(ResponseData.RequestOpCode == RSCS_SC_CONTROL_POINT_OPCODE_START_SENSOR_CALIBRATION)
                           Result = RSCM_PROCEDURE_STATUS_SUCCESS;
                        else
                           Result = RSCM_PROCEDURE_STATUS_FAILURE_INVALID_RESPONSE;
                        break;
                     default:
                        DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected procedure completed\n"));
                        Result = RSCM_PROCEDURE_STATUS_FAILURE_INVALID_RESPONSE;
                        break;
                  }

                  /* Now notify that the procedure is complete.         */
                  DispatchProcedureCompleteEvent(DeviceEntry, Result, 0);
               }
               else
               {
                  /* Dispatch the procedure error.                      */
                  DispatchProcedureCompleteEvent(DeviceEntry, RSCM_PROCEDURE_STATUS_FAILURE_ERROR_RESPONSE, ResponseData.ResponseOpCode);
               }
            }
            else
            {
               if((DeviceEntry->ConfigurationState == csGettingSupportedLocations) && (ResponseData.RequestOpCode == RSCS_SC_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS))
               {
                  if(ResponseData.ResponseOpCode == RSCS_SC_CONTROL_POINT_RESPONSE_CODE_SUCCESS)
                  {
                     DeviceEntry->SupportedSensorLocations = ConvertLocationListToMask(&ResponseData.SupportedSensorLocations);
                     DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Supported Locations Known: %08X\n", DeviceEntry->SupportedSensorLocations));
                     DeviceEntry->ConfigurationState = csConfigured;
                     DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_SUCCESS);

                     /* Save the new state information if necessary.    */
                     if(DeviceEntry->Flags & DEVICE_ENTRY_FLAGS_STORE_STATE)
                        StoreRSCMDeviceToFile(DeviceEntry);

                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Supported Locations Procedure Failed: %d\n", ResponseData.ResponseOpCode));
                     DeviceEntry->ConfigurationState = csNotConfigured;
                     DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_UNKNOWN_ERROR);
                  }
               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected configuration state: %d\n", DeviceEntry->ConfigurationState));
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to parse control point response data: %d\n", Result));

            if(DeviceEntry->ConfigurationState == csConfigured)
            {
               /* Just dispatch the invalid response failure.           */
               DispatchProcedureCompleteEvent(DeviceEntry, RSCM_PROCEDURE_STATUS_FAILURE_INVALID_RESPONSE, 0);
            }
            else
            {
               DeviceEntry->ConfigurationState = csNotConfigured;
               DispatchConfigurationStatusChangedEvent(DeviceEntry, RSCM_CONFIGURATION_STATUS_UNKNOWN_ERROR);
            }
         }
      }
      else
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Indication for unexpected handle: %04X\n", EventData->AttributeHandle));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the callback provided to the timer for  */
   /* tracking a timeout for a control point procedure.                 */
static Boolean_t BTPSAPI ProcedureTimerCallback(unsigned int TimerID, void *CallbackParameter)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HFRM)\n"));

   /* Simply queue a Timer Callback Event to process.                   */
   if(BTPM_QueueMailboxCallback(BTPMDispatchCallback_TMR, CallbackParameter))
      ret_val = FALSE;
   else
      ret_val = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HFRM): %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is the callback provided to GATM for       */
   /* processing GATM events.                                           */
static void BTPSAPI GATMEventCallback(GATM_Event_Data_t *EventData, void *CallbackParameter)
{
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EventData)
   {
      /* Acquire the lock the protects the module's state.              */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case getGATTWriteResponse:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("GATM Write Response\n"));

               /* Remove the completed transaction.                     */
               if((TransactionEntry = DeleteTransactionEntryByGATMTransactionID(&TransactionEntryList, EventData->EventData.WriteResponseEventData.TransactionID)) != NULL)
               {
                  /* Determine which transaction was written.           */
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttEnableMeasurementCCCD:
                     case ttEnableControlCCCD:
                     case ttDisableControlCCCD:
                     case ttDisableMeasurementCCCD:
                        /* These transaction are all part of the        */
                        /* configuration process.                       */
                        ProcessConfigurationResponse(EventData, TransactionEntry, TRUE);
                        break;
                     case ttWriteControlPoint:
                        /* * NOTE * A succesful write response does not */
                        /*          change our state machine at all. We */
                        /*          are still waiting for the response  */
                        /*          indication.                         */
                        break;
                     default:
                        DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Write Transaction Type\n"));
                        break;
                  }

                  FreeTransactionEntryMemory(TransactionEntry);
               }
               break;
            case getGATTReadResponse:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("GATM Read Response\n"));

               /* Remove the completed transaction.                     */
               if((TransactionEntry = DeleteTransactionEntryByGATMTransactionID(&TransactionEntryList, EventData->EventData.ReadResponseEventData.TransactionID)) != NULL)
               {
                  /* Determine which transaction has finished.          */
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttReadFeatures:
                        /* This is done as part of configuration.       */
                        ProcessConfigurationResponse(EventData, TransactionEntry, TRUE);
                        break;
                     case ttReadLocation:
                        ProcessReadLocationResponse(EventData, TransactionEntry, TRUE);
                        break;
                     default:
                        DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Read Transaction Type\n"));
                  }

                  FreeTransactionEntryMemory(TransactionEntry);
               }
               break;
            case getGATTHandleValueData:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("GATM Handle Value Data\n"));

               if(EventData->EventData.HandleValueDataEventData.HandleValueIndication)
                  ProcessGATTIndication(&EventData->EventData.HandleValueDataEventData);
               else
                  ProcessGATTNotification(&EventData->EventData.HandleValueDataEventData);

               break;
            case getGATTErrorResponse:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("GATM Error Response: %d, %d\n", EventData->EventData.ErrorResponseEventData.ErrorType, EventData->EventData.ErrorResponseEventData.AttributeProtocolErrorCode));

               /* Remove the completed transaction.                     */
               if((TransactionEntry = DeleteTransactionEntryByGATMTransactionID(&TransactionEntryList, EventData->EventData.ErrorResponseEventData.TransactionID)) != NULL)
               {
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttEnableMeasurementCCCD:
                     case ttEnableControlCCCD:
                     case ttReadFeatures:
                     case ttDisableControlCCCD:
                     case ttDisableMeasurementCCCD:
                        ProcessConfigurationResponse(EventData, TransactionEntry, FALSE);
                        break;
                     case ttWriteControlPoint:
                        ProcessControlPointErrorResponse(&EventData->EventData.ErrorResponseEventData, TransactionEntry);
                        break;
                     case ttReadLocation:
                        ProcessReadLocationResponse(EventData, TransactionEntry, FALSE);
                        break;
                     default:
                        DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Transaction Type\n"));
                  }
                  FreeTransactionEntryMemory(TransactionEntry);
               }
               break;
            default:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled GATM Event: %u\n", EventData->EventType));
               break;
         }

         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process RSC Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_RSCM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Process the Message.                                     */
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Note we do not have to release the Lock because          */
            /* ProcessReceivedMessage() is documented that it will be   */
            /* called with the Lock being held and it will release the  */
            /* Lock when it is finished with it.                        */
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Timer Expirations.                          */
static void BTPSAPI BTPMDispatchCallback_TMR(void *CallbackParameter)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackParameter)
   {
      if(Initialized)
      {
         if(DEVM_AcquireLock())
         {
            /* If we still know about the device, dispatch the procedure*/
            /* timeout.                                                 */
            if((DeviceEntry = SearchDeviceEntryByProcedureID(&DeviceEntryList, (unsigned int)CallbackParameter)) != NULL)
               DispatchProcedureCompleteEvent(DeviceEntry, RSCM_PROCEDURE_STATUS_FAILURE_TIMEOUT, 0);

            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Process the Client Un-Register Message.                  */
            ProcessClientUnRegister((unsigned int)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all RSC Manager Messages.   */
static void BTPSAPI RSCManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("RSC Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a RSC Manager defined    */
            /* Message.  If it is it will be within the range:          */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Still processing, go ahead and post the message to the*/
               /* RSC Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_RSCM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue RSC Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

                  MSG_FreeReceivedMessageGroupHandlerMessage(Message);
               }
            }
            else
            {
               /* Dispatch to the main handler that a client has        */
               /* un-registered.                                        */
               if(Message->MessageHeader.MessageFunction == BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION)
               {
                  if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE) && (!(((BTPM_Client_Registration_Message_t *)Message)->Registered)))
                  {
                     if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_MSG, (void *)(((BTPM_Client_Registration_Message_t *)Message)->AddressID)))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue RSC Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an RSC Manager defined   */
            /* Message.  If it is it will be within the range:          */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
               MSG_FreeReceivedMessageGroupHandlerMessage(Message);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Non RSC Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager RSC Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI RSCM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing RSC Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process RSC Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER, RSCManagerGroupHandler, NULL))
         {
            if((Result = GATM_RegisterEventCallback(GATMEventCallback, NULL)) > 0)
            {
               GATMCallbackID = (unsigned int)Result;
               Result         = 0;

               Initialized    = TRUE;

               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("RSC Initialized: %08X\n", BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER));

            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("RSC Initialization Failed: %d\n", Result));
            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("RSC Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Un-register from GATM.                                   */
            GATM_UnRegisterEventCallback(GATMCallbackID);

            GATMCallbackID = 0;

            /* Free all Entry Lists.                                    */
            FreeDeviceEntryList(&DeviceEntryList);
            FreeCallbackEntryList(&CallbackEntryList);
            FreeTransactionEntryList(&TransactionEntryList);

            /* Flag that this module is no longer initialized.          */
            Initialized = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI RSCM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the RSC Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Free the Device and Transaction Entry list.           */
               FreeDeviceEntryList(&DeviceEntryList);
               FreeTransactionEntryList(&TransactionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDevicePropertiesChanged\n"));

               if(EventData->EventLength >= DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_EVENT_DATA_SIZE)
               {
                  /* Process the Remote Device Properties Changed Event.*/
                  ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               }
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDeviceDeleted\n"));

               if(EventData->EventLength >= DEVM_REMOTE_DEVICE_DELETED_EVENT_DATA_SIZE)
               {
                  /* Process a Remote Device Deleted Event.             */
                  ProcessRemoteDeviceDeletedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR);
               }
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Collector callback function with the RSC    */
   /* Manager Service.  This Callback will be dispatched by the RSC     */
   /* Manager when various RSC Manager Collector Events occur.  This    */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a RSC Manager Collector Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          RSCM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI RSCM_Register_Collector_Event_Callback(RSCM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Make sure that the parameters seem valid.                      */
      if(CallbackFunction)
      {
         /* Acquire the lock the guards access to the module's state.   */
         if(DEVM_AcquireLock())
         {
            /* Simply submit the call to the processing function.       */
            ret_val = ProcessRegisterCollectorEvents(MSG_GetServerAddressID(), CallbackFunction, CallbackParameter);

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered RSC Manager Collector         */
   /* Event Callback (registered via a successful call to the           */
   /* RSCM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the Collector Event Callback ID (return value    */
   /* from RSCM_Register_Collector_Event_Callback() function).          */
void BTPSAPI RSCM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         ProcessUnRegisterCollectorEvents(MSG_GetServerAddressID(), CollectorCallbackID);
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of entries that the buffer will      */
   /* support (i.e. can be copied into the buffer).  The next parameter */
   /* is optional and, if specified, will be populated with the total   */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI RSCM_Query_Connected_Sensors(unsigned int MaximumRemoteDeviceListEntries, RSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessQueryConnectedSensors(MSG_GetServerAddressID(), MaximumRemoteDeviceListEntries, ConnectedDeviceList, TotalNumberConnectedDevices);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function will attempt to configure a remote device  */
   /* which supports the RSC sensor role. It will register measurement  */
   /* and control point notifications and set the device up for         */
   /* usage. The RemoteDeviceAddress is the address of the remote       */
   /* sensor. This function returns zero if successful and a negative   */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return from this call does not mean the     */
   /*          device has been configured. An                           */
   /*          etRSCConfigurationStatusChanged event will indicate the  */
   /*          status of the attempt to configure.                      */
int BTPSAPI RSCM_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessConfigureRemoteSensor(MSG_GetServerAddressID(), RemoteDeviceAddress, Flags);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function will un-configure a remote sensor which was*/
   /* previously configured. All notifications will be disabled. The    */
   /* RemoteDeviceAddress is the address of the remote sensor. This     */
   /* function returns zero if success and a negative return code if    */
   /* there was an error.                                               */
int BTPSAPI RSCM_Un_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessUnConfigureRemoteSensor(MSG_GetServerAddressID(), RemoteDeviceAddress);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function queries information about a known remote   */
   /* sensor. The RemoteDeviceAddress parameter is the address of       */
   /* the remote sensor. The DeviceInfo parameter is a pointer to       */
   /* the Connected Sensor structure in which the data should be        */
   /* populated. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
int BTPSAPI RSCM_Get_Connected_Sensor_Info(BD_ADDR_t RemoteDeviceAddress, RSCM_Connected_Sensor_t *DeviceInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessGetConnectedSensorInfo(MSG_GetServerAddressID(), RemoteDeviceAddress, DeviceInfo);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function queries information about a known remote   */
   /* sensor. The RemoteDeviceAddress parameter is the address of       */
   /* the remote sensor. The DeviceInfo parameter is a pointer to       */
   /* the Connected Sensor structure in which the data should be        */
   /* populated. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
int BTPSAPI RSCM_Get_Sensor_Location(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessGetSensorLocation(MSG_GetServerAddressID(), RemoteDeviceAddress);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function sends a control point opcode to update     */
   /* the cumulative value on a remote sensor. The RemoteDeviceAddress  */
   /* parameter is the address of the remote sensor. The CumulativeValue*/
   /* parameter is the value to set on the remote sensor. If successful,*/
   /* this function returns a postive integer which represents the      */
   /* Transaction ID associated with this procedure. If there is an     */
   /* error, this function returns a negative error code.               */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetRSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetRSCCumulativeValueUpdated event will notify all       */
   /*          registered callbacks that the value has changed.         */
int BTPSAPI RSCM_Update_Cumulative_Value(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessUpdateCumulativeValue(MSG_GetServerAddressID(), RemoteDeviceAddress, CumulativeValue);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function sends a control point opcode to update     */
   /* the sensor location on a remote sensor. The RemoteDeviceAddress   */
   /* parameter is the address of the remote sensor. The SensorLocation */
   /* parameter is the new location to set.  If successful, this        */
   /* function returns a postive integer which represents the           */
   /* Transaction ID associated with this procedure. If there is an     */
   /* error, this function returns a negative error code.               */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetRSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetRSCSensorLocationUpdated event will notify all        */
   /*          registered callbacks that the location has changed.      */
int BTPSAPI RSCM_Update_Sensor_Location(BD_ADDR_t RemoteDeviceAddress, RSCM_Sensor_Location_t SensorLocation)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessUpdateSensorLocation(MSG_GetServerAddressID(), RemoteDeviceAddress, SensorLocation);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function sends a control point opcode to start      */
   /* sensor calibration on a remote sensor. The RemoteDeviceAddress    */
   /* parameter is the address of the remote sensor.  If successful,    */
   /* this function returns a postive integer which represents the      */
   /* Procedure ID associated with this procedure. If there is an error,*/
   /* this function returns a negative error code.                      */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          retRSCProcedureComplete then attempt to resend.          */
int BTPSAPI RSCM_Start_Sensor_Calibration(BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the module has been initialized.                            */
   if(Initialized)
   {
      /* Acquire the lock the guards access to the module's state.      */
      if(DEVM_AcquireLock())
      {
         /* Simply submit the call to the processing function.          */
         ret_val = ProcessStartSensorCalibration(MSG_GetServerAddressID(), RemoteDeviceAddress);

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_RSC_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_RUNNING_SPEED | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}
