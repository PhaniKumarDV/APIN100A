/*****< btpmblpm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMBLPM - BLP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMBLPM.h"            /* BTPM BLP Manager Prototypes/Constants.    */
#include "BLPMAPI.h"             /* BLP Manager Prototypes/Constants.         */
#include "BLPMMSG.h"             /* BTPM BLP Manager Message Formats.         */
#include "BLPMGR.h"              /* BLP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following defines the BLPM LE Configuration File Section Name.*/
#define BLPM_LE_CONFIGURATION_FILE_SECTION_NAME                   "BLPM-Collector"

   /* The following defines the Maximum Key Size that is used in the    */
   /* BLPM LE Configuration File.                                       */
#define BLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH             (4+(BD_ADDR_SIZE*2))

   /* The following defines the BLPM LE Configuration File Maximum Line */
   /* Length.                                                           */
#define BLPM_LE_CONFIGURATION_FILE_MAXIMUM_LINE_LENGTH            ((BTPM_CONFIGURATION_SETTINGS_MAXIMUM_FILE_LINE_LENGTH/2)-BLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH)

   /* The following define the Key Names that are used with the BLPM LE */
   /* Configuration File.                                               */
#define BLPM_LE_KEY_NAME_BLOOD_PRESSURE_CCCD                      "BC-%02X%02X%02X%02X%02X%02X"
#define BLPM_LE_KEY_NAME_INTERMEDIATE_CUFF_PRESSURE_CCCD          "IC-%02X%02X%02X%02X%02X%02X"

   /* Structure which is used to track what Blood Pressure instances    */
   /* about which a client will be notified.                            */
typedef struct _tagEnabled_Entry_t
{
   BD_ADDR_t                   BluetoothAddress;
   struct _tagEnabled_Entry_t *NextEnabledEntry;
} Enabled_Entry_t;

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagCallback_Entry_t
{
   unsigned int                 CallbackID;
   unsigned int                 AddressID;
   BLPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   Enabled_Entry_t             *BPMIndicateEntryList;
   Enabled_Entry_t             *ICPNotifyEntryList;
   struct _tagCallback_Entry_t *NextCallbackEntry;
} Callback_Entry_t;

   /* Structure which is used to track information pertaining to known  */
   /* connected LE devices supporting at least one Blood Pressure       */
   /* Service.                                                          */
typedef struct _tagDevice_Entry_t
{
   BD_ADDR_t                  BluetoothAddress;
   unsigned int               Flags;
   unsigned int               BPMIndicateCount;
   unsigned int               ICPNotifyCount;
   Boolean_t                  BPMIndicationsEnabled;
   Boolean_t                  ICPNotificationsEnabled;
   BLS_Client_Information_t   ServerInformationHandles;
   struct _tagDevice_Entry_t *NextDeviceEntry;
} Device_Entry_t;

   /* The following enumeration is used to identify the type of         */
   /* transaction being processed.                                      */
typedef enum _tagTransaction_Type_t
{
   ttBloodPressureFeature = 0,
   ttBloodPressureMeasurementCCCDEnable,
   ttBloodPressureMeasurementCCCDDisable,
   ttIntermediateCuffPressureCCCDEnable,
   ttIntermediateCuffPressureCCCDDisable
} Transaction_Type_t;

   /* The following structure is used to store transaction and response */
   /* information for the duration of the outstanding transaction.      */
typedef struct _tagTransaction_Entry_t
{
   Transaction_Type_t              TransactionType;
   unsigned int                    TransactionID;
   unsigned int                    GATTTransactionID;
   BD_ADDR_t                       BluetoothAddress;
   unsigned int                    CallbackID;
   Word_t                          AttributeHandle;
   struct _tagTransaction_Entry_t *NextTransactionEntry;
} Transaction_Entry_t;

   /* The following structure is used to store Intermediate Cuff        */
   /* Pressure Notification data and Blood Pressure Measurement         */
   /* Indication data internal to this module.                          */
typedef struct _tagPressure_Event_t
{
   BD_ADDR_t                             RemoteSensor;
   BLS_Blood_Pressure_Measurement_Data_t BloodPressureMeasurement;
} Pressure_Event_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextTransactionID;

   /* Variable which holds a pointer to the first element of the        */
   /* Callback List (which holds registered event callbacks).           */
static Callback_Entry_t *CallbackEntryList;

   /* Variable which holds a pointer to the first element of the Device */
   /* Information List (which holds connected BLP LE devices).          */
static Device_Entry_t *DeviceEntryList;

   /* Variable which holds a pointer to the first transaction in the    */
   /* outstanding transaction list.                                     */
static Transaction_Entry_t *TransactionEntryList;

   /* List used to decode the Transaction Type into a Blood Pressure    */
   /* Instance Handle Offset.                                           */
static BTPSCONST unsigned int TransactionTypeToHandleOffsetTable[] =
{
   BTPS_STRUCTURE_OFFSET(Device_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BLS_Client_Information_t, Blood_Pressure_Feature),
   BTPS_STRUCTURE_OFFSET(Device_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BLS_Client_Information_t, Blood_Pressure_Measurement_Client_Configuration),
   BTPS_STRUCTURE_OFFSET(Device_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BLS_Client_Information_t, Blood_Pressure_Measurement_Client_Configuration),
   BTPS_STRUCTURE_OFFSET(Device_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BLS_Client_Information_t, Intermediate_Cuff_Pressure_Client_Configuration),
   BTPS_STRUCTURE_OFFSET(Device_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BLS_Client_Information_t, Intermediate_Cuff_Pressure_Client_Configuration)
} ;

   /* Internal Function Prototypes.                                     */
static Enabled_Entry_t *AddEnabledEntry(Enabled_Entry_t **EntryList, Enabled_Entry_t *EntryToAdd);
static Enabled_Entry_t *SearchEnabledEntry(Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress);
static Enabled_Entry_t *DeleteEnabledEntry(Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress);
static void FreeEnabledEntryMemory(Enabled_Entry_t *EntryToFree);
static void FreeEnabledEntryList(Enabled_Entry_t **EntryList);

static unsigned int GetNextCallbackID(void);
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t *EntryToAdd);
static Callback_Entry_t *SearchCallbackEntry(unsigned int CallbackID);
static Callback_Entry_t *DeleteCallbackEntry(unsigned int CallbackID);
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);
static void FreeCallbackEntryList(void);

static Boolean_t AddDeviceEntryActual(Device_Entry_t *EntryToAdd);
static Device_Entry_t *SearchDeviceEntry(BD_ADDR_t *Address);
static Device_Entry_t *DeleteDeviceEntry(BD_ADDR_t *Address);
static void FreeDeviceEntryMemory(Device_Entry_t *EntryToFree);
static void FreeDeviceEntryList(void);

static unsigned int GetNextTransactionID(void);
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t *EntryToAdd);
static Transaction_Entry_t *SearchTransactionEntry(BD_ADDR_t *BluetoothAddress, Transaction_Type_t TransactionType, unsigned int CallbackID);
static Transaction_Entry_t *DeleteTransactionEntry(unsigned int TransactionID);
static Transaction_Entry_t *DeleteTransactionEntryByGATTTransactionID(unsigned int GATTTransactionID);
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);
static void FreeTransactionEntryList(void);

static void FormatResponseMessageHeader(BLPM_Response_Message_t *ResponseMessage, BTPM_Message_Header_t *MessageHeader);

static void ProcessRegisterCollectorEventsRequestMessage(BLPM_Register_Collector_Events_Request_t *Message);
static void ProcessUnRegisterCollectorEventsRequestMessage(BLPM_Un_Register_Collector_Events_Request_t *Message);
static void ProcessEnableBPMIndicateRequestMessage(BLPM_Device_Request_Message_t *Message);
static void ProcessDisableBPMIndicateRequestMessage(BLPM_Device_Request_Message_t *Message);
static void ProcessEnableICPNotifyRequestMessage(BLPM_Device_Request_Message_t *Message);
static void ProcessDisableICPNotifyRequestMessage(BLPM_Device_Request_Message_t *Message);
static void ProcessGetBloodPressureFeatureRequestMessage(BLPM_Device_Request_Message_t *Message);
static void ProcessCancelTransactionRequestMessage(BLPM_Cancel_Transaction_Request_t *Message);
static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessCollectorUnRegister(unsigned int AddressID);

static void DispatchLocalEvent(BLPM_Event_Data_t *BLPMEventData);
static void DispatchLocalBloodPressureFeatureEvent(Transaction_Entry_t *TransactionEntry, int Status, Word_t BloodPressureFeature);
static void DispatchRemoteBloodPressureFeatureEvent(Transaction_Entry_t *TransactionEntry, int Status, Word_t BloodPressureFeature, unsigned int AddressID);
static void DispatchBloodPressureFeatureEvent(Transaction_Entry_t *TransactionEntry, int Status, Word_t BloodPressureFeature);
static void DispatchBPMCCCDEvent(Transaction_Entry_t *TransactionEntry, int Response);
static void DispatchICPCCCDEvent(Transaction_Entry_t *TransactionEntry, int Response);
static void DispatchBPMIndicateEvent(Pressure_Event_t *Event);
static void DispatchICPNotifyEvent(Pressure_Event_t *Event);
static void DispatchConnectedEvent(Device_Entry_t *DeviceEntry);
static void DispatchDisconnectedEvent(Device_Entry_t *DeviceEntry);

static void DeleteEnabledEntriesForDevice(BD_ADDR_t BD_ADDR);
static void DeleteTransactionEntriesForDevice(BD_ADDR_t BD_ADDR);

static void StoreBLPMKey(char *KeyName, unsigned int Value);
static void StoreBLPMInformation(BD_ADDR_t RemoteDeviceAddress, Boolean_t BPMIndicationsEnabled, Boolean_t ICPNotificationsEnabled);
static void UpdateDeviceFile(Device_Entry_t *DeviceEntry);
static Boolean_t ReloadDeviceEntry(BD_ADDR_t BD_ADDR, Boolean_t *BPMIndicationsEnabled, Boolean_t *ICPNotificationsEnabled);

static Device_Entry_t *DiscoverBloodPressureSensor(BD_ADDR_t *BluetoothAddress);

static int FormatAddTransactionEntry(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, Transaction_Type_t TransactionType, Transaction_Entry_t **ReturnTransactionEntry);
static int SubmitReadValue(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, Transaction_Type_t TransactionType);
static int SubmitWriteValue(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, Transaction_Type_t TransactionType, unsigned int DataLength, Byte_t *Data);
static int EnableBPMIndicate(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);
static int DisableBPMIndicate(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);
static int EnableICPNotify(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);
static int DisableICPNotify(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);
static int GetBloodPressureFeature(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);
static int CancelTransaction(unsigned int TransactionID);
static int WriteBPMCCCD(BD_ADDR_t *RemoteSensor, Boolean_t EnableIndications);
static int WriteICPCCCD(BD_ADDR_t *RemoteSensor, Boolean_t EnableNotifications);

static void ProcessBPMIndicationStateChange(Transaction_Entry_t *TransactionEntry, Boolean_t Enable, Boolean_t Success);
static void ProcessICPNotificationStateChange(Transaction_Entry_t *TransactionEntry, Boolean_t Enable, Boolean_t Success);

static void ProcessBLPMConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessBLPMDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessBLPMDevicePaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessBLPMDeviceUnPaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRemoteDeviceDeletedEvent(BD_ADDR_t RemoteDeviceAddress);

static void BTPSAPI BTPMDispatchCallback_BLPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);
static void BTPSAPI BTPSDispatchCallback_GATT(void *CallbackParameter);
static void BTPSAPI BTPSDispatchCallback_BLP_Collector(void *CallbackParameter);

static void BTPSAPI BLPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function adds the specified Notifications Enabled   */
   /* Entry to the module's list.  This function will allocate and add a*/
   /* copy of the entry to the list.  This function will return NULL if */
   /* NO Entry was added.  This can occur if the element passed in was  */
   /* deemed invalid.                                                   */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the Bluetooth Address and Instance ID pair is the same */
   /*            as an entry already in the list.  When this occurs,    */
   /*            this function returns NULL.                            */
static Enabled_Entry_t *AddEnabledEntry(Enabled_Entry_t **EntryList, Enabled_Entry_t *EntryToAdd)
{
   return((Enabled_Entry_t *)BSC_AddGenericListEntry(sizeof(Enabled_Entry_t), ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(Enabled_Entry_t, BluetoothAddress), sizeof(Enabled_Entry_t), BTPS_STRUCTURE_OFFSET(Enabled_Entry_t, NextEnabledEntry), (void **)EntryList, ((void *)EntryToAdd)));
}

   /* The following function searches the module's Notifications Enabled*/
   /* Entry List for a Notifications Enabled Entry based on the         */
   /* specified Bluetooth Address and Instance ID pair.  This function  */
   /* returns NULL if either the Bluetooth Address is invalid, or the   */
   /* specified Entry was NOT present in the list.                      */
static Enabled_Entry_t *SearchEnabledEntry(Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress)
{
   return((Enabled_Entry_t *)BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)BluetoothAddress, BTPS_STRUCTURE_OFFSET(Enabled_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Enabled_Entry_t, NextEnabledEntry), (void **)EntryList));
}

   /* The following function searches the module's Notifications Enabled*/
   /* Entry List for the Notifications Enabled Entry with the specified */
   /* Bluetooth Address and Instance ID pair and removes it from the    */
   /* List.  This function returns NULL if either the Bluetooth Address */
   /* is invalid, or the specified Entry was NOT present in the list.   */
   /* The entry returned will have the Next Entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeCallbackEntryMemory().                  */
static Enabled_Entry_t *DeleteEnabledEntry(Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress)
{
   return(BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)BluetoothAddress, BTPS_STRUCTURE_OFFSET(Enabled_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Enabled_Entry_t, NextEnabledEntry), (void **)EntryList));
}

   /* This function frees the specified Notifications Enabled Entry     */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeEnabledEntryMemory(Enabled_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Notifications Enabled Entry List.  Upon   */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeEnabledEntryList(Enabled_Entry_t **EntryList)
{
   BSC_FreeGenericListEntryList((void **)EntryList, BTPS_STRUCTURE_OFFSET(Enabled_Entry_t, NextEnabledEntry));
}

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
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t *EntryToAdd)
{
   return((Callback_Entry_t *)BSC_AddGenericListEntry(sizeof(Callback_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), sizeof(Callback_Entry_t), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntry), (void **)(&CallbackEntryList), ((void *)EntryToAdd)));
}

   /* The following function searches the module's Callback Entry List  */
   /* for a Callback Entry based on the specified Callback ID.  This    */
   /* function returns NULL if either the Callback ID is invalid, or the*/
   /* specified Entry was NOT present in the list.                      */
static Callback_Entry_t *SearchCallbackEntry(unsigned int CallbackID)
{
   return((Callback_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntry), (void **)&CallbackEntryList));
}

   /* The following function searches the module's Callback Entry List  */
   /* for the Callback Entry with the specified Callback ID and removes */
   /* it from the List.  This function returns NULL if either the       */
   /* Callback ID is invalid, or the specified Entry was NOT present in */
   /* the list.  The entry returned will have the Next Entry field set  */
   /* to NULL, and the caller is responsible for deleting the memory    */
   /* associated with this entry by calling FreeCallbackEntryMemory().  */
static Callback_Entry_t *DeleteCallbackEntry(unsigned int CallbackID)
{
   return((Callback_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntry), (void **)&CallbackEntryList));
}

   /* This function frees the specified Callback Entry member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree)
{
   /* Free the Enabled Entry Lists before freeing the callback entry.   */
   FreeEnabledEntryList(&(EntryToFree->BPMIndicateEntryList));
   FreeEnabledEntryList(&(EntryToFree->ICPNotifyEntryList));

   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Callback Entry List.  Upon return of this */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeCallbackEntryList()
{
   Callback_Entry_t *CallbackEntry;
   Callback_Entry_t *tmpCallbackEntry;

   /* Loop through the list and free each callback entry.  This will    */
   /* also free any notifications enabled entries.                      */
   for(CallbackEntry = CallbackEntryList; CallbackEntry;)
   {
      tmpCallbackEntry = CallbackEntry;
      CallbackEntry    = CallbackEntry->NextCallbackEntry;

      FreeCallbackEntryMemory(tmpCallbackEntry);
   }

   CallbackEntryList = NULL;
}

   /* The following function adds the specified Device Entry to the     */
   /* module's list.  This function simply adds the entry to the list,  */
   /* it does not allocate a new buffer to store the entry.  This       */
   /* function will return NULL if NO Entry was added.  This can occur  */
   /* if the element passed in was deemed invalid.                      */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Boolean_t AddDeviceEntryActual(Device_Entry_t *EntryToAdd)
{
   return(BSC_AddGenericListEntry_Actual(ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntry), (void **)&DeviceEntryList, (void *)EntryToAdd));
}

   /* The following function searches the module's Device Entry List for*/
   /* a Device Entry based on the specified Bluetooth Address.  This    */
   /* function returns NULL if either the Bluetooth Device Address is   */
   /* invalid, or the specified Entry was NOT present in the list.      */
static Device_Entry_t *SearchDeviceEntry(BD_ADDR_t *Address)
{
   return((Device_Entry_t *)BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)Address, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntry), (void **)&DeviceEntryList));
}

   /* The following function searches the module's Device Entry List for*/
   /* the Device Entry with the specified Bluetooth Address and removes */
   /* it from the List.  This function returns NULL if either the       */
   /* Bluetooth Device Address is invalid, or the specified Entry was   */
   /* NOT present in the list.  The entry returned will have the Next   */
   /* Entry field set to NULL, and the caller is responsible for        */
   /* deleting the memory associated with this entry by calling         */
   /* FreeDeviceEntryMemory().                                          */
static Device_Entry_t *DeleteDeviceEntry(BD_ADDR_t *Address)
{
   return((Device_Entry_t *)BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)Address, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntry), (void **)&DeviceEntryList));
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
static void FreeDeviceEntryList()
{
   Device_Entry_t *DeviceEntry;
   Device_Entry_t *tmpDeviceEntry;

   /* Loop through the list and free each Device entry.  This will also */
   /* free any Instance entries.                                        */
   for(DeviceEntry = DeviceEntryList; DeviceEntry;)
   {
      tmpDeviceEntry = DeviceEntry;
      DeviceEntry    = DeviceEntry->NextDeviceEntry;

      FreeDeviceEntryMemory(tmpDeviceEntry);
   }

   DeviceEntryList = NULL;
}


   /* The following function is a utility function that exists to       */
   /* retrieve a unique Transaction ID that can be used to add an entry */
   /* into the Transaction List.                                        */
static unsigned int GetNextTransactionID(void)
{
   ++NextTransactionID;

   if(NextTransactionID & 0x80000000)
      NextTransactionID = 1;

   return(NextTransactionID);
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
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t *EntryToAdd)
{
   return((Transaction_Entry_t *)BSC_AddGenericListEntry(sizeof(Transaction_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), sizeof(Transaction_Entry_t), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntry), (void **)(&TransactionEntryList), ((void *)EntryToAdd)));
}

   /* The following function searches the module's Transaction Entry    */
   /* List for a Transaction Entry based on the specified Bluetooth     */
   /* Address, Transaction Type, and Callback ID pair.  This function   */
   /* returns NULL if either the Bluetooth Device Address is invalid, or*/
   /* the specified Entry was NOT present in the list.                  */
static Transaction_Entry_t *SearchTransactionEntry(BD_ADDR_t *BluetoothAddress, Transaction_Type_t TransactionType, unsigned int CallbackID)
{
   Transaction_Entry_t *ret_val;

   if(BluetoothAddress)
   {
      for(ret_val = TransactionEntryList; ret_val; ret_val = ret_val->NextTransactionEntry)
      {
         if(COMPARE_BD_ADDR(ret_val->BluetoothAddress, *BluetoothAddress) && (ret_val->TransactionType == TransactionType) && (ret_val->CallbackID == CallbackID))
            break;
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function searches the module's Transaction Entry    */
   /* List for the Transaction Entry with the specified Transaction ID  */
   /* and removes it from the List.  This function returns NULL if the  */
   /* specified Entry was NOT present in the list.  The entry returned  */
   /* will have the Next Entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeTransactionEntryMemory().                             */
static Transaction_Entry_t *DeleteTransactionEntry(unsigned int TransactionID)
{
   return((Transaction_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&TransactionID, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntry), (void **)(&TransactionEntryList)));
}

   /* The following function searches the module's Transaction Entry    */
   /* List for the Transaction Entry with the specified GATT Transaction*/
   /* ID and removes it from the List.  This function returns NULL if   */
   /* the specified Entry was NOT present in the list.  The entry       */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionEntryMemory().                    */
static Transaction_Entry_t *DeleteTransactionEntryByGATTTransactionID(unsigned int GATTTransactionID)
{
   return((Transaction_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&GATTTransactionID, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, GATTTransactionID), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntry), (void **)(&TransactionEntryList)));
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
static void FreeTransactionEntryList(void)
{
   BSC_FreeGenericListEntryList((void **)(&TransactionEntryList), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntry));
}

   /* The following function formats the standard response message      */
   /* header for this module.  The request message header must be       */
   /* supplied.                                                         */
static void FormatResponseMessageHeader(BLPM_Response_Message_t *ResponseMessage, BTPM_Message_Header_t *MessageHeader)
{
   ResponseMessage->MessageHeader                = *MessageHeader;
   ResponseMessage->MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
   ResponseMessage->MessageHeader.MessageLength  = BLPM_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
}

   /* The following function processes a Register Client Events Request */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to validate the Message */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessRegisterCollectorEventsRequestMessage(BLPM_Register_Collector_Events_Request_t *Message)
{
   Callback_Entry_t        *CallbackEntryPtr;
   Callback_Entry_t         CallbackEntry;
   BLPM_Response_Message_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Initialize the Callback Entry.                                 */
      BTPS_MemInitialize(&CallbackEntry, 0, sizeof(Callback_Entry_t));

      CallbackEntry.AddressID     = Message->MessageHeader.AddressID;
      CallbackEntry.EventCallback = NULL;

      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      /* This module only supports one callback at this time.           */
      if(!CallbackEntryList)
      {
         if((CallbackEntryPtr = AddCallbackEntry(&CallbackEntry)) != NULL)
         {
            /* Give the callback an ID and indicate success.            */
            CallbackEntryPtr->CallbackID = GetNextCallbackID();
            ResponseMessage.Status       = CallbackEntryPtr->CallbackID;
         }
         else
            ResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ResponseMessage.Status = BTPM_ERROR_CODE_BLOOD_PRESSURE_CALLBACK_ALREADY_REGISTERED;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Un Register Client Events      */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to validate the */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessUnRegisterCollectorEventsRequestMessage(BLPM_Un_Register_Collector_Events_Request_t *Message)
{
   Callback_Entry_t        *CallbackEntryPtr;
   BLPM_Response_Message_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      if((CallbackEntryPtr = DeleteCallbackEntry(Message->CallbackID)) != NULL)
      {
         /* Indicate success.                                           */
         ResponseMessage.Status = 0;

         /* Free the callback entry.                                    */
         FreeCallbackEntryMemory(CallbackEntryPtr);
      }
      else
         ResponseMessage.Status = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes an Enable Notifications Request  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to validate the Message */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessEnableBPMIndicateRequestMessage(BLPM_Device_Request_Message_t *Message)
{
   BLPM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status  = EnableBPMIndicate(Message->CallbackID, &(Message->RemoteDeviceAddress));

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Disable Notifications Request  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to validate the Message */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessDisableBPMIndicateRequestMessage(BLPM_Device_Request_Message_t *Message)
{
   BLPM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status  = DisableBPMIndicate(Message->CallbackID, &(Message->RemoteDeviceAddress));

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes an Enable Notifications Request  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to validate the Message */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessEnableICPNotifyRequestMessage(BLPM_Device_Request_Message_t *Message)
{
   BLPM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status  = EnableICPNotify(Message->CallbackID, &(Message->RemoteDeviceAddress));

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Disable Notifications Request  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to validate the Message */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessDisableICPNotifyRequestMessage(BLPM_Device_Request_Message_t *Message)
{
   BLPM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status  = DisableICPNotify(Message->CallbackID, &(Message->RemoteDeviceAddress));

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Get Blood Pressure Feature     */
   /* Request Message. It does not verify the integrity of the message  */
   /* because the caller should verify that the message is formatted    */
   /* properly before calling this function.                            */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessGetBloodPressureFeatureRequestMessage(BLPM_Device_Request_Message_t *Message)
{
   BLPM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      if(SearchCallbackEntry(Message->CallbackID))
         ResponseMessage.Status = GetBloodPressureFeature(Message->CallbackID, &(Message->RemoteDeviceAddress));
      else
         ResponseMessage.Status = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Cancel Transaction Request     */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to validate the Message */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessCancelTransactionRequestMessage(BLPM_Cancel_Transaction_Request_t *Message)
{
   BLPM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status = CancelTransaction(Message->TransactionID);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.  This function will release the Lock before it*/
   /*          exits (i.e.  the caller SHOULD NOT RELEASE THE LOCK).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case BLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register BLP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
               ProcessRegisterCollectorEventsRequestMessage((BLPM_Register_Collector_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register BLP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterCollectorEventsRequestMessage((BLPM_Un_Register_Collector_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BLPM_MESSAGE_FUNCTION_ENABLE_BPM_INDICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable BPM Indications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_DEVICE_REQUEST_MESSAGE_SIZE)
               ProcessEnableBPMIndicateRequestMessage((BLPM_Device_Request_Message_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BLPM_MESSAGE_FUNCTION_DISABLE_BPM_INDICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable BPM Indications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_DEVICE_REQUEST_MESSAGE_SIZE)
               ProcessDisableBPMIndicateRequestMessage((BLPM_Device_Request_Message_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BLPM_MESSAGE_FUNCTION_ENABLE_ICP_NOTIFICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable ICP Notifications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_DEVICE_REQUEST_MESSAGE_SIZE)
               ProcessEnableICPNotifyRequestMessage((BLPM_Device_Request_Message_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BLPM_MESSAGE_FUNCTION_DISABLE_ICP_NOTIFICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable ICP Notifications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_DEVICE_REQUEST_MESSAGE_SIZE)
               ProcessDisableICPNotifyRequestMessage((BLPM_Device_Request_Message_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BLPM_MESSAGE_FUNCTION_GET_BLOOD_PRESSURE_FEATURE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Blood Pressure Feature Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_DEVICE_REQUEST_MESSAGE_SIZE)
               ProcessGetBloodPressureFeatureRequestMessage((BLPM_Device_Request_Message_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BLPM_MESSAGE_FUNCTION_CANCEL_TRANSACTION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Cancel Transaction Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BLPM_CANCEL_TRANSACTION_REQUEST_SIZE)
               ProcessCancelTransactionRequestMessage((BLPM_Cancel_Transaction_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessCollectorUnRegister(unsigned int AddressID)
{
   Callback_Entry_t *CallbackEntry;
   Callback_Entry_t *tmpCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", AddressID));

   /* Search the event callback entry list for any callbacks with the   */
   /* unregistered Client ID and delete them.                           */
   for(CallbackEntry = CallbackEntryList; CallbackEntry;)
   {
      if(CallbackEntry->AddressID == AddressID)
      {
         /* Temporarily store the current callback entry and advance the*/
         /* pointer.                                                    */
         tmpCallbackEntry = CallbackEntry;
         CallbackEntry    = CallbackEntry->NextCallbackEntry;

         /* Delete the entry from the list.                             */
         if((tmpCallbackEntry = DeleteCallbackEntry(tmpCallbackEntry->CallbackID)) != NULL)
            FreeCallbackEntryMemory(tmpCallbackEntry);
      }
      else
      {
         /* Advance the pointer.                                        */
         CallbackEntry = CallbackEntry->NextCallbackEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process the Module Manager's Asynchronous Events.   */
static void BTPSAPI BTPMDispatchCallback_BLPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            ProcessCollectorUnRegister((unsigned int)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified module event to the registered local event */
   /* callbacks.  The function will verify that a callback has been     */
   /* registered.                                                       */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalEvent(BLPM_Event_Data_t *EventData)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Search the event callback entry list for the specified callback.  */
   if((CallbackEntry = SearchCallbackEntry(EventData->EventCallbackID)) != NULL)
   {
      /* Make sure that this is a local client.                         */
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
      {
         /* Release the Lock before dispatching the callback.           */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(CallbackEntry->EventCallback)
               (*CallbackEntry->EventCallback)(EventData, CallbackEntry->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         /* Re-acquire the Lock.                                        */
         DEVM_AcquireLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Local Blood Pressure Feature Event. It is the caller's */
   /* responsibility to verify the Response before launching this       */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalBloodPressureFeatureEvent(Transaction_Entry_t *TransactionEntry, int Status, Word_t BloodPressureFeature)
{
   BLPM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                                           = betBLPBloodPressureFeatureResponse;
   EventData.EventLength                                                         = BLPM_BLOOD_PRESSURE_FEATURE_RESPONSE_EVENT_DATA_SIZE;
   EventData.EventCallbackID                                                     = TransactionEntry->CallbackID;

   EventData.EventData.BloodPressureFeatureResponseEventData.RemoteDeviceAddress = TransactionEntry->BluetoothAddress;
   EventData.EventData.BloodPressureFeatureResponseEventData.TransactionID       = TransactionEntry->TransactionID;
   EventData.EventData.BloodPressureFeatureResponseEventData.Status              = Status;
   EventData.EventData.BloodPressureFeatureResponseEventData.Feature             = BloodPressureFeature;

   /* Dispatch the event to the registered callback.                    */
   DispatchLocalEvent(&EventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Remote Blood Pressure Feature Message.  It is the      */
   /* caller's responsibility to validate the Response before launching */
   /* this function.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteBloodPressureFeatureEvent(Transaction_Entry_t *TransactionEntry, int Status, Word_t BloodPressureFeature, unsigned int AddressID)
{
   BLPM_Blood_Pressure_Feature_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Message.MessageHeader.AddressID       = AddressID;
   Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
   Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER;
   Message.MessageHeader.MessageFunction = BLPM_MESSAGE_FUNCTION_BLOOD_PRESSURE_FEATURE_RESPONSE;
   Message.MessageHeader.MessageLength   = (BLPM_BLOOD_PRESSURE_FEATURE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.CallbackID                    = TransactionEntry->CallbackID;
   Message.RemoteDeviceAddress           = TransactionEntry->BluetoothAddress;
   Message.TransactionID                 = TransactionEntry->TransactionID;
   Message.Status                        = Status;
   Message.Feature                       = BloodPressureFeature;

   MSG_SendMessage((BTPM_Message_t *)&Message);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Blood Pressure Feature Event.  It is the caller's      */
   /* responsibility to validate the Response before launching this     */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchBloodPressureFeatureEvent(Transaction_Entry_t *TransactionEntry, int Status, Word_t BloodPressureFeature)
{
   Callback_Entry_t *CallbackEntry;

   /* Verify that there is a callback to dispatch to.                   */
   if(((CallbackEntry = SearchCallbackEntry(TransactionEntry->CallbackID))) != NULL)
   {
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
         DispatchLocalBloodPressureFeatureEvent(TransactionEntry, Status, BloodPressureFeature);
      else
         DispatchRemoteBloodPressureFeatureEvent(TransactionEntry, Status, BloodPressureFeature, CallbackEntry->AddressID);
   }
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Blood Pressure Measurement CCD Event. It is the        */
   /* caller's responsibility to validate the Response before launching */
   /* this function.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchBPMCCCDEvent(Transaction_Entry_t *TransactionEntry, int Response)
{
   /* There is nothing to do in this function at this time.  It is here */
   /* in case hooking the CCD response is needed in the future.         */
   if(!Response)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("BPM Indications Enabled on device %02X:%02X:%02X:%02X:%02X:%02X\n",
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR5,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR4,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR3,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR2,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR1,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR0));
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("BPM Indications Enable Failure on device %02X:%02X:%02X:%02X:%02X:%02X\n",
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR5,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR4,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR3,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR2,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR1,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR0));
   }
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Intermediate Cuff Pressure CCD Event. It is the        */
   /* caller's responsibility to validate the Response before launching */
   /* this function.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchICPCCCDEvent(Transaction_Entry_t *TransactionEntry, int Response)
{
   /* There is nothing to do in this function at this time.  It is here */
   /* in case hooking the CCD response is needed in the future.         */
   if(!Response)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("ICP Notifications Enabled on device %02X:%02X:%02X:%02X:%02X:%02X\n",
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR5,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR4,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR3,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR2,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR1,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR0));
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("ICP Notifications Enable Failure on device %02X:%02X:%02X:%02X:%02X:%02X\n",
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR5,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR4,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR3,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR2,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR1,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR0));
   }
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Blood Pressure Measurement Indication to all clients   */
   /* that have registered for indications. It is the caller's          */
   /* responsibility to validate the Response before launching this     */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchBPMIndicateEvent(Pressure_Event_t *Event)
{
   Callback_Entry_t                          *CallbackEntry;
   BLPM_Event_Data_t                          EventData;
   BLPM_Intermediate_Cuff_Pressure_Message_t  Message;

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                                        = betBLPBloodPressureMeasurement;
   EventData.EventLength                                                      = BLPM_BLOOD_PRESSURE_MEASUREMENT_EVENT_DATA_SIZE;
   EventData.EventData.BloodPressureMeasurementEventData.RemoteDeviceAddress  = Event->RemoteSensor;
   EventData.EventData.BloodPressureMeasurementEventData.MeasurementFlags     = Event->BloodPressureMeasurement.Flags;
   EventData.EventData.BloodPressureMeasurementEventData.SystolicPressure     = Event->BloodPressureMeasurement.CompoundValue.Systolic;
   EventData.EventData.BloodPressureMeasurementEventData.DiastolicPressure    = Event->BloodPressureMeasurement.CompoundValue.Diastolic;
   EventData.EventData.BloodPressureMeasurementEventData.MeanArterialPressure = Event->BloodPressureMeasurement.CompoundValue.Mean_Arterial_Pressure;
   EventData.EventData.BloodPressureMeasurementEventData.TimeStamp.Year       = Event->BloodPressureMeasurement.TimeStamp.Year;
   EventData.EventData.BloodPressureMeasurementEventData.TimeStamp.Month      = Event->BloodPressureMeasurement.TimeStamp.Month;
   EventData.EventData.BloodPressureMeasurementEventData.TimeStamp.Day        = Event->BloodPressureMeasurement.TimeStamp.Day;
   EventData.EventData.BloodPressureMeasurementEventData.TimeStamp.Hours      = Event->BloodPressureMeasurement.TimeStamp.Hours;
   EventData.EventData.BloodPressureMeasurementEventData.TimeStamp.Minutes    = Event->BloodPressureMeasurement.TimeStamp.Minutes;
   EventData.EventData.BloodPressureMeasurementEventData.TimeStamp.Seconds    = Event->BloodPressureMeasurement.TimeStamp.Seconds;
   EventData.EventData.BloodPressureMeasurementEventData.PulseRate            = Event->BloodPressureMeasurement.PulseRate;
   EventData.EventData.BloodPressureMeasurementEventData.UserID               = Event->BloodPressureMeasurement.UserID;
   EventData.EventData.BloodPressureMeasurementEventData.MeasurementStatus    = Event->BloodPressureMeasurement.MeasurementStatus;

   /* Format the message that will be dispatched remotely.              */
   Message.MessageHeader.MessageGroup                                         = BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER;
   Message.MessageHeader.MessageFunction                                      = BLPM_MESSAGE_FUNCTION_BLOOD_PRESSURE_MEASUREMENT;
   Message.MessageHeader.MessageLength                                        = (BLPM_BLOOD_PRESSURE_MEASUREMENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   BTPS_MemCopy((void *)&(Message.RemoteDeviceAddress), (void *)&(EventData.EventData.BloodPressureMeasurementEventData), sizeof(EventData.EventData.BloodPressureMeasurementEventData));

   /* Iterate through each callback entry looking for any clients that  */
   /* have enabled notifications for this Blood Pressure Instance.      */
   for(CallbackEntry = CallbackEntryList; CallbackEntry; CallbackEntry = CallbackEntry->NextCallbackEntry)
   {
      if(SearchEnabledEntry(&(CallbackEntry->BPMIndicateEntryList), &(Event->RemoteSensor)) != NULL)
      {
         /* Check the callback type.                                    */
         if(CallbackEntry->AddressID == MSG_GetServerAddressID())
         {
            /* Populate the Callback ID before dispatching.             */
            EventData.EventCallbackID = CallbackEntry->CallbackID;

            /* Dispatch the event to the registered callback.           */
            DispatchLocalEvent(&EventData);
         }
         else
         {
            if(CallbackEntry->AddressID)
            {
               /* Populate the Address, Message, and Callback ID before */
               /* dispatching.                                          */
               Message.MessageHeader.MessageID = MSG_GetNextMessageID();
               Message.MessageHeader.AddressID = CallbackEntry->AddressID;
               Message.CallbackID              = CallbackEntry->CallbackID;

               MSG_SendMessage((BTPM_Message_t *)&Message);
            }
         }
      }
   }
}

   /* The following function is a utility function that exists to       */
   /* dispatch an Intermediate Cuff Pressure Notification to all        */
   /* registered clients that have requested notifications.  It is the  */
   /* caller's responsibility to validate the Response before launching */
   /* this function.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchICPNotifyEvent(Pressure_Event_t *Event)
{
   BLPM_Event_Data_t                          EventData;
   Callback_Entry_t                          *CallbackEntry;
   BLPM_Intermediate_Cuff_Pressure_Message_t  Message;

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                                            = betBLPIntermediateCuffPressure;
   EventData.EventLength                                                          = BLPM_INTERMEDIATE_CUFF_PRESSURE_EVENT_DATA_SIZE;
   EventData.EventData.IntermediateCuffPressureEventData.RemoteDeviceAddress      = Event->RemoteSensor;
   EventData.EventData.IntermediateCuffPressureEventData.Flags                    = Event->BloodPressureMeasurement.Flags;
   EventData.EventData.IntermediateCuffPressureEventData.IntermediateCuffPressure = Event->BloodPressureMeasurement.CompoundValue.Systolic;
   EventData.EventData.IntermediateCuffPressureEventData.TimeStamp.Year           = Event->BloodPressureMeasurement.TimeStamp.Year;
   EventData.EventData.IntermediateCuffPressureEventData.TimeStamp.Month          = Event->BloodPressureMeasurement.TimeStamp.Month;
   EventData.EventData.IntermediateCuffPressureEventData.TimeStamp.Day            = Event->BloodPressureMeasurement.TimeStamp.Day;
   EventData.EventData.IntermediateCuffPressureEventData.TimeStamp.Hours          = Event->BloodPressureMeasurement.TimeStamp.Hours;
   EventData.EventData.IntermediateCuffPressureEventData.TimeStamp.Minutes        = Event->BloodPressureMeasurement.TimeStamp.Minutes;
   EventData.EventData.IntermediateCuffPressureEventData.TimeStamp.Seconds        = Event->BloodPressureMeasurement.TimeStamp.Seconds;
   EventData.EventData.IntermediateCuffPressureEventData.PulseRate                = Event->BloodPressureMeasurement.PulseRate;
   EventData.EventData.IntermediateCuffPressureEventData.UserID                   = Event->BloodPressureMeasurement.UserID;
   EventData.EventData.IntermediateCuffPressureEventData.MeasurementStatus        = Event->BloodPressureMeasurement.MeasurementStatus;

   /* Format the message that will be dispatched remotely.              */
   Message.MessageHeader.MessageGroup                                             = BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER;
   Message.MessageHeader.MessageFunction                                          = BLPM_MESSAGE_FUNCTION_INTERMEDIATE_CUFF_PRESSURE;
   Message.MessageHeader.MessageLength                                            = (BLPM_INTERMEDIATE_CUFF_PRESSURE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   BTPS_MemCopy((void *)&(Message.RemoteDeviceAddress), (void *)&(EventData.EventData.IntermediateCuffPressureEventData), sizeof(EventData.EventData.IntermediateCuffPressureEventData));

   /* Iterate through each callback entry looking for any clients that  */
   /* have enabled notifications for this Blood Pressure Instance.      */
   for(CallbackEntry = CallbackEntryList; CallbackEntry; CallbackEntry = CallbackEntry->NextCallbackEntry)
   {
      if(SearchEnabledEntry(&(CallbackEntry->ICPNotifyEntryList), &(Event->RemoteSensor)) != NULL)
      {
         /* Check the callback type.                                    */
         if(CallbackEntry->AddressID == MSG_GetServerAddressID())
         {
            /* Populate the Callback ID before dispatching.             */
            EventData.EventCallbackID = CallbackEntry->CallbackID;

            /* Dispatch the event to the registered callback.           */
            DispatchLocalEvent(&EventData);
         }
         else
         {
            if(CallbackEntry->AddressID)
            {
               /* Populate the Address, Message, and Callback ID before */
               /* dispatching.                                          */
               Message.MessageHeader.MessageID = MSG_GetNextMessageID();
               Message.MessageHeader.AddressID = CallbackEntry->AddressID;
               Message.CallbackID              = CallbackEntry->CallbackID;

               MSG_SendMessage((BTPM_Message_t *)&Message);
            }
         }
      }
   }
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Connected Event to every registered callback.  It is   */
   /* the caller's responsibility to validate the Response before       */
   /* launching this function.                                          */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchConnectedEvent(Device_Entry_t *DeviceEntry)
{
   Callback_Entry_t         *CallbackEntry;
   BLPM_Event_Data_t         EventData;
   BLPM_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                        = betBLPConnected;
   EventData.EventLength                                      = BLPM_CONNECTED_EVENT_DATA_SIZE;
   EventData.EventData.ConnectedEventData.RemoteDeviceAddress = DeviceEntry->BluetoothAddress;
   EventData.EventData.ConnectedEventData.ConnectionType      = bctCollector;
   EventData.EventData.ConnectedEventData.ConnectedFlags      = DeviceEntry->Flags;

   /* Format the message that will be dispatched remotely.              */
   Message.MessageHeader.MessageGroup                         = BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER;
   Message.MessageHeader.MessageFunction                      = BLPM_MESSAGE_FUNCTION_CONNECTED;
   Message.MessageHeader.MessageLength                        = (BLPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.RemoteDeviceAddress                                = DeviceEntry->BluetoothAddress;
   Message.ConnectionType                                     = bctCollector;
   Message.ConnectedFlags                                     = DeviceEntry->Flags;

   /* Iterate through each callback entry.                              */
   for(CallbackEntry=CallbackEntryList;CallbackEntry;CallbackEntry=CallbackEntry->NextCallbackEntry)
   {
      /* Check the callback type.                                       */
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
      {
         /* Populate the Callback ID before dispatching.                */
         EventData.EventCallbackID = CallbackEntry->CallbackID;

         /* Dispatch the event to the registered callback.              */
         DispatchLocalEvent(&EventData);
      }
      else
      {
         if(CallbackEntry->AddressID)
         {
            /* Populate the Address, Message, and Callback ID before    */
            /* dispatching.                                             */
            Message.MessageHeader.MessageID = MSG_GetNextMessageID();
            Message.MessageHeader.AddressID = CallbackEntry->AddressID;
            Message.CallbackID              = CallbackEntry->CallbackID;

            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Disconnected Event to every registered callback. It is */
   /* the caller's responsibility to validate the Response before       */
   /* launching this function.                                          */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchDisconnectedEvent(Device_Entry_t *DeviceEntry)
{
   Callback_Entry_t            *CallbackEntry;
   BLPM_Event_Data_t            EventData;
   BLPM_Disconnected_Message_t  Message;

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                           = betBLPDisconnected;
   EventData.EventLength                                         = BLPM_DISCONNECTED_EVENT_DATA_SIZE;
   EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = DeviceEntry->BluetoothAddress;
   EventData.EventData.DisconnectedEventData.ConnectionType      = bctCollector;

   /* Format the message that will be dispatched remotely.              */
   Message.MessageHeader.MessageGroup                            = BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER;
   Message.MessageHeader.MessageFunction                         = BLPM_MESSAGE_FUNCTION_DISCONNECTED;
   Message.MessageHeader.MessageLength                           = (BLPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.RemoteDeviceAddress                                   = DeviceEntry->BluetoothAddress;
   Message.ConnectionType                                        = bctCollector;

   /* Iterate through each callback entry.                              */
   for(CallbackEntry = CallbackEntryList; CallbackEntry; CallbackEntry = CallbackEntry->NextCallbackEntry)
   {
      /* Check the callback type.                                       */
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
      {
         /* Populate the Callback ID before dispatching.                */
         EventData.EventCallbackID = CallbackEntry->CallbackID;

         /* Dispatch the event to the registered callback.              */
         DispatchLocalEvent(&EventData);
      }
      else
      {
         if(CallbackEntry->AddressID)
         {
            /* Populate the Address, Message, and Callback ID before    */
            /* dispatching.                                             */
            Message.MessageHeader.MessageID = MSG_GetNextMessageID();
            Message.MessageHeader.AddressID = CallbackEntry->AddressID;
            Message.CallbackID              = CallbackEntry->CallbackID;

            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
   }
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GATT Notification Events.                   */
static void BTPSAPI BTPSDispatchCallback_GATT(void *CallbackParameter)
{
   Device_Entry_t     *DeviceEntry;
   Pressure_Event_t    PressureEvent;
   BLPM_Update_Data_t *UpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, let's check to see if we need to process it.          */
         if(Initialized)
         {
            UpdateData = (BLPM_Update_Data_t *)CallbackParameter;

            switch(UpdateData->UpdateType)
            {
               case utGATTNotificationEvent:
                  /* Find the device that notified us.                  */
                  if((DeviceEntry = SearchDeviceEntry(&(UpdateData->UpdateData.GATTServerNotificationData.RemoteDevice))) != NULL)
                  {
                     if(UpdateData->UpdateData.GATTServerNotificationData.AttributeValueLength >= BLS_BLOOD_PRESSURE_MEASUREMENT_SIZE(0))
                     {
                        /* The Notification is for a Blood Pressure     */
                        /* Measurement. Format the event and send it to */
                        /* clients that have enabled notifications for  */
                        /* this Blood Pressure Instance.                */
                        PressureEvent.RemoteSensor = DeviceEntry->BluetoothAddress;

                        if(BLS_Decode_Blood_Pressure_Measurement(UpdateData->UpdateData.GATTServerNotificationData.AttributeValueLength, UpdateData->UpdateData.GATTServerNotificationData.AttributeValue, &(PressureEvent.BloodPressureMeasurement)) == 0)
                           DispatchICPNotifyEvent(&PressureEvent);
                     }
                  }
                  break;
               case utGATTIndicationEvent:
                  /* Find the device that indicated to us.              */
                  if((DeviceEntry = SearchDeviceEntry(&(UpdateData->UpdateData.GATTServerIndicationData.RemoteDevice))) != NULL)
                  {
                     if(UpdateData->UpdateData.GATTServerIndicationData.AttributeValueLength >= BLS_BLOOD_PRESSURE_MEASUREMENT_SIZE(0))
                     {
                        /* The Notification is for a Blood Pressure     */
                        /* Measurement. Format the event and send it to */
                        /* clients that have enabled notifications for  */
                        /* this Blood Pressure Instance.                */
                        PressureEvent.RemoteSensor = DeviceEntry->BluetoothAddress;

                        if(BLS_Decode_Blood_Pressure_Measurement(UpdateData->UpdateData.GATTServerIndicationData.AttributeValueLength, UpdateData->UpdateData.GATTServerIndicationData.AttributeValue, &(PressureEvent.BloodPressureMeasurement)) == 0)
                           DispatchBPMIndicateEvent(&PressureEvent);
                     }
                  }
                  break;
               default:
                  break;
            }
         }

         /* Release the lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process BLP Client Events.                          */
static void BTPSAPI BTPSDispatchCallback_BLP_Collector(void *CallbackParameter)
{
   Transaction_Entry_t      *TransactionEntry;
   GATT_Client_Event_Data_t *EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, let's check to see if we need to process it.          */
         if(Initialized)
         {
            EventData = &(((BLPM_Update_Data_t *)CallbackParameter)->UpdateData.GATTClientEventData);

            switch(EventData->Event_Data_Type)
            {
               case etGATT_Client_Error_Response:
                  /* Verify that there is an outstanding transaction for*/
                  /* this event.                                        */
                  if((TransactionEntry = DeleteTransactionEntryByGATTTransactionID(EventData->Event_Data.GATT_Request_Error_Data->TransactionID)) != NULL)
                  {
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttBloodPressureFeature:
                           DispatchBloodPressureFeatureEvent(TransactionEntry, BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID, 0);
                           break;
                        case ttBloodPressureMeasurementCCCDEnable:
                           ProcessBPMIndicationStateChange(TransactionEntry, TRUE, FALSE);
                           break;
                        case ttBloodPressureMeasurementCCCDDisable:
                           ProcessBPMIndicationStateChange(TransactionEntry, FALSE, FALSE);
                           break;
                        case ttIntermediateCuffPressureCCCDEnable:
                           ProcessICPNotificationStateChange(TransactionEntry, TRUE, FALSE);
                           break;
                        case ttIntermediateCuffPressureCCCDDisable:
                           ProcessICPNotificationStateChange(TransactionEntry, FALSE, FALSE);
                           break;
                        default:
                           DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Error_Response, Unkown Transaction Type = %u\n", (unsigned int)TransactionEntry->TransactionType));
                           break;
                     }

                     FreeTransactionEntryMemory(TransactionEntry);
                  }
                  break;
               case etGATT_Client_Write_Response:
                  /* Verify that there is an outstanding transaction for*/
                  /* this event.                                        */
                  if((TransactionEntry = DeleteTransactionEntryByGATTTransactionID(EventData->Event_Data.GATT_Write_Response_Data->TransactionID)) != NULL)
                  {
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttBloodPressureMeasurementCCCDEnable:
                           ProcessBPMIndicationStateChange(TransactionEntry, TRUE, TRUE);
                           break;
                        case ttBloodPressureMeasurementCCCDDisable:
                           ProcessBPMIndicationStateChange(TransactionEntry, FALSE, TRUE);
                           break;
                        case ttIntermediateCuffPressureCCCDEnable:
                           ProcessICPNotificationStateChange(TransactionEntry, TRUE, TRUE);
                           break;
                        case ttIntermediateCuffPressureCCCDDisable:
                           ProcessICPNotificationStateChange(TransactionEntry, FALSE, TRUE);
                           break;
                        default:
                           DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Write_Response, Unkown Transaction Type = %u\n", (unsigned int)TransactionEntry->TransactionType));
                           break;
                     }

                     FreeTransactionEntryMemory(TransactionEntry);
                  }
                  break;
               case etGATT_Client_Read_Response:
                  /* Verify that there is an outstanding transaction for*/
                  /* this event.                                        */
                  if((TransactionEntry = DeleteTransactionEntryByGATTTransactionID(EventData->Event_Data.GATT_Read_Response_Data->TransactionID)) != NULL)
                  {
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttBloodPressureFeature:
                           if(EventData->Event_Data.GATT_Read_Response_Data->AttributeValueLength >= sizeof(NonAlignedWord_t))
                              DispatchBloodPressureFeatureEvent(TransactionEntry, 0, READ_UNALIGNED_WORD_LITTLE_ENDIAN(EventData->Event_Data.GATT_Read_Response_Data->AttributeValue));
                           else
                              DispatchBloodPressureFeatureEvent(TransactionEntry, BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID, 0);
                           break;
                        default:
                           DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Read_Response, Unkown Transaction Type = %u\n", (unsigned int)TransactionEntry->TransactionType));
                           break;
                     }

                     FreeTransactionEntryMemory(TransactionEntry);
                  }
                  break;
               default:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown CLIENT event type.\n"));
                  break;
            }
         }

         /* Release the lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* delete the enabled entries for a specified device for all         */
   /* registered callbacks.                                             */
static void DeleteEnabledEntriesForDevice(BD_ADDR_t BD_ADDR)
{
   Enabled_Entry_t  *EnabledEntry;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Walk the list and delete all enabled entries for this device.  */
      CallbackEntry = CallbackEntryList;
      while(CallbackEntry)
      {
         if((EnabledEntry = DeleteEnabledEntry(&(CallbackEntry->BPMIndicateEntryList), &BD_ADDR)) != NULL)
            FreeEnabledEntryMemory(EnabledEntry);

         if((EnabledEntry = DeleteEnabledEntry(&(CallbackEntry->ICPNotifyEntryList), &BD_ADDR)) != NULL)
            FreeEnabledEntryMemory(EnabledEntry);

         CallbackEntry = CallbackEntry->NextCallbackEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* delete the transaction entries for a specified device for all     */
   /* outstanding GATT transactions.                                    */
static void DeleteTransactionEntriesForDevice(BD_ADDR_t BD_ADDR)
{
   Transaction_Entry_t *NextEntry;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Walk the list and delete all entries for this device.          */
      TransactionEntry = TransactionEntryList;
      while(TransactionEntry)
      {
         if(COMPARE_BD_ADDR(TransactionEntry->BluetoothAddress, BD_ADDR))
         {
            if((TransactionEntry = DeleteTransactionEntry(TransactionEntry->TransactionID)) != NULL)
            {
               switch(TransactionEntry->TransactionType)
               {
                  case ttBloodPressureFeature:
                     /* Report that the transaction failed due to       */
                     /* disconnection.                                  */
                     DispatchBloodPressureFeatureEvent(TransactionEntry, BTPM_ERROR_CODE_LOW_ENERGY_NOT_CONNECTED, 0);
                     break;
                  case ttBloodPressureMeasurementCCCDEnable:
                  case ttBloodPressureMeasurementCCCDDisable:
                  case ttIntermediateCuffPressureCCCDEnable:
                  case ttIntermediateCuffPressureCCCDDisable:
                     /* Currently, no event is triggered when           */
                     /* transactions of these types are completed.      */
                     break;
               }

               /* Delete the entry and move to the next.                */
               NextEntry = TransactionEntry->NextTransactionEntry;
               FreeTransactionEntryMemory(TransactionEntry);
               TransactionEntry = NextEntry;
            }
         }
         else
            TransactionEntry = TransactionEntry->NextTransactionEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* store/delete a specified Key that is stored in the BLPM           */
   /* Configuration Section.                                            */
static void StoreBLPMKey(char *KeyName, unsigned int Value)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(KeyName)
      SET_WriteInteger(BLPM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, Value, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* store/delete BLPM Information for the specified device to file.   */
static void StoreBLPMInformation(BD_ADDR_t RemoteDeviceAddress, Boolean_t BPMIndicationsEnabled, Boolean_t ICPNotificationsEnabled)
{
   char KeyName[BLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Build the Key Name for the Blood Pressure CCCD.                */
      sprintf(KeyName, BLPM_LE_KEY_NAME_BLOOD_PRESSURE_CCCD, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0);

      StoreBLPMKey(KeyName, (unsigned int)BPMIndicationsEnabled);

      /* Build the Key Name for the Intermediate Cuff Pressure CCCD.    */
      sprintf(KeyName, BLPM_LE_KEY_NAME_INTERMEDIATE_CUFF_PRESSURE_CCCD, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0);

      StoreBLPMKey(KeyName, (unsigned int)ICPNotificationsEnabled);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* update the Configuration File store for the specified device to   */
   /* the LE Configuration file (if and only if the device is paired).  */
static void UpdateDeviceFile(Device_Entry_t *DeviceEntry)
{
   unsigned long                   RequiredFlags;
   DEVM_Remote_Device_Properties_t DeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(DeviceEntry)
   {
      /* Attempt to query the remote device properties.                 */
      if(!DEVM_QueryRemoteDeviceProperties(DeviceEntry->BluetoothAddress, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &DeviceProperties))
      {
         /* Set the required flags for us to write this entry to the    */
         /* device file.                                                */
         RequiredFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED);

         /* Check to see if all required flags are set.                 */
         if((DeviceProperties.RemoteDeviceFlags & RequiredFlags) == RequiredFlags)
            StoreBLPMInformation(DeviceEntry->BluetoothAddress, DeviceEntry->BPMIndicationsEnabled, DeviceEntry->ICPNotificationsEnabled);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* update the Configuration File store for the specified device from */
   /* the LE Configuration file.  This function returns TRUE if there is*/
   /* valid data stored for the selected device or false otherwise.     */
static Boolean_t ReloadDeviceEntry(BD_ADDR_t BD_ADDR, Boolean_t *BPMIndicationsEnabled, Boolean_t *ICPNotificationsEnabled)
{
   char         KeyName[BLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Boolean_t    ret_val = FALSE;
   unsigned int Value;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Build the Key Name for the Blood Pressure CCCD.                */
      sprintf(KeyName, BLPM_LE_KEY_NAME_BLOOD_PRESSURE_CCCD, BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

      /* Attempt to read the Blood Pressure CCCD.                       */
      Value = SET_ReadInteger(BLPM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);

      /* Only update the Device Entry if necessary.                     */
      if(Value)
      {
         /* Updated the Device Entry if specified.                      */
         if(BPMIndicationsEnabled)
            *BPMIndicationsEnabled = TRUE;

         /* Make sure we return success.                                */
         ret_val = TRUE;
      }

      /* Build the Key Name for the Intermediate Cuff Pressure CCCD.    */
      sprintf(KeyName, BLPM_LE_KEY_NAME_INTERMEDIATE_CUFF_PRESSURE_CCCD, BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

      /* Attempt to read the Intermediate Cuff Pressure CCCD.           */
      Value = SET_ReadInteger(BLPM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);

      /* Only update the Device Entry if necessary.                     */
      if(Value)
      {
         /* Updated the Device Entry if specified.                      */
         if(ICPNotificationsEnabled)
            *ICPNotificationsEnabled = TRUE;

         /* Make sure we return success.                                */
         ret_val = TRUE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", (unsigned int)ret_val));

   return(ret_val);
}

   /* The following is a utility function used to discover a single     */
   /* valid Blood Pressure Service Instances on a remote device.  On    */
   /* success, a pointer to the new device entry will be returned;      */
   /* otherwise, NULL will be returned.                                 */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static Device_Entry_t *DiscoverBloodPressureSensor(BD_ADDR_t *BluetoothAddress)
{
   int                                       Result;
   GATT_UUID_t                               UUID;
   unsigned int                              ServiceDataSize;
   unsigned int                              Index;
   unsigned int                              Index1;
   unsigned int                              Index2;
   unsigned char                            *ServiceData;
   Device_Entry_t                           *DeviceEntry = NULL;
   DEVM_Parsed_Services_Data_t               ParsedGATTData;
   GATT_Characteristic_Information_t        *CharacteristicInformation;
   GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            Result = DEVM_ConvertRawServicesStreamToParsedServicesData(ServiceDataSize, ServiceData, &ParsedGATTData);
            if(!Result)
            {
               /* Iterate the services on the remote device.            */
               for(Index=ParsedGATTData.NumberServices;Index--;)
               {
                  UUID = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID;

                  /* Check for BLS.                                     */
                  if((UUID.UUID_Type == guUUID_16) && (BLS_COMPARE_BLS_SERVICE_UUID_TO_UUID_16(UUID.UUID.UUID_16)))
                  {
                     /* Create a device entry.                          */
                     if((DeviceEntry = (Device_Entry_t *)BTPS_AllocateMemory(sizeof(Device_Entry_t))) != NULL)
                     {
                        /* Initalize the device entry.                  */
                        BTPS_MemInitialize(DeviceEntry, 0, sizeof(Device_Entry_t));
                        DeviceEntry->BluetoothAddress = *BluetoothAddress;

                        GATTServiceDiscoveryIndicationData = &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index]);

                        /* Iterate the BLS characteristics.             */
                        for(Index1 = GATTServiceDiscoveryIndicationData->NumberOfCharacteristics;Index1--;)
                        {
                           CharacteristicInformation = &(GATTServiceDiscoveryIndicationData->CharacteristicInformationList[Index1]);
                           UUID                      = CharacteristicInformation->Characteristic_UUID;

                           if(UUID.UUID_Type == guUUID_16)
                           {
                              /* Check for Blood Pressure Measurement   */
                              /* UUID.                                  */
                              if((BLS_COMPARE_BLOOD_PRESSURE_MEASUREMENT_UUID_TO_UUID_16(UUID.UUID.UUID_16)) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE) && !(DeviceEntry->ServerInformationHandles.Blood_Pressure_Measurement))
                              {
                                 DeviceEntry->ServerInformationHandles.Blood_Pressure_Measurement = CharacteristicInformation->Characteristic_Handle;

                                 for(Index2=CharacteristicInformation->NumberOfDescriptors;Index2--;)
                                 {
                                    UUID = CharacteristicInformation->DescriptorList[Index2].Characteristic_Descriptor_UUID;

                                    /* We only care about 16-bit        */
                                    /* descriptor UUIDs                 */
                                    if(UUID.UUID_Type == guUUID_16)
                                    {
                                       /* Check for Blood Pressure      */
                                       /* Measurement CCD UUID.         */
                                       if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16))
                                       {
                                          DeviceEntry->ServerInformationHandles.Blood_Pressure_Measurement_Client_Configuration = CharacteristicInformation->DescriptorList[Index2].Characteristic_Descriptor_Handle;

                                          break;
                                       }
                                    }
                                 }

                                 /* This characteristic must have a CCD;*/
                                 /* otherwise, it is invalid.           */
                                 if(!(DeviceEntry->ServerInformationHandles.Blood_Pressure_Measurement_Client_Configuration))
                                    DeviceEntry->ServerInformationHandles.Blood_Pressure_Measurement = 0;
                              }
                              else
                              {
                                 /* Check for Intermediate Cuff Pressure*/
                                 /* UUID.                               */
                                 if((BLS_COMPARE_INTERMEDIATE_CUFF_PRESSURE_UUID_TO_UUID_16(UUID.UUID.UUID_16)) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY) && !(DeviceEntry->ServerInformationHandles.Intermediate_Cuff_Pressure))
                                 {
                                    DeviceEntry->ServerInformationHandles.Intermediate_Cuff_Pressure = CharacteristicInformation->Characteristic_Handle;

                                    for(Index2=CharacteristicInformation->NumberOfDescriptors;Index2--;)
                                    {
                                       UUID = CharacteristicInformation->DescriptorList[Index2].Characteristic_Descriptor_UUID;

                                       /* We only care about 16-bit     */
                                       /* descriptor UUIDs              */
                                       if(UUID.UUID_Type == guUUID_16)
                                       {
                                          /* Check for Intermediate     */
                                          /* Cuff Pressure CCD UUID.    */
                                          if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16))
                                          {
                                             DeviceEntry->ServerInformationHandles.Intermediate_Cuff_Pressure_Client_Configuration = CharacteristicInformation->DescriptorList[Index2].Characteristic_Descriptor_Handle;

                                             /* Set the flag in the     */
                                             /* device entry to show    */
                                             /* that Intermediate Cuff  */
                                             /* Pressure is supported.  */
                                             DeviceEntry->Flags |= BLPM_CONNECTED_FLAGS_INTERMEDIATE_CUFF_PRESSURE_SUPPORTED;

                                             break;
                                          }
                                       }
                                    }

                                    /* This characteristic must have a  */
                                    /* CCD; otherwise, it is invalid.   */
                                    if(!(DeviceEntry->ServerInformationHandles.Intermediate_Cuff_Pressure_Client_Configuration))
                                       DeviceEntry->ServerInformationHandles.Intermediate_Cuff_Pressure = 0;
                                 }
                                 else
                                 {
                                    /* Check for Blood Pressure Feature */
                                    /* UUID.                            */
                                    if((BLS_COMPARE_BLOOD_PRESSURE_FEATURE_UUID_TO_UUID_16(UUID.UUID.UUID_16)) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                                       DeviceEntry->ServerInformationHandles.Blood_Pressure_Feature = CharacteristicInformation->Characteristic_Handle;
                                 }
                              }
                           }
                        }

                        /* At a minimum, BLS must support the Blood     */
                        /* Pressure Measurement and Blood Pressure      */
                        /* Feature Characteristics.                     */
                        if((DeviceEntry->ServerInformationHandles.Blood_Pressure_Measurement) && (DeviceEntry->ServerInformationHandles.Blood_Pressure_Feature))
                        {
                           /* Add it to the list and notify all PM      */
                           /* clients about the device.                 */
                           if(AddDeviceEntryActual(DeviceEntry))
                           {
                              /* Dispatch the connected event for this  */
                              /* device.                                */
                              DispatchConnectedEvent(DeviceEntry);
                           }
                           else
                           {
                              FreeDeviceEntryMemory(DeviceEntry);

                              DeviceEntry = NULL;
                           }
                        }
                        else
                        {
                           BTPS_FreeMemory(DeviceEntry);

                           DeviceEntry = NULL;
                        }
                     }

                     /* Exit loop after parsing BLS information.        */
                     break;
                  }
               }

               /* All finished with the parsed data, so free it.        */
               DEVM_FreeParsedServicesData(&ParsedGATTData);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_ConvertRawServicesStreamToParsedServicesData returned %d.\n", Result));
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
         }

         /* Free the previously allocated buffer holding service data   */
         /* information.                                                */
         BTPS_FreeMemory(ServiceData);
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FAILURE), ("Allocation request for Service Data failed, size = %u.\n", ServiceDataSize));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit %p\n", DeviceEntry));

   return(DeviceEntry);
}

   /* The following is a utility function used to format and add a      */
   /* Unique Transaction Entry to the transaction list.  Zero is        */
   /* returned on success and the ReturnTransactionEntry will point to  */
   /* the newly created transaction.  A negative error code is returned */
   /* on failure, and the returned transaction pointer should not be    */
   /* used.                                                             */
   /* * NOTE * This function will return with the Module Manager's lock */
   /*          acquired on success.  It is the caller's responsibility  */
   /*          to free the lock on success.                             */
static int FormatAddTransactionEntry(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, Transaction_Type_t TransactionType, Transaction_Entry_t **ReturnTransactionEntry)
{
   int                  ret_val;
   Device_Entry_t      *DeviceEntry;
   Transaction_Entry_t  TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock that guards access to this module.    */
   if(DEVM_AcquireLock())
   {
      /* Find the remote Blood Pressure Instance.                       */
      if((DeviceEntry = SearchDeviceEntry(RemoteSensor)) != NULL)
      {
         /* Verify that this transaction is unique for the provided     */
         /* callback.                                                   */
         if(!SearchTransactionEntry(RemoteSensor, TransactionType, CallbackID))
         {
            /* Populate the Transaction entry.                          */
            TransactionEntry.BluetoothAddress = *RemoteSensor;
            TransactionEntry.TransactionType  = TransactionType;
            TransactionEntry.CallbackID       = CallbackID;
            TransactionEntry.AttributeHandle  = *((Word_t *)(((Byte_t *)DeviceEntry) + TransactionTypeToHandleOffsetTable[TransactionType]));

            /* Add the transaction to the list.                         */
            if((*ReturnTransactionEntry = AddTransactionEntry(&TransactionEntry)) != NULL)
            {
               (*ReturnTransactionEntry)->TransactionID = GetNextTransactionID();
               ret_val                                  = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
            ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_SAME_REQUEST_OUTSTANDING;
      }
      else
         ret_val = BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE;

      /* Release the Lock on failure.                                   */
      if(ret_val)
         DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following is a utility function used to submit a read request */
   /* to the provided Blood Pressure Instance given a Transaction Type. */
   /* A transaction entry used to identify the response will also be    */
   /* created on success.  A positive Transaction ID will be returned on*/
   /* success; otherwise, a negative error code will be returned.       */
static int SubmitReadValue(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, Transaction_Type_t TransactionType)
{
   int                  ret_val;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Add a transaction entry for this request.                         */
   if(!(ret_val = FormatAddTransactionEntry(CallbackID, RemoteSensor, TransactionType, &TransactionEntry)))
   {
      /* Submit the command request.                                    */
      if((ret_val = _BLPM_Read_Value(RemoteSensor, TransactionEntry->AttributeHandle)) > 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("_BLPM_Read_Value(), TransactionID = %u\n", (unsigned int)ret_val));

         /* Save the GATT Transaction ID and return our Transaction ID. */
         TransactionEntry->GATTTransactionID = ret_val;
         ret_val                             = TransactionEntry->TransactionID;
      }
      else
      {
         /* Free the transaction on error.                              */
         if((TransactionEntry = DeleteTransactionEntry(TransactionEntry->TransactionID)) != NULL)
            FreeTransactionEntryMemory(TransactionEntry);
      }

      DEVM_ReleaseLock();
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function used to submit a write request*/
   /* to the provided Blood Pressure Instance given a Transaction Type. */
   /* A transaction entry used to identify the response will also be    */
   /* created on success.  A positive Transaction ID will be returned   */
   /* on success; otherwise, a negative error code will be returned.    */
static int SubmitWriteValue(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, Transaction_Type_t TransactionType, unsigned int DataLength, Byte_t *Data)
{
   int                  ret_val;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Add a transaction entry for this request.                         */
   if((ret_val = FormatAddTransactionEntry(CallbackID, RemoteSensor, TransactionType, &TransactionEntry)) == 0)
   {
      /* Submit the command request.                                    */
      if((ret_val = _BLPM_Write_Value(RemoteSensor, TransactionEntry->AttributeHandle, (Word_t)DataLength, Data)) > 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("_BLPM_Write_Value(), TransactionID = %u\n", (unsigned int)ret_val));

          /* Save the GATT Transaction ID and return our Transaction ID.*/
         TransactionEntry->GATTTransactionID = ret_val;
         ret_val                             = TransactionEntry->TransactionID;
      }
      else
      {
         if((TransactionEntry = DeleteTransactionEntry(TransactionEntry->TransactionID)) != NULL)
            FreeTransactionEntryMemory(TransactionEntry);
      }

      DEVM_ReleaseLock();
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function used to enable Blood Pressure */
   /* notifications on the provided Blood Pressure Sensor.  Zero will be*/
   /* returned on success; otherwise, a negative error code will be     */
   /* returned.                                                         */
static int EnableBPMIndicate(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   int               ret_val;
   Device_Entry_t   *DeviceEntry;
   Enabled_Entry_t   EnabledEntry;
   Enabled_Entry_t  *EnabledEntryPtr;
   Callback_Entry_t *CallbackEntryPtr;

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Search for the callback associated with this request.          */
      if((CallbackEntryPtr = SearchCallbackEntry(CallbackID)) != NULL)
      {
         /* Verify that the instance entry associated with this request */
         /* exists.                                                     */
         if((DeviceEntry = SearchDeviceEntry(RemoteSensor)) != NULL)
         {
            /* The instance exists, so add the entry into the enabled   */
            /* notifications list for the specified callback.           */
            EnabledEntry.BluetoothAddress = *RemoteSensor;

            if(AddEnabledEntry(&(CallbackEntryPtr->BPMIndicateEntryList), &EnabledEntry))
            {
               /* Configure the CCD only when the count goes from zero  */
               /* to positive.                                          */
               if((!(DeviceEntry->BPMIndicateCount)) && (DeviceEntry->BPMIndicationsEnabled == FALSE))
               {
                  if((ret_val = WriteBPMCCCD(RemoteSensor, TRUE)) > 0)
                  {
                     DeviceEntry->BPMIndicateCount++;

                     ret_val = 0;
                  }
                  else
                  {
                     /* Remove the entry in the list since configuring  */
                     /* the CCD failed.                                 */
                     if((EnabledEntryPtr = DeleteEnabledEntry(&(CallbackEntryPtr->BPMIndicateEntryList), RemoteSensor)) != NULL)
                        FreeEnabledEntryMemory(EnabledEntryPtr);
                  }
               }
               else
               {
                  DeviceEntry->BPMIndicateCount++;

                  ret_val = 0;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
            ret_val = BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   return(ret_val);
}

   /* The following is a utility function used to disable notifications */
   /* on the provided Blood Pressure Instance given a Transaction Type. */
   /* Zero will be returned on success; otherwise, a negative error     */
   /* code will be returned.                                            */
static int DisableBPMIndicate(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   int               ret_val;
   Device_Entry_t   *DeviceEntry;
   Enabled_Entry_t  *EnabledEntry;
   Callback_Entry_t *CallbackEntryPtr;

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Search for the callback associated with this request.          */
      if((CallbackEntryPtr = SearchCallbackEntry(CallbackID)) != NULL)
      {
         /* Try to delete the requested Notifications Enabled entry from*/
         /* the list.                                                   */
         if((EnabledEntry = DeleteEnabledEntry(&(CallbackEntryPtr->BPMIndicateEntryList), RemoteSensor)) != NULL)
         {
            /* Decrement the notification count from the instance.      */
            if((DeviceEntry = SearchDeviceEntry(RemoteSensor)) != NULL)
            {
               /* Configure the CCD only when the count goes from       */
               /* positive to zero.                                     */
               if((DeviceEntry->BPMIndicateCount) && (!(--DeviceEntry->BPMIndicateCount)))
               {
                  /* Only disable indications if they are actually      */
                  /* enabled.                                           */
                  if(DeviceEntry->BPMIndicationsEnabled)
                  {
                     if((ret_val = WriteBPMCCCD(RemoteSensor, FALSE)) > 0)
                        ret_val = 0;
                  }
                  else
                     ret_val = 0;
               }
               else
                  ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE;

            FreeEnabledEntryMemory(EnabledEntry);
         }
         else
            ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_BPM_INDICATIONS_DISABLED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   return(ret_val);
}

   /* The following is used to enable intermediate cuff pressure        */
   /* notifications on the provided Blood Pressure Sensor. Zero will be */
   /* returned on success; otherwise, a negative error code will be     */
   /* returned.                                                         */
static int EnableICPNotify(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   int               ret_val;
   Device_Entry_t   *DeviceEntry;
   Enabled_Entry_t   EnabledEntry;
   Enabled_Entry_t  *EnabledEntryPtr;
   Callback_Entry_t *CallbackEntryPtr;

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Search for the callback associated with this request.          */
      if((CallbackEntryPtr = SearchCallbackEntry(CallbackID)) != NULL)
      {
         /* Verify that the instance entry associated with this request */
         /* exists.                                                     */
         if((DeviceEntry = SearchDeviceEntry(RemoteSensor)) != NULL)
         {
            /* The instance exists, so add the entry into the enabled   */
            /* notifications list for the specified callback.           */
            EnabledEntry.BluetoothAddress = *RemoteSensor;

            if(AddEnabledEntry(&(CallbackEntryPtr->ICPNotifyEntryList), &EnabledEntry))
            {
               /* Configure the CCD only when the count goes from zero  */
               /* to positive.                                          */
               if((!(DeviceEntry->ICPNotifyCount)) && (DeviceEntry->ICPNotificationsEnabled == FALSE))
               {
                  if((ret_val = WriteICPCCCD(RemoteSensor, TRUE)) > 0)
                  {
                     DeviceEntry->ICPNotifyCount++;

                     ret_val = 0;
                  }
                  else
                  {
                     /* Remove the entry in the list since configuring  */
                     /* the CCD failed.                                 */
                     if((EnabledEntryPtr = DeleteEnabledEntry(&(CallbackEntryPtr->ICPNotifyEntryList), RemoteSensor)) != NULL)
                        FreeEnabledEntryMemory(EnabledEntryPtr);
                  }
               }
               else
               {
                  DeviceEntry->ICPNotifyCount++;

                  ret_val = 0;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
            ret_val = BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   return(ret_val);
}

   /* The following is used to disable intermediate cuff pressure       */
   /* notifications on the provided Blood Pressure Instance. Zero will  */
   /* be returned on success; otherwise, a negative error code will be  */
   /* returned.                                                         */
static int DisableICPNotify(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   int               ret_val;
   Device_Entry_t   *DeviceEntry;
   Enabled_Entry_t  *EnabledEntry;
   Callback_Entry_t *CallbackEntryPtr;

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Search for the callback associated with this request.          */
      if((CallbackEntryPtr = SearchCallbackEntry(CallbackID)) != NULL)
      {
         /* Try to delete the requested Notifications Enabled entry from*/
         /* the list.                                                   */
         if((EnabledEntry = DeleteEnabledEntry(&(CallbackEntryPtr->ICPNotifyEntryList), RemoteSensor)) != NULL)
         {
            /* Decrement the notification count from the instance.      */
            if((DeviceEntry = SearchDeviceEntry(RemoteSensor)) != NULL)
            {
               /* Configure the CCD only when the count goes from       */
               /* positive to zero.                                     */
               if((DeviceEntry->ICPNotifyCount) && (!(--DeviceEntry->ICPNotifyCount)))
               {
                  /* Only disable it if actually enabled.               */
                  if(DeviceEntry->ICPNotificationsEnabled)
                  {
                     if((ret_val = WriteICPCCCD(RemoteSensor, FALSE)) > 0)
                        ret_val = 0;
                  }
                  else
                     ret_val = 0;
               }
               else
                  ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE;

            FreeEnabledEntryMemory(EnabledEntry);
         }
         else
            ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_ICP_NOTIFICATIONS_DISABLED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   return(ret_val);
}

   /* The following is a utility function that submits a Get Blood      */
   /* Pressure Feature request to the lower level BLP implementation.   */
   /* The Callback should also be passed in to identify where the       */
   /* response should be sent.                                          */
static int GetBloodPressureFeature(unsigned int CallbackID, BD_ADDR_t *RemoteSensor)
{
   return(SubmitReadValue(CallbackID, RemoteSensor, ttBloodPressureFeature));
}

   /* The following is a utility function that submits a Cancel         */
   /* Transaction request to the lower level BLP implementation.  Zero  */
   /* is returned on success; otherwise, a negative error value is      */
   /* returned.                                                         */
static int CancelTransaction(unsigned int TransactionID)
{
   int                  ret_val;
   Transaction_Entry_t *TransactionEntry;

   /* First try to delete the transaction entry.                        */
   if((TransactionEntry = DeleteTransactionEntry(TransactionID)) != NULL)
   {
      FreeTransactionEntryMemory(TransactionEntry);

      ret_val = _BLPM_Cancel_Transaction(TransactionID);
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following is a utility function that submits a Write Blood    */
   /* Pressure Measurement CCD request to the lower level BLP           */
   /* implementation.  A positive Transaction ID is returned on success;*/
   /* otherwise, a negative error value is returned.                    */
static int WriteBPMCCCD(BD_ADDR_t *RemoteSensor, Boolean_t EnableNotifications)
{
   NonAlignedWord_t CCD;

   ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCD, EnableNotifications ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE : 0);

   return(SubmitWriteValue(0, RemoteSensor, (EnableNotifications ? ttBloodPressureMeasurementCCCDEnable : ttBloodPressureMeasurementCCCDDisable), sizeof(NonAlignedWord_t), (Byte_t *)&CCD));
}

   /* The following is a utility function that submits a Write          */
   /* Intermediate Cuff Pressure CCD request to the lower level BLP     */
   /* implementation.  A positive Transaction ID is returned on success;*/
   /* otherwise, a negative error value is returned.                    */
static int WriteICPCCCD(BD_ADDR_t *RemoteSensor, Boolean_t EnableIndications)
{
   NonAlignedWord_t CCD;

   ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCD, EnableIndications ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE: 0);

   return(SubmitWriteValue(0, RemoteSensor, (EnableIndications ? ttIntermediateCuffPressureCCCDEnable : ttIntermediateCuffPressureCCCDDisable), sizeof(NonAlignedWord_t), (Byte_t *)&CCD));
}

   /* The following function is a utility function that is used to      */
   /* process a Blood Pressure CCCD Indication State Change.            */
static void ProcessBPMIndicationStateChange(Transaction_Entry_t *TransactionEntry, Boolean_t Enable, Boolean_t Success)
{
   Device_Entry_t   *DeviceEntry;
   Enabled_Entry_t  *EnabledEntryPtr;
   Callback_Entry_t *CallbackEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(TransactionEntry)
   {
      /* Dispatch the BPM CCCD Event.                                   */
      DispatchBPMCCCDEvent(TransactionEntry, (Success?0:BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID));

      /* Search for the device entry for the specified pointer.         */
      if((DeviceEntry = SearchDeviceEntry(&(TransactionEntry->BluetoothAddress))) != NULL)
      {
         /* If successfull update the enabled entry for this device.    */
         if(Success)
         {
            /* Flag that indications are enabled.                       */
            DeviceEntry->BPMIndicationsEnabled = Enable;

            /* Update the device file if necessary.                     */
            UpdateDeviceFile(DeviceEntry);
         }
         else
         {
            /* We failed to write the CCCD so increment or decrement the*/
            /* count depending on whether this is an enable or disable. */
            if(Enable)
            {
               if(DeviceEntry->BPMIndicateCount)
                  --(DeviceEntry->BPMIndicateCount);
            }
            else
               ++(DeviceEntry->BPMIndicateCount);

            /* Delete the enabled callback entry.                       */
            if((CallbackEntryPtr = SearchCallbackEntry(TransactionEntry->CallbackID)) != NULL)
            {
               /* Remove the entry in the list since configuring the CCD*/
               /* failed.                                               */
               if((EnabledEntryPtr = DeleteEnabledEntry(&(CallbackEntryPtr->BPMIndicateEntryList), &(DeviceEntry->BluetoothAddress))) != NULL)
                  FreeEnabledEntryMemory(EnabledEntryPtr);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Intermediate Cuff Pressure CCCD Indication State Change.*/
static void ProcessICPNotificationStateChange(Transaction_Entry_t *TransactionEntry, Boolean_t Enable, Boolean_t Success)
{
   Device_Entry_t   *DeviceEntry;
   Enabled_Entry_t  *EnabledEntryPtr;
   Callback_Entry_t *CallbackEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(TransactionEntry)
   {
      /* Dispatch the BPM CCCD Event.                                   */
      DispatchICPCCCDEvent(TransactionEntry, (Success?0:BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID));

      /* Search for the device entry for the specified pointer.         */
      if((DeviceEntry = SearchDeviceEntry(&(TransactionEntry->BluetoothAddress))) != NULL)
      {
         /* If successfull update the enabled entry for this device.    */
         if(Success)
         {
            /* Flag that notifications are enabled.                     */
            DeviceEntry->ICPNotificationsEnabled = Enable;

            /* Update the device file if necessary.                     */
            UpdateDeviceFile(DeviceEntry);
         }
         else
         {
            /* We failed to write the CCCD so increment or decrement the*/
            /* count depending on whether this is an enable or disable. */
            if(Enable)
            {
               if(DeviceEntry->ICPNotifyCount)
                  --(DeviceEntry->ICPNotifyCount);
            }
            else
               ++(DeviceEntry->ICPNotifyCount);

            /* Delete the enabled callback entry.                       */
            if((CallbackEntryPtr = SearchCallbackEntry(TransactionEntry->CallbackID)) != NULL)
            {
               /* Remove the entry in the list since configuring the CCD*/
               /* failed.                                               */
               if((EnabledEntryPtr = DeleteEnabledEntry(&(CallbackEntryPtr->ICPNotifyEntryList), &(DeviceEntry->BluetoothAddress))) != NULL)
                  FreeEnabledEntryMemory(EnabledEntryPtr);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BLPM Connection.                               */
static void ProcessBLPMConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Update Device Entry with service discovery information.        */
      DiscoverBloodPressureSensor(&(RemoteDeviceProperties->BD_ADDR));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BLPM disconnection.                            */
static void ProcessBLPMDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Delete the device if it is known.                              */
      if((DeviceEntry = DeleteDeviceEntry(&(RemoteDeviceProperties->BD_ADDR))) != NULL)
      {
         /* Dispatch the Device Disconnection Event.                    */
         DispatchDisconnectedEvent(DeviceEntry);

         /* Free the Device Entry.                                      */
         FreeDeviceEntryMemory(DeviceEntry);
      }

      /* Delete the enabled entries for this device.                    */
      DeleteEnabledEntriesForDevice(RemoteDeviceProperties->BD_ADDR);
      DeleteTransactionEntriesForDevice(RemoteDeviceProperties->BD_ADDR);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BLPM Device Paired Event.                      */
static void ProcessBLPMDevicePaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Boolean_t       BPMIndicationsEnabled;
   Boolean_t       ICPNotificationsEnabled;
   Boolean_t       StoreToFile;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Make sure that we have a device entry at this stage.           */
      if((DeviceEntry = SearchDeviceEntry(&(RemoteDeviceProperties->BD_ADDR))) != NULL)
      {
         /* Initialize local variables.                                 */
         BPMIndicationsEnabled   = FALSE;
         ICPNotificationsEnabled = FALSE;

         /* Attempt to reload information from the device file.         */
         if(ReloadDeviceEntry(DeviceEntry->BluetoothAddress, &BPMIndicationsEnabled, &ICPNotificationsEnabled))
         {
            /* Initialize that the device file does not need to be      */
            /* updated.                                                 */
            StoreToFile = FALSE;

            /* Check to see if we need to update the device entry or the*/
            /* device file.                                             */
            if(DeviceEntry->BPMIndicationsEnabled)
            {
               if(!BPMIndicationsEnabled)
                  StoreToFile = TRUE;
            }
            else
               DeviceEntry->BPMIndicationsEnabled = BPMIndicationsEnabled;

            if(DeviceEntry->ICPNotificationsEnabled)
            {
               if(!ICPNotificationsEnabled)
                  StoreToFile = TRUE;
            }
            else
               DeviceEntry->ICPNotificationsEnabled = ICPNotificationsEnabled;
         }
         else
         {
            /* If either the BPM or ICP CCCDs are enabled then we need  */
            /* to update the device file.                               */
            if((DeviceEntry->BPMIndicationsEnabled) || (DeviceEntry->ICPNotificationsEnabled))
               StoreToFile = TRUE;
            else
               StoreToFile = FALSE;
         }

         /* If we need to store to the file do so here.                 */
         if(StoreToFile)
            StoreBLPMInformation(DeviceEntry->BluetoothAddress, DeviceEntry->BPMIndicationsEnabled, DeviceEntry->ICPNotificationsEnabled);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BLPM Device Un-Paired Event.                   */
static void ProcessBLPMDeviceUnPaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Make sure that any device entry stored for this device is      */
      /* deleted if we have written any information for the device to   */
      /* the file.                                                      */
      if(ReloadDeviceEntry(RemoteDeviceProperties->BD_ADDR, NULL, NULL))
         StoreBLPMInformation(RemoteDeviceProperties->BD_ADDR, FALSE, FALSE);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the HOGM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Make sure that any device entry stored for this device is      */
      /* deleted if we have written any information for the device to   */
      /* the file.                                                      */
      if(ReloadDeviceEntry(RemoteDeviceProperties->PriorResolvableBD_ADDR, NULL, NULL))
         StoreBLPMInformation(RemoteDeviceProperties->PriorResolvableBD_ADDR, FALSE, FALSE);

      /* Walk the Connection List and update any BD_ADDRs as needed.    */
      DeviceEntry = DeviceEntryList;
      while(DeviceEntry)
      {
         /* Check to see if this entry needs to be updated.             */
         if(COMPARE_BD_ADDR(DeviceEntry->BluetoothAddress, RemoteDeviceProperties->PriorResolvableBD_ADDR))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("BLPM Address Updated\n"));

            /* Save the new Base Address.                               */
            DeviceEntry->BluetoothAddress = RemoteDeviceProperties->BD_ADDR;

            /* Update the device file if necessary.                     */
            UpdateDeviceFile(DeviceEntry);
         }

         /* Advance to the next entry in the list.                      */
         DeviceEntry = DeviceEntry->NextDeviceEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the BLPM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long RequiredConnectionFlags;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Address is updated or the LE Pairing State changes.            */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

         /* Process the LE Connection State Change Event if necessary.  */
         if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
         {
            /* Set the required flags.  We must be currently connected  */
            /* over LE and must know all of the remote device's services*/
            /* before processing the connection.                        */
            RequiredConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN);

            /* Check to see that the required flags are set for a       */
            /* connection.                                              */
            if((RemoteDeviceProperties->RemoteDeviceFlags & RequiredConnectionFlags) == RequiredConnectionFlags)
            {
               /* Process a possible BLPM Connection.                   */
               ProcessBLPMConnection(RemoteDeviceProperties);
            }
            else
            {
               /* Process a possible BLPM Disconnection Event.          */
               ProcessBLPMDisconnection(RemoteDeviceProperties);

               /* If services are no longer known we also need to check */
               /* to see if we have written any information to the LE   */
               /* Configuration file.  If the services are no longer    */
               /* known we must delete any information stored to said   */
               /* file.                                                 */
               if(!(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN))
               {
                  /* Make sure that any device entry stored for this    */
                  /* device is deleted if we have written any           */
                  /* information for the device to the file.            */
                  if(ReloadDeviceEntry(RemoteDeviceProperties->BD_ADDR, NULL, NULL))
                     StoreBLPMInformation(RemoteDeviceProperties->BD_ADDR, FALSE, FALSE);
               }
            }
         }

         /* Process the LE Pairing State Change Event if necessary.     */
         if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE))
         {
            /* Set the required flags for writing to the device file.   */
            RequiredConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED);

            /* Check to see that the required flags are set for a       */
            /* connection.                                              */
            if((RemoteDeviceProperties->RemoteDeviceFlags & RequiredConnectionFlags) == RequiredConnectionFlags)
               ProcessBLPMDevicePaired(RemoteDeviceProperties);
            else
            {
               /* If we are no longer paired then we need to process    */
               /* this event.                                           */
               if(!(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE))
                  ProcessBLPMDeviceUnPaired(RemoteDeviceProperties);
            }
         }

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Deleted Event.                            */
   /* * NOTE * This function *MUST* be called with the BLPM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDeviceDeletedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Delete the device if it is known.                              */
      if((DeviceEntry = DeleteDeviceEntry(&RemoteDeviceAddress)) != NULL)
      {
         /* Dispatch the Device Disconnection Event.                    */
         DispatchDisconnectedEvent(DeviceEntry);

         /* Free the Device Entry.                                      */
         FreeDeviceEntryMemory(DeviceEntry);
      }

      /* Delete the enabled entries for this device.                    */
      DeleteEnabledEntriesForDevice(RemoteDeviceAddress);
      DeleteTransactionEntriesForDevice(RemoteDeviceAddress);

      /* Make sure that any device entry stored for this device is      */
      /* deleted if we have written any information for the device to   */
      /* the file.                                                      */
      if(ReloadDeviceEntry(RemoteDeviceAddress, NULL, NULL))
         StoreBLPMInformation(RemoteDeviceAddress, FALSE, FALSE);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Module Manager Messages.*/
static void BTPSAPI BLPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("BLP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a BLP Manager defined    */
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
               /* BLP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_BLPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an BLP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* this Bluetopia Platform Manager Module.  This function should be  */
   /* registered with the Bluetopia Platform Manager Module Handler and */
   /* will be called when the Platform Manager is initialized (or shut  */
   /* down).                                                            */
void BTPSAPI BLPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing BLP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process BLP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER, BLPManagerGroupHandler, NULL))
         {
            /* Initialize the BLP Manager Implementation Module (this is*/
            /* the module that is responsible for implementing the BLP  */
            /* Manager functionality - this module is just the framework*/
            /* shell).                                                  */
            if(!(Result = _BLPM_Initialize()))
            {
               /* Flag that there is no Callback handler registered.    */
               CallbackEntryList = NULL;

               /* Flag that this module is initialized.                 */
               Initialized = TRUE;

               /* Flag success.                                         */
               Result = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _BLPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("BLP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the BLP Manager Implementation that  */
            /* we are shutting down.                                    */
            _BLPM_Cleanup();

            /* Free all Entry Lists.                                    */
            FreeDeviceEntryList();
            FreeCallbackEntryList();
            FreeTransactionEntryList();

            /* Flag that this module is no longer initialized.          */
            Initialized = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI BLPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the BLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Set the Bluetooth ID in the lower level module if     */
               /* available.                                            */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _BLPM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Clear the Bluetooth ID in the lower level module .    */
               _BLPM_SetBluetoothStackID(0);

               /* Free the Device and Transaction Entry list.           */
               FreeDeviceEntryList();
               FreeTransactionEntryList();
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDevicePropertiesChanged\n"));

               if(EventData->EventLength >= DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_EVENT_DATA_SIZE)
               {
                  /* Process the Remote Device Properties Changed Event.*/
                  ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               }
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDeviceDeleted\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Module Manager of a specific Update Event.  The     */
   /* Module Manager can then take the correct action to process the    */
   /* update.                                                           */
Boolean_t BLPM_NotifyUpdate(BLPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utGATTNotificationEvent:
         case utGATTIndicationEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing GATT Event\n"));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPSDispatchCallback_GATT, (void *)UpdateData);
            break;
         case utGATTClientEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Client Event: %d\n", UpdateData->UpdateData.GATTClientEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPSDispatchCallback_BLP_Collector, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Blood Pressure   */
   /* (BLP) Manager Service.  This Callback will be dispatched by the   */
   /* BLP Manager when various BLP Manager Events occur.  This function */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a BLP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BLPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI BLPM_Register_Collector_Event_Callback(BLPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   Callback_Entry_t *CallbackEntryPtr;
   Callback_Entry_t  CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Initialize the Callback Entry.                           */
            BTPS_MemInitialize(&CallbackEntry, 0, sizeof(Callback_Entry_t));

            CallbackEntry.AddressID         = MSG_GetServerAddressID();
            CallbackEntry.EventCallback     = CallbackFunction;
            CallbackEntry.CallbackParameter = CallbackParameter;

            /* This module only supports one callback at this time.     */
            if(!CallbackEntryList)
            {
               if((CallbackEntryPtr = AddCallbackEntry(&CallbackEntry)) != NULL)
               {
                  /* Give the callback an ID and indicate success.      */
                  CallbackEntryPtr->CallbackID = GetNextCallbackID();
                  ret_val                      = CallbackEntryPtr->CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_CALLBACK_ALREADY_REGISTERED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BLP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BLPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the BLP Manager Event Callback ID (return value  */
   /* from BLPM_Register_Collector_Event_Callback() function).          */
void BTPSAPI BLPM_Un_Register_Collector_Event_Callback(unsigned int CallbackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         DeleteCallbackEntry(CallbackID);

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

int BTPSAPI BLPM_Enable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t RemoteSensor)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = EnableBPMIndicate(CallbackID, &RemoteSensor);
   else
      ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int BTPSAPI BLPM_Disable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t RemoteSensor)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = DisableBPMIndicate(CallbackID, &RemoteSensor);
   else
      ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int BTPSAPI BLPM_Enable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t RemoteSensor)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = EnableICPNotify(CallbackID, &RemoteSensor);
   else
      ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

int BTPSAPI BLPM_Disable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t RemoteSensor)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = DisableICPNotify(CallbackID, &RemoteSensor);
   else
      ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Blood Pressure Feature Request to a remote sensor.  This    */
   /* function accepts as input the Callback ID (return value from      */
   /* BLPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Blood Pressure Feature from.  This   */
   /* function returns a positive Transaction ID on success; otherwise, */
   /* a negative error value is returned.                               */
int BTPSAPI BLPM_Get_Blood_Pressure_Feature(unsigned int CallbackID, BD_ADDR_t RemoteSensor)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = GetBloodPressureFeature(CallbackID, &RemoteSensor);
   else
      ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (return value from               */
   /* BLPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Transaction ID returned by*/
   /* a previously called function in this module.  This function       */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int BTPSAPI BLPM_Cancel_Transaction(unsigned int CallbackID, unsigned int TransactionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Wait for access to the lock for this module.                   */
      if(DEVM_AcquireLock())
      {
         /* Check for a registered callback.                            */
         if(SearchCallbackEntry(CallbackID) != NULL)
         {
            /* Cancel the transaction.                                  */
            ret_val = CancelTransaction(TransactionID);
         }
         else
            ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_CALLBACK_NOT_REGISTERED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_BLOOD_PRESSURE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
